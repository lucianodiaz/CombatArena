// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"

#include "DrawDebugHelpers.h"
#include "CombatArena/CombatArena.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));

	RootComponent = SkeletalMeshComponent;

	RateOfFire = 600; //600 bullet per minute

	MaxDistanceEnd = 10000;
	
	MuzzleSocketName = "Muzzle";

	TracerTargetName = "Target";

	StartingBulletAngle = 0;

	MaxBulletAngle = 5;

	BaseDamage = 7;

	MaxAmmo = 250;
	AvailableAmmo = 30;
	
	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = AvailableAmmo;
	
	BulletAngle = StartingBulletAngle;
	
	TimeBetweenShot = 60/RateOfFire;
}

void ASWeapon::Fire()
{
	if(!HasAuthority())
	{
		ServerFire();
	}
	
	if (CurrentAmmo > 0)
	{
		AActor* MyOwner = GetOwner();

		if (MyOwner)
		{
			CurrentAmmo --;
			UE_LOG(LogTemp,Log,TEXT("Ammo: %i / MaxAmmo: %i"),CurrentAmmo,MaxAmmo);
			if (BulletAngle <= MaxBulletAngle)
			{
				BulletAngle += 0.3;
			}
			else
			{
				BulletAngle = MaxBulletAngle;
			}
			PlayAnimFire();
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);

			FVector ShotDirection = EyeRotation.Vector();
			FVector TraceEnd = MuzzleLocation + (ShotDirection * MaxDistanceEnd);

			FCollisionQueryParams QueryParams;

			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			//float Spread = FMath::RandRange(BulletSpread.X,BulletSpread.Y);
			float Radius = FMath::Tan(FMath::DegreesToRadians(BulletAngle)) * MaxDistanceEnd;;
			FVector2D Point = RandomPointInCircle(Radius);

			FVector TracerEndPoint = TraceEnd + (EyeLocation.RightVector * Point.X) + (EyeLocation.UpVector * Point.Y);

			FHitResult HitResult;

			EPhysicalSurface SurfaceType = SurfaceType_Default;


			if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TracerEndPoint,COLLISION_WEAPON,
			                                         QueryParams))
			{
				AActor* HitActor = HitResult.GetActor();

				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

				float ActualDamage = BaseDamage;

				//Multiply damage depending SurfaceType
				switch (SurfaceType)
				{
					case SURFACE_FLESHVULNERABLE:
						ActualDamage *=2;
						break;
					default:
						break;
				}

				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult,
				                                   MyOwner->GetInstigatorController(), this, DamageType);

				PlayImpactEffects(SurfaceType, HitResult.ImpactPoint);
				TracerEndPoint = HitResult.ImpactPoint;
			}

			//DrawDebugLine(GetWorld(),MuzzleLocation,TracerEndPoint,FColor::Red,false,4,0,2);


			if (HasAuthority())
			{
				HitScanTrace.TraceTo = TracerEndPoint;
				HitScanTrace.MuzzleLocation = MuzzleLocation;
			}
			PlayWeaponEffects(TracerEndPoint);
			LastFireTime = GetWorld()->TimeSeconds;
		}
	}
	else
	{
		Reload();
	}
	
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::OnRep_HitScanTrace()
{
	PlayAnimFire();
	PlayWeaponEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType,HitScanTrace.TraceTo);
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShot - GetWorld()->TimeSeconds,0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimerBetweenShot,this,&ASWeapon::Fire,TimeBetweenShot,true, FirstDelay);
	//Fire();
}

void ASWeapon::StopFire()
{
	BulletAngle = StartingBulletAngle;
	GetWorldTimerManager().ClearTimer(TimerHandle_TimerBetweenShot);
}

void ASWeapon::Reload()
{
	UE_LOG(LogTemp,Log,TEXT("Ammo: %i / MaxAmmo: %i"),CurrentAmmo,MaxAmmo);
	if(MaxAmmo <= 0)
	{
		return;
	}

	if((MaxAmmo - (AvailableAmmo - CurrentAmmo)) <= 0 )
	{
		CurrentAmmo = MaxAmmo;
		MaxAmmo -= CurrentAmmo;
	}
	else
	{
		CurrentAmmo = FMath::Clamp(AvailableAmmo - CurrentAmmo,0,AvailableAmmo);
		MaxAmmo -=CurrentAmmo;
	}
	
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
	FVector ShotDirection = ImpactPoint - MuzzleLocation;
	ShotDirection.Normalize();
	UParticleSystem* SelectedImpactEffect = nullptr;
	SelectedImpactEffect = DefaultImpactEffect;
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),SelectedImpactEffect,ImpactPoint,ShotDirection.Rotation());
//	a->SetIsReplicated(true);
}

void ASWeapon::PlayWeaponEffects(FVector TraceEnd)
{
	
	FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);

	if(TracerEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),TracerEffect,MuzzleLocation);

		if(TracerComp)
		{
			//TracerComp->SetVectorParameter("InitialLocation",MuzzleLocation);
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}
}

void ASWeapon::PlayAnimFire()
{
	if(FireAnimAsset)
	{
		SkeletalMeshComponent->PlayAnimation(FireAnimAsset,false);
	}
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int ASWeapon::GetMaxAmmo() const
{
	return MaxAmmo;
}

int ASWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

USkeletalMeshComponent* ASWeapon::GetMesh() const
{
	return SkeletalMeshComponent;
}

FVector2D ASWeapon::RandomPointInCircle(float Radius)
{
	float Angle = FMath::FRandRange(0,360);

	float DistanceFromCenter = FMath::FRandRange(0,Radius);

	FVector2D Point;
	
	Point.X = DistanceFromCenter * FMath::Cos(Angle);
	Point.Y = DistanceFromCenter * FMath::Sin(Angle);

	

	return Point;
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASWeapon,HitScanTrace);
}

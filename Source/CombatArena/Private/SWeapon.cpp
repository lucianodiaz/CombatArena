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

	StartingBulletAngle = 5;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	BulletAngle = StartingBulletAngle;
	
	TimeBetweenShot = 60/RateOfFire;
}

void ASWeapon::Fire()
{

	if(!HasAuthority())
	{
		ServerFire();
	}
	AActor* MyOwner = GetOwner();

	if(MyOwner)
	{
		UE_LOG(LogTemp,Log,TEXT("Fire"));
		PlayAnimFire();
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation,EyeRotation);
		
		FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
		
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = MuzzleLocation + (ShotDirection * MaxDistanceEnd);

		FCollisionQueryParams QueryParams;

		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		//float Spread = FMath::RandRange(BulletSpread.X,BulletSpread.Y);
		float Radius = FMath::Tan(FMath::DegreesToRadians(BulletAngle))* MaxDistanceEnd;;
		FVector2D Point = RandomPointInCircle(Radius);
		
		FVector TracerEndPoint = TraceEnd + (EyeLocation.RightVector * Point.X) + (EyeLocation.UpVector * Point.Y);

		FHitResult HitResult;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

	
		
		if(GetWorld()->LineTraceSingleByChannel(HitResult,EyeLocation,TracerEndPoint,COLLISION_WEAPON,QueryParams))
		{
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
			
			PlayImpactEffects(SurfaceType,HitResult.ImpactPoint);
			TracerEndPoint = HitResult.ImpactPoint;
		}

		DrawDebugLine(GetWorld(),MuzzleLocation,TracerEndPoint,FColor::Red,false,4,0,2);
		

		if(HasAuthority())
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.MuzzleLocation = MuzzleLocation;
		}
		PlayWeaponEffects(TracerEndPoint);
		LastFireTime = GetWorld()->TimeSeconds;
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
	GetWorldTimerManager().ClearTimer(TimerHandle_TimerBetweenShot);
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
			TracerComp->SetVectorParameter("InitialLocation",MuzzleLocation);
			//TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
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

	DOREPLIFETIME_CONDITION(ASWeapon,HitScanTrace,COND_SkipOwner);
}

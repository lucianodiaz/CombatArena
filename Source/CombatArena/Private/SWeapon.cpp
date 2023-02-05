// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

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

	bulletsMax = 50;

	currentBullets = bulletsMax;
	
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
	MyOwner = GetOwner();

	if(MyOwner)
	{
	
		
		if(currentBullets > 0)
		{
			UE_LOG(LogTemp,Log,TEXT("Fire"));
			
			shoot();
		}
		else
		{
			currentBullets = bulletsMax;
		}
		UE_LOG(LogTemp,Log,TEXT("Bullets :%d of %d"),currentBullets,bulletsMax);
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

void ASWeapon::shoot()
{
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

	DOREPLIFETIME(ASWeapon,currentBullets);
	DOREPLIFETIME_CONDITION(ASWeapon,HitScanTrace,COND_SkipOwner);
}

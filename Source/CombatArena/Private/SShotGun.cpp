// Fill out your copyright notice in the Description page of Project Settings.


#include "SShotGun.h"

#include "CombatArena/CombatArena.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

ASShotGun::ASShotGun()
{
	NumPellets = 10;
}

void ASShotGun::BeginPlay()
{
	Super::BeginPlay();

	BulletAngle = StartingBulletAngle;
}

void ASShotGun::Fire()
{
	Super::Fire();
}

void ASShotGun::shoot()
{
	currentBullets--;
	
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

	FHitResult HitResult;


	EPhysicalSurface SurfaceType = SurfaceType_Default;

	FVector TracerEndPoint;
	PlayAnimFire();
	float Radius;
	for (int i = 0; i<NumPellets;i++)
	{
		Radius = FMath::Tan(FMath::DegreesToRadians(BulletAngle))* MaxDistanceEnd;
		FVector2D Point = RandomPointInCircle(Radius);

		TracerEndPoint = TraceEnd + (EyeLocation.RightVector * Point.X) + (EyeLocation.UpVector * Point.Y);
			
		if(GetWorld()->LineTraceSingleByChannel(HitResult,EyeLocation,TracerEndPoint,COLLISION_WEAPON,QueryParams))
		{
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
			
			PlayImpactEffects(SurfaceType,HitResult.ImpactPoint);
			TracerEndPoint = HitResult.ImpactPoint;
		}
			
		if(HasAuthority())
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.MuzzleLocation = MuzzleLocation;
			OnRep_HitScanTrace();
		}
			
	}
		
	PlayWeaponEffects(TracerEndPoint);
		
	LastFireTime = GetWorld()->TimeSeconds;
}


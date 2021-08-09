// Fill out your copyright notice in the Description page of Project Settings.


#include "SShotGun.h"

#include <valarray>

#include "DrawDebugHelpers.h"
#include "CombatArena/CombatArena.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

ASShotGun::ASShotGun()
{
	NumPellets = 10;
	BaseDamage = 10;
	StartingBulletAngle = 7;

	MaxAmmo = 20;
	AvailableAmmo = 6;
}

void ASShotGun::BeginPlay()
{
	Super::BeginPlay();

	BulletAngle = StartingBulletAngle;
}

void ASShotGun::Fire()
{
	if(!HasAuthority())
	{
		ServerFire();
	}
	if(CurrentAmmo > 0)
	{
		AActor* MyOwner = GetOwner();
		if(MyOwner)
		{
			CurrentAmmo --;
			UE_LOG(LogTemp,Log,TEXT("Ammo: %i / MaxAmmo: %i"),CurrentAmmo,MaxAmmo);
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
					AActor* HitActor = HitResult.GetActor();
			
					SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
			
					float ActualDamage = BaseDamage;

					//Multiply damage depending SurfaceType

					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult,
													MyOwner->GetInstigatorController(), this, DamageType);
			
					PlayImpactEffects(SurfaceType,HitResult.ImpactPoint);
					TracerEndPoint = HitResult.ImpactPoint;
					if(HasAuthority())
					{
						HitScanTrace.TraceTo = TracerEndPoint;
						HitScanTrace.MuzzleLocation = MuzzleLocation;
						OnRep_HitScanTrace();
					}
				}
			
				
			
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


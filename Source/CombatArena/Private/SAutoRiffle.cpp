#include "SAutoRiffle.h"

#include "DrawDebugHelpers.h"
#include "CombatArena/CombatArena.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


ASAutoRiffle::ASAutoRiffle()
{
	
}

void ASAutoRiffle::BeginPlay()
{
	Super::BeginPlay();
}

void ASAutoRiffle::Fire()
{
	Super::Fire();
}

void ASAutoRiffle::shoot()
{
	currentBullets --;
	
			
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



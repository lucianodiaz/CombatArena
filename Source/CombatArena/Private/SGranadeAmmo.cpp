// Fill out your copyright notice in the Description page of Project Settings.


#include "SGranadeAmmo.h"

#include "DrawDebugHelpers.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ASGranadeAmmo::ASGranadeAmmo()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->InitSphereRadius(32.0f);
	SphereComponent->SetCollisionProfileName("Projectile");
	SphereComponent->OnComponentHit.AddDynamic(this,&ASGranadeAmmo::OnHit);

	SphereComponent->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable,0.0f));
	SphereComponent->CanCharacterStepUpOn = ECB_No;

	RootComponent = SphereComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = SphereComponent;
	ProjectileMovementComponent->InitialSpeed = 700.0f;
	ProjectileMovementComponent->MaxSpeed = 700.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->Bounciness = 1.0f;
	
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
    RadialForceComponent->SetupAttachment(RootComponent);
    RadialForceComponent->Radius = 400;
	RadialForceComponent->ImpulseStrength = 1700.0f;
    RadialForceComponent->bImpulseVelChange = true;
    RadialForceComponent->bAutoActivate = false;
    RadialForceComponent->bIgnoreOwningActor = true;

	ExplotionTime = 5.0f;

	BaseDamage = 100;
	DamageRadius = 100;
	
	bReplicates = true;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	
}

// Called when the game starts or when spawned
void ASGranadeAmmo::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Explotion,this,&ASGranadeAmmo::Explosion,ExplotionTime);
	}
}

void ASGranadeAmmo::PlayExplosionEffects()
{
	if(ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ExplosionEffect,GetActorLocation());
	}
}

void ASGranadeAmmo::OnRep_Explosion()
{
	Explosion();
}

void ASGranadeAmmo::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                      FVector NormalImpulse, const FHitResult& Hit)
{
	if(!bExplode)
	{
		if(!bAlwaysExplode)
		{
			if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
			{
				Explosion();		
			}
			ASCharacter* MyPawn = Cast<ASCharacter>(OtherActor);
			if(MyPawn)
			{
				Explosion();
			}
		}
		else
		{
			Explosion();
		}

	}
}

void ASGranadeAmmo::Explosion()
{
	if(bExplode)
	{
		return;
	}
	bExplode = true;
	OnRep_Explosion();
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	if(ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),ExplosionSound,GetActorLocation());
	}
	PlayExplosionEffects();
			
	float ActualDamage = BaseDamage;
	
	UGameplayStatics::ApplyRadialDamage(GetWorld(), ActualDamage, GetActorLocation(), DamageRadius, DamageType,
										IgnoredActors,this, GetInstigatorController(), true);
	
	RadialForceComponent->FireImpulse();
	DrawDebugSphere(GetWorld(),GetActorLocation(),DamageRadius,10,FColor::Magenta,false,5);
	MeshComponent->SetVisibility(false);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(3.0f);
}

void ASGranadeAmmo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGranadeAmmo,bExplode);
}

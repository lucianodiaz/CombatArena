// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SGranadeAmmo.generated.h"

class URadialForceComponent;
class USoundCue;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class COMBATARENA_API ASGranadeAmmo : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGranadeAmmo();

	protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void PlayExplosionEffects();

	UFUNCTION()
	void OnRep_Explosion();
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly,Category="Components")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly,Category="Components")
	TSubclassOf<UDamageType> DamageType;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    URadialForceComponent* RadialForceComponent;

	UPROPERTY(Replicated)
	bool bExplode;

	UPROPERTY(EditDefaultsOnly,Category="Weapons")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly,Category="Weapons")
	float DamageRadius;

	UPROPERTY(EditDefaultsOnly,Category="Weapons")
	bool bAlwaysExplode;

	UPROPERTY(EditDefaultsOnly,Category="WeaponFX")
	USoundCue* ExplosionSound;
	
	FTimerHandle TimerHandle_Explotion;

	float ExplotionTime;

	public:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return SphereComponent; }

	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovementComponent; }

	void Explosion();

};

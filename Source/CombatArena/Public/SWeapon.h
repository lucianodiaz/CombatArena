// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector TraceTo;

	UPROPERTY()
	FVector_NetQuantize MuzzleLocation;
	
};


UCLASS()
class COMBATARENA_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Fire();

	FVector2D RandomPointInCircle(float Radius);
    
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	void PlayWeaponEffects(FVector TraceEnd);

	void PlayAnimFire();
	
	UFUNCTION(Server,Reliable,WithValidation)
    void ServerFire();
    	
    UFUNCTION()
    void OnRep_HitScanTrace();

	//Variables
	
	FTimerHandle TimerHandle_TimerBetweenShot;

	float TimeBetweenShot;

	float LastFireTime;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	float BaseDamage;

	UPROPERTY(Replicated,EditDefaultsOnly,Category="Weapon")
	UAnimationAsset* FireAnimAsset;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	int MaxAmmo;

	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	int AvailableAmmo;

	int CurrentAmmo;

	UPROPERTY(EditDefaultsOnly,Category="Weapon");
	float RateOfFire;

	UPROPERTY(BlueprintReadOnly,Category="Weapon")
	float BulletAngle;

	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	float MaxBulletAngle;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	float StartingBulletAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	UParticleSystem* TracerEffect;
	
	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	float MaxDistanceEnd;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Weapon")
	FName MuzzleSocketName;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Weapon")
	USkeletalMeshComponent* SkeletalMeshComponent;
public:

	void StartFire();

	void StopFire();

	void Reload();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	int GetMaxAmmo() const;
	
	UFUNCTION(BlueprintCallable)
	int GetCurrentAmmo() const;
	
	USkeletalMeshComponent* GetMesh() const;
};

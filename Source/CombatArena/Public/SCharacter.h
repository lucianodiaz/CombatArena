// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class ASDeathCamera;
class USpringArmComponent;
class UCameraComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COMBATARENA_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Methods
	void MoveRight(float Value);

	void MoveForward(float Value);

	void StartJump();

	void StartCrouch();

	void StartFire();

	void StopFire();

	void IndexUp();

	void IndexDown();

	void Reload();

	void HiddePreviusWeapon(int PrevIndex);

	void ChangeWeapon();

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerIndexUp();

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerIndexDown();

	UFUNCTION()
	void OnRep_ChangeWeapon();
	//Variables
	bool bIsCrunch;

	UPROPERTY(ReplicatedUsing=OnRep_ChangeWeapon)
	int Index;

	UPROPERTY(Replicated,BlueprintReadOnly,Category="Player")
	bool bDied;

	UPROPERTY(BlueprintReadWrite,Category="Player")
	bool bJump;

	UPROPERTY(EditDefaultsOnly,Category="Player")
	TArray<TSubclassOf<ASWeapon>> AllWeaponsClass;

	UPROPERTY(Replicated)
	TArray<ASWeapon*> InventoryWeapons;
	
	UPROPERTY(Replicated,BlueprintReadOnly)
	ASWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly,Category="Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	FName WeaponSocketName;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	FName BackSocketName;

	UPROPERTY(EditDefaultsOnly,Category="Player")
	TSubclassOf<ASDeathCamera> DeathCameraClass;

	ASDeathCamera* DeathCamera;
	
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USHealthComponent* HealthComponent;


	UFUNCTION()
	void OnHealthChanged(float Health, float HealthDelta, const class UDamageType* DamageType,
						class AController* InstigatedBy, AActor* DamageCauser);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UCameraComponent* GetCameraComponent() const;
	
};

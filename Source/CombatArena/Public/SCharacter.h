// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ASWeapon;

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
	
	//Variables
	
	bool bIsCrunch;

	UPROPERTY(BlueprintReadWrite,Category="Player")
	bool bJump;

	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly,Category="Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category="Weapon")
	FName WeaponSocketName;
	
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USpringArmComponent* SpringArmComp;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UCameraComponent* GetCameraComponent() const;
	
};

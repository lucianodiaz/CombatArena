// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SShotGun.generated.h"

/**
 * 
 */
UCLASS()
class COMBATARENA_API ASShotGun : public ASWeapon
{
	GENERATED_BODY()
	
	public:
	ASShotGun();

	protected:

	virtual void BeginPlay() override;
	
	virtual void Fire() override;

	virtual void shoot() override;
	
	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	int32 NumPellets;
};

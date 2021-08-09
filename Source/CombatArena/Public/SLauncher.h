// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SGranadeAmmo.h"
#include "SWeapon.h"
#include "SLauncher.generated.h"

/**
 * 
 */
UCLASS()
class COMBATARENA_API ASLauncher : public ASWeapon
{
	GENERATED_BODY()
	public:
	ASLauncher();

	protected:

	virtual void Fire() override;

	void LogicFire();
	
	UPROPERTY(EditDefaultsOnly,Category="Weapon")
	TSubclassOf<ASGranadeAmmo> ProjectileClass;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SDeathCamera.generated.h"

class USpringArmComponent;
UCLASS()
class COMBATARENA_API ASDeathCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASDeathCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USpringArmComponent* SpringArmComponent;

	
};

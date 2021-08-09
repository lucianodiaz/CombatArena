// Fill out your copyright notice in the Description page of Project Settings.


#include "SDeathCamera.h"

#include "GameFramework/SpringArmComponent.h"

// Sets default values
ASDeathCamera::ASDeathCamera()
{

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmDeathComponent"));

	RootComponent = SpringArmComponent;
	
	SpringArmComponent->TargetArmLength = 700;

	SpringArmComponent->SetWorldRotation(FRotator(0,160,0));

}

// Called when the game starts or when spawned
void ASDeathCamera::BeginPlay()
{
	Super::BeginPlay();
	
}
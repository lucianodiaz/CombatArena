// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"

#include "SWeapon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));

	SpringArmComp->bUsePawnControlRotation = true;

	SpringArmComp->SetupAttachment(GetMesh(),"head");

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));

	CameraComponent->SetupAttachment(SpringArmComp);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	bIsCrunch = false;

	bJump = false;
	
	WeaponSocketName = "GunSocket";
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass,FVector::ZeroVector,FRotator::ZeroRotator,SpawnParameters);
	CurrentWeapon->SetOwner(this);
	CurrentWeapon->AttachToComponent(GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,WeaponSocketName);
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector()*Value);
}

void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector()*Value);
}

void ASCharacter::StartJump()
{
	bJump = true;
	Jump();
}

void ASCharacter::StartCrouch()
{
	if(bIsCrunch)
	{
		bIsCrunch = false;
		UnCrouch();
	}
	else
	{
		bIsCrunch = true;
		Crouch();
	}
}

void ASCharacter::StartFire()
{
	if(CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if(CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward",this, &ASCharacter::MoveForward);

	PlayerInputComponent->BindAxis("MoveRight",this,&ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp",this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn",this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ASCharacter::StartJump);

	PlayerInputComponent->BindAction("Crouch",IE_Pressed,this,&ASCharacter::StartCrouch);

	PlayerInputComponent->BindAction("Fire",IE_Pressed,this,&ASCharacter::StartFire);

	PlayerInputComponent->BindAction("Fire",IE_Released,this,&ASCharacter::StopFire);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	if(CameraComponent)
	{
		return CameraComponent->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

UCameraComponent* ASCharacter::GetCameraComponent() const
{
	return CameraComponent;
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ASCharacter,bDied);
	DOREPLIFETIME(ASCharacter,CurrentWeapon);
}


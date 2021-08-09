// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"

#include "SDeathCamera.h"
#include "SWeapon.h"
#include "Camera/CameraComponent.h"
#include "CombatArena/CombatArena.h"
#include "Components/CapsuleComponent.h"
#include "Components/SHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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
	
	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	HealthComponent->OnHealthChanged.AddDynamic(this,&ASCharacter::OnHealthChanged);

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON,ECollisionResponse::ECR_Ignore);

	bIsCrunch = false;

	bJump = false;

	Index = 0;
	
	WeaponSocketName = "GunSocket";

	BackSocketName = "BackSocket";

	bReplicates = true;
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	// if(IsLocallyControlled())
	// {
	// 	GetMesh()->SetVisibility(false);
	// }
	FActorSpawnParameters SpawnParameters;
	
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	for (int i = 0; i < AllWeaponsClass.Num(); i++)
	{
		InventoryWeapons.Add(GetWorld()->SpawnActor<ASWeapon>(AllWeaponsClass[i], FVector::ZeroVector,
															FRotator::ZeroRotator, SpawnParameters));
		InventoryWeapons[i]->SetOwner(this);
		InventoryWeapons[i]->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
											BackSocketName);
		InventoryWeapons[i]->SetActorHiddenInGame(true);
	}

	DeathCamera = GetWorld()->SpawnActor<ASDeathCamera>(DeathCameraClass, FVector(GetActorLocation().X,GetActorLocation().Y,GetActorLocation().Z + 700), FRotator(0,120,0),
	                                                    SpawnParameters);
	DeathCamera->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	FRotator PlayerRotation = UKismetMathLibrary::FindLookAtRotation(DeathCamera->GetActorLocation(),GetActorLocation());

	DeathCamera->SetActorRotation(PlayerRotation);
	
	if (HasAuthority())
	{

		ChangeWeapon();
	}

	
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


void ASCharacter::OnHealthChanged(float Health, float HealthDelta, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{

	if(Health <= 0.0f && !bDied)
	{
		//die!
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
		// APlayerController* PC = UGameplayStatics::GetPlayerController(this,0);
		//
		// if(PC)
		// {
		// 	PC->SetViewTargetWithBlend(DeathCamera,1.0f);
		// }

		DetachFromControllerPendingDestroy();
		SetLifeSpan(4.0f);
		CurrentWeapon->SetLifeSpan(4.0f);
		for (AActor* Weapon : InventoryWeapons)
		{
			Weapon->SetLifeSpan(2.0f);
		}
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

	PlayerInputComponent->BindAction("IndexUp", IE_Pressed,this,&ASCharacter::IndexUp);
	
	PlayerInputComponent->BindAction("IndexDown", IE_Pressed,this,&ASCharacter::IndexDown);

	PlayerInputComponent->BindAction("Reload", IE_Pressed,this,&ASCharacter::Reload);
}
void ASCharacter::IndexUp()
{
	UE_LOG(LogTemp,Log,TEXT("IndexUp"));
	if(!HasAuthority())
	{
		ServerIndexUp();
	}

	HiddePreviusWeapon(Index);
	if(Index >= InventoryWeapons.Num()-1)
	{
		Index = 0;
	}
	else
	{
		Index += 1;
	}

	UE_LOG(LogTemp,Log,TEXT("Index: %i"),Index);
	ChangeWeapon();
}

void ASCharacter::IndexDown()
{
	UE_LOG(LogTemp,Log,TEXT("IndexDown"));

	if(!HasAuthority())
	{
		ServerIndexDown();
	}
	HiddePreviusWeapon(Index);
	if(Index <= 0)
	{
		Index = InventoryWeapons.Num()-1;
	}
	else if(Index <= InventoryWeapons.Num()-1 && Index > 0)
	{
		Index -= 1;
	}
	UE_LOG(LogTemp,Log,TEXT("Index: %i"),Index);
	ChangeWeapon();
	
}

void ASCharacter::Reload()
{
	
}

void ASCharacter::HiddePreviusWeapon(int PrevIndex)
{
	InventoryWeapons[PrevIndex]->AttachToComponent(GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,BackSocketName);
	InventoryWeapons[PrevIndex]->SetActorHiddenInGame(true);
}

void ASCharacter::ChangeWeapon()
{
	CurrentWeapon = InventoryWeapons[Index];//GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass,FVector::ZeroVector,FRotator::ZeroRotator,SpawnParameters);
	CurrentWeapon->SetActorHiddenInGame(false);
	CurrentWeapon->AttachToComponent(GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,WeaponSocketName);
}

void ASCharacter::ServerIndexUp_Implementation()
{
	HiddePreviusWeapon(Index);
	if(Index >= InventoryWeapons.Num()-1)
	{
		Index = 0;
	}
	else
	{
		Index += 1;
	}

	UE_LOG(LogTemp,Log,TEXT("Index: %i"),Index);
	ChangeWeapon();
}

bool ASCharacter::ServerIndexUp_Validate()
{
	return  true;
}


void ASCharacter::ServerIndexDown_Implementation()
{
	HiddePreviusWeapon(Index);
	if(Index <= 0)
	{
		Index = InventoryWeapons.Num()-1;
	}
	else if(Index <= InventoryWeapons.Num()-1 && Index > 0)
	{
		Index -= 1;
	}
	UE_LOG(LogTemp,Log,TEXT("Index: %i"),Index);
	ChangeWeapon();
}

bool ASCharacter::ServerIndexDown_Validate()
{
	return  true;
}

void ASCharacter::OnRep_ChangeWeapon()
{
	ChangeWeapon();
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
	DOREPLIFETIME(ASCharacter,bDied);
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "SLauncher.h"


ASLauncher::ASLauncher()
{
	MaxAmmo = 15;
	AvailableAmmo = 5;
}

void ASLauncher::Fire()
{
	
	if(!HasAuthority())
	{
		if(CurrentAmmo > 0)
		{
			ServerFire();
			
			PlayAnimFire();
		}
		else
		{
			Reload();
		}
	}
	else
	{
		if(CurrentAmmo > 0)
		{
			LogicFire();
			PlayAnimFire();
		}
		else
		{
			Reload();
		}
	}

}

void ASLauncher::LogicFire()
{
	AActor* MyOwner = GetOwner();

	if(MyOwner)
	{
		if(ProjectileClass)
		{
			CurrentAmmo --;
			UE_LOG(LogTemp,Log,TEXT("Ammo: %i / MaxAmmo: %i"),CurrentAmmo,MaxAmmo);
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
			FVector ShotDirection = EyeRotation.Vector();
			
			FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
			//FRotator MuzzleRotation = SkeletalMeshComponent->GetSocketRotation(MuzzleSocketName);
			
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	
			ActorSpawnParams.Instigator = MyOwner->GetInstigator();
			GetWorld()->SpawnActor<ASGranadeAmmo>(ProjectileClass,MuzzleLocation,EyeRotation,ActorSpawnParams);

			LastFireTime = GetWorld()->TimeSeconds;
				
		}
	}
}

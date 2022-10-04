// Fill out your copyright notice in the Description page of Project Settings.


#include "SGrenadeLauncher.h"
#include "SCharacter.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

ASGrenadeLauncher::ASGrenadeLauncher()
{
}

void ASGrenadeLauncher::BeginPlay()
{
	Super::BeginPlay();
}

void ASGrenadeLauncher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASGrenadeLauncher::Fire()
{
	if (BulletsInClip > 0)
	{
		if (ProjectileClass)
		{
			MuzzleSocketName = "MuzzleSocket";
			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

			AActor* MyOwner = GetOwner();
			if (MyOwner)
			{
				FVector EyeLocation;
				FRotator EyeRotation;
				MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
				LastFireTime = GetWorld()->TimeSeconds;
				BulletsInClip--;
			}
		}

		if (MuzzleEffectOnFire)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffectOnFire, MeshComp, MuzzleSocketName);
		}
	}
	else
	{
		TryToFire = true;
	}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.f;

	bReplicates = true;

	bIsPowerupActive = false;
}


void ASPowerupActor::OnTickPowerup()
{
	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerupActive = true;
		OnRep_PowerupActive(); // make sure manually call

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);

		return;
	}

	TicksProcessed++;
	OnPowerupTicked();
}


void ASPowerupActor::ActivatePowerup(AActor* ActivateFor)
{
	OnActivated(ActivateFor);

	bIsPowerupActive = true;
	OnRep_PowerupActive(); // make sure manually call

	if (PowerupInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true); // tick only after first interval expired
	}
	else
	{
		OnTickPowerup();
	}
}


void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}


void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}
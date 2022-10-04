// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSGameState.h"
#include "EngineUtils.h"
#include <FPSGame/Public/FPSPlayerController.h>

void AFPSGameState::MulticastOnMissionComplete_Implementation(APawn* InstigatorPawn, bool bMissionSuccess)
{
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		AFPSPlayerController* PC = Cast<AFPSPlayerController>(*It);
		if (PC && PC->IsLocalController())
		{
			PC->OnMissionCompleted(InstigatorPawn, bMissionSuccess);
		}

		// Disable input
		APawn* MyPawn = PC->GetPawn();
		if (MyPawn)
		{
			MyPawn->DisableInput(PC);
		}
	}
}
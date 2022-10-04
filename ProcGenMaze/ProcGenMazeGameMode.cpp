// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProcGenMazeGameMode.h"
#include "ProcGenMazeHUD.h"
#include "ProcGenMazeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProcGenMazeGameMode::AProcGenMazeGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AProcGenMazeHUD::StaticClass();
}

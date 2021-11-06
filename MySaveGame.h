// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Engine/DataTable.h"
#include "MySaveGame.generated.h"

class ACPP_Mob;
class ACPP_Tower;
class ATowerSelector;

/**
 * 
 */
UCLASS()
class TOWER_DEFENSE_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UMySaveGame();

	UPROPERTY()
	uint32 UserIndex;

	UPROPERTY()
	FString SaveSlotName;

	UPROPERTY()
	float MainTowerHealth_Save;

	UPROPERTY()
	int32 Currency_Save;

	UPROPERTY()
	int32 Level_Save;

	UPROPERTY()
	int32 MTGUsesLeft_Save;

	UPROPERTY()
	TArray<FVector> TowersPositions_Save;
	
	UPROPERTY()
	TArray<FDataTableRowHandle> TowerType_Save;

	UPROPERTY()
	TArray<FVector> TowerSelectorPositions_Save;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PGPlayerController.h"
#include "PGInventoryItem.h"
#include "PGSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PROCGENMAZE_API UPGSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	UPGSaveGame();

	UPROPERTY()
	uint32 UserIndex;

	UPROPERTY()
	FString SaveSlotName;

	UPROPERTY()
	TMap<FIntPoint, FChunkContainer> ChunkData;

	UPROPERTY()
	int32 CurrentSeed;

	UPROPERTY()
	FIntPoint PlayerChunk;

	UPROPERTY()
	FVector PlayerLocation;

	UPROPERTY()
	int32 NextVillage;

	UPROPERTY()
	int32 NextMonsterField;

	UPROPERTY()
	TArray<TSubclassOf<APGInventoryItem>> InventoryItems;

};

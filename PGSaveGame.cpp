// Fill out your copyright notice in the Description page of Project Settings.


#include "PGSaveGame.h"

UPGSaveGame::UPGSaveGame()
{
	SaveSlotName = TEXT("SaveSlot");
	UserIndex = 0;

	ChunkData = TMap<FIntPoint, FChunkContainer>();
	InventoryItems = TArray<TSubclassOf<APGInventoryItem>>();
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "MySaveGame.h"
#include "Kismet/GameplayStatics.h"

UMySaveGame::UMySaveGame()
{
	SaveSlotName = TEXT("TestSaveSlot");
	UserIndex = 0;

	TowersPositions_Save = {};
	TowerType_Save = {};
	TowerSelectorPositions_Save = {};
}


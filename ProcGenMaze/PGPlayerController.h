// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Math/IntPoint.h"
#include "PGPlayerController.generated.h"

class APGMaze;

USTRUCT()
struct FChunkContainer
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SeedForChunk;

	UPROPERTY()
	TArray<FIntPoint> PointsInChunk;

	UPROPERTY()
	bool HasBeenLoaded;
};

/**
 * 
 */
UCLASS()
class PROCGENMAZE_API APGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	APGPlayerController();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Monster")
	bool IsInMonsterField(); 

	TMap<FIntPoint, FChunkContainer> GetChunkData() { return LocInfoPairs; }

	void SetChunkData(TMap<FIntPoint, FChunkContainer> Setter) { LocInfoPairs = Setter; }

	int32 GetSeed() { return CurrentSeed; }

	void SetCurrSeed(int32 NewSeed) { CurrentSeed = NewSeed; }

	FIntPoint GetPlayerChunk() { return PlayerChunk; }

	void SetPlayerChunk(FIntPoint Setter) { PlayerChunk = Setter; }

	int32 GetNextMonsterField() { return NextMonsterField; }

	void SetNextMonsterField(int32 Setter) { NextMonsterField = Setter; }

	int32 GetNextVillage() { return NextVillage; }

	void SetNextVillage(int32 Setter) { NextVillage = Setter; }

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Monster")
	int32 NextMonsterField; // for initial value and keeping track if running through maze for a while

	UPROPERTY(EditDefaultsOnly, Category = "Monster")
	int32 ChunksUntilNextMonsterFieldMin;

	UPROPERTY(EditDefaultsOnly, Category = "Monster")
	int32 ChunksUntilNextMonsterFieldMax;

	virtual void BeginPlay() override;

	void ControlMazeLoading(); // function controls whether a portion of the maze is loaded or unloaded

	void InitialLoad();

	void AddNewChunk(FIntPoint ChunkToAdd); // Adds a new maze chunk to the viewport

	void UpdateChunksAround(FIntPoint ChunkToUpdate); // updates the chunks around the one called so that they accurately can join in the maze

	TArray<FIntPoint> AddPoints(FRandomStream& RandStream, bool XZero, bool XMax, bool YZero, bool YMax);

	UPROPERTY() // to not get garbage collected
	TMap<FIntPoint, FChunkContainer> LocInfoPairs; // contains all maze chunks that have been loaded and their information

	UPROPERTY(BlueprintReadOnly, Category = "Monster")
	FVector PlayerExitMonsterField;

	APGMaze* MazeInGame; // a reference to the BP in the game

	FIntPoint MazeSize; // sizes of the chunks in the maze

	int32 CurrentSeed; // keeps track of the current seed we're using for generating maze chunks

	FTimerHandle TimerHandle_NeedToLoadMazeCaller; // for getting the player's location every second

	FIntPoint PlayerChunk; // chunk that the player is currently in

	const int32 MAZE_SIZE_LOADED = 3; // 3 X 3 maze chunk loaded around player at all times

	int32 WhileLoopStopper; // to ensure there aren't any infinite while loops

	int32 NextVillage;
	
};

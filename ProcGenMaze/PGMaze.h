// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/IntPoint.h"
#include "PGMaze.generated.h"

USTRUCT()
struct FIndexAndLengthInMap
{
	GENERATED_BODY()

	UPROPERTY()
	int32 StartingIndexMeshes;

	UPROPERTY()
	int32 LengthOfIndexes;
};

UCLASS()
class PROCGENMAZE_API APGMaze : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGMaze();

	void NewMaze(int32 SeedForGeneration, FIntPoint Offset, TArray<FIntPoint> StartPoints); // add a maze chunk to the viewport

	int32 GetSeed() { return Seed; } // for initial starting value

	FIntPoint GetMazeSize() { return MazeSize; } // for initial starting value

	void RemoveMazeFromViewport(FIntPoint MazeChunkToRemove, bool NeedToDespawnMonster); // remove a maze chunk from the viewport

	void Village(FIntPoint Location); // a village is spawned in the maze

	void MonsterField(FIntPoint Location, TArray<FIntPoint> ExitLocs); // a field with a monster in it is spawned in the maze

	FVector GetBlockLocationFromIntPoints(FIntPoint OffsetToChunk, FIntPoint TileInChunk);

	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	float TileSize;

	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	FIntPoint AddSoStartIsZeroZero;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	UInstancedStaticMeshComponent* FloorMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	UInstancedStaticMeshComponent* WallMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maze")
	FIntPoint MazeSize;

	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	int32 Seed;

	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	int32 OneBlockNoiseChance; // likelihood of moving one block over in path

	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	int32 TwoBlocksNoiseChance;

	UPROPERTY(EditDefaultsOnly, Category = "Maze")
	int32 ThreeBlocksNoiseChance;

	UPROPERTY() // so data isn't garbage collected
	TMap<FIntPoint, FIndexAndLengthInMap> FloorInstancesMap;

	UPROPERTY()
	TMap<FIntPoint, FIndexAndLengthInMap> WallInstancesMap;

	UFUNCTION(BlueprintImplementableEvent, Category = "Monster Field")
	void SpawnMonster(FVector SpawnLoc);

	UFUNCTION(BlueprintImplementableEvent, Category = "Monster Field")
	void DespawnMonster();

	UFUNCTION(BlueprintImplementableEvent, Category = "Monster Field")
	void UpdateNavMesh(FVector Location, FVector Scale);

	UPROPERTY(EditDefaultsOnly, Category = "Monster Field")
	float ScaleUnitNavMesh;

	FIntPoint MazeChunkOffset;

	int32 WhileLoopStopper; // to ensure there aren't any infinite while loops

	TArray<FIntPoint> PointsConnecting;

	TArray<FIntPoint> GetLineBetweenTwoPoints(FRandomStream& RandStream, FIntPoint StartingLoc, FIntPoint EndingLoc);

	TArray<FIntPoint> XNoisePos(FRandomStream& RandStream, FIntPoint CurrTile);

	TArray<FIntPoint> XNoiseNeg(FRandomStream& RandStream, FIntPoint CurrTile);

	TArray<FIntPoint> YNoisePos(FRandomStream& RandStream, FIntPoint CurrTile);

	TArray<FIntPoint> YNoiseNeg(FRandomStream& RandStream, FIntPoint CurrTile);

	void ErrantPaths(FRandomStream& RandStream, TArray<FIntPoint>& PathTiles, int32 NumPaths);

	FIntPoint PickDirection(FRandomStream& RandStream, FIntPoint PrevDirection, FIntPoint Location);

	void GetNeighborsAround(FIntPoint CurrentLocation, TArray<FIntPoint> AllTiles, bool& North, bool& East, bool& South, bool& West);
	
	void AddFloor(FIntPoint TileLoc);

	void AddNorthWall(FIntPoint TileLoc);

	void AddEastWall(FIntPoint TileLoc);

	void AddSouthWall(FIntPoint TileLoc);

	void AddWestWall(FIntPoint TileLoc);

};

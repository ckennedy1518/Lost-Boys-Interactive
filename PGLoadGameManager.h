// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGMaze.h"
#include "PGLoadGameManager.generated.h"

UCLASS()
class PROCGENMAZE_API APGLoadGameManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGLoadGameManager();

	UFUNCTION(BlueprintCallable, Category = "Save Game")
	void LoadGame();

	UFUNCTION(BlueprintCallable, Category = "Save Game")
	void SaveGame();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	APGMaze* MazeInGame;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "CPP_Tower.h"
#include "Components/StaticMeshComponent.h"
#include "TowerSelector.generated.h"

UCLASS()
class TOWER_DEFENSE_API ATowerSelector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATowerSelector();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void PlaceTower(FDataTableRowHandle TowerType);

	UPROPERTY(EditDefaultsOnly)
		UStaticMeshComponent* CylinderForClicking;

};

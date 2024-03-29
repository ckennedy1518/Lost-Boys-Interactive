// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGInventoryItem.h"
#include "PGInventoryManager.generated.h"

UCLASS()
class PROCGENMAZE_API APGInventoryManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGInventoryManager();

	TArray<TSubclassOf<APGInventoryItem>> SortItemsOnRarity(TArray<TSubclassOf<APGInventoryItem>> ItemArray);

	void AddItem(TSubclassOf<APGInventoryItem> ItemToAdd);

	TArray<TSubclassOf<APGInventoryItem>> GetInventory() { return Inventory; }

	void ClearInventory() { Inventory.Empty(); }

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	TArray<TSubclassOf<APGInventoryItem>> GetUsefulItems();

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	TArray<TSubclassOf<APGInventoryItem>> GetCollectableItems();

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	TArray<TSubclassOf<APGInventoryItem>> GetPetItems();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<TSubclassOf<APGInventoryItem>> Inventory; // the array of all inventory items

};

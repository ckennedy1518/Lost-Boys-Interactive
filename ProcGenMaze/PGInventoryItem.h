// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGInventoryItem.generated.h"

class USphereComponent;
class APGInventoryManager;

UENUM()
enum class EInventoryType
{
	Collectable,
	Useful,
	Pet
};

UENUM()
enum class EItemRarity
{
	Common,
	Uncommon,
	Rare,
	UltraRare
};

UCLASS()
class PROCGENMAZE_API APGInventoryItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGInventoryItem();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	EInventoryType GetItemType() { return ItemType; }

	EItemRarity GetItemRarity() { return ItemRarity; }

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "UI")
	void UpdateUI();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<APGInventoryItem> BPItem;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	EInventoryType ItemType;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	EItemRarity ItemRarity;

	APGInventoryManager* InventoryManager;

};

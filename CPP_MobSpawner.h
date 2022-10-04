// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "CPP_Mob.h"
#include "Templates/SubclassOf.h"
#include "CPP_MobSpawner.generated.h"

class USphereComponent;
class ATowerSelector;

USTRUCT(BlueprintType)
struct FCPP_MobSpawnerStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnPeriod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumSofts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumRunners;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumWizards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumBosses;
};

UCLASS()
class TOWER_DEFENSE_API ACPP_MobSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_MobSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitializeMobSpawner(FName Level);

	void SpawnMob(int32 MobType);

	void EndLevel();

	int32 GetMobTypeToSpawn();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetCurrency(int32 ValAdd) { Currency += ValAdd; }

	int32 GetCurrency() { return Currency; }

	UFUNCTION(BlueprintCallable)
	void MTGCalled();

	void DamageMainTower(float Damage);

	TArray<ACPP_Mob*> GetMobArray() { return MobArray; }

	void RemoveMob(ACPP_Mob* toRemove) { MobArray.Remove(toRemove); }

	USplineComponent* GetSplineComponent() { return PathSplineComponent; }

	UPROPERTY(BlueprintReadWrite)
	bool UIVisibleDamageTower;

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame();

	UPROPERTY(BlueprintReadOnly)
	bool DoneSpawning; // for UMG

	UFUNCTION(BlueprintCallable)
	void MyDoOnce();

protected:

	float SpawnPeriod;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Currency;

	UPROPERTY(EditDefaultsOnly)
	int32 EndOfLevelCurrencyAddition;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrLevel;

	int32 SoftsToSpawn;

	int32 RunnersToSpawn;

	int32 WizardsToSpawn;

	int32 BossesToSpawn;

	int32 MobsSpawned;

	int32 LevelMobsToSpawn;

	UPROPERTY(BlueprintReadWrite)
	int32 MTGUsesLeft;

	float ElapsedTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MainTowerHealth;

	UPROPERTY(BlueprintReadOnly)
	float OrigMTH;

	TArray<ACPP_Mob*> MobArray;

	UPROPERTY(BlueprintReadOnly)
	bool CanUseMTG = true;

	UDataTable* MobSpawnerDataTable;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* Cylinder;

	UPROPERTY(BlueprintReadOnly)
	USplineComponent* PathSplineComponent;

	UFUNCTION(BlueprintImplementableEvent)
	void UIToViewport();

	void GameOverForPlayer();

	void CanUseMTGAgain();

	UPROPERTY(BlueprintReadOnly)
	bool DisplayGameOver;

	UPROPERTY(BlueprintReadOnly)
	bool DisplayYouWin;

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayWaveStart(bool ShowSave);

	void BeginWave();

	bool CanBeginWave;
	
	UPROPERTY(EditDefaultsOnly, Category = "Load and Save")
	TSubclassOf<ATowerSelector> BPTowerSelector;

private: // for do once loop

	bool bDo = true;

	void ResetMyDoOnce() { bDo = true; }

	USphereComponent* GetTowerSelectorAssociatedSave;

	bool KilledAllMobsInLevel;

};
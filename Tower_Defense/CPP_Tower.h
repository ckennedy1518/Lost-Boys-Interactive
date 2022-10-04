// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CPP_Tower.generated.h"

class ACPP_MobSpawner;
class ACPP_Mob;
class ATowerSelector;

USTRUCT(BlueprintType)
struct FCPP_TowerStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FirePeriod;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDataTableRowHandle ProjectileType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Decoration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UpgradeCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SellAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UpgradedTowerRowName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ZOffsetToShootProjectile;
};

UCLASS()
class TOWER_DEFENSE_API ACPP_Tower : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACPP_Tower();

	UFUNCTION(BlueprintCallable)
		void SellTower();

	UFUNCTION(BlueprintCallable)
		void UpgradeTower();

	UFUNCTION(BlueprintCallable)
		void SetRMSkelMesh(bool RockInHand);

	UFUNCTION(BlueprintCallable)
		void RMThrow();

	void AddToITD();

	void InitializeTower(FName NewRowName);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float Range;

	float FirePeriod;

	int32 Cost;

	UPROPERTY(BlueprintReadOnly)
		int32 UpgradeCost;

	UPROPERTY(BlueprintReadOnly)
		int32 SellAmount;

	FName UpgradedRowName;

	FVector ZOffsetShootProj;

	ACPP_Mob* CurrentTarget;

	ACPP_MobSpawner* MobSpawner;

	TArray<ACPP_Mob*> InRangeArray;

	float TimeSinceFired;

	float NearestDistance;

	int32 ProjectilesFiredAtTarget;

	UObject* AnimationBP;

	FDataTableRowHandle TowerType;

	UPROPERTY(BlueprintReadOnly)
	FDataTableRowHandle ProjectileObject;

	UStaticMeshComponent* Skin;

	USkeletalMeshComponent* RockMan;

	UStaticMeshComponent* ThrowRock;

	UDataTable* TowerDataTable;

	UClass* RockThrowAnim1;

	UClass* RockThrowAnim2;

	UClass* RockThrowAnim3;

	UPROPERTY(BlueprintReadOnly)
		bool StartThrowing = false;

	UPROPERTY(BlueprintReadOnly)
		bool Idle = true;

	ATowerSelector* TS;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ExposeOnSpawn(ATowerSelector* TowerSelector);

	ATowerSelector* GetTS() { return TS; }

	FDataTableRowHandle GetTowerType() { return ProjectileObject; }

};
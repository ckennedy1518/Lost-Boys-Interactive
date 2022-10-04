// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Components/SplineComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "CPP_Mob.generated.h"

class ACPP_MobSpawner;

USTRUCT(BlueprintType)
struct FCPP_MobStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle MobType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeathWorth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* WalkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* DieAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* HitByCGAnimation;
};

UCLASS()
class TOWER_DEFENSE_API ACPP_Mob : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Mob();

	UFUNCTION(BlueprintCallable)
	float GetImminentProjectileDamage() { return ImminentProjectileDamage; }

	UFUNCTION(BlueprintCallable)
	void SetImminentProjectileDamage(float IPD) { ImminentProjectileDamage = IPD; }

	UFUNCTION(BlueprintCallable)
	float GetHealth() { return Health; }

	void ApplyTowerDamage(float ProjDamage);

	UFUNCTION(BlueprintCallable)
	void HitByCastleGuard();

	UFUNCTION(BlueprintCallable)
	FDataTableRowHandle GetMobType() { return MobType; }

	UFUNCTION(BlueprintCallable)
	bool GetBossCloseToTower() { return BossCloseToTower; }

	UFUNCTION(BlueprintCallable)
	float GetSpeed() { return Speed; }

	void ExposeOnSpawn(USplineComponent* SplineFromMS, FDataTableRowHandle TypeOfMob);

	UDataTable* GetDataTable() { return MobDataTable; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	ACPP_MobSpawner* MobSpawner;

	float ImminentProjectileDamage;

	float Health;

	float Speed;

	UPROPERTY(BlueprintReadOnly)
	float AnimationSpeed;

	float SpeedMultiplier; // for wizrds and bosses, data driven

	float Damage;

	UPROPERTY(BlueprintReadOnly)
	float CurrDistance;

	FDataTableRowHandle MobType;

	int32 DeathCurrencyAdd;

	bool GoingToDie = false;

	bool BossCloseToTower = false;

	float TimeDead = 0.f;

	UPROPERTY(BlueprintReadOnly)
	USplineComponent* PathSplineComponent;

	UDataTable* MobDataTable;

	UPROPERTY(BlueprintReadOnly)
	USkeletalMeshComponent* Skin;

	// animations

	UClass* WalkAnim;

	UClass* DieAnim;

	UClass* HitByCGAnim;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitializeMob();
};
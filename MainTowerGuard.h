// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_MobSpawner.h"
#include "Components/SplineComponent.h"
#include "MainTowerGuard.generated.h"

UCLASS()
class TOWER_DEFENSE_API AMainTowerGuard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMainTowerGuard();

	UFUNCTION(BlueprintCallable)
	void RunAndAttack() { CanTick = true; }

	UFUNCTION(BlueprintCallable)
	void AnimationSequenceCompleted();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(BluePrintReadOnly)
	bool CanTick = false;

	bool ToPath = false;

	UPROPERTY(BluePrintReadOnly)
	bool ShouldRun = true;

	UPROPERTY(BluePrintReadOnly)
	bool Running = false;

	UPROPERTY(BlueprintReadWrite)
	bool AboutToHitMob = false;

	bool HitMob = false;

	UPROPERTY(BlueprintReadOnly)
	float Distance;

	UPROPERTY(BlueprintReadOnly)
	ACPP_Mob* MobHit;

	ACPP_MobSpawner* MobSpawner;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* CastleGuard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* PathSplineComponent;

	UClass* AnimationStateMachine;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetCGMesh(USkeletalMeshComponent* NewSkelMesh) { CastleGuard = NewSkelMesh; }
};
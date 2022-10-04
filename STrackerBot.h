// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	FVector GetNextPathPoint();

	FVector NextPoint; // next point in navigation path
	
	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	USHealthComponent* HealthComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, 
		class AController* InstigatedBy, AActor* DamageCauser);

	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	UParticleSystem* ExplosionEffect;

	bool bExploded = false;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float ExplosionDamage;

	FTimerHandle TimerHandle_SelfDamage;

	FTimerHandle TimerHandle_RefreshPath;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float SelfDamageInterval;

	bool bStartedSelfDestruction = false;

	void DamageSelf();

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	USoundCue* ExplodeSound;

	void NearbyAI();

	int32 PowerLevel = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float CloseToOtherBotRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	int32 MaxPowerLevel;

	void RefreshPath();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};

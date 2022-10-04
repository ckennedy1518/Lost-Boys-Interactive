// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class URadialForceComponent;
class UMaterial;
class UParticleSystem;
class UStaticMeshComponent;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Explosive Barrel")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Explosive Barrel")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Explosive Barrel")
	UMaterial* AfterExplosionMaterial;

	UPROPERTY(VisibleAnywhere, Category = "Explosive Barrel")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Explosive Barrel")
	URadialForceComponent* RadialForceComp;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Explosive Barrel")
	float ExplosionImpulse;

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded = false;

	UFUNCTION()
	void OnRep_Exploded();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

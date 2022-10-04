// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_Mob.h"
#include "Engine/DataTable.h"
#include "NiagaraComponent.h"
#include "CPP_Projectile.generated.h"

USTRUCT(BlueprintType)
struct FCPP_ProjectileStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Decoration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectLength;
};

UCLASS()
class TOWER_DEFENSE_API ACPP_Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Projectile();

	void ExposeOnSpawn(ACPP_Mob* CurrentTarget, FDataTableRowHandle ProjObject);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float Speed;

	float Damage;

	FDataTableRowHandle ProjectileType;

	float EffectLength;

	ACPP_Mob* Target;

	FVector Direction;

	bool DamageDealt;

	bool LifeSpanSet;

	UNiagaraComponent* LaserBeamSystem;

	UDataTable* ProjectileDataTable;

	UStaticMeshComponent* Skin;

	UNiagaraSystem* Effect;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	void DeleteProjectile();

	void InitializeProjectile();

};

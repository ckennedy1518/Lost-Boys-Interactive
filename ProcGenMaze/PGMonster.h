// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PGMonster.generated.h"

class UPawnSensingComponent;

UCLASS()
class PROCGENMAZE_API APGMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APGMonster();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	UFUNCTION()
	void HearPlayer(APawn* NoiseInstigator, const FVector& Location, float Volume);

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;

	float TimeSinceHeard;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

// Fill out your copyright notice in the Description page of Project Settings.


#include "TowerSelector.h"
#include "CPP_Tower.h"
#include <Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h>
#include "Kismet/GameplayStatics.h"

// Sets default values
ATowerSelector::ATowerSelector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CylinderForClicking = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerPlacedForClicking"));

	RootComponent = CylinderForClicking;
}

void ATowerSelector::BeginPlay()
{
	Super::BeginPlay();

}

void ATowerSelector::PlaceTower(FDataTableRowHandle TowerType)
{
	FVector location = GetActorLocation(); // spawn tower selector
	FRotator rotation(0.f, 0.f, 0.f);
	FActorSpawnParameters SpawnInfo;

	ACPP_Tower* SpawnedActor = GetWorld()->SpawnActorDeferred<ACPP_Tower>(ACPP_Tower::StaticClass(), FTransform(rotation, location));

	if (SpawnedActor)
	{
		SpawnedActor->ExposeOnSpawn(this);
		UGameplayStatics::FinishSpawningActor(SpawnedActor, FTransform(FRotator(0.f, 0.f, 0.f), GetActorLocation()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not finishing Mob Spawn"));
	}
	
	if (TowerType.RowName == "Orange1")
	{
		SpawnedActor->InitializeTower(FName(TEXT("Orange")));
	}
	else if (TowerType.RowName == "Stone1")
	{
		SpawnedActor->InitializeTower(FName(TEXT("Stone")));
	}
	else if (TowerType.RowName == "Blue1")
	{
		SpawnedActor->InitializeTower(FName(TEXT("Blue")));
	}
	else if (TowerType.RowName == "Yellow1")
	{
		SpawnedActor->InitializeTower(FName(TEXT("Yellow")));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Tower Initialize Function call"));
	}

	SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, -1000.f)); // put below map, don't destroy in case tower is sold
}


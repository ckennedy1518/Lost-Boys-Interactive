// Fill out your copyright notice in the Description page of Project Settings.


#include "PGMonster.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "ProcGenMaze/ProcGenMazeCharacter.h"

// Sets default values
APGMonster::APGMonster()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
}

// Called when the game starts or when spawned
void APGMonster::BeginPlay()
{
	Super::BeginPlay();
	
	PawnSensingComp->OnHearNoise.AddDynamic(this, &APGMonster::HearPlayer);
}

// Called every frame
void APGMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceHeard += DeltaTime;

	Speed = GetVelocity().Size();
}


void APGMonster::HearPlayer(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	AProcGenMazeCharacter* Player = Cast<AProcGenMazeCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());

	if (Player && FMath::IsNearlyEqual(Player->GetPlayerMonsterStartLocation().X, 0.f) && FMath::IsNearlyEqual(Player->GetPlayerMonsterStartLocation().Y, 0.f))
	{
		Player->SetPlayerMonsterStartLocation(Player->GetActorLocation()); // the first time we heard the player is where they'll begin
		TimeSinceHeard = 0;
	}

	if (Player && TimeSinceHeard >= 3)
	{
		Player->SetPlayerMonsterStartLocation(FVector::ZeroVector);
	}
	TimeSinceHeard = 0;
}
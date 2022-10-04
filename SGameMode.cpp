// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"
#include "TimerManager.h"
#include "SHealthComponent.h"
#include "SGameState.h"
#include "SPlayerState.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.f;
	WaveCount = 0;

	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f; // tick on low frequency
}


void ASGameMode::StartWave()
{
	WaveCount++;
	NumBotsToSpawn = 2 * WaveCount;

	ASGameState* GS = Cast<ASGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->AddToWaveNumber();
	}

	if (WaveCount == 1) // only need to start checking if players are alive once
	{
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ASGameMode::CheckAnyPlayerAlive, 1.f, true, 0.f);
	}

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);

	SetWaveState(EWaveState::WaveInProgress);
}


void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
}


void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	if (--NumBotsToSpawn <= 0)
	{
		EndWave();
	}
}


void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}


void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}


void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves);

	SetWaveState(EWaveState::WaitingToStart);
	RestartDeadPlayers();
}


void ASGameMode::CheckWaveState()
{
	if (NumBotsToSpawn > 0 || GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart))
		return;

	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (!TestPawn || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);

		PrepareForNextWave();
	}
}


void ASGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.f)
			{ // player doesn't have health component
				return; // player still alive
			}
		}
	}

	GameOver(); // no player alive
}


void ASGameMode::GameOver()
{
	EndWave();

	// @TODO: Finish match
	UE_LOG(LogTemp, Warning, TEXT("Game over, players died!"));

	SetWaveState(EWaveState::GameOver);
}


void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}


void ASGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && !PC->GetPawn()) // player is dead
		{
			RestartPlayer(PC);
		}
	}
}
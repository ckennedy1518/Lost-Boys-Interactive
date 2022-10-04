// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLoadGameManager.h"
#include "PGSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "PGPlayerController.h"
#include "ProcGenMaze/ProcGenMazeCharacter.h"
#include "PGInventoryManager.h"

// Sets default values
APGLoadGameManager::APGLoadGameManager()
{

}

// Called when the game starts or when spawned
void APGLoadGameManager::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> MazeActors;
	UGameplayStatics::GetAllActorsOfClass(this, APGMaze::StaticClass(), MazeActors);

	if (MazeActors.Num() > 0)
	{
		MazeInGame = Cast<APGMaze>(MazeActors[0]);
	}
}


void APGLoadGameManager::SaveGame()
{
	UPGSaveGame* SaveGameInstance = Cast<UPGSaveGame>(UGameplayStatics::CreateSaveGameObject(UPGSaveGame::StaticClass()));

	APGPlayerController* PC = Cast<APGPlayerController>(GetWorld()->GetFirstPlayerController());
	AProcGenMazeCharacter* PGMC = Cast<AProcGenMazeCharacter>(PC->GetPawn());

	SaveGameInstance->ChunkData = PC->GetChunkData();
	SaveGameInstance->CurrentSeed = PC->GetSeed();
	SaveGameInstance->PlayerChunk = PC->GetPlayerChunk();
	SaveGameInstance->PlayerLocation = PGMC->GetActorLocation();
	SaveGameInstance->NextMonsterField = PC->GetNextMonsterField();
	SaveGameInstance->NextVillage = PC->GetNextVillage();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, APGInventoryManager::StaticClass(), FoundActors);
	APGInventoryManager* IM;
	if (FoundActors.Num() > 0)
	{
		IM = Cast<APGInventoryManager>(FoundActors[0]);

		SaveGameInstance->InventoryItems = IM->GetInventory();
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);
}


void APGLoadGameManager::LoadGame()
{
	UPGSaveGame* LoadGameInstance = Cast<UPGSaveGame>(UGameplayStatics::LoadGameFromSlot("SaveSlot", 0));

	APGPlayerController* PC = Cast<APGPlayerController>(GetWorld()->GetFirstPlayerController());
	AProcGenMazeCharacter* PGMC = Cast<AProcGenMazeCharacter>(PC->GetPawn());

	PGMC->DisableInput(PC); // don't want the player to be able to move when loading

	FIntPoint PlayerLocWhenLoading = PC->GetPlayerChunk();

	for (int32 i = 0; i < 3; i++) // unload the nine chunks around where the player is
	{
		for (int32 j = 0; j < 3; j++)
		{
			FIntPoint Loc(PlayerLocWhenLoading.X + i - 1, PlayerLocWhenLoading.Y + j - 1);
			bool MonsterField = PC->GetChunkData()[Loc].PointsInChunk.Num() == 5;
			MazeInGame->RemoveMazeFromViewport(Loc, MonsterField);
		}
	}

	PC->SetChunkData(LoadGameInstance->ChunkData);
	PC->SetCurrSeed(LoadGameInstance->CurrentSeed);
	PC->SetPlayerChunk(LoadGameInstance->PlayerChunk);

	for (int32 i = 0; i < 3; i++) // create the nine blocks around the player
	{
		for (int32 j = 0; j < 3; j++)
		{
			if (MazeInGame)
			{
				FChunkContainer Temp = LoadGameInstance->ChunkData[FIntPoint(LoadGameInstance->PlayerChunk.X + i - 1, LoadGameInstance->PlayerChunk.Y + j - 1)];
				if (Temp.PointsInChunk.Num() == 5)
				{
					MazeInGame->MonsterField(FIntPoint(LoadGameInstance->PlayerChunk.X + i - 1, LoadGameInstance->PlayerChunk.Y + j - 1), Temp.PointsInChunk);
				}
				else
				{
					MazeInGame->NewMaze(Temp.SeedForChunk, FIntPoint(LoadGameInstance->PlayerChunk.X + i - 1, LoadGameInstance->PlayerChunk.Y + j - 1), Temp.PointsInChunk);
				}
			}
		}
	}

	PGMC->SetActorLocation(LoadGameInstance->PlayerLocation); // move player
	PC->SetNextMonsterField(LoadGameInstance->NextMonsterField);
	PC->SetNextVillage(LoadGameInstance->NextVillage);

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, APGInventoryManager::StaticClass(), FoundActors);
	APGInventoryManager* IM;
	if (FoundActors.Num() > 0)
	{
		IM = Cast<APGInventoryManager>(FoundActors[0]);
		if (IM)
		{
			IM->ClearInventory(); // unload inventory information
			PGMC->ClearVarsOnLoad(); // get rid of saved info
			UE_LOG(LogTemp, Warning, TEXT("Clearing inventory"));

			for (int32 i = 0; i < LoadGameInstance->InventoryItems.Num(); i++)
			{
				IM->AddItem(LoadGameInstance->InventoryItems[i]);
				UE_LOG(LogTemp, Warning, TEXT("Adding item to inventory"));
			} // inventory is now updated to be the same as it was

			PGMC->ReinstantiateUIVariables();
		}
	}

	PGMC->EnableInput(PC);
}
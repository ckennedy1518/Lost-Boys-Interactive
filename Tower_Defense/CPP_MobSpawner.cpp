// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_MobSpawner.h"
#include "Kismet/KismetSystemLibrary.h"
#include <Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "MySaveGame.h"
#include "CPP_Tower.h"
#include "TowerSelector.h"
#include "CPP_Mob.h"
#include "Kismet/KismetStringLibrary.h"
#include "Components/SphereComponent.h"
#include "MainTowerGuard.h"

// Sets default values
ACPP_MobSpawner::ACPP_MobSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Cylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cylinder"));
	PathSplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");

	RootComponent = Cylinder;
	if (PathSplineComponent)
	{
		PathSplineComponent->SetupAttachment(Cylinder);
	}

	Currency = 5000.f;
	MTGUsesLeft = 2;
	MainTowerHealth = 1000.f;
	EndOfLevelCurrencyAddition = 1000;
	DisplayYouWin = false;
	DisplayGameOver = false;

	GetTowerSelectorAssociatedSave = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	GetTowerSelectorAssociatedSave->SetSphereRadius(5.f);
}

// Called when the game starts or when spawned
void ACPP_MobSpawner::BeginPlay()
{
	Super::BeginPlay();

	FString DataTableLoad = "DataTable'/Game/TowerDefense/Blueprints/DataTables/DT_MobSpawner.DT_MobSpawner'";
	MobSpawnerDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, *DataTableLoad));

	if (!MobSpawnerDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Data table isn't working"));
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
	}
	
	FName LevelStart = "Level1";
	InitializeMobSpawner(LevelStart);

	OrigMTH = MainTowerHealth;
}

// Called every frame
void ACPP_MobSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CanBeginWave)
	{
		ElapsedTime += DeltaTime;
		if (ElapsedTime >= SpawnPeriod)
		{
			if (MobsSpawned < LevelMobsToSpawn)
			{
				ElapsedTime = 0;
				SpawnMob(GetMobTypeToSpawn());
			}
			else
			{
				if (MobArray.Num() == 0)
				{
					KilledAllMobsInLevel = true;
					MyDoOnce();
				}
				else
				{
					DoneSpawning = true;
				}
			}
		}
	}
}


void ACPP_MobSpawner::BeginWave()
{
	CanBeginWave = true;
}


void ACPP_MobSpawner::InitializeMobSpawner(FName Level)
{
	FCPP_MobSpawnerStruct* MobSpawnerData = MobSpawnerDataTable->FindRow<FCPP_MobSpawnerStruct>(Level, FString("Find MobSpawnerStruct"));
	if (MobSpawnerData) // "RowFound" option
	{
		SpawnPeriod = MobSpawnerData->SpawnPeriod;
		CurrLevel = MobSpawnerData->Level;
		SoftsToSpawn = MobSpawnerData->NumSofts;
		RunnersToSpawn = MobSpawnerData->NumRunners;
		WizardsToSpawn = MobSpawnerData->NumWizards;
		BossesToSpawn = MobSpawnerData->NumBosses;
		LevelMobsToSpawn = SoftsToSpawn + RunnersToSpawn + WizardsToSpawn + BossesToSpawn;
		MobsSpawned = 0; // explicit about this value
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MobSpawner data isn't being initialized"));
	}

	DisplayWaveStart(KilledAllMobsInLevel);
	DoneSpawning = false;
	KilledAllMobsInLevel = false;

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ACPP_MobSpawner::BeginWave, 10.0f, false);
}


void ACPP_MobSpawner::SpawnMob(int32 MobType)
{
	FActorSpawnParameters SpawnInfo;
	ACPP_Mob* TempSpawnedMob = GetWorld()->SpawnActor<ACPP_Mob>(GetActorLocation(), FRotator(0.f, 0.f, 0.f), SpawnInfo);

	FName MobRowName;
	if (MobType == 0)
	{
		MobRowName = "Softs";
	}
	else if (MobType == 1)
	{
		MobRowName = "Runners";
	}
	else if (MobType == 2)
	{
		MobRowName = "Wizard";
	}
	else if (MobType == 3)
	{
		MobRowName = "Boss";
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MobType (an int) wasn't 0-3"));
	}

	FDataTableRowHandle MobTypeToSpawn;
	UDataTable* TempDataTable = TempSpawnedMob->GetDataTable();
	FCPP_MobStruct* TempRefMobStruct = nullptr;
	if (TempDataTable)
	{
		TempRefMobStruct = TempDataTable->FindRow<FCPP_MobStruct>(MobRowName, FString("Spawning Mob"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't find Mob Data Table"));
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
	}

	if (TempRefMobStruct)
	{
		MobTypeToSpawn = TempRefMobStruct->MobType;
		TempSpawnedMob->Destroy(); // delete temporary spawn
	}

	FVector SpawnLoc = PathSplineComponent->GetLocationAtDistanceAlongSpline(PathSplineComponent->GetSplineLength() - 1.f, ESplineCoordinateSpace::World);

	ACPP_Mob* MobSpawned = GetWorld()->SpawnActorDeferred<ACPP_Mob>(ACPP_Mob::StaticClass(), FTransform(FRotator(0.f, 0.f, 0.f), SpawnLoc));

	if (MobSpawned)
	{
		MobSpawned->ExposeOnSpawn(PathSplineComponent, MobTypeToSpawn);
		UGameplayStatics::FinishSpawningActor(MobSpawned, FTransform(FRotator(0.f, 0.f, 0.f), SpawnLoc));
		MobSpawned->InitializeMob();
		MobArray.Add(MobSpawned);
		MobsSpawned++;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not finishing Mob Spawn"));
	}
}


void ACPP_MobSpawner::EndLevel()
{
	CanBeginWave = false;
	CurrLevel++;
	TArray<FName> MSRowNames = MobSpawnerDataTable->GetRowNames();
	if (MSRowNames.Num() >= CurrLevel)
	{
		FString LevelTemp = "Level";
		LevelTemp.AppendInt(CurrLevel); // concatenate number
		InitializeMobSpawner(FName(*LevelTemp));
	}
	else
	{
		DisplayYouWin = true;
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ACPP_MobSpawner::GameOverForPlayer, 5.0f, false); // delay for 5 seconds before quitting
	}
}

int32 ACPP_MobSpawner::GetMobTypeToSpawn()
{
	int32 counter = 0; // make sure not in infinite loop
	while (counter < 1000) 
	{
		counter++; // worst case scenario leave game

		int32 temp = UKismetMathLibrary::RandomInteger(4); // want int between 0 and 3
		switch (temp)
		{
		case(0):
			if (SoftsToSpawn != 0)
			{
				SoftsToSpawn--;
				return temp;
			}
			break;
		case(1):
			if (RunnersToSpawn != 0)
			{
				RunnersToSpawn--;
				return temp;
			}
			break;
		case(2):
			if (WizardsToSpawn != 0)
			{
				WizardsToSpawn--;
				return temp;
			}
			break;
		case(3):
			if (BossesToSpawn != 0)
			{
				BossesToSpawn--;
				return temp;
			}
			break;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Infinite while loop hit"));
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
	return 0;
}

void ACPP_MobSpawner::MTGCalled()
{
	CanUseMTG = false;
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &ACPP_MobSpawner::CanUseMTGAgain, 5.0f, false); // delay for 5 seconds
}


void ACPP_MobSpawner::CanUseMTGAgain()
{
	CanUseMTG = true;
}

void ACPP_MobSpawner::DamageMainTower(float Damage)
{
	UIToViewport(); // implementation in blueprint

	MainTowerHealth -= Damage;
	if (MainTowerHealth <= 0)
	{
		DisplayGameOver = true;
		FTimerHandle UnusedHandle;
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &ACPP_MobSpawner::GameOverForPlayer, 5.0f, false); // delay for 5 seconds before quitting
	}
}

void ACPP_MobSpawner::MyDoOnce()
{
	if (bDo)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 200, FColor::Red, TEXT("In MyDoOnce"));
		bDo = false;
		Currency += EndOfLevelCurrencyAddition; // end of level amount

		EndLevel();
		ResetMyDoOnce();
	}
}


void ACPP_MobSpawner::GameOverForPlayer()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}


void ACPP_MobSpawner::SaveGame()
{
	UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));

	SaveGameInstance->MainTowerHealth_Save = MainTowerHealth;
	SaveGameInstance->Currency_Save = Currency;
	SaveGameInstance->Level_Save = CurrLevel; // all the MobSpawner data
	SaveGameInstance->MTGUsesLeft_Save = MTGUsesLeft;

	TArray<AActor*> FoundActors;

	TArray<FVector> TowersPositions = {};
	TArray<FDataTableRowHandle> TowerTypes = {};
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_Tower::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (AActor* TempTower : FoundActors)
		{
			ACPP_Tower* Tower = Cast<ACPP_Tower>(TempTower);
			TowersPositions.Add(Tower->GetActorLocation());
			TowerTypes.Add(Tower->GetTowerType());
		}
	}
	SaveGameInstance->TowersPositions_Save = TowersPositions;
	SaveGameInstance->TowerType_Save = TowerTypes;

	TArray<FVector> TowerSelectorPositions = {}; // starts empty, will be populated
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATowerSelector::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		for (AActor* TempTowerSelector : FoundActors)
		{
			ATowerSelector* TowerSelector = Cast<ATowerSelector>(TempTowerSelector);
			if (TowerSelector)
			{
				TowerSelectorPositions.Add(TowerSelector->GetActorLocation());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TowerSelector after cast is null when saving game"));
			}
		}
	}

	SaveGameInstance->TowerSelectorPositions_Save = TowerSelectorPositions;

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);
}


void ACPP_MobSpawner::LoadGame()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading game"));

	UMySaveGame* LoadGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())); // delete this line and only use one below?
	LoadGameInstance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot("TestSaveSlot", 0));

	MainTowerHealth = LoadGameInstance->MainTowerHealth_Save;
	Currency = LoadGameInstance->Currency_Save;
	CurrLevel = LoadGameInstance->Level_Save;

	if (LoadGameInstance->MTGUsesLeft_Save == 0)
	{
		MTGUsesLeft = LoadGameInstance->MTGUsesLeft_Save;
		TArray<AActor*> AllCastleGuards;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainTowerGuard::StaticClass(), AllCastleGuards);
		for (AActor* Temp : AllCastleGuards)
		{
			AMainTowerGuard* MTG = Cast<AMainTowerGuard>(Temp);
			MTG->SetCGMesh(nullptr);
			MTG->Destroy();
		}
	}
	else if (LoadGameInstance->MTGUsesLeft_Save == 1)
	{
		MTGUsesLeft = LoadGameInstance->MTGUsesLeft_Save;
		TArray<AActor*> AllCastleGuards;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainTowerGuard::StaticClass(), AllCastleGuards);
		if (AllCastleGuards.Num() > 0)
		{
			AMainTowerGuard* MTG = Cast<AMainTowerGuard>(AllCastleGuards[0]);
			MTG->SetCGMesh(nullptr);
			MTG->Destroy();
		}
	} // else it's two, carry on as normal

	FString LevelName = "Level";
	FString LevelNumber = UKismetStringLibrary::Conv_IntToString(CurrLevel);
	LevelName.Append(LevelNumber);
	FName ToCallInitialize(LevelName);

	InitializeMobSpawner(ToCallInitialize);

	TArray<AActor*> FoundActors; // destroy current TowerSelectors
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATowerSelector::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		for (AActor* TempTowerSelector : FoundActors)
		{
			ATowerSelector* TowerSelector = Cast<ATowerSelector>(TempTowerSelector);
			TowerSelector->CylinderForClicking->SetStaticMesh(nullptr);
			TowerSelector->Destroy();
		}
	}

	// spawn new tower selectors
	for (int32 i = 0; i < LoadGameInstance->TowerSelectorPositions_Save.Num(); i++)
	{
		FVector Location = LoadGameInstance->TowerSelectorPositions_Save[i];
		FActorSpawnParameters SpawnInfo;
		if (BPTowerSelector)
		{
			GetWorld()->SpawnActor<ATowerSelector>(BPTowerSelector, Location, FRotator(0.f, 0.f, 0.f), SpawnInfo);
		}
	}

	// spawn towers
	for (int32 i = 0; i < LoadGameInstance->TowersPositions_Save.Num(); i++)
	{
		FActorSpawnParameters SpawnInfo;

		ACPP_Tower* SpawnedActor = GetWorld()->SpawnActorDeferred<ACPP_Tower>(ACPP_Tower::StaticClass(), 
			FTransform(FRotator(0.f, 0.f, 0.f), LoadGameInstance->TowersPositions_Save[i]));

		if (SpawnedActor)
		{
			TArray<AActor*> OverlappingActor = {};
			GetTowerSelectorAssociatedSave->SetWorldLocation(LoadGameInstance->TowersPositions_Save[i] - FVector(0.f, 0.f, 1000.f));
			GetTowerSelectorAssociatedSave->GetOverlappingActors(OverlappingActor);

			ATowerSelector* TSAssociated = nullptr;
			if (OverlappingActor.Num() > 0)
			{
				TSAssociated = Cast<ATowerSelector>(OverlappingActor[0]);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Not overlapping anything"));
			}
			if (TSAssociated)
			{
				SpawnedActor->ExposeOnSpawn(TSAssociated);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TSAssociated is null"));
			}
			UGameplayStatics::FinishSpawningActor(SpawnedActor, FTransform(FRotator(0.f, 0.f, 0.f), LoadGameInstance->TowersPositions_Save[i]));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Not finishing Mob Spawn"));
		}

		if (LoadGameInstance->TowerType_Save[i].RowName == "Orange1")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Orange")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Orange2")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Orange_1")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Orange3")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Orange_2")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Stone1")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Stone")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Stone2")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Stone_1")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Stone3")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Stone_2")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Blue1")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Blue")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Blue2")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Blue_1")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Blue3")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Blue_2")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Yellow1")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Yellow")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Yellow2")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Yellow_1")));
		}
		else if (LoadGameInstance->TowerType_Save[i].RowName == "Yellow3")
		{
			SpawnedActor->InitializeTower(FName(TEXT("Yellow_2")));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Tower Initialize Function call"));
		}
	}
}
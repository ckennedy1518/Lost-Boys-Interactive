// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Tower.h"
#include "CPP_MobSpawner.h"
#include "CPP_Projectile.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/DataTable.h"
#include "TowerSelector.h"

// Sets default values
ACPP_Tower::ACPP_Tower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Skin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skin"));
	RockMan = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RockMan"));
	ThrowRock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrowRock"));

	RootComponent = Skin;
	RockMan->SetupAttachment(Skin);
	ThrowRock->SetupAttachment(RockMan);
}

// Called when the game starts or when spawned
void ACPP_Tower::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> FoundActors; // set MobSpawner - only have to "get all actors" once
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_MobSpawner::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		MobSpawner = Cast<ACPP_MobSpawner>(FoundActors[0]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find MobSpawner, TOWER"));
	}

	FString DataTableLoad = "DataTable'/Game/TowerDefense/Blueprints/DataTables/DT_CPPTowerData.DT_CPPTowerData'";
	TowerDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, *DataTableLoad));

	if (!TowerDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tower table isn't working"));
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
	}
}

// Called every frame
void ACPP_Tower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InRangeArray.Num() != 0)
		InRangeArray.Empty(); // clear array, will be repopulated

	TimeSinceFired += DeltaTime;
	if (TimeSinceFired >= FirePeriod)
	{
		NearestDistance = Range + 1;
		CurrentTarget = nullptr;
		if (MobSpawner->GetMobArray().Num() != 0) // level isn't over
		{
			for (ACPP_Mob* Mob : MobSpawner->GetMobArray())
			{ // set our current target
				float DistanceToMob = FVector::Dist(GetActorLocation(), Mob->GetActorLocation());
				if (DistanceToMob <= Range)
				{
					InRangeArray.Add(Mob);
				}
				if (DistanceToMob < NearestDistance)
				{
					CurrentTarget = Mob;
					NearestDistance = DistanceToMob;
				}
			}

			if (CurrentTarget)
			{
				TimeSinceFired = 0;
				if (ProjectileObject.RowName == "Stone1" || ProjectileObject.RowName == "Stone2" || ProjectileObject.RowName == "Stone3")
				{
					StartThrowing = true;
					Idle = false;

					FRotator temp = UKismetMathLibrary::FindLookAtRotation((GetActorLocation() + ZOffsetShootProj),
						CurrentTarget->GetActorLocation()); // this chunk points the RockMan at the mob he's throwing at
					float GarbageTemp;
					float yaw;
					UKismetMathLibrary::BreakRotator(temp, GarbageTemp, GarbageTemp, yaw);
					temp = UKismetMathLibrary::MakeRotator(0, 0, yaw);
					RockMan->SetWorldRotation(UKismetMathLibrary::ComposeRotators(temp, FRotator(0, -90, 0))); // like combine rotators from BP
				}
				else
				{ // regular tower functionality, spawn a projectile
					ACPP_Projectile* ProjFired = GetWorld()->SpawnActorDeferred<ACPP_Projectile>(ACPP_Projectile::StaticClass(), 
						FTransform(FRotator(0.f, 0.f, 0.f), GetActorLocation() + ZOffsetShootProj));
					if (ProjFired)
					{
						ProjFired->ExposeOnSpawn(CurrentTarget, ProjectileObject);
						UGameplayStatics::FinishSpawningActor(ProjFired, FTransform(FRotator(0.f, 0.f, 0.f), GetActorLocation() + ZOffsetShootProj));
					}

					if (ProjectileObject.RowName != "Yellow3")
					{
						AddToITD();
					}
				}
			}
			else
			{
				if (InRangeArray.Num() == 0 && (ProjectileObject.RowName == "Stone1" ||
					ProjectileObject.RowName == "Stone2" || ProjectileObject.RowName == "Stone3"))
				{ // no objects in range and tower is Rock Solid
					Idle = true;
					StartThrowing = false;
				}
			}
		}
		else
		{
			Idle = true;
			StartThrowing = false;
			ThrowRock->SetStaticMesh(nullptr);
		}
	}
}

void ACPP_Tower::SellTower()
{
	MobSpawner->SetCurrency(SellAmount); // update currency

	if (TS)
	{
		TS->SetActorLocation(TS->GetActorLocation() + FVector(0.f, 0.f, 1000.f)); // translate actor back to right position
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TS wasn't owner"));
	}

	Destroy(); // destroy current actor
}

void ACPP_Tower::UpgradeTower()
{
	MobSpawner->SetCurrency(UpgradeCost * -1); // update currency
	InitializeTower(UpgradedRowName);
}

void ACPP_Tower::SetRMSkelMesh(bool RockInHand)
{
	if (RockInHand)
	{ // set rock to be in the hand
		if (ProjectileObject.RowName == "Stone1")
		{
			FString RockInHandMesh = "StaticMesh'/Game/TowerDefense/Meshes/ProjectileMeshes/Rock1.Rock1'";
			UStaticMesh* TempRockRef = LoadObject<UStaticMesh>(NULL, *RockInHandMesh);
			if (TempRockRef)
			{
				ThrowRock->SetStaticMesh(TempRockRef);
			}
		}
		else if (ProjectileObject.RowName == "Stone2")
		{
			FString RockInHandMesh = "StaticMesh'/Game/TowerDefense/Meshes/ProjectileMeshes/Rock2.Rock2'";
			UStaticMesh* TempRockRef = LoadObject<UStaticMesh>(NULL, *RockInHandMesh);
			if (TempRockRef)
			{
				ThrowRock->SetStaticMesh(TempRockRef);
			}
		}
		else
		{
			FString RockInHandMesh = "StaticMesh'/Game/TowerDefense/Meshes/ProjectileMeshes/Rock3.Rock3'";
			UStaticMesh* TempRockRef = LoadObject<UStaticMesh>(NULL, *RockInHandMesh);
			if (TempRockRef)
			{
				ThrowRock->SetStaticMesh(TempRockRef);
			}
		}
	}
	else
	{ // take rock out of the hand
		ThrowRock->SetStaticMesh(nullptr);
	}
}

void ACPP_Tower::RMThrow()
{
	SetRMSkelMesh(false);

	FActorSpawnParameters SpawnInfo;

	ACPP_Projectile* ProjFired = GetWorld()->SpawnActorDeferred<ACPP_Projectile>(ACPP_Projectile::StaticClass(),
		FTransform(FRotator(0.f, 0.f, 0.f), ThrowRock->GetComponentLocation()));
	if (ProjFired)
	{
		ProjFired->ExposeOnSpawn(CurrentTarget, ProjectileObject);
		UGameplayStatics::FinishSpawningActor(ProjFired, FTransform(FRotator(0.f, 0.f, 0.f), ThrowRock->GetComponentLocation()));
	}

	AddToITD();
}

void ACPP_Tower::AddToITD()
{
	static const FString temp = "Projectile for getting damage";
	FCPP_ProjectileStruct* SeeDamage = ProjectileObject.DataTable->FindRow<FCPP_ProjectileStruct>(ProjectileObject.RowName, temp); // find damage done by projectile
	if (CurrentTarget && SeeDamage)
	{
		CurrentTarget->SetImminentProjectileDamage(CurrentTarget->GetImminentProjectileDamage() + SeeDamage->Damage);
	}

	if (CurrentTarget && CurrentTarget->GetImminentProjectileDamage() >= CurrentTarget->GetHealth()) // will mob die?
	{
		MobSpawner->RemoveMob(CurrentTarget);
	}
}

void ACPP_Tower::InitializeTower(FName NewRowName)
{
	FCPP_TowerStruct* TowerData = TowerDataTable->FindRow<FCPP_TowerStruct>(NewRowName, FString("Find TowerStruct"));
	if (TowerData) // "RowFound" option
	{
		Range = TowerData->Range;
		FirePeriod = TowerData->FirePeriod;
		ProjectileObject = TowerData->ProjectileType;
		Cost = TowerData->Cost;
		Skin->SetStaticMesh(TowerData->Decoration);
		UpgradeCost = TowerData->UpgradeCost;
		SellAmount = TowerData->SellAmount;
		UpgradedRowName = TowerData->UpgradedTowerRowName;
		ZOffsetShootProj = TowerData->ZOffsetToShootProjectile;

		if (ProjectileObject.RowName == "Stone1") // this portion is for if a rock solid tower is placed - it loads the skeletal mesh and animation
		{
			FString RockThrowerReference = "SkeletalMesh'/Game/TowerDefense/Animations/RockThrower/RockThrower.RockThrower'";
			USkeletalMesh* SkinRef = LoadObject<USkeletalMesh>(NULL, *RockThrowerReference);
			if (SkinRef)
			{
				RockMan->SetSkeletalMesh(SkinRef);

				RockMan->SetWorldLocation(RockMan->GetComponentLocation() + ZOffsetShootProj);

				FString ThrowRockAnimation1 = "AnimBlueprint'/Game/TowerDefense/Animations/RockThrower/AnimBP_RockThrower_StateMachine1.AnimBP_RockThrower_StateMachine1_C'";
				RockThrowAnim1 = LoadObject<UClass>(NULL, *ThrowRockAnimation1);
				if (RockThrowAnim1)
				{
					Idle = true;
					StartThrowing = false;
					RockMan->SetAnimInstanceClass(RockThrowAnim1);
				}
				ThrowRock->AttachToComponent(RockMan, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("RightHandIndex1"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SkinRef is invalid"));
			}
		}
		else if (ProjectileObject.RowName == "Stone2")
		{
			RockMan->SetRelativeLocation(ZOffsetShootProj);

			FString ThrowRockAnimation2 = "AnimBlueprint'/Game/TowerDefense/Animations/RockThrower/AnimBP_RockThrower_StateMachine2.AnimBP_RockThrower_StateMachine2_C'";
			RockThrowAnim2 = LoadObject<UClass>(NULL, *ThrowRockAnimation2);
			if (RockThrowAnim2)
			{
				Idle = true;
				StartThrowing = false;
				RockMan->SetAnimInstanceClass(RockThrowAnim2);
			}
		}
		else if (ProjectileObject.RowName == "Stone3")
		{
			RockMan->SetRelativeLocation(ZOffsetShootProj);

			FString ThrowRockAnimation3 = "AnimBlueprint'/Game/TowerDefense/Animations/RockThrower/AnimBP_RockThrower_StateMachine3.AnimBP_RockThrower_StateMachine3_C'";
			RockThrowAnim3 = LoadObject<UClass>(NULL, *ThrowRockAnimation3);
			if (RockThrowAnim3)
			{
				Idle = true;
				StartThrowing = false;
				RockMan->SetAnimInstanceClass(RockThrowAnim3);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Tower data isn't being initialized"));
	}
}


void ACPP_Tower::ExposeOnSpawn(ATowerSelector* TowerSelector)
{
	TS = TowerSelector;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Mob.h"
#include <Runtime/Engine/Classes/Engine/StreamableManager.h>
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "CPP_MobSpawner.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include <CPP_Tower.h>
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimSingleNodeInstance.h"

// Sets default values
ACPP_Mob::ACPP_Mob()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Skin = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skin"));

	RootComponent = Skin;
}

// Called when the game starts or when spawned
void ACPP_Mob::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors; // set MobSpawner - only have to "get all actors" once
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_MobSpawner::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		MobSpawner = Cast<ACPP_MobSpawner>(FoundActors[0]);
		PathSplineComponent = MobSpawner->GetSplineComponent();
		CurrDistance = PathSplineComponent->GetSplineLength();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find MobSpawner, MOB"));
	}

	FString DataTableLoad = "DataTable'/Game/TowerDefense/Blueprints/DataTables/DT_CPPMobDataTable.DT_CPPMobDataTable'";
	MobDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, *DataTableLoad));

	if (!MobDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Data table isn't working"));
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
	}
}

// Called every frame
void ACPP_Mob::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MobSpawner->GetMobArray().Find(this) == INDEX_NONE)
	{ // safe guard so that mob doesn't run down the path when it shouldn't
		TimeDead += DeltaTime;
		if (TimeDead >= 4.f && !GoingToDie)
		{
			if (DieAnim)
			{
				Skin->SetAnimInstanceClass(DieAnim);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Mob won't death animation"));
			}

			GoingToDie = true;
			MobSpawner->SetCurrency(DeathCurrencyAdd);
			UE_LOG(LogTemp, Warning, TEXT("Mob is running down path and not being killed."));
		}
	}

	if (!GoingToDie) {
		if (MobType.RowName == "Boss") {
			TArray<AActor*> FoundActors; // set MobSpawner - only have to "get all actors" once
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_Tower::StaticClass(), FoundActors);

			if (FoundActors.Num() > 0) {
				ACPP_Tower* TempTower;
				for (int i = 0; i < FoundActors.Num(); i++) {
					TempTower = Cast<ACPP_Tower>(FoundActors[i]);
					if (FVector::Dist(GetActorLocation(), TempTower->GetActorLocation()) < 2250)
					{ // Boss is close-ish to tower, animation starts to warm up
						AnimationSpeed = 40;

						BossCloseToTower = FVector::Dist(GetActorLocation(), TempTower->GetActorLocation()) < 2000;
						if (BossCloseToTower)
							break;
					}
				}
				if (BossCloseToTower)
				{
					AnimationSpeed = 50;

					CurrDistance -= DeltaTime * Speed;
				}
				else
				{
					AnimationSpeed = 0;

					CurrDistance -= DeltaTime * (Speed / SpeedMultiplier);
				}
			}
			else
			{ // no towers have been placed
				AnimationSpeed = 0;

				CurrDistance -= DeltaTime * (Speed / SpeedMultiplier);
			}
		}
		else if (MobType.RowName == "Wizard") {
			Speed += SpeedMultiplier * DeltaTime;
			AnimationSpeed = -50 * ((CurrDistance - PathSplineComponent->GetSplineLength()) / PathSplineComponent->GetSplineLength());
			CurrDistance -= Speed * DeltaTime;
		}
		else 
		{ // Mob is Soft or Runner
			CurrDistance -= Speed * DeltaTime;
		}

		FVector NewLocation = PathSplineComponent->GetLocationAtDistanceAlongSpline(CurrDistance, ESplineCoordinateSpace::World) + FVector(0.f, 0.f, 10.f);
		FRotator NewRotation = PathSplineComponent->GetRotationAtDistanceAlongSpline(CurrDistance, ESplineCoordinateSpace::World);
		float GarbageVals;
		float yaw;
		UKismetMathLibrary::BreakRotator(NewRotation, GarbageVals, GarbageVals, yaw);
		NewRotation = UKismetMathLibrary::MakeRotator(0.f, 0.f, yaw + 90);
		SetActorLocationAndRotation(NewLocation, NewRotation);

		if (CurrDistance <= 2)
		{ // the mob is sufficiently close to the end of the path
			MobSpawner->RemoveMob(this);
			Destroy();
			MobSpawner->DamageMainTower(Damage);
		}
	}
}


void ACPP_Mob::ApplyTowerDamage(float ProjDamage)
{
	if (!GoingToDie)
	{
		ImminentProjectileDamage -= ProjDamage;
		Health -= ProjDamage;
		if (Health <= 0)
		{
			
			if (MobSpawner->GetMobArray().Find(this) != -1)
			{ // mob is still in array
				MobSpawner->RemoveMob(this);
			}
			MobSpawner->SetCurrency(DeathCurrencyAdd);
			SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, 10.f)); // translate 10 units up
			GoingToDie = true;

			if (Skin)
			{
				Skin->SetAnimInstanceClass(DieAnim);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Mob won't death animation"));
			}
		}
	}
}


void ACPP_Mob::HitByCastleGuard()
{
	GoingToDie = true;

	if (HitByCGAnim)
	{
		Skin->SetAnimInstanceClass(HitByCGAnim);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mob won't CG animation"));
	}

	MobSpawner->SetCurrency(DeathCurrencyAdd);
}


void ACPP_Mob::InitializeMob()
{
	FCPP_MobStruct* MobData = MobDataTable->FindRow<FCPP_MobStruct>(MobType.RowName, FString("Find MobStruct"));
	if (MobData) // "RowFound" option
	{
		Health = MobData->Health;
		Speed = MobData->Speed;
		SpeedMultiplier = MobData->SpeedMultiplier;
		Damage = MobData->Damage;
		MobType = MobData->MobType;
		DeathCurrencyAdd = MobData->DeathWorth;
		Skin->SetSkeletalMesh(MobData->SkeletalMeshAsset);
		WalkAnim = MobData->WalkAnimation;
		DieAnim = MobData->DieAnimation;
		HitByCGAnim = MobData->HitByCGAnimation;

		if (WalkAnim)
		{
			Skin->SetAnimInstanceClass(WalkAnim);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mob data isn't being initialized"));
	}
}


void ACPP_Mob::ExposeOnSpawn(USplineComponent* SplineFromMS, FDataTableRowHandle TypeOfMob)
{
	PathSplineComponent = SplineFromMS;
	MobType = TypeOfMob;
}
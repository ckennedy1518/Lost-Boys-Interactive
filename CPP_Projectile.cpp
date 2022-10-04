// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Projectile.h"
#include "Kismet/KismetMathLibrary.h"
#include <Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h>
#include "NiagaraFunctionLibrary.h"

// Sets default values
ACPP_Projectile::ACPP_Projectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Skin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skin"));

	RootComponent = Skin;
}

// Called when the game starts or when spawned
void ACPP_Projectile::BeginPlay()
{
	Super::BeginPlay();

	FString DataTableLoad = "DataTable'/Game/TowerDefense/Blueprints/DataTables/DT_CPPProjectileData.DT_CPPProjectileData'";
	ProjectileDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, *DataTableLoad));

	if (!ProjectileDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Data table isn't working"));
		UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
	}
	
	InitializeProjectile();

	if (Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(Effect, Skin, TEXT("Effect"), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
}

// Called every frame
void ACPP_Projectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LifeSpanSet)
	{
		DeleteProjectile();
	}
	else
	{
		if (Target)
		{
			Direction = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), Target->GetActorLocation() + FVector(0.f, 0.f, 125.f));
			Direction *= Speed * DeltaTime;
			FRotator TempRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation() + FVector(0.f, 0.f, 125.f));
			SetActorLocationAndRotation(GetActorLocation() + Direction, UKismetMathLibrary::ComposeRotators(TempRot, FRotator(0.f, -65.f, 0)));
			// mesh as weird offset, only matters for blue towers (statue projectiles)

			if (ProjectileType.RowName == "Yellow3") {
				FVector TempVec = UKismetMathLibrary::Normal(Direction, .00001);
				TempVec *= FVector::Dist(GetActorLocation(), Target->GetActorLocation());
				LaserBeamSystem->SetVectorParameter(TEXT("BeamEnd"), TempVec);
			}

			if (FVector::Dist(GetActorLocation(), Target->GetActorLocation() + FVector(0.f, 0.f, 125.f)) <= 15)
			{ // less than 15 units away from target (number is kinda large bc projectiles move fast)
				DeleteProjectile();
				if (!DamageDealt)
				{
					Target->ApplyTowerDamage(Damage);
					DamageDealt = true;
				}
			}
		}
		else
		{
			SetActorLocation(GetActorLocation() + (UKismetMathLibrary::Normal(Direction, .0001) * (Speed * DeltaTime)));
			SetLifeSpan(1.f);
			LifeSpanSet = true;
		}
	}
}

void ACPP_Projectile::DeleteProjectile()
{
	if (ProjectileType.RowName == "Yellow3")
	{
		LaserBeamSystem->SetAsset(nullptr);
		Destroy();
	}
	else
	{
		Skin->SetStaticMesh(nullptr);

		if (GetLifeSpan() - .5 > 0)
		{
			if (!LifeSpanSet)
			{
				SetLifeSpan(GetLifeSpan() - 1);
				LifeSpanSet = true;
			}
		}
		else
		{
			Destroy();
		}
	}
}

void ACPP_Projectile::InitializeProjectile()
{ // ProjectileType was exposed on spawn
	FCPP_ProjectileStruct* ProjectileData = ProjectileDataTable->FindRow<FCPP_ProjectileStruct>(ProjectileType.RowName, FString("Projectile Data Table"));
	if (ProjectileData)
	{
		SetLifeSpan(ProjectileData->EffectLength);
		Speed = ProjectileData->Speed;
		Damage = ProjectileData->Damage;
		Skin->SetStaticMesh(ProjectileData->Decoration);

		// Load effect if needed
		if (ProjectileType.RowName == "Blue1")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/NS_FireLikeCascade_Blue.NS_FireLikeCascade_Blue'"));
		}
		else if (ProjectileType.RowName == "Orange1")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/NS_FireLikeCascade_Basic.NS_FireLikeCascade_Basic'"));
		}
		else if (ProjectileType.RowName == "Yellow1")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/N_YellowFire_System.N_YellowFire_System'"));
		}
		else if (ProjectileType.RowName == "Blue2")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/NS_FireLikeCascade_BlueBrighter.NS_FireLikeCascade_BlueBrighter'"));
		}
		else if (ProjectileType.RowName == "Orange2")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/NS_FireLikeCascade_Brighter.NS_FireLikeCascade_Brighter'"));
		}
		else if (ProjectileType.RowName == "Yellow2")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/N_MoreYellowFire_System.N_MoreYellowFire_System'"));
		}
		else if (ProjectileType.RowName == "Blue3")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/NS_FireLikeCascade_Blue_Bluer.NS_FireLikeCascade_Blue_Bluer'"));
		}
		else if (ProjectileType.RowName == "Orange3")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/NS_FireLikeCascade_Redder.NS_FireLikeCascade_Redder'"));
		}
		else if (ProjectileType.RowName == "Yellow3")
		{
			Effect = LoadObject<UNiagaraSystem>(nullptr, TEXT("NiagaraSystem'/Game/TowerDefense/Effects/Systems/N_YellowLaser_System.N_YellowLaser_System'"));
			LaserBeamSystem = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, GetActorLocation());
		}
		// rock man doesn't get effects
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile data isn't being initialized"));
	}
}

void ACPP_Projectile::ExposeOnSpawn(ACPP_Mob* CurrentTarget, FDataTableRowHandle ProjObject)
{
	Target = CurrentTarget;
	ProjectileType = ProjObject;
}
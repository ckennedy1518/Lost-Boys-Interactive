// Fill out your copyright notice in the Description page of Project Settings.


#include "STrackerBot.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "Components/SphereComponent.h"
#include <CoopGame/Public/SCharacter.h>
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(
	TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBotDrawing,
	TEXT("Draw Debug Lines for TrackerBot"),
	ECVF_Cheat);


// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCanEverAffectNavigation(false);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;

	RequiredDistanceToTarget = 100.f;

	ExplosionRadius = 350.f;

	ExplosionDamage = 60.f;

	SelfDamageInterval = 0.25f;

	CloseToOtherBotRadius = 1000;

	bReplicates = true;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetLocalRole() == ROLE_Authority)
	{
		NextPoint = GetNextPathPoint(); // initial move to
	}

	FTimerHandle TimerHandle_CheckPowerLevel;
	GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::NearbyAI, 1.f, true);

	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPoint = GetNextPathPoint();

			if (DebugTrackerBotDrawing)
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
		}
		else
		{
			// keep moving towards next target
			FVector ForceDirection = NextPoint - GetActorLocation();
			ForceDirection.GetSafeNormal();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing)
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.f, 0, 1.f);
		}

		if (DebugTrackerBotDrawing)
			DrawDebugSphere(GetWorld(), NextPoint, 20, 12, FColor::Yellow, false, 0.f, 1.f);
	}
}


FVector ASTrackerBot::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (!TestPawn || USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue; // only interested in enemies
		}

		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.f)
		{
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (NearestTargetDistance > Distance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	UNavigationPath* NavPath;
	if (BestTarget)
	{
		NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.f);

		if (NavPath)
		{
			if (NavPath->PathPoints.Num() > 1)
			{
				return NavPath->PathPoints[1]; // return next point in path
			}

			return GetActorLocation();
		}
	}

	return FVector();
}


void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// Explode on hitpoints == 0

	if (!MatInst)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	if (Health <= 0)
	{
		SelfDestruct();
	}
}


void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
		return;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GetLocalRole() == ROLE_Authority)
	{
		bExploded = true;

		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// Apply damage
		float DamageDealing = ExplosionDamage + ((float)PowerLevel * ExplosionDamage);
		UGameplayStatics::ApplyRadialDamage(this, DamageDealing, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugTrackerBotDrawing)
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.f, 0, 1.f);

		SetLifeSpan(2.f);
	}
}


void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this))
		{
			// overlapped with a player!
			if (GetLocalRole() == ROLE_Authority)
			{
				// start self destruction sequence
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.f);
			}

			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}


void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}


void ASTrackerBot::NearbyAI()
{
	PowerLevel = 0.f; // this will be reconfigured

	TArray<AActor*> AllTrackerBots;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASTrackerBot::StaticClass(), AllTrackerBots);

	for (AActor* TempBot : AllTrackerBots)
	{
		ASTrackerBot* TrackerBot = Cast<ASTrackerBot>(TempBot);

		if ((GetActorLocation() - TrackerBot->GetActorLocation()).Size() < CloseToOtherBotRadius)
		{
			if (PowerLevel < MaxPowerLevel)
				PowerLevel++;
		}
	}

	PowerLevel--; // don't count self

	if (!MatInst)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("PowerLevelAlpha", float(PowerLevel) / (float)MaxPowerLevel);
	}
}


void ASTrackerBot::RefreshPath()
{
	NextPoint = GetNextPathPoint();
}
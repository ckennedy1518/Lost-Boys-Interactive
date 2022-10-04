// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include <FPSGame/Public/FPSGameMode.h>
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "FPSGameMode.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateAbstractDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	GuardState = EAIState::Idle;
}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();
	
	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnPawnSeen);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnNoiseHeard);

	OriginalRotation = GetActorRotation();

	if (GuardStart)
	{
		SetActorLocation(GuardStart->GetActorLocation());
		bGoingTowardsEnd = true;
	}
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GuardStart && GuardEnd && GuardState == EAIState::Idle)
	{
		if (bGoingTowardsEnd)
		{ // update towards second point
			SetActorLocation(GetActorLocation() + (UKismetMathLibrary::Normal(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GuardEnd->GetActorLocation()).Vector()) * 250 * DeltaTime));
			FRotator TempRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GuardEnd->GetActorLocation());
			float GarbageVals;
			float yaw;
			UKismetMathLibrary::BreakRotator(TempRot, GarbageVals, GarbageVals, yaw);
			SetActorRotation(UKismetMathLibrary::MakeRotator(0.f, 0.f, yaw));

			if (FVector::Dist(GetActorLocation(), GuardEnd->GetActorLocation()) < 120.f) // account for z value
			{
				bGoingTowardsEnd = false;
			}
		}
		else
		{ // update towards first point
			SetActorLocation(GetActorLocation() + (UKismetMathLibrary::Normal(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GuardStart->GetActorLocation()).Vector()) * 250 * DeltaTime));
			FRotator TempRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GuardStart->GetActorLocation());
			float GarbageVals;
			float yaw;
			UKismetMathLibrary::BreakRotator(TempRot, GarbageVals, GarbageVals, yaw);
			SetActorRotation(UKismetMathLibrary::MakeRotator(0.f, 0.f, yaw));

			if (FVector::Dist(GetActorLocation(), GuardStart->GetActorLocation()) < 120.f)
			{
				bGoingTowardsEnd = true;
			}
		}
	}
}

void AFPSAIGuard::OnPawnSeen(APawn* SeenPawn)
{
	if (!SeenPawn)
	{
		return;
	}

	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.f, 12, FColor::Red, false, 10.f);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CompleteMission(SeenPawn, false);
	}

	SetGuardState(EAIState::Alerted);
}

void AFPSAIGuard::OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	if (GuardState == EAIState::Alerted)
		return;

	DrawDebugSphere(GetWorld(), Location, 32.f, 12, FColor::Green, false, 10.f);

	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();

	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = NewLookAt.Roll = 0.f;

	SetActorRotation(NewLookAt);

	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);

	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.f);

	SetGuardState(EAIState::Suspicious);
}

void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alerted)
		return;

	SetActorRotation(OriginalRotation);

	SetGuardState(EAIState::Idle);
}

void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	if (GuardState != NewState)
	{
		AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			if (GM->bSuccessfullyCompletedMission && NewState == EAIState::Alerted)
				return;

			GuardState = NewState;
			OnRep_GuardState();
		}
	}
}

void AFPSAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
	
}

void AFPSAIGuard::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSAIGuard, GuardState);
}
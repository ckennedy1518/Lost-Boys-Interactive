// Fill out your copyright notice in the Description page of Project Settings.


#include "MainTowerGuard.h"
#include <Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"

// Sets default values
AMainTowerGuard::AMainTowerGuard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CastleGuard = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CastleGuard"));
	PathSplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");

	RootComponent = CastleGuard;
	PathSplineComponent->SetupAttachment(CastleGuard);
}

// Called when the game starts or when spawned
void AMainTowerGuard::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors; // set MobSpawner - only have to "get all actors" once
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_MobSpawner::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		MobSpawner = Cast<ACPP_MobSpawner>(FoundActors[0]);
		PathSplineComponent = MobSpawner->GetSplineComponent();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find MobSpawner, MTG"));
	}

	FString StateMachineRef = "AnimBlueprint'/Game/TowerDefense/Animations/CastleGuard/MTG_StateMachine.MTG_StateMachine_C'";
	AnimationStateMachine = LoadObject<UClass>(NULL, *StateMachineRef);
	if (AnimationStateMachine)
	{
		CastleGuard->SetAnimInstanceClass(AnimationStateMachine);
	}
}

// Called every frame
void AMainTowerGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CanTick && !AboutToHitMob)
	{
		if (ToPath)
		{ // run to path first
			if (HitMob)
			{ // already hit mob, on way back
				Distance -= 525 * DeltaTime;

				FRotator TempRot = PathSplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
				float GarbageVals;
				float yaw;
				UKismetMathLibrary::BreakRotator(TempRot, GarbageVals, GarbageVals, yaw);
				SetActorLocationAndRotation(PathSplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World),
					UKismetMathLibrary::ComposeRotators(UKismetMathLibrary::MakeRotator(0.f, 0.f, yaw), FRotator(0.f, 90.f, 0.f)));

				if (FVector::Dist(GetActorLocation(), PathSplineComponent->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World)) < 3)
				{ // ran all the way back to the main tower
					SetActorLocation(FVector(11040.f, -10640.f, 0.f)); // hard coded location of the castle
					CanTick = false;
				}
			}
			else
			{ // running towards mobs, yet to hit
				Distance += 525 * DeltaTime;

				FRotator TempRot = PathSplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
				float GarbageVals;
				float yaw;
				UKismetMathLibrary::BreakRotator(TempRot, GarbageVals, GarbageVals, yaw);
				SetActorLocationAndRotation(PathSplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World),
					UKismetMathLibrary::ComposeRotators(UKismetMathLibrary::MakeRotator(0.f, 0.f, yaw), FRotator(0.f, -90.f, 0.f)));

				if (FVector::Dist(GetActorLocation(), PathSplineComponent->GetLocationAtDistanceAlongSpline(
					PathSplineComponent->GetSplineLength(), ESplineCoordinateSpace::World)) > 3)
				{ // hasn't reached beginning of the path (where mobs are spawned)
					TArray<AActor*> FoundActors; 
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACPP_Mob::StaticClass(), FoundActors);

					bool LoopThruMobs = true;
					for (AActor* CastToMob : FoundActors)
					{
						if (LoopThruMobs)
						{
							ACPP_Mob* Mob = Cast<ACPP_Mob>(CastToMob);
							if (Mob->GetMobType().RowName == "Softs")
							{
								if (FVector::Dist(Mob->GetActorLocation(), GetActorLocation()) < 710)
								{ // MTG is close to Soft

									LoopThruMobs = false;
									AboutToHitMob = true;
									SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, 20.f)); // lower for animation
									MobHit = Mob;
									if (MobSpawner)
										MobSpawner->RemoveMob(MobHit);
								}
							}
							else if (Mob->GetMobType().RowName == "Runners")
							{
								if (FVector::Dist(Mob->GetActorLocation(), GetActorLocation()) < 1000)
								{ // MTG is close to Runner

									LoopThruMobs = false;
									AboutToHitMob = true;
									SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, 20.f)); // lower for animation
									MobHit = Mob;
									if (MobSpawner)
										MobSpawner->RemoveMob(MobHit);
								}
							}
							else if (Mob->GetMobType().RowName == "Wizard")
							{
								if (FVector::Dist(Mob->GetActorLocation(), GetActorLocation()) < ((Mob->GetSpeed() * 2.1) + 100))
								{ // MTG is close to Wizard

									LoopThruMobs = false;
									AboutToHitMob = true;
									SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, 20.f)); // lower for animation
									MobHit = Mob;
									if (MobSpawner)
										MobSpawner->RemoveMob(MobHit);
								}
							}
							else if (Mob->GetMobType().RowName == "Boss")
							{
								if ((Mob->GetBossCloseToTower() && FVector::Dist(Mob->GetActorLocation(), GetActorLocation()) < 1025)
									|| FVector::Dist(Mob->GetActorLocation(), GetActorLocation()) < 700)
								{ // MTG is close to Boss

									LoopThruMobs = false;
									AboutToHitMob = true;
									SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, 20.f)); // lower for animation
									MobHit = Mob;
									if (MobSpawner)
										MobSpawner->RemoveMob(MobHit);
								}
							}
							else
							{
								UE_LOG(LogTemp, Warning, TEXT("Mob wasn't of type Soft Runner Wizard or Boss (MTG.cpp)"));
							}
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					SetActorLocation(FVector(11040.f, -10640.f, 0.f)); // hard coded location of the castle
					CanTick = false;
				}
			}
		}
		else
		{ // Not to path yet
			FVector InitialSplineLoc = PathSplineComponent->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
			FVector temp = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), InitialSplineLoc);
			temp = (temp * 700 * DeltaTime) + GetActorLocation();
			SetActorLocationAndRotation(temp, UKismetMathLibrary::ComposeRotators(
				UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), InitialSplineLoc), FRotator(0.f, -90.f, 0.f))); // update MTG to run towards path

			if (FVector::Dist(GetActorLocation(), InitialSplineLoc) < 10)
			{ // mob has reached path and can run down it now
				FRotator TempRot = PathSplineComponent->GetRotationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::Local);
				float GarbageVals;
				float yaw;
				UKismetMathLibrary::BreakRotator(TempRot, GarbageVals, GarbageVals, yaw);
				SetActorLocationAndRotation(InitialSplineLoc, UKismetMathLibrary::MakeRotator(0.f, 0.f, yaw)); // set MTG on spline
				ToPath = true;
			}
		}
	}
}

void AMainTowerGuard::AnimationSequenceCompleted()
{
	AboutToHitMob = false;
	HitMob = true;
	Distance += 450;
}
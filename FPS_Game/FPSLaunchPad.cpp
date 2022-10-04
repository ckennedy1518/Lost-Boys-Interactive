// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSLaunchPad.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include <FPSGame/Public/FPSCharacter.h>
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFPSLaunchPad::AFPSLaunchPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	RootComponent = Collision;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(Collision);

	LaunchStrength = 1500.f;
	LaunchPitchAngle = 35.f;
}

// Called when the game starts or when spawned
void AFPSLaunchPad::BeginPlay()
{
	Super::BeginPlay();
	
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AFPSLaunchPad::OverlapLaunch);
}

// Called every frame
void AFPSLaunchPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFPSLaunchPad::OverlapLaunch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FRotator LaunchRotation = GetActorRotation();
	LaunchRotation.Pitch += LaunchPitchAngle;
	FVector LaunchVelocity = LaunchRotation.Vector() * LaunchStrength;

	if (OtherActor)
	{ // launching character
		AFPSCharacter* MyCharacter = Cast<AFPSCharacter>(OtherActor);
		if (MyCharacter)
		{
			MyCharacter->LaunchCharacter(LaunchVelocity, true, true);

			if (LaunchFX)
			{ // spawn the effects
				UGameplayStatics::SpawnEmitterAtLocation(this, LaunchFX, GetActorLocation());
			}
		}
		else if (OtherComp && OtherComp->IsSimulatingPhysics())
		{
			OtherComp->AddImpulse(LaunchVelocity, NAME_None, true);

			if (LaunchFX)
			{ // spawn the effects
				UGameplayStatics::SpawnEmitterAtLocation(this, LaunchFX, GetActorLocation());
			}
		}
	}
}
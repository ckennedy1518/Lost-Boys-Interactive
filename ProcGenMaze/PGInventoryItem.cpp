// Fill out your copyright notice in the Description page of Project Settings.


#include "PGInventoryItem.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PGInventoryManager.h"

// Sets default values
APGInventoryItem::APGInventoryItem()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionArea"));
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));

	RootComponent = SphereComp;
	MeshComp->SetupAttachment(SphereComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Called when the game starts or when spawned
void APGInventoryItem::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> FoundInventoryManager;
	UGameplayStatics::GetAllActorsOfClass(this, APGInventoryManager::StaticClass(), FoundInventoryManager);

	if (FoundInventoryManager.Num() > 0)
	{
		InventoryManager = Cast<APGInventoryManager>(FoundInventoryManager[0]);
	}
}


void APGInventoryItem::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	InventoryManager->AddItem(BPItem);

	UpdateUI();

	Destroy(); // Item is now in our inventory, not on the ground

	// pickup effect and sound would go here
}
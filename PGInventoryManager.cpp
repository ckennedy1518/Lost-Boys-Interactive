// Fill out your copyright notice in the Description page of Project Settings.


#include "PGInventoryManager.h"

// Sets default values
APGInventoryManager::APGInventoryManager()
{

}

// Called when the game starts or when spawned
void APGInventoryManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// sorts items so that the array passed in is rearranged to most rare at the front to least rare in the back
TArray<TSubclassOf<APGInventoryItem>> APGInventoryManager::SortItemsOnRarity(TArray<TSubclassOf<APGInventoryItem>> ItemArray)
{
	TArray<TSubclassOf<APGInventoryItem>> ToRet = TArray<TSubclassOf<APGInventoryItem>>();
	int32 RareIndex = 0;
	int32 UncommonIndex = 0;

	for (TSubclassOf<APGInventoryItem> Item : ItemArray)
	{
		if (Item->GetDefaultObject<APGInventoryItem>()->GetItemRarity() == EItemRarity::UltraRare)
		{
			if (ToRet.Num() == 0)
			{
				ToRet.Add(Item);
				RareIndex++;
				UncommonIndex++;
			}
			else
			{
				ToRet.Insert(Item, 0);
				RareIndex++;
				UncommonIndex++;
			}
		}
		else if (Item->GetDefaultObject<APGInventoryItem>()->GetItemRarity() == EItemRarity::Rare)
		{
			if (ToRet.Num() > RareIndex)
			{
				ToRet.Insert(Item, RareIndex);
				UncommonIndex++;
			}
			else
			{
				ToRet.Add(Item);
				UncommonIndex++;
			}
		}
		else if (Item->GetDefaultObject<APGInventoryItem>()->GetItemRarity() == EItemRarity::Uncommon)
		{
			if (ToRet.Num() > UncommonIndex)
			{
				ToRet.Insert(Item, UncommonIndex);
			}
			else
			{
				ToRet.Add(Item);
			}
		}
		else
		{ // common item
			ToRet.Add(Item); // attach to end of array
		}
	}

	return ToRet;
}


void APGInventoryManager::AddItem(TSubclassOf<APGInventoryItem> ItemToAdd)
{
	Inventory.Add(ItemToAdd);

	TArray<TSubclassOf<APGInventoryItem>> Temp = TArray<TSubclassOf<APGInventoryItem>>();

	Temp = SortItemsOnRarity(GetUsefulItems());
	Temp.Append(SortItemsOnRarity(GetCollectableItems()));
	Temp.Append(SortItemsOnRarity(GetPetItems()));

	Inventory = Temp;
}


TArray<TSubclassOf<APGInventoryItem>> APGInventoryManager::GetUsefulItems()
{
	TArray<TSubclassOf<APGInventoryItem>> ToRet = TArray<TSubclassOf<APGInventoryItem>>();

	for (TSubclassOf<APGInventoryItem> Item : Inventory)
	{
		if (Item->GetDefaultObject<APGInventoryItem>()->GetItemType() == EInventoryType::Useful)
		{
			ToRet.Add(Item);
		}
	}
	return ToRet;
}


TArray<TSubclassOf<APGInventoryItem>> APGInventoryManager::GetCollectableItems()
{
	TArray<TSubclassOf<APGInventoryItem>> ToRet = TArray<TSubclassOf<APGInventoryItem>>();

	for (TSubclassOf<APGInventoryItem> Item : Inventory)
	{
		if (Item->GetDefaultObject<APGInventoryItem>()->GetItemType() == EInventoryType::Collectable)
		{
			ToRet.Add(Item);
		}
	}
	return ToRet;
}


TArray<TSubclassOf<APGInventoryItem>> APGInventoryManager::GetPetItems()
{
	TArray<TSubclassOf<APGInventoryItem>> ToRet = TArray<TSubclassOf<APGInventoryItem>>();

	for (TSubclassOf<APGInventoryItem> Item : Inventory)
	{
		if (Item->GetDefaultObject<APGInventoryItem>()->GetItemType() == EInventoryType::Pet)
		{
			ToRet.Add(Item);
		}
	}
	return ToRet;
}
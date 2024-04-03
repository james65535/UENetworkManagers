// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "GameFramework/Character.h"

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ACharacter>(GetOwner());
}

void UInventoryComponent::PostReplication(const TArray<FInventoryItem>& InventoryItems)
{
	Items = InventoryItems;

	OnInventoryItemUpdate.Broadcast(OwningCharacter, false);
}

void UInventoryComponent::AddItem(const FInventoryItem& InventoryItem)
{
	ensureAlwaysMsgf(!IsRunningClientOnly(), TEXT("Client attempted to add inventory"));

	/** Check if we already have the item */
	const int8 FoundItemIndex = Items.IndexOfByPredicate(
		[InventoryItem](const FInventoryItem& InvItem)
			{ return InvItem.PID == InventoryItem.PID; });
	
	/** Increment Item if it is already in inventory, otherwise add it it */
	FPrimaryAssetId PID;
	if (FoundItemIndex != INDEX_NONE && Items[FoundItemIndex].Quantity >= 1)
	{
		PID = Items[FoundItemIndex].PID;
		Items[FoundItemIndex].Quantity += InventoryItem.Quantity;
	}
	else
	{
		Items.Add(InventoryItem);
		PID = InventoryItem.PID;
	}

	UE_LOG(LogTemp, Warning, TEXT("Inventory Component added Item with PID: %s"), *PID.ToString());
			
	OnInventoryItemUpdate.Broadcast(OwningCharacter, false);
}

void UInventoryComponent::RemoveItem(const FInventoryItem& InventoryItem)
{
	ensureAlwaysMsgf(!IsRunningClientOnly(),
		TEXT("Client attempted to remove from inventory"));

	/** Check if item exists already in inventory */
	const int32 ItemIndex = Items.IndexOfByPredicate(
		[InventoryItem](const FInventoryItem& InvItem)
			{ return InvItem.PID == InventoryItem.PID; });

	/** Early out if there is nothing to remove */
	if (ItemIndex == INDEX_NONE)
	{ return; }

	/** reduce quantity and remove from inventory if quantity is less than 1 */
	Items[ItemIndex].Quantity--;
	if (Items[ItemIndex].Quantity < 1)
	{
		/** Item quantity is zero so remove from inventory */
		Items.RemoveAt(ItemIndex);
		Items.Shrink();
	}
	
	OnInventoryItemUpdate.Broadcast(OwningCharacter, true);
}

void UInventoryComponent::GetItems(TArray<FInventoryItem>& InventoryItems)
{
	InventoryItems = Items;
}

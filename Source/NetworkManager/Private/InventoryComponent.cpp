// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "InventoryItemBaseAsset.h"
#include "GameFramework/Character.h"

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ACharacter>(GetOwner());
}

TArray<uint8> UInventoryComponent::Encode()
{
	ensureAlwaysMsgf(!IsRunningClientOnly(), TEXT("Client attempted to encode inventory for replication"));

	TArray<uint8> Payload;
	FMemoryWriter Archive(Payload);
	Archive.Serialize(&Items, Items.GetAllocatedSize());
	return Payload;
}

void UInventoryComponent::Decode(const TArray<uint8>& Payload)
{
	FMemoryReader Archive(Payload);
	Archive.Serialize(&Items, Items.GetAllocatedSize());
	for (FInventoryItem& ItemInfo : Items)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Decoded Item: %s, ID: %s Quantity: %i"),
			*ItemInfo.Item->GetName(),
			*ItemInfo.PID.ToString(),
			ItemInfo.Quantity);
	}
}

void UInventoryComponent::PostReplication(const TArray<uint8>& Payload)
{
	Decode(Payload);
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
	int8 Quantity = 0u;
	if (FoundItemIndex != INDEX_NONE)
	{
		Quantity = Items[FoundItemIndex].Quantity++;
		PID = Items[FoundItemIndex].PID;
	}
	else
	{
		Items.Add(InventoryItem);
		Quantity = InventoryItem.Quantity;
		PID = InventoryItem.PID;
	}

	UE_LOG(LogTemp, Warning, TEXT("Added Item with PID: %s"), *PID.ToString());
			
	OnInventoryItemUpdate.Broadcast(OwningCharacter, PID, Quantity, false);
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

		OnInventoryItemUpdate.Broadcast(OwningCharacter, InventoryItem.PID, 0u, true);
	}
	else
	{
		OnInventoryItemUpdate.Broadcast(
			OwningCharacter,
			Items[ItemIndex].PID,
			Items[ItemIndex].Quantity,
			false);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryNetManager.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "NetworkManager/PlayerCharacter.h"

// Sets default values
AInventoryNetManager::AInventoryNetManager()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;
}

void AInventoryNetManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, InventoryRegistry);
}

void AInventoryNetManager::RegisterCharacter(APlayerCharacter* InCharacter)
{
	if (UInventoryComponent* InventoryComponent = InCharacter->GetInventoryComponent())
	{
		InventoryComponent->OnInventoryItemUpdate.AddUObject(this, &ThisClass::InventoryUpdateDelegate);
		RegisterInventory(InventoryComponent);
	}
}

void AInventoryNetManager::RegisterInventory(UInventoryComponent* InventoryComponent)
{
	if (!IsValid(InventoryComponent))
	{ return; }

	TArray<FInventoryItem> InventoryList;
	InventoryComponent->GetItems(InventoryList);
	FItemEntry InventoryAssetEntry(InventoryComponent,InventoryList);
	InventoryRegistry.MarkItemDirty(InventoryRegistry.Items.Add_GetRef(InventoryAssetEntry));
}

void AInventoryNetManager::UpdateInventory(UInventoryComponent* InventoryComponent)
{
	const uint8 RegistryIndex = InventoryRegistry.Items.IndexOfByPredicate(
		[InventoryComponent](const FItemEntry& InItem)
			{ return InItem.OwnerActor == InventoryComponent->GetOwner(); });

	TArray<FInventoryItem> InventoryList;
	InventoryComponent->GetItems(InventoryList);
	InventoryRegistry.Items[RegistryIndex].InventoryItems = InventoryList;
	InventoryRegistry.MarkItemDirty(InventoryRegistry.Items[RegistryIndex]);
}

void AInventoryNetManager::InventoryUpdateDelegate(const ACharacter* InCharacterOwner, const bool bDeletion)
{
	if (UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(
		InCharacterOwner->GetComponentByClass(UInventoryComponent::StaticClass())))
	{
		if (bDeletion)
		{
			// TODO implement deletion
		}
		else
		{ UpdateInventory(InventoryComponent); }
		
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Network Manager tried to update a non-existant inventory component of actor: %s"),
			*InCharacterOwner->GetName());
	}
}

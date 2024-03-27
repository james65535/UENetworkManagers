// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryNetManager.h"

#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AInventoryNetManager::AInventoryNetManager()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AInventoryNetManager::BeginPlay()
{
	Super::BeginPlay();

	for (ACharacter* Character : TActorRange<ACharacter>(GetWorld()))
	{
		if (UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(
			Character->GetComponentByClass(UInventoryComponent::StaticClass())))
		{
			RegisterInventory(InventoryComponent, InventoryComponent->Encode());
			InventoryComponent->OnInventoryItemUpdate.AddUObject(this, &ThisClass::InventoryUpdateDelegate);	
			UE_LOG(LogTemp, Warning, TEXT("Network manager registered an inventory component of actor: %s"),
				*Character->GetFullName());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Inventory Network Manager Running"));
}

void AInventoryNetManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, ItemRegistry);
}

void AInventoryNetManager::RegisterInventory(UInventoryComponent* InventoryComponent, const TArray<uint8>& Payload)
{
	// TODO Update as follows:
	// Payload: Inventory Item, quantity...
	if (IsValid(InventoryComponent))
	{
		FItemEntry InventoryAssetEntry(InventoryComponent, Payload);
		ItemRegistry.MarkItemDirty(ItemRegistry.Items.Add_GetRef(InventoryAssetEntry));
		UE_LOG(LogTemp, Warning, TEXT("Network Manager register inventory"));
	}
}

void AInventoryNetManager::UpdateInventory(UInventoryComponent* InventoryComponent, const TArray<uint8>& Payload)
{
	FItemEntry* FoundEntry = ItemRegistry.Items.FindByPredicate([InventoryComponent](const FItemEntry& InItem)
		{ return InItem.OwnerActor == InventoryComponent->GetOwner(); });
	FoundEntry->Data = Payload;
	ItemRegistry.MarkItemDirty(*FoundEntry);

	UE_LOG(LogTemp, Warning, TEXT("Network Manager updated inventory"));
}

void AInventoryNetManager::InventoryUpdateDelegate(ACharacter* InCharacterOwner, const FPrimaryAssetId& PID, uint8 InQuantity, bool bIsRemoved)
{
	if (UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(
		InCharacterOwner->GetComponentByClass(UInventoryComponent::StaticClass())))
	{
		UpdateInventory(InventoryComponent, InventoryComponent->Encode());
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Network Manager tried to update a non-existant inventory component of actor: %s"),
			*InCharacterOwner->GetName());
	}
}

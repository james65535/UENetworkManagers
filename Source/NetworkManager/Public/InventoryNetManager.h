// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryComponent.h"
#include "NetworkManager/PlayerCharacter.h"
#include "InventoryNetManager.generated.h"

class APlayerCharacter;

/**
 * Entry per actor owner which contains information on item in inventory
 */
USTRUCT()
struct FItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TArray<FInventoryItem> InventoryItems;

	FItemEntry()
	{
		OwnerActor = nullptr;
	}

	FItemEntry(const UInventoryComponent* InventoryComponent, const TArray<FInventoryItem>& InInventoryItems)
	{
		if (ensureAlways(InventoryComponent))
		{
			OwnerActor = InventoryComponent->GetOwner();
			InventoryItems = InInventoryItems;
		}
	}
};

USTRUCT()
struct FItemContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FItemEntry> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FItemEntry, FItemContainer>(
			Items,
			DeltaParms,
			*this);
	}

	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
	{
		for  (const int32 ChangedArrayIndex : ChangedIndices)
		{
			if (Items.IsValidIndex(ChangedArrayIndex))
			{
				if (const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Items[ChangedArrayIndex].OwnerActor))
				{
					if (UInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent())
					{ InventoryComponent->PostReplication(Items[ChangedArrayIndex].InventoryItems); }
				}
			}
		}
	}
};

template<>
struct TStructOpsTypeTraits<FItemContainer> : public TStructOpsTypeTraitsBase2<FItemContainer>
{
	enum
	{ WithNetDeltaSerializer = true };
};

UCLASS()
class NETWORKMANAGER_API AInventoryNetManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInventoryNetManager();

	UFUNCTION(BlueprintCallable)
	void RegisterCharacter(APlayerCharacter* InCharacter);
	
	/**
	 * @brief Adds an element to our fast array constructing a ItemsCollection that gets added to the array
	 */
	UFUNCTION(BlueprintCallable)
	void RegisterInventory(UInventoryComponent* InventoryComponent);

	/**
	 * @brief Updates the data of an element in our fast array. For that, we find the owner actor within the array and update the data on the found entry 
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateInventory(UInventoryComponent* InventoryComponent);

	/**
	 * @brief Function delegate to trigger from InventoryComponent Updates
	 */
	void InventoryUpdateDelegate(const ACharacter* InCharacterOwner, const bool bDeletion);

protected:

	/** Class Overrides */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Item Fast Array Replication Lifecycle Methods */
	UPROPERTY(Replicated)
	FItemContainer InventoryRegistry;

};

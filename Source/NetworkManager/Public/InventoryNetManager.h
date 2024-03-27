// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryComponent.h"
#include "InventoryNetManager.generated.h"

/**
 * Entry per actor owner which contains information on item in inventory
 */
USTRUCT()
struct FItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	// TODO Update to single entry on owner
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;

	TArray<uint8> Data;

	FItemEntry();

	FItemEntry(
		const UInventoryComponent* InventoryComponent,
		const TArray<uint8>& Payload);

	void PostReplicatedChange(const struct FItemContainer& InArraySerializer);
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
};

template<>
struct TStructOpsTypeTraits<FItemContainer> : public TStructOpsTypeTraitsBase2<FItemContainer>
{
	enum
	{ WithNetDeltaSerializer = true };
};


inline FItemEntry::FItemEntry()
{ OwnerActor = nullptr; }

inline FItemEntry::FItemEntry(const UInventoryComponent* InventoryComponent, const TArray<uint8>& Payload)
{
	if (ensureAlways(InventoryComponent))
	{
		OwnerActor = InventoryComponent->GetOwner();
		Data = Payload;
	}
}

inline void FItemEntry::PostReplicatedChange(const struct FItemContainer& InArraySerializer)
{
	if (OwnerActor.Get())
	{
		if (UInventoryComponent* InventoryComponent = OwnerActor.Get()->FindComponentByClass<UInventoryComponent>())
		{ InventoryComponent->PostReplication(Data); }
	}
}

UCLASS()
class NETWORKMANAGER_API AInventoryNetManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInventoryNetManager();

	/**
	 * @brief Adds an element to our fast array constructing a ItemsCollection that gets added to the array
	 */
	UFUNCTION(BlueprintCallable)
	void RegisterInventory(UInventoryComponent* InventoryComponent, const TArray<uint8>& Payload);

	/**
	 * @brief Updates the data of an element in our fast array. For that, we find the owner actor within the array and update the data on the found entry 
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateInventory(UInventoryComponent* InventoryComponent, const TArray<uint8>& Payload);

	/**
	 * @brief Function delegate to trigger from InventoryComponent Updates
	 */
	void InventoryUpdateDelegate(ACharacter* InCharacterOwner, const FPrimaryAssetId& PID, uint8 InQuantity, bool bIsRemoved);

protected:

	/** Class Overrides */
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Item Fast Array Replication Lifecycle Methods */
	UPROPERTY(Replicated)
	FItemContainer ItemRegistry;

};

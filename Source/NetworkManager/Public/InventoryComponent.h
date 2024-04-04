// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemUpdate, const ACharacter*, const bool);

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FPrimaryAssetId PID;
	UPROPERTY(BlueprintReadOnly)
	uint8 Quantity;

	FInventoryItem()
	{
		PID = FPrimaryAssetId();
		Quantity = 0u;
	}

	FInventoryItem(const FPrimaryAssetId& InPID, const uint8 InQuantity)
	{
		PID = InPID;
		Quantity = InQuantity;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETWORKMANAGER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	/**
	 * @param InventoryItems Collection of Inventory Items to Update After Replication
	 */
	void PostReplication(const TArray<FInventoryItem>& InventoryItems);

	void AddItem(const FInventoryItem& InventoryItem);
	void RemoveItem(const FInventoryItem& InventoryItem);
	/**
	 * @result bool Success
	 */
	bool UseCurrentItem();
	void GetItems(TArray<FInventoryItem>& InventoryItems) const;

	FOnInventoryItemUpdate OnInventoryItemUpdate;

protected:

	virtual void BeginPlay() override;

	UPROPERTY()
	ACharacter* OwningCharacter;

	UPROPERTY()
	TArray<FInventoryItem> Items;
	
};

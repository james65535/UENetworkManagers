// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UInventoryItemBaseAsset;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnInventoryItemUpdate, ACharacter*, const FPrimaryAssetId&, uint8, bool);

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()
	
	UPROPERTY()
	UInventoryItemBaseAsset* Item;

	FPrimaryAssetId PID;
	uint8 Quantity;

	FInventoryItem()
	{
		Item = nullptr;
		PID = FPrimaryAssetId();
		Quantity = 0u;
	}

	FInventoryItem(UInventoryItemBaseAsset* InItem, const FPrimaryAssetId& InPID, const uint8 InQuantity)
	{
		Item = InItem;
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
	 * @param Payload Buffer Array to Decode
	 */
	void PostReplication(const TArray<uint8>& Payload);
	TArray<uint8> Encode();
	void Decode(const TArray<uint8>& Payload);

	void AddItem(const FInventoryItem& InventoryItem);
	void RemoveItem(const FInventoryItem& InventoryItem);

	FOnInventoryItemUpdate OnInventoryItemUpdate;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	ACharacter* OwningCharacter;

	UPROPERTY()
	TArray<FInventoryItem> Items;
	
};

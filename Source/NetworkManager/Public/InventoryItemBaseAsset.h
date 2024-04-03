// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryItemBaseAsset.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKMANAGER_API UInventoryItemBaseAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory")
	FName InventoryItemName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory")
	FText InventoryItemDescription;

	/** Image to be used for visual depiction in Inventory */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory")
	UTexture2D* ItemInventoryImage;
	
	// UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory")
	// TSubclassOf<UInventoryItemComponent> ItemClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory")
	UStaticMesh* Mesh;

	/** -1 Signifies unlimited */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory")
	int Quantity = -1;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId("InventoryBaseAsset", GetFName()); }
	
};

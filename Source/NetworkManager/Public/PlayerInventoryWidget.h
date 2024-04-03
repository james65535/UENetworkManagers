// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryComponent.h"
#include "PlayerInventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKMANAGER_API UPlayerInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateDisplayedInventory(const TArray<FInventoryItem>& InInventoryList);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/HUD.h"
#include "PlayerInventoryWidget.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKMANAGER_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void DisplayInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateDisplayedInventory();

	/**
	 * @brief Function delegate to trigger from InventoryComponent Updates
	 */
	void OnInventoryUpdate(const ACharacter* InCharacterOwner, const bool bDeletion);

private:

	/** Inventory Menu */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (AllowPrivateAccess), Category = "UI")
	TSubclassOf<UPlayerInventoryWidget> InventoryWidgetClass;
	UPROPERTY()
	UPlayerInventoryWidget* InventoryWidget; 

protected:

	virtual void BeginPlay() override;
	
};

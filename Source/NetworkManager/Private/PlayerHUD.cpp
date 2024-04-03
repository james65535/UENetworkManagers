// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"

#include "InventoryComponent.h"
#include "NetworkManager/PlayerCharacter.h"


void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	/** Add a delegate to get notified of player inventory updates */
	if (const APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerController()->GetCharacter()))
	{
		Character->GetInventoryComponent()->OnInventoryItemUpdate.AddUObject(this, &ThisClass::OnInventoryUpdate);
	}

	DisplayInventory();
}

void APlayerHUD::DisplayInventory()
{
	InventoryWidget = CreateWidget<UPlayerInventoryWidget>(GetOwningPlayerController(), InventoryWidgetClass);
	if (IsValid(InventoryWidget))
	{ InventoryWidget->AddToPlayerScreen(); }
}

void APlayerHUD::UpdateDisplayedInventory()
{
	if (!IsValid(InventoryWidget))
	{ return; }
	
	if (const APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningPlayerController()->GetCharacter()))
	{
		TArray<FInventoryItem> InventoryItems;
		Character->GetInventoryComponent()->GetItems(InventoryItems);
		InventoryWidget->UpdateDisplayedInventory(InventoryItems);
		UE_LOG(LogTemp, Warning, TEXT("Found inv items: %i"), InventoryItems.Num());
	}
}

void APlayerHUD::OnInventoryUpdate(const ACharacter* InCharacterOwner, const bool bDeletion)
{
	if (InCharacterOwner == GetOwningPlayerController()->GetCharacter())
	{ UpdateDisplayedInventory(); }
}

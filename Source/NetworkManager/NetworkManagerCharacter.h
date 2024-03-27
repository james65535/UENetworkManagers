// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InventoryComponent.h"
#include "NetworkManagerCharacter.generated.h"


UCLASS(config=Game)
class ANetworkManagerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;


public:
	
	ANetworkManagerCharacter();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// UFUNCTION(Category = "Inventory")
	// void AddInventoryItem();
	// UFUNCTION(Category = "Inventory")
	// void RemoveInventoryItem();

protected:

	// To add mapping context
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	UInventoryComponent* InventoryComponent;		
	
	// UFUNCTION(Server, Reliable, Category = "Inventory")
	// void S_AddInventoryItem();
	// UFUNCTION(Server, Reliable, Category = "Inventory")
	// void S_RemoveInventoryItem();
	
};


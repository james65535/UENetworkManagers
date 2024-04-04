// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "MyPlayerController.generated.h"

class APlayerCharacter;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class NETWORKMANAGER_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

private:

	UPROPERTY()
	APlayerCharacter* PlayerCharacter;
	
	/** Enhanced Input Setup */
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	/** Character Movement Requests */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RequestMove(const FInputActionValue& ActionValue);
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RequestLook(const FInputActionValue& ActionValue);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestUseInventoryItem();

protected:
	/** Class Overrides */
	virtual void BeginPlay() override;
	virtual void BeginPlayingState() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AddInventoryItemAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* UseInventoryItemAction;

	UFUNCTION(Server, Reliable)
	void S_RequestUseInventoryItem();
	
};

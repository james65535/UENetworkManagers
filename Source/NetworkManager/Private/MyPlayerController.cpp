// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "NetworkManager/PlayerCharacter.h"


void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsRunningDedicatedServer())
	{
		/* Set Enhanced Input Mapping Context to Game Context */
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AMyPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	/** Get Input Component as Enhanced Input Component */
	if (UEnhancedInputComponent* EIPlayerComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		/** Moving */
		EIPlayerComponent->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::RequestMove);

		/** Looking */
		EIPlayerComponent->BindAction(
			LookAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::RequestLook);

		/** Inventory Control */
		EIPlayerComponent->BindAction(
			AddInventoryItemAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::RequestAddInventoryItem);

		EIPlayerComponent->BindAction(
			RemoveInventoryItemAction,
			ETriggerEvent::Triggered,
			this,
			&ThisClass::RequestRemoveInventoryItem);
	}
}

void AMyPlayerController::RequestMove(const FInputActionValue& ActionValue)
{
	
	ensureAlwaysMsgf(IsValid(PlayerCharacter),
		TEXT("Controller has an invalid Player Character upon move"));
	
	/** input is a Vector2D */
	const FVector2D MovementVector = ActionValue.Get<FVector2D>();
	
	/** find out which way is forward */
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	/** get forward vector */
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	/** get right vector */
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	/** add movement */
	PlayerCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
	PlayerCharacter->AddMovementInput(RightDirection, MovementVector.X);
}

void AMyPlayerController::RequestLook(const FInputActionValue& ActionValue)
{
	ensureAlwaysMsgf(IsValid(PlayerCharacter),
		TEXT("Controller has an invalid Player Character upon look"));
	
	// input is a Vector2D
	const FVector2D LookAxisVector = ActionValue.Get<FVector2D>();

	AddYawInput(LookAxisVector.X);
	AddPitchInput(LookAxisVector.Y);
}

void AMyPlayerController::RequestAddInventoryItem()
{
	ensureAlwaysMsgf(IsValid(PlayerCharacter),
		TEXT("Controller has an invalid Player Character upon add item"));

	// PlayerCharacter->AddInventoryItem();
}

void AMyPlayerController::RequestRemoveInventoryItem()
{
	ensureAlwaysMsgf(IsValid(PlayerCharacter),
		TEXT("Controller has an invalid Player Character upon remove item"));
	
	// PlayerCharacter->RemoveInventoryItem();
}



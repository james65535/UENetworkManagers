// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkManagerGameMode.h"

#include "InventoryNetManager.h"
#include "UObject/ConstructorHelpers.h"

ANetworkManagerGameMode::ANetworkManagerGameMode()
{
	// set default pawn class to our Blueprinted character
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	// if (PlayerPawnBPClass.Class != NULL)
	// {
	// 	DefaultPawnClass = PlayerPawnBPClass.Class;
	// }
}

void ANetworkManagerGameMode::BeginPlay()
{
	Super::BeginPlay();

	/** Startup Inventory Network Manager
	 * Manager has bNetLoadOnClients set to true */
	GetWorld()->SpawnActor<AInventoryNetManager>();
}

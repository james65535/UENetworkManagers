// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetworkManagerGameMode.generated.h"

UCLASS(minimalapi)
class ANetworkManagerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetworkManagerGameMode();

protected:
	virtual void BeginPlay() override;
};




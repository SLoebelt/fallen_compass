// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FCGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFallenCompassGameMode, Log, All);

/**
 * Simple GameMode stub for early prototypes.
 * Keeps template defaults until FC-specific pawn/controller are ready.
 */
UCLASS()
class FC_API AFCGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFCGameMode();

    virtual void BeginPlay() override;
};
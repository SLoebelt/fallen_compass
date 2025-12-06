// Copyright Slomotion Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FCGameMode.generated.h"

class AFCFirstPersonCharacter;

DECLARE_LOG_CATEGORY_EXTERN(LogFallenCompassOldGameMode, Log, All);

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

	/**
	 * Override pawn spawning to handle special cases like Camp where explorers
	 * are pre-placed in the level and should not be spawned by GameMode.
	 */
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#include "FCGameMode.h"
#include "FCFirstPersonCharacter.h"
#include "FCPlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Core/FCLevelTransitionManager.h"

DEFINE_LOG_CATEGORY(LogFallenCompassGameMode);

AFCGameMode::AFCGameMode()
{
    // Use AFCFirstPersonCharacter for Office level (Task 3.1)
    DefaultPawnClass = AFCFirstPersonCharacter::StaticClass();
    PlayerControllerClass = AFCPlayerController::StaticClass();

    UE_LOG(LogFallenCompassGameMode, Log, TEXT("AFCGameMode configured with AFCFirstPersonCharacter and AFCPlayerController."));
}

void AFCGameMode::BeginPlay()
{
    Super::BeginPlay();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("AFCPlayerController CONSTRUCTOR CALLED"));
	}

    const UClass* PawnClass = DefaultPawnClass ? DefaultPawnClass.Get() : GetDefaultPawnClassForController(nullptr);
    const FString PawnName = PawnClass ? PawnClass->GetName() : TEXT("None");
    const FString ControllerName = PlayerControllerClass ? PlayerControllerClass->GetName() : TEXT("None");
    const FString MapName = GetWorld() ? GetWorld()->GetMapName() : TEXT("Unknown");

    UE_LOG(LogFallenCompassGameMode, Log, TEXT("AFCGameMode active | Pawn=%s | Controller=%s | Level=%s"),
        *PawnName,
        *ControllerName,
        *MapName);

    // TODO: Remove once proper UI system is in place.
    if (GEngine)
	{
		const FString DebugLine = FString::Printf(TEXT("AFCGameMode active | Pawn=%s | Controller=%s | Level=%s"), *PawnName, *ControllerName, *MapName);
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, DebugLine);
	}

    // Notify LevelTransitionManager so it can complete any pending
    // TransitionViaLoading() flows (e.g., Overworld -> Office -> ExpeditionSummary).
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UFCLevelTransitionManager* TransitionMgr = GI->GetSubsystem<UFCLevelTransitionManager>())
        {
            TransitionMgr->InitializeLevelTransitionOnLevelStart();
        }
    }
}

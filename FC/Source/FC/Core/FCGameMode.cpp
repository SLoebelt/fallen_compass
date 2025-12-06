// Copyright Slomotion Games. All Rights Reserved.

#include "FCGameMode.h"
#include "FCFirstPersonCharacter.h"
#include "FCPlayerController.h"
#include "Characters/FC_ExplorerCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Core/FCLevelTransitionManager.h"
#include "Core/FCLevelManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogFallenCompassOldGameMode);

AFCGameMode::AFCGameMode()
{
    // Use AFCFirstPersonCharacter for Office level (Task 3.1)
    DefaultPawnClass = AFCFirstPersonCharacter::StaticClass();
    PlayerControllerClass = AFCPlayerController::StaticClass();

    UE_LOG(LogFallenCompassOldGameMode, Log, TEXT("AFCGameMode configured with AFCFirstPersonCharacter and AFCPlayerController."));
}

void AFCGameMode::BeginPlay()
{
    Super::BeginPlay();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("AFCPlayerController CONSTRUCTOR CALLED"));
	}

    const UClass* PawnClass = DefaultPawnClass ? DefaultPawnClass.Get() : nullptr;
    const FString PawnName = PawnClass ? PawnClass->GetName() : TEXT("None");
    const FString ControllerName = PlayerControllerClass ? PlayerControllerClass->GetName() : TEXT("None");
    const FString MapName = GetWorld() ? GetWorld()->GetMapName() : TEXT("Unknown");

    UE_LOG(LogFallenCompassOldGameMode, Log, TEXT("AFCGameMode active | Pawn=%s | Controller=%s | Level=%s"),
        *PawnName,
        *ControllerName,
        *MapName);

    // TODO - Remove once proper UI system is in place.
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

APawn* AFCGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	// Special case: In Camp/POI levels, explorers are pre-placed in the level.
	// Check if an explorer already exists before spawning a default pawn.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UFCLevelManager* LevelMgr = GI->GetSubsystem<UFCLevelManager>())
		{
			EFCLevelType LevelType = LevelMgr->GetCurrentLevelType();

			// In Camp/POI levels, check for pre-placed explorers
			if (LevelType == EFCLevelType::Camp || LevelType == EFCLevelType::POI)
			{
				TArray<AActor*> FoundExplorers;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFC_ExplorerCharacter::StaticClass(), FoundExplorers);

				if (FoundExplorers.Num() > 0)
				{
					// Explorer already exists in level, don't spawn another one
					UE_LOG(LogFallenCompassOldGameMode, Log,
						TEXT("SpawnDefaultPawnAtTransform: Skipping spawn - explorer already placed in %s level"),
						*UEnum::GetValueAsString(LevelType));
					return nullptr;
				}
			}
		}
	}

	// Default behavior: spawn pawn normally
	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
}

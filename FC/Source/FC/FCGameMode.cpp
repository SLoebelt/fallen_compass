// Copyright Epic Games, Inc. All Rights Reserved.

#include "FCGameMode.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FCPlayerController.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY(LogFallenCompassGameMode);

AFCGameMode::AFCGameMode()
{
    // Keep template defaults for PIE until AFCFirstPersonCharacter arrives (task 3.x).
    static ConstructorHelpers::FClassFinder<APawn> DefaultPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
    if (DefaultPawnBPClass.Succeeded())
    {
        DefaultPawnClass = DefaultPawnBPClass.Class;
    }
    else
    {
        UE_LOG(LogFallenCompassGameMode, Warning, TEXT("Missing BP_TopDownCharacter asset. Falling back to ADefaultPawn."));
        DefaultPawnClass = ADefaultPawn::StaticClass();
    }
	
	PlayerControllerClass = AFCPlayerController::StaticClass();
	UE_LOG(LogFallenCompassGameMode, Log, TEXT("Defaulting to AFCPlayerController C++ class."));
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

    UE_LOG(LogFallenCompassGameMode, Verbose, TEXT("TODO: Hook up AFCFirstPersonCharacter/AFCPlayerController and Office level once tasks 3.x & 4.x complete."));

    // TODO: Remove once proper UI system is in place.
    if (GEngine)
	{
		const FString DebugLine = FString::Printf(TEXT("AFCGameMode active | Pawn=%s | Controller=%s | Level=%s"), *PawnName, *ControllerName, *MapName);
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, DebugLine);
	}
}
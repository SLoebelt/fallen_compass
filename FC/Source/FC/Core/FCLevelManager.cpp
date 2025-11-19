// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "Core/FCLevelManager.h"
#include "Core/FCTransitionManager.h"
#include "Core/UFCGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogFCLevelManager);

void UFCLevelManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get current world
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		UE_LOG(LogFCLevelManager, Warning, TEXT("Initialize called without valid game world"));
		return;
	}

	// Get raw level name from world
	FString RawMapName = World->GetMapName();
	
	// Normalize and detect type
	CurrentLevelName = NormalizeLevelName(FName(*RawMapName));
	CurrentLevelType = DetermineLevelType(CurrentLevelName);

	// Initialize PreviousLevelName to None
	PreviousLevelName = NAME_None;

	UE_LOG(LogFCLevelManager, Log, TEXT("Initialized: Level=%s, Type=%s"), 
		*CurrentLevelName.ToString(), 
		*UEnum::GetValueAsString(CurrentLevelType));
}

void UFCLevelManager::UpdateCurrentLevel(const FName& NewLevelName)
{
	// Store previous level before updating
	PreviousLevelName = CurrentLevelName;

	CurrentLevelName = NormalizeLevelName(NewLevelName);
	CurrentLevelType = DetermineLevelType(CurrentLevelName);
	
	UE_LOG(LogFCLevelManager, Log, TEXT("UpdateCurrentLevel: Level=%s, Type=%s, PreviousLevel=%s"), 
		*CurrentLevelName.ToString(), 
		*UEnum::GetValueAsString(CurrentLevelType),
		*PreviousLevelName.ToString());
}

FName UFCLevelManager::NormalizeLevelName(const FName& RawLevelName) const
{
	FString LevelNameStr = RawLevelName.ToString();

	// Strip PIE prefix (UEDPIE_0_, UEDPIE_1_, etc.)
	if (LevelNameStr.StartsWith(TEXT("UEDPIE_")))
	{
		// Find the underscore after the number (e.g., "UEDPIE_0_")
		int32 UnderscoreIndex = LevelNameStr.Find(TEXT("_"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 7);
		if (UnderscoreIndex != INDEX_NONE)
		{
			LevelNameStr = LevelNameStr.RightChop(UnderscoreIndex + 1);
		}
	}

	// Trim whitespace
	LevelNameStr.TrimStartAndEndInline();

	return FName(*LevelNameStr);
}

EFCLevelType UFCLevelManager::DetermineLevelType(const FName& LevelName) const
{
	const FString LevelNameStr = LevelName.ToString();

	// Check exact match first for main menu
	if (LevelNameStr.Equals(TEXT("L_MainMenu"), ESearchCase::IgnoreCase))
	{
		return EFCLevelType::MainMenu;
	}

	// Check for substring matches (allows for variants like L_Office_Act1, L_Office_Tutorial, etc.)
	if (LevelNameStr.Contains(TEXT("MainMenu"), ESearchCase::IgnoreCase))
	{
		return EFCLevelType::MainMenu;
	}
	if (LevelNameStr.Contains(TEXT("Office"), ESearchCase::IgnoreCase))
	{
		return EFCLevelType::Office;
	}
	if (LevelNameStr.Contains(TEXT("Overworld"), ESearchCase::IgnoreCase))
	{
		return EFCLevelType::Overworld;
	}
	if (LevelNameStr.Contains(TEXT("Camp"), ESearchCase::IgnoreCase))
	{
		return EFCLevelType::Camp;
	}
	if (LevelNameStr.Contains(TEXT("Combat"), ESearchCase::IgnoreCase))
	{
		return EFCLevelType::Combat;
	}
	if (LevelNameStr.Contains(TEXT("POI"), ESearchCase::IgnoreCase))
	{
		return EFCLevelType::POI;
	}
	if (LevelNameStr.Contains(TEXT("Village"), ESearchCase::IgnoreCase))
	{
		return EFCLevelType::Village;
	}

	// Default to Unknown if no match found
	return EFCLevelType::Unknown;
}

bool UFCLevelManager::IsGameplayLevel() const
{
	return CurrentLevelType == EFCLevelType::Office ||
	       CurrentLevelType == EFCLevelType::Overworld ||
	       CurrentLevelType == EFCLevelType::Camp ||
	       CurrentLevelType == EFCLevelType::POI ||
	       CurrentLevelType == EFCLevelType::Village;
}

void UFCLevelManager::LoadLevel(FName LevelName, bool bShowLoadingScreen)
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogFCLevelManager, Error, TEXT("LoadLevel: Invalid level name"));
		return;
	}

	// Normalize level name
	FName NormalizedLevelName = NormalizeLevelName(LevelName);

	UE_LOG(LogFCLevelManager, Log, TEXT("LoadLevel: Loading %s (ShowLoadingScreen: %s)"),
		*NormalizedLevelName.ToString(),
		bShowLoadingScreen ? TEXT("true") : TEXT("false"));

	// Get TransitionManager from GameInstance
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogFCLevelManager, Error, TEXT("LoadLevel: Failed to get GameInstance"));
		return;
	}

	UFCTransitionManager* TransitionMgr = GI->GetSubsystem<UFCTransitionManager>();
	if (!TransitionMgr)
	{
		UE_LOG(LogFCLevelManager, Error, TEXT("LoadLevel: Failed to get TransitionManager"));
		return;
	}

	// Store level name for callback
	LevelToLoad = NormalizedLevelName;

	// Bind to OnFadeOutComplete delegate
	TransitionMgr->OnFadeOutComplete.AddDynamic(this, &UFCLevelManager::OnFadeOutCompleteForLevelLoad);

	// Start fade out
	TransitionMgr->BeginFadeOut();
}

void UFCLevelManager::OnFadeOutCompleteForLevelLoad()
{
	// Unbind delegate (one-shot callback)
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (GI)
	{
		UFCTransitionManager* TransitionMgr = GI->GetSubsystem<UFCTransitionManager>();
		if (TransitionMgr)
		{
			TransitionMgr->OnFadeOutComplete.RemoveDynamic(this, &UFCLevelManager::OnFadeOutCompleteForLevelLoad);
		}
	}

	// Load new level
	UGameplayStatics::OpenLevel(this, LevelToLoad);

	// Note: FadeIn will be handled automatically by new level's BeginPlay or PlayerController
	// For Week 2, we rely on existing fade-in logic; Week 3+ will add explicit loading screen handling
}

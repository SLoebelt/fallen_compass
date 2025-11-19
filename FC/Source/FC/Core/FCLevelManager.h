// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FCLevelManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFCLevelManager, Log, All);

// Forward declarations
class UFCTransitionManager;

/**
 * EFCLevelType
 * 
 * Defines the type/category of a level in the game.
 * Used to determine appropriate input modes, camera settings, and UI behavior.
 */
UENUM(BlueprintType)
enum class EFCLevelType : uint8
{
	Unknown     UMETA(DisplayName = "Unknown"),
	MainMenu    UMETA(DisplayName = "Main Menu"),
	Office      UMETA(DisplayName = "Office"),
	Overworld   UMETA(DisplayName = "Overworld"),
	Camp        UMETA(DisplayName = "Camp"),
	Combat      UMETA(DisplayName = "Combat"),
	POI         UMETA(DisplayName = "Point of Interest"),
	Village     UMETA(DisplayName = "Village")
};

/**
 * UFCLevelManager
 * 
 * Game Instance Subsystem responsible for tracking current level state and providing
 * centralized level information to other systems.
 * 
 * Responsibilities:
 * - Track current level name (normalized, PIE-prefix stripped)
 * - Detect level type (MainMenu, Office, Overworld, etc.)
 * - Provide level type queries (IsMenuLevel, IsGameplayLevel, etc.)
 * 
 * Usage:
 *   UFCLevelManager* LevelMgr = GetGameInstance()->GetSubsystem<UFCLevelManager>();
 *   FName CurrentLevel = LevelMgr->GetCurrentLevelName();
 */
UCLASS()
class FC_API UFCLevelManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Get the current level name (normalized, no PIE prefix) */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	FName GetCurrentLevelName() const { return CurrentLevelName; }

	/** Get the current level type */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	EFCLevelType GetCurrentLevelType() const { return CurrentLevelType; }

	/** Get the previous level name (for back navigation) */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	FName GetPreviousLevelName() const { return PreviousLevelName; }

	/** Check if current level is a menu level */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	bool IsMenuLevel() const { return CurrentLevelType == EFCLevelType::MainMenu; }

	/** Check if current level is a gameplay level (Office, Overworld, Camp, POI, Village) */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	bool IsGameplayLevel() const;

	/** Update the current level (called after level transitions) */
	void UpdateCurrentLevel(const FName& NewLevelName);

	/**
	 * Load a new level with optional fade transition and loading screen
	 * @param LevelName Name of level to load (will be normalized)
	 * @param bShowLoadingScreen Whether to show loading screen during transition (not implemented in Week 2)
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	void LoadLevel(FName LevelName, bool bShowLoadingScreen = false);

	/** Normalize level name (strip PIE prefix, trim whitespace) */
	FName NormalizeLevelName(const FName& RawLevelName) const;

	/** Determine level type from level name */
	EFCLevelType DetermineLevelType(const FName& LevelName) const;

private:
	/** Current level name (normalized) */
	UPROPERTY()
	FName CurrentLevelName;

	/** Current level type */
	UPROPERTY()
	EFCLevelType CurrentLevelType;

	/** Previously loaded level name (for back navigation) */
	UPROPERTY()
	FName PreviousLevelName;

	/** Level name pending load (used by LoadLevel callback) */
	FName LevelToLoad;

	/** Callback when fade out completes before level load */
	UFUNCTION()
	void OnFadeOutCompleteForLevelLoad();
};

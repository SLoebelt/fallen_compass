// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
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
 * FFCLevelMetadata
 * 
 * Data-driven metadata for a level. Allows designers to configure level behavior
 * without C++ changes. Used as a row in the DT_LevelMetadata DataTable.
 * 
 * Properties:
 * - LevelType: Category/type of level (MainMenu, Office, Overworld, etc.)
 * - DefaultInputMode: Input mode to use when entering this level (UIOnly, GameAndUI, GameOnly)
 * - bRequiresFadeTransition: Whether level loads should use fade transitions
 * - bShowCursor: Whether to show mouse cursor in this level
 * - bRequiresLoadingScreen: Whether to show loading screen for this level (future)
 * - DisplayName: Human-readable name for UI/debug purposes
 * - Description: Optional description for designers
 */
USTRUCT(BlueprintType)
struct FFCLevelMetadata : public FTableRowBase
{
	GENERATED_BODY()

	/** Level type/category */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	EFCLevelType LevelType = EFCLevelType::Unknown;

	/** Default input mode for this level (UIOnly=0, GameAndUI=1, GameOnly=2) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	uint8 DefaultInputMode = 1; // GameAndUI

	/** Whether this level requires fade transitions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	bool bRequiresFadeTransition = true;

	/** Whether to show cursor in this level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	bool bShowCursor = true;

	/** Whether to show loading screen for this level (future feature) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	bool bRequiresLoadingScreen = false;

	/** Human-readable display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	FText DisplayName;

	/** Optional description for designers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level", meta = (MultiLine = true))
	FText Description;

	/** Default constructor */
	FFCLevelMetadata()
		: LevelType(EFCLevelType::Unknown)
		, DefaultInputMode(1) // GameAndUI
		, bRequiresFadeTransition(true)
		, bShowCursor(true)
		, bRequiresLoadingScreen(false)
	{}
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

	/** 
	 * Get metadata for a specific level from the DataTable
	 * @param LevelName Name of the level (will be normalized)
	 * @param OutMetadata Output parameter containing the metadata if found
	 * @return True if metadata was found, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	bool GetLevelMetadata(FName LevelName, FFCLevelMetadata& OutMetadata) const;

	/** 
	 * Get metadata for the current level
	 * @param OutMetadata Output parameter containing the metadata if found
	 * @return True if metadata was found, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	bool GetCurrentLevelMetadata(FFCLevelMetadata& OutMetadata) const;

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

	/** Level metadata DataTable (set via Blueprint in BP_FC_GameInstance) */
	UPROPERTY()
	TObjectPtr<UDataTable> LevelMetadataTable;

	/** Callback when fade out completes before level load */
	UFUNCTION()
	void OnFadeOutCompleteForLevelLoad();

public:
	/** Set the level metadata DataTable (called from GameInstance initialization) */
	UFUNCTION(BlueprintCallable, Category = "FC|Level")
	void SetLevelMetadataTable(UDataTable* InMetadataTable);
};

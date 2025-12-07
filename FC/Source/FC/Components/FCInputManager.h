// Copyright Slomotion Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FCInputManager.generated.h"

class UInputMappingContext;
class UFCInputConfig;
class UEnhancedInputLocalPlayerSubsystem;

DECLARE_LOG_CATEGORY_EXTERN(LogFCInputManager, Log, All);

/**
 * Defines the available input mapping modes for the game.
 * Each mode corresponds to a different gameplay context with unique input bindings.
 */
UENUM(BlueprintType)
enum class EFCInputMappingMode : uint8
{
	/** First-person office exploration with WASD movement and mouse look */
	FirstPerson = 0,

	/** Top-down overworld navigation (Week 3: Overworld camera mode) */
	TopDown,

	/** Combat/fight sequences with combat-specific controls */
	Fight,

	/** Static scenes (cutscenes, dialogue) with minimal interaction */
	StaticScene,

	/**
	 * Point-of-interest/local scene (e.g. Camp) with click-to-move explorer
	 * and UI interaction, but without free camera pan/zoom.
	 */
	POIScene
};

/**
 * UFCInputManager
 *
 * Manages Enhanced Input Mapping Context switching for different gameplay modes.
 * Extracted from AFCPlayerController for reusability across multiple controller types.
 *
 * Week 3 Integration:
 * - FirstPersonPlayerController uses FirstPerson mode for office exploration
 * - TopDownPlayerController will use TopDown mode for overworld navigation
 * - Both can switch to Fight mode for combat sequences
 * - StaticScene mode disables input during cutscenes/dialogue
 *
 * Architecture:
 * - Component-based design allows attachment to any PlayerController
 * - Centralized input context registry avoids duplication
 * - Blueprint-friendly for designer iteration on input modes
 *
 * Usage:
 *   UFCInputManager* InputManager = PlayerController->FindComponentByClass<UFCInputManager>();
 *   if (InputManager)
 *   {
 *       InputManager->SetInputMappingMode(EFCInputMappingMode::TopDown);
 *   }
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FC_API UFCInputManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UFCInputManager();

	virtual void BeginPlay() override;

	void SetInputConfig(UFCInputConfig* NewConfig);
	const UFCInputConfig* GetInputConfig() const { return InputConfig; }

	/**
	 * Switch to a different input mapping context.
	 * Clears all existing mappings and applies the specified mode's context.
	 *
	 * @param NewMode The input mapping mode to activate
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|Input")
	void SetInputMappingMode(EFCInputMappingMode NewMode);

	/**
	 * Get the currently active input mapping mode.
	 *
	 * @return Current input mapping mode
	 */
	UFUNCTION(BlueprintPure, Category = "FC|Input")
	EFCInputMappingMode GetCurrentMappingMode() const { return CurrentMappingMode; }

protected:
	/**
	 * Get the Enhanced Input subsystem for the owning player controller.
	 *
	 * @return Enhanced Input subsystem, or nullptr if not found
	 */
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** Current input mapping mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Input|State")
	EFCInputMappingMode CurrentMappingMode;

	/** Priority for input mapping contexts (higher priority overrides lower) */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Settings")
	int32 DefaultMappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, Category="FC|Input")
    TObjectPtr<UFCInputConfig> InputConfig;
};

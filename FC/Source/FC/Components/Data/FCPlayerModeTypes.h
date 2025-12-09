#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"      // EMouseLockMode
#include "UObject/SoftObjectPtr.h"   // TSoftObjectPtr

#include "FCPlayerCameraTypes.h"     // EFCPlayerCameraMode

#include "FCPlayerModeTypes.generated.h"

class UFCInputConfig;
class UFCInteractionProfile;

/**
 * High-level click intent for a given player mode.
 * Allows data-driven click behavior independent of camera mode.
 */
UENUM(BlueprintType)
enum class EFCClickPolicy : uint8
{
	None               UMETA(DisplayName = "None"),
	OverworldClickMove UMETA(DisplayName = "Overworld: Click Move"),
	CampClickMove      UMETA(DisplayName = "Camp: Click Move"),
	TableObjectClick   UMETA(DisplayName = "Office/Table: Object Click"),
};

UENUM(BlueprintType)
enum class EFCPlayerMode : uint8
{
	MainMenu   UMETA(DisplayName = "Main Menu"),
	Office     UMETA(DisplayName = "Office"),
	Overworld  UMETA(DisplayName = "Overworld"),
	Camp       UMETA(DisplayName = "Camp"),
	Static     UMETA(DisplayName = "Static"),
};

/**
 * Designer-editable profile describing how a given player mode should behave.
 * Stored in a DataAsset (ProfileSet) and applied by the mode coordinator.
 */
USTRUCT(BlueprintType)
struct FPlayerModeProfile
{
	GENERATED_BODY()

	/** Input configuration asset to apply for this mode (Enhanced Input contexts, etc.). */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Mode")
	TSoftObjectPtr<UFCInputConfig> InputConfig;

	/** Camera mode to apply when entering this mode. */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Mode")
	EFCPlayerCameraMode CameraMode = EFCPlayerCameraMode::FirstPerson;

	/** Placeholder for future mode-specific interaction profile asset/type. */
	UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
	TSoftObjectPtr<UFCInteractionProfile> InteractionProfile;

	/** High-level click intent for this mode (move, interact, none). */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Mode")
	EFCClickPolicy ClickPolicy = EFCClickPolicy::None;

	/** Should mouse cursor be visible while in this mode. */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Mode")
	bool bShowMouseCursor = true;

	/** Mouse lock behavior for this mode (used with SetInputMode config). */
	UPROPERTY(EditDefaultsOnly, Category = "FC|Mode")
	EMouseLockMode MouseLockMode = EMouseLockMode::DoNotLock;
};

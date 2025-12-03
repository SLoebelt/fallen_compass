// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "FCCameraManager.generated.h"

class ACameraActor;

// Forward declare enum from FCPlayerController.h
enum class EFCPlayerCameraMode : uint8;

DECLARE_LOG_CATEGORY_EXTERN(LogFCCameraManager, Log, All);

/**
 * UFCCameraManager
 * 
 * Component responsible for managing camera transitions and view targets.
 * Handles camera mode switching, blend times, and temporary camera spawning.
 * 
 * Priority 3 Refactoring: Extracted from AFCPlayerController to reduce complexity
 * and enable reusability across different controller types.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FC_API UFCCameraManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UFCCameraManager();

	virtual void BeginPlay() override;

	// --- Camera Mode Management ---

	/** Get current camera mode */
	UFUNCTION(BlueprintPure, Category = "Camera")
	EFCPlayerCameraMode GetCameraMode() const { return CurrentCameraMode; }

	/** Get original view target (before any camera transitions) */
	UFUNCTION(BlueprintPure, Category = "Camera")
	AActor* GetOriginalViewTarget() const { return OriginalViewTarget; }

	/** Check if currently in a camera transition */
	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsInTransition() const { return bIsTransitioning; }

	// --- Camera Transition API ---

	/**
	 * Blend camera to main menu view
	 * @param BlendTime Blend duration in seconds (-1 uses DefaultBlendTime)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BlendToMenuCamera(float BlendTime = -1.0f);

	/**
	 * Blend camera back to first-person view
	 * @param BlendTime Blend duration in seconds (-1 uses DefaultBlendTime)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BlendToFirstPerson(float BlendTime = -1.0f);

	/**
	 * Blend camera to table object view
	 * @param TableObject Target table object with camera target point
	 * @param BlendTime Blend duration in seconds (-1 uses DefaultBlendTime)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BlendToTableObject(AActor* TableObject, float BlendTime = -1.0f);

	/**
	 * Blend camera to top-down perspective (Week 3 preparation)
	 * @param BlendTime Blend duration in seconds (-1 uses DefaultBlendTime)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BlendToTopDown(float BlendTime = -1.0f);

	/**
	 * Restore camera to previous view target
	 * @param BlendTime Blend duration in seconds (-1 uses DefaultBlendTime)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RestorePreviousViewTarget(float BlendTime = -1.0f);

	/**
	 * Restore camera to previous table view camera (used when closing widget)
	 * @param BlendTime Blend duration in seconds (-1 uses DefaultBlendTime)
	 */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RestorePreviousTableCamera(float BlendTime = -1.0f);

	// --- Configuration ---

	/** Set main menu camera reference */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetMenuCamera(ACameraActor* InMenuCamera);

protected:
	// --- Camera References ---

	/** Main menu camera actor (static camera in office). Editable per-instance in BP. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Menu")
	TObjectPtr<ACameraActor> MenuCamera;

	/** Temporary table view camera (spawned dynamically) */
	UPROPERTY()
	TObjectPtr<ACameraActor> TableViewCamera;

	/** Previous table view camera (for transitions between table objects) */
	UPROPERTY()
	TObjectPtr<ACameraActor> PreviousTableViewCamera;

	/** Original view target before any camera transitions */
	UPROPERTY()
	TObjectPtr<AActor> OriginalViewTarget;

	// --- Camera State ---

	/** Current camera mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	EFCPlayerCameraMode CurrentCameraMode;

	/** Previous camera mode (for restoration) */
	UPROPERTY()
	EFCPlayerCameraMode PreviousCameraMode;

	/** Whether currently transitioning between cameras */
	UPROPERTY()
	bool bIsTransitioning;

	// --- Configuration ---

	/** Default blend time for camera transitions (configurable in Blueprint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Config")
	float DefaultBlendTime;

	/** Default blend function for camera transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Config")
	TEnumAsByte<EViewTargetBlendFunction> DefaultBlendFunction;

private:
	// --- Internal Helpers ---

	/** Get effective blend time (uses default if input is -1) */
	float GetEffectiveBlendTime(float BlendTime) const;

	/** Perform camera blend to target */
	void BlendToTarget(AActor* Target, float BlendTime, EViewTargetBlendFunction BlendFunc);

	/** Set camera mode and log transition */
	void SetCameraMode(EFCPlayerCameraMode NewMode);

	/** Cleanup table view camera after transition complete */
	void CleanupTableViewCamera();

	/** Timer handle for camera cleanup */
	FTimerHandle CameraCleanupTimerHandle;

	/** Get player controller (convenience) */
	APlayerController* GetPlayerController() const;
};

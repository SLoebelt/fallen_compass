// Copyright Slomotion Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IFCInteractablePOI.h"
#include "FCInteractionProfile.h"
#include "FCInteractionComponent.generated.h"

class IIFCInteractable;
class UUserWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogFCInteraction, Log, All);

// Optional: logging-only phase enum
UENUM()
enum class EFCInteractionPhase : uint8
{
	Idle,
	Selecting,
	MovingToPOI,
	Executing
};

/**
 * UFCInteractionComponent - Handles detection and execution of player interactions
 * Attach this to the player character to enable interaction with IFCInteractable objects
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FC_API UFCInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFCInteractionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** Attempt to interact with the currently focused interactable */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	/** Returns the currently focused interactable actor, if any */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractable() const { return CurrentInteractable.Get(); }

	/** Returns true if there's a valid interactable in focus */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasInteractable() const { return CurrentInteractable.IsValid(); }

	/** Handle POI click interaction (queries actions, shows selection widget if needed) */
	UFUNCTION(BlueprintCallable, Category = "Interaction|POI")
	void HandlePOIClick(AActor* POIActor);

	/** Called when convoy overlaps with POI (executes pending action or shows selection) */
	UFUNCTION(BlueprintCallable, Category = "Interaction|POI")
	void NotifyPOIOverlap(AActor* POIActor);

	// Called by convoy/explorer when they arrive at a POI (overlap or path-complete).
	// If bAwaitingArrival is true and PendingPOI matches POIActor, executes PendingAction.
	// Otherwise falls back to incidental overlap handling (enemy ambush, LMB move).
	// Always clears latches on a valid pending execution (idempotent).
	UFUNCTION()
	void NotifyArrivedAtPOI(AActor* POIActor);

	/** Called when user selects an action from the selection widget */
	UFUNCTION()
	void OnPOIActionSelected(EFCPOIAction SelectedAction);

protected:
	/** Performs a line trace to detect interactables */
	void DetectInteractables();

	/** Updates the interaction prompt widget */
	void UpdatePromptWidget();

protected:
	/** Maximum distance for interaction line trace */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionTraceDistance = 500.0f;

	/** How often to check for interactables (in seconds). Lower = more responsive, higher = better performance */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckFrequency = 0.1f;

	/** Widget class for the interaction prompt (e.g., "Press E to Open Door") */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|UI")
	TSubclassOf<UUserWidget> InteractionPromptWidgetClass;

	/** Whether to show debug lines for interaction traces */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Debug")
	bool bShowDebugTrace = false;

	UPROPERTY(EditDefaultsOnly, Category="Interaction|Profile")
	TSoftObjectPtr<UFCInteractionProfile> DefaultProfile;

private:
	UPROPERTY()
	TObjectPtr<UFCInteractionProfile> ActiveProfile;

	UFCInteractionProfile* GetEffectiveProfile() const;

	/** POI actor currently targeted by click (separate from first-person CurrentInteractable). */
	TWeakObjectPtr<AActor> FocusedTarget;

	/** POI actor for the active POI interaction flow (selection → move → execute). */
	TWeakObjectPtr<AActor> PendingPOI;

	/** Selected action to execute for PendingPOI once conditions are met (arrival-gated next task). */
	TOptional<EFCPOIAction> PendingAction;

	/** True when UI selection is open / required before we can proceed. */
	bool bAwaitingSelection = false;

	/** True when we have a chosen action and must wait until we arrive at PendingPOI. */
	bool bAwaitingArrival = false;

	/**
	 * True if we began selection while already at the POI (overlap happened before selection),
	 * so after selection we execute immediately without issuing movement again.
	 */
	bool bPendingPOIAlreadyReached = false;

	EFCInteractionPhase GetCurrentInteractionPhase() const;

	/** Clears pending POI state (safe to call multiple times). */
	void ResetInteractionState();

	void ExecutePOIActionNow(AActor* POIActor, EFCPOIAction Action);

	// TODO - check if still needed after refactor
	/** Currently focused interactable actor */
	TWeakObjectPtr<AActor> CurrentInteractable;

	/** Instance of the interaction prompt widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> InteractionPromptWidget;

	/** Timer for interaction checks */
	float InteractionCheckTimer = 0.0f;

	// Cached because Owner is the PlayerController (GetInstigatorController() can be null here)
	TWeakObjectPtr<class AFCPlayerController> OwnerPC;

	// Prevent log spam
	mutable bool bLoggedMissingOwnerPC = false;

	/** Returns cached owner PC, logs once if missing */
	AFCPlayerController* GetOwnerPCCheckedOrNull() const;

    /** Whether FirstPerson focus tracing / prompts are currently enabled. */
    bool bFirstPersonFocusEnabled = false;

    /** Clears current focus target and hides the prompt widget, if any. */
    void ClearFocusAndHidePrompt();

public:
	AActor* GetPendingInteractionPOI() const { return PendingPOI.Get(); }

	bool HasPendingPOIInteraction() const
	{
		return bAwaitingSelection || bAwaitingArrival;
	}

	void SetFirstPersonFocusEnabled(bool bEnabled);

	void ApplyInteractionProfile(UFCInteractionProfile* NewProfile);
};

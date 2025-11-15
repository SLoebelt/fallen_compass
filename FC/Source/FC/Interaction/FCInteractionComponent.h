// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FCInteractionComponent.generated.h"

class IIFCInteractable;
class UUserWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogFCInteraction, Log, All);

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

private:
	/** Currently focused interactable actor */
	TWeakObjectPtr<AActor> CurrentInteractable;

	/** Instance of the interaction prompt widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> InteractionPromptWidget;

	/** Timer for interaction checks */
	float InteractionCheckTimer = 0.0f;
};

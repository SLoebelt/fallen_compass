// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IFCInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UIFCInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IFCInteractable - Interface for all interactable objects in the game
 * Implement this on actors that the player can interact with (doors, desk, items, NPCs, etc.)
 */
class FC_API IIFCInteractable
{
	GENERATED_BODY()

public:
	/**
	 * Called when the player interacts with this object
	 * @param Interactor - The actor performing the interaction (typically the player character)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(AActor* Interactor);

	/**
	 * Returns the text to display in the interaction prompt
	 * Example: "Open Door", "Examine Desk", "Pick Up Item"
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt() const;

	/**
	 * Checks if this object can currently be interacted with
	 * Use this for conditions like locked doors, quest requirements, etc.
	 * @param Interactor - The actor attempting to interact
	 * @return true if interaction is allowed, false otherwise
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor) const;

	/**
	 * Returns the maximum distance from which this object can be interacted with
	 * Default is 200 units (2 meters)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	float GetInteractionRange() const;

	/**
	 * Returns an optional priority value for overlapping interactables
	 * Higher values are preferred when multiple objects are in range
	 * Default is 0. Use for things like "important quest items > background objects"
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	int32 GetInteractionPriority() const;
};

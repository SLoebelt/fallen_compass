// Copyright Slomotion Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FCTableInteractable.generated.h"

/**
 * UFCTableInteractable - Reflection wrapper for IFCTableInteractable.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UFCTableInteractable : public UInterface
{
    GENERATED_BODY()
};

/**
 * IFCTableInteractable
 * Interface for map table objects (Map, Logbook, Letters, Glass, etc.).
 * Provides camera target transform, interaction gating, and widget class retrieval.
 *
 * C++ Usage:
 *   if (Actor->GetClass()->ImplementsInterface(UFCTableInteractable::StaticClass()))
 *   {
 *       IFCTableInteractable::Execute_OnTableObjectClicked(Actor, PlayerController);
 *   }
 */
class FC_API IFCTableInteractable
{
    GENERATED_BODY()

public:
    /** Called when player clicks the object */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|Table Interaction")
    void OnTableObjectClicked(APlayerController* PlayerController);

    /** Camera target transform for focus */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|Table Interaction")
    FTransform GetCameraTargetTransform() const;

    /** Whether interaction is allowed */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|Table Interaction")
    bool CanInteract() const;

    /** Widget class to display (nullptr if none) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|Table Interaction")
    TSubclassOf<UUserWidget> GetWidgetClass() const;
};

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IFCInteractablePOI.generated.h"

/**
 * EFCPOIAction - Enum for POI action types
 * Extensible for future action types
 */
UENUM(BlueprintType)
enum class EFCPOIAction : uint8
{
	Talk        UMETA(DisplayName = "Talk"),
	Ambush      UMETA(DisplayName = "Ambush"),
	Enter       UMETA(DisplayName = "Enter"),
	Trade       UMETA(DisplayName = "Trade"),
	Harvest     UMETA(DisplayName = "Harvest"),
	Observe     UMETA(DisplayName = "Observe")
};

/**
 * FFCPOIActionData - Struct containing action display information
 */
USTRUCT(BlueprintType)
struct FFCPOIActionData
{
	GENERATED_BODY()

	/** Action type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFCPOIAction ActionType;

	/** Display text for action button (e.g., "Talk to Merchant") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ActionText;

	/** Optional icon for action (future UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ActionIcon = nullptr;

	FFCPOIActionData()
		: ActionType(EFCPOIAction::Talk)
		, ActionText(FText::FromString(TEXT("Interact")))
	{}
};

// UInterface class (required by Unreal)
UINTERFACE(MinimalAPI, Blueprintable)
class UIFCInteractablePOI : public UInterface
{
	GENERATED_BODY()
};

/**
 * IIFCInteractablePOI - Interface for overworld POI interaction
 * Provides action-based interaction system for POIs
 */
class FC_API IIFCInteractablePOI
{
	GENERATED_BODY()

public:
	/**
	 * Get all available actions for this POI
	 * Called when right-clicking POI or on convoy overlap
	 * @return Array of available actions with display data
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
	TArray<FFCPOIActionData> GetAvailableActions() const;
	virtual TArray<FFCPOIActionData> GetAvailableActions_Implementation() const
	{
		// Default: no actions available
		return TArray<FFCPOIActionData>();
	}

	/**
	 * Execute a specific action on this POI
	 * Called when action is selected (or auto-executed if only one action)
	 * @param Action - The action to execute
	 * @param Interactor - The actor performing the action (typically convoy)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
	void ExecuteAction(EFCPOIAction Action, AActor* Interactor);
	virtual void ExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor)
	{
		// Default: log action (override in derived classes)
		UE_LOG(LogTemp, Warning, TEXT("IIFCInteractablePOI::ExecuteAction: Action not implemented"));
	}

	/**
	 * Get POI display name for UI and logging
	 * @return Display name (e.g., "Village", "Ruins", "Enemy Camp")
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
	FString GetPOIName() const;
	virtual FString GetPOIName_Implementation() const
	{
		// Default: return "Unknown POI"
		return TEXT("Unknown POI");
	}

	/**
	 * Check if action can be executed (conditions, requirements)
	 * Optional - default implementation returns true
	 * @param Action - Action to check
	 * @param Interactor - Actor attempting action
	 * @return true if action can be executed
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
	bool CanExecuteAction(EFCPOIAction Action, AActor* Interactor) const;
	virtual bool CanExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) const
	{
		// Default: all actions allowed
		return true;
	}
};

// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Logging/LogMacros.h"
// Log category for transition manager and related UI
DECLARE_LOG_CATEGORY_EXTERN(LogFCTransitions, Log, All);
#include "FCTransitionManager.generated.h"

class UFCScreenTransitionWidget;

/**
 * Delegate fired when a screen transition completes (fade in or fade out)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionComplete);

/**
 * Game Instance Subsystem managing screen transitions (fades, loading screens, etc.)
 * Persists across level loads and provides unified API for all transition needs.
 */
UCLASS()
class FC_API UFCTransitionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Begin fade to black transition
	 * @param Duration How long the fade should take in seconds (default 1.0s)
	 * @param bShowLoadingIndicator Whether to show loading spinner during fade
	 */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void BeginFadeOut(float Duration = 1.0f, bool bShowLoadingIndicator = false);

	/**
	 * Begin fade from black to clear transition
	 * @param Duration How long the fade should take in seconds (default 1.0s)
	 */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void BeginFadeIn(float Duration = 1.0f);

	/**
	 * Check if a transition is currently in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	bool IsFading() const { return bCurrentlyFading; }

	/**
	 * Check if loading into the same level we're currently in
	 * @param TargetLevelName Name of the level being loaded
	 * @return True if target level matches current level
	 */
	UFUNCTION(BlueprintPure, Category = "Transition")
	bool IsSameLevelLoad(const FName& TargetLevelName) const;

	/**
	 * Update current level tracking (called after level loads)
	 * @param NewLevelName Name of the newly loaded level
	 */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void UpdateCurrentLevel(const FName& NewLevelName);

	/**
	 * Delegate broadcast when fade out completes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Transition")
	FOnTransitionComplete OnFadeOutComplete;

	/**
	 * Delegate broadcast when fade in completes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Transition")
	FOnTransitionComplete OnFadeInComplete;

protected:
	/**
	 * Widget class reference for screen transition overlay (soft reference).
	 * Use a soft class pointer so we can safely load the asset at runtime
	 * without using ConstructorHelpers outside constructors.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Transition")
	TSoftClassPtr<UFCScreenTransitionWidget> TransitionWidgetClass;

	/**
	 * Persistent instance of transition widget (created once, reused)
	 */
	UPROPERTY()
	UFCScreenTransitionWidget* TransitionWidget;

	/**
	 * Flag tracking whether a transition is currently in progress
	 */
	bool bCurrentlyFading;

	/**
	 * Name of the currently loaded level (for detecting same-level loads)
	 */
	FName CurrentLevelName;

	/**
	 * Create and add transition widget to viewport
	 */
	void CreateTransitionWidget();

	/**
	 * Called when widget's fade out animation completes
	 */
	UFUNCTION()
	void OnWidgetFadeOutComplete();

	/**
	 * Called when widget's fade in animation completes
	 */
	UFUNCTION()
	void OnWidgetFadeInComplete();
};

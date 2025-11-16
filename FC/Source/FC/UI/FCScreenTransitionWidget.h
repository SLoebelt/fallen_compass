// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FCScreenTransitionWidget.generated.h"

class UImage;
class UOverlay;

/**
 * Delegate fired when fade animation completes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFadeComplete);

/**
 * Full-screen overlay widget for screen transitions (fades, loading screens, etc.)
 * Handles fade in/out animations and optional loading indicator display.
 */
UCLASS()
class FC_API UFCScreenTransitionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Begin fade to black (opacity 0 → 1)
	 * @param Duration How long the fade should take in seconds
	 * @param bShowLoading Whether to display loading indicator during fade
	 */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void BeginFadeOut(float Duration, bool bShowLoading);

	/**
	 * Begin fade from black to clear (opacity 1 → 0)
	 * @param Duration How long the fade should take in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void BeginFadeIn(float Duration);

	/**
	 * Delegate broadcast when fade out animation completes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Transition")
	FOnFadeComplete OnFadeOutComplete;

	/**
	 * Delegate broadcast when fade in animation completes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Transition")
	FOnFadeComplete OnFadeInComplete;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void NativeConstruct() override;

	/**
	 * Current opacity value (0 = transparent, 1 = fully opaque black)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Transition")
	float CurrentOpacity;

	/**
	 * Target opacity we're fading towards
	 */
	float TargetOpacity;

	/**
	 * Duration of current fade operation
	 */
	float FadeDuration;

	/**
	 * Time elapsed in current fade operation
	 */
	float FadeElapsed;

	/**
	 * Whether a fade is currently in progress
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Transition")
	bool bIsFading;

	/**
	 * Whether loading indicator should be visible
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Transition")
	bool bShowLoadingIndicator;

	/**
	 * Whether current fade is fading out (true) or fading in (false)
	 */
	bool bIsFadingOut;

	/**
	 * Main overlay containing fade image and loading indicator
	 * Bind this in the UMG designer
	 */
	UPROPERTY(meta = (BindWidget))
	UOverlay* Overlay_Main;

	/**
	 * Black image that covers the screen during fade
	 * Bind this in the UMG designer
	 */
	UPROPERTY(meta = (BindWidget))
	UImage* Image_Fade;

	/**
	 * Overlay containing loading indicator elements
	 * Bind this in the UMG designer
	 */
	UPROPERTY(meta = (BindWidget))
	UOverlay* Overlay_Loading;

	/**
	 * Update opacity during fade animation
	 */
	void UpdateFadeOpacity(float DeltaTime);

	/**
	 * Complete current fade operation and fire appropriate delegate
	 */
	void CompleteFade();
};

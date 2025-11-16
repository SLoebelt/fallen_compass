// Copyright Iron Anchor Interactive. All Rights Reserved.

#include "UI/FCScreenTransitionWidget.h"
#include "Core/FCTransitionManager.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"

void UFCScreenTransitionWidget::BeginFadeOut(float Duration, bool bShowLoading)
{
	if (bIsFading)
	{
		UE_LOG(LogFCTransitions, Warning, TEXT("FCScreenTransitionWidget: Fade already in progress"));
		return;
	}

	// Set up fade parameters
	CurrentOpacity = 0.0f;
	TargetOpacity = 1.0f;
	FadeDuration = Duration;
	FadeElapsed = 0.0f;
	bIsFading = true;
	bIsFadingOut = true;
	bShowLoadingIndicator = bShowLoading;

	// Show/hide loading indicator - use Visible/Hidden so layout remains intact
	if (Overlay_Loading)
	{
		Overlay_Loading->SetVisibility(bShowLoading ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// Ensure Image_Fade is visible and initialized to transparent black
	if (Image_Fade)
	{
		Image_Fade->SetVisibility(ESlateVisibility::Visible);
		Image_Fade->SetColorAndOpacity(FLinearColor::Black);
		Image_Fade->SetOpacity(CurrentOpacity);
	}
	else
	{
		UE_LOG(LogFCTransitions, Warning, TEXT("FCScreenTransitionWidget: BeginFadeOut - Image_Fade is not bound"));
	}

	// Ensure widget is visible
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// Also set render opacity so the whole widget can be faded if the Image brush doesn't draw
	SetRenderOpacity(CurrentOpacity);

	UE_LOG(LogFCTransitions, Log, TEXT("FCScreenTransitionWidget: Fade out started"));
}

void UFCScreenTransitionWidget::BeginFadeIn(float Duration)
{
	if (bIsFading)
	{
		UE_LOG(LogFCTransitions, Warning, TEXT("FCScreenTransitionWidget: Fade already in progress"));
		return;
	}

	// Set up fade parameters
	CurrentOpacity = 1.0f;
	TargetOpacity = 0.0f;
	FadeDuration = Duration;
	FadeElapsed = 0.0f;
	bIsFading = true;
	bIsFadingOut = false;

	// Hide loading indicator during fade in (use Hidden so it can be shown later)
	if (Overlay_Loading)
	{
		Overlay_Loading->SetVisibility(ESlateVisibility::Hidden);
	}
	bShowLoadingIndicator = false;

	// Ensure widget is visible
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// Also set render opacity as a fallback
	SetRenderOpacity(CurrentOpacity);

	UE_LOG(LogFCTransitions, Log, TEXT("FCScreenTransitionWidget: Fade in started"));
}

void UFCScreenTransitionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsFading)
	{
		UpdateFadeOpacity(InDeltaTime);
	}
}

void UFCScreenTransitionWidget::UpdateFadeOpacity(float DeltaTime)
{
	FadeElapsed += DeltaTime;

	// Calculate alpha (0 to 1 over duration)
	float Alpha = FMath::Clamp(FadeElapsed / FadeDuration, 0.0f, 1.0f);

	// Apply smooth easing (ease in-out cubic)
	Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	// Lerp between current and target opacity
	CurrentOpacity = FMath::Lerp(bIsFadingOut ? 0.0f : 1.0f, TargetOpacity, Alpha);

	// Update image opacity
	if (Image_Fade)
	{
		// Apply render opacity and also ensure the tint's alpha reflects the value
		Image_Fade->SetOpacity(CurrentOpacity);
		FLinearColor Tint = Image_Fade->GetColorAndOpacity();
		Tint.A = CurrentOpacity;
		Image_Fade->SetColorAndOpacity(Tint);

		// If the image has no brush resource (designer left it blank), assign the engine default texture so it actually draws
		const FSlateBrush& ImageBrush = Image_Fade->GetBrush();
		if (ImageBrush.GetResourceObject() == nullptr && GEngine && GEngine->DefaultTexture)
		{
			Image_Fade->SetBrushFromTexture(Cast<UTexture2D>(GEngine->DefaultTexture), true);
		}
	}

	// Fallback: set widget-level render opacity so the whole widget fades even if the image/material ignores tint
	SetRenderOpacity(CurrentOpacity);

	UE_LOG(LogFCTransitions, Verbose, TEXT("FCScreenTransitionWidget: UpdateFadeOpacity | Current=%.3f Target=%.3f Elapsed=%.3f/%0.3f IsFadingOut=%d"),
		CurrentOpacity, TargetOpacity, FadeElapsed, FadeDuration, bIsFadingOut ? 1 : 0);

	// Check if fade is complete
	if (Alpha >= 1.0f)
	{
		CompleteFade();
	}
}

void UFCScreenTransitionWidget::CompleteFade()
{
	bIsFading = false;

	// Ensure final opacity is exact
	CurrentOpacity = TargetOpacity;
	if (Image_Fade)
	{
		Image_Fade->SetOpacity(CurrentOpacity);
	}

	// Ensure render opacity matches final state
	SetRenderOpacity(CurrentOpacity);

	// Broadcast appropriate delegate
	if (bIsFadingOut)
	{
		OnFadeOutComplete.Broadcast();
		UE_LOG(LogFCTransitions, Log, TEXT("FCScreenTransitionWidget: Fade out complete"));
	}
	else
	{
		OnFadeInComplete.Broadcast();
		UE_LOG(LogFCTransitions, Log, TEXT("FCScreenTransitionWidget: Fade in complete"));

		// After fading in completely, can hide widget to save rendering
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFCScreenTransitionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize internal state
	bIsFading = false;
	bIsFadingOut = false;
	CurrentOpacity = 0.0f;
	TargetOpacity = 0.0f;
	FadeDuration = 0.0f;
	FadeElapsed = 0.0f;
	bShowLoadingIndicator = false;

	// Ensure Image_Fade starts as fully transparent black (will be driven by code)
	if (Image_Fade)
	{
		Image_Fade->SetColorAndOpacity(FLinearColor::Black);
		Image_Fade->SetOpacity(0.0f);

		// If the designer didn't supply a brush, assign the engine default texture so it renders as a solid quad
		const FSlateBrush& ImageBrush = Image_Fade->GetBrush();
		if (ImageBrush.GetResourceObject() == nullptr && GEngine && GEngine->DefaultTexture)
		{
			Image_Fade->SetBrushFromTexture(Cast<UTexture2D>(GEngine->DefaultTexture), true);
		}
	}

	// Set loading overlay to Hidden (so it can be shown later without breaking layout)
	if (Overlay_Loading)
	{
		Overlay_Loading->SetVisibility(ESlateVisibility::Hidden);
	}

	// Ensure the widget is not visible until used
	SetVisibility(ESlateVisibility::Collapsed);

	// Start with fully transparent render opacity
	SetRenderOpacity(0.0f);

	UE_LOG(LogFCTransitions, Verbose, TEXT("FCScreenTransitionWidget: NativeConstruct initialized (Image_Fade=%s, Overlay_Loading=%s)"),
		Image_Fade ? TEXT("present") : TEXT("missing"), Overlay_Loading ? TEXT("present") : TEXT("missing"));
}

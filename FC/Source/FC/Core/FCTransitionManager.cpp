// Copyright Iron Anchor Interactive. All Rights Reserved.

#include "Core/FCTransitionManager.h"
#include "UI/FCScreenTransitionWidget.h"
#include "Core/UFCGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "UObject/SoftObjectPtr.h"

// Define the transitions log category declared in the header
DEFINE_LOG_CATEGORY(LogFCTransitions);

void UFCTransitionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Initialized"));

	// Get widget class from GameInstance configuration
	UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->TransitionWidgetClass.ToSoftObjectPath().IsValid())
	{
		TransitionWidgetClass = GameInstance->TransitionWidgetClass;
		UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Widget class loaded from GameInstance configuration"));
	}
	else
	{
		UE_LOG(LogFCTransitions, Warning, TEXT("FCTransitionManager: No widget class configured in GameInstance! Set TransitionWidgetClass in BP_FCGameInstance."));
	}

	// Widget will be created on first use (lazy initialization)
	bCurrentlyFading = false;
}

void UFCTransitionManager::Deinitialize()
{
	// Clean up persistent widget using RemoveViewportWidgetContent
	if (TransitionWidget)
	{
		UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
		if (ViewportClient)
		{
			TSharedPtr<SWidget> WidgetSlatePtr = TransitionWidget->GetCachedWidget();
			if (WidgetSlatePtr.IsValid())
			{
				ViewportClient->RemoveViewportWidgetContent(WidgetSlatePtr.ToSharedRef());
				UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Removed persistent widget from viewport"));
			}
		}
		TransitionWidget = nullptr;
	}

	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Deinitialized"));

	Super::Deinitialize();
}

void UFCTransitionManager::CreateTransitionWidget()
{
	// Only create once
	if (TransitionWidget)
	{
		return;
	}

	// Check if we have a valid widget class (load if soft reference)
	UClass* WidgetClass = nullptr;
	if (TransitionWidgetClass.IsValid())
	{
		WidgetClass = TransitionWidgetClass.Get();
	}
	else if (TransitionWidgetClass.ToSoftObjectPath().IsValid())
	{
		// Load synchronously here (safe during runtime initialization)
		UObject* Loaded = TransitionWidgetClass.LoadSynchronous();
		WidgetClass = Cast<UClass>(Loaded);
	}

	if (!WidgetClass)
	{
		UE_LOG(LogFCTransitions, Error, TEXT("FCTransitionManager: TransitionWidgetClass not set or failed to load! Cannot create transition widget."));
		return;
	}

	// Create widget instance with GameInstance as outer for persistence across level loads
	// Using GameInstance instead of PlayerController ensures widget survives controller destruction
	TransitionWidget = CreateWidget<UFCScreenTransitionWidget>(GetGameInstance(), WidgetClass);
	if (!TransitionWidget)
	{
		UE_LOG(LogFCTransitions, Error, TEXT("FCTransitionManager: Failed to create TransitionWidget"));
		return;
	}

	// Bind to widget's completion events
	TransitionWidget->OnFadeOutComplete.AddDynamic(this, &UFCTransitionManager::OnWidgetFadeOutComplete);
	TransitionWidget->OnFadeInComplete.AddDynamic(this, &UFCTransitionManager::OnWidgetFadeInComplete);

	// Add to viewport using AddViewportWidgetContent for persistence across level loads
	// This method keeps the widget alive even when PlayerController is destroyed
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ViewportClient)
	{
		ViewportClient->AddViewportWidgetContent(
			TransitionWidget->TakeWidget(),
			1000  // ZOrder - very high to be on top of everything
		);
		UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: TransitionWidget added to viewport (persistent)"));
	}
	else
	{
		UE_LOG(LogFCTransitions, Error, TEXT("FCTransitionManager: Failed to get GameViewportClient"));
		return;
	}

	// Start invisible (opacity 0)
	TransitionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: TransitionWidget created successfully"));
}

void UFCTransitionManager::BeginFadeOut(float Duration, bool bShowLoadingIndicator)
{
	if (bCurrentlyFading)
	{
		UE_LOG(LogFCTransitions, Warning, TEXT("FCTransitionManager: Fade already in progress, ignoring BeginFadeOut"));
		return;
	}

	// Ensure widget exists
	if (!TransitionWidget)
	{
		CreateTransitionWidget();
	}

	if (!TransitionWidget)
	{
		UE_LOG(LogFCTransitions, Error, TEXT("FCTransitionManager: Cannot begin fade out - widget creation failed"));
		return;
	}

	bCurrentlyFading = true;
	TransitionWidget->BeginFadeOut(Duration, bShowLoadingIndicator);

	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Fade out started (Duration: %.2fs, Loading: %s)"), 
		Duration, bShowLoadingIndicator ? TEXT("Yes") : TEXT("No"));
}

void UFCTransitionManager::BeginFadeIn(float Duration)
{
	if (bCurrentlyFading)
	{
		UE_LOG(LogFCTransitions, Warning, TEXT("FCTransitionManager: Fade already in progress, ignoring BeginFadeIn"));
		return;
	}

	// Create widget if it doesn't exist yet
	if (!TransitionWidget)
	{
		CreateTransitionWidget();
	}

	if (!TransitionWidget)
	{
		UE_LOG(LogFCTransitions, Error, TEXT("FCTransitionManager: Cannot begin fade in - widget creation failed"));
		return;
	}

	bCurrentlyFading = true;
	TransitionWidget->BeginFadeIn(Duration);

	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Fade in started (Duration: %.2fs)"), Duration);
}

void UFCTransitionManager::OnWidgetFadeOutComplete()
{
	bCurrentlyFading = false;
	OnFadeOutComplete.Broadcast();

	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Fade out complete"));
}

void UFCTransitionManager::OnWidgetFadeInComplete()
{
	bCurrentlyFading = false;
	OnFadeInComplete.Broadcast();

	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Fade in complete"));
}

bool UFCTransitionManager::IsSameLevelLoad(const FName& TargetLevelName) const
{
	if (CurrentLevelName.IsNone() || TargetLevelName.IsNone())
	{
		return false;
	}

	return CurrentLevelName == TargetLevelName;
}

void UFCTransitionManager::UpdateCurrentLevel(const FName& NewLevelName)
{
	CurrentLevelName = NewLevelName;
	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Current level updated to %s"), *NewLevelName.ToString());
}

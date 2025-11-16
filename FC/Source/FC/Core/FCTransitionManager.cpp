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

	// Create widget immediately and initialize to black for clean startup (Step 1C.4)
	// This ensures PIE starts with a black screen instead of showing content before fade-in
	CreateTransitionWidget();
	
	// If widget was created but ViewportClient wasn't ready, set up a timer to retry
	if (TransitionWidget && !TransitionWidget->IsInViewport())
	{
		// Use a timer to retry adding to viewport once it's ready
		FTimerHandle RetryHandle;
		GameInstance->GetTimerManager().SetTimer(
			RetryHandle,
			[this]()
			{
				EnsureWidgetInViewport();
				
				// Start game with automatic fade-in from black (Step 1C.5)
				// Trigger fade-in after a short delay to ensure widget is fully initialized
				FTimerHandle FadeInHandle;
				GetGameInstance()->GetTimerManager().SetTimer(
					FadeInHandle,
					[this]()
					{
						// Trigger automatic fade-in from black screen (Step 1C.5)
						if (TransitionWidget)
						{
							BeginFadeIn(1.5f);  // 1.5 second fade-in to reveal game
						}
					},
					0.2f,  // 200ms delay after viewport addition
					false
				);
			},
			0.1f,  // Try again in 100ms
			false
		);
	}
	
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

	UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: TransitionWidget created successfully"));
}

void UFCTransitionManager::EnsureWidgetInViewport()
{
	if (!TransitionWidget)
	{
		return;
	}

	// Check if widget is already in viewport
	if (TransitionWidget->IsInViewport())
	{
		return;
	}

	// Try to add to viewport now
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ViewportClient)
	{
		ViewportClient->AddViewportWidgetContent(
			TransitionWidget->TakeWidget(),
			1000
		);
		
		// Now that it's in the viewport, initialize to black
		TransitionWidget->InitializeToBlack();
		
		UE_LOG(LogFCTransitions, Log, TEXT("FCTransitionManager: Widget added to viewport and initialized to black"));
	}
	else
	{
		UE_LOG(LogFCTransitions, Error, TEXT("FCTransitionManager: ViewportClient not available, cannot add widget to viewport"));
	}
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

	// Ensure widget is in viewport (handles deferred addition from Initialize)
	EnsureWidgetInViewport();

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

	// Ensure widget is in viewport (handles deferred addition from Initialize)
	EnsureWidgetInViewport();

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

bool UFCTransitionManager::IsBlack() const
{
	return TransitionWidget && TransitionWidget->IsBlack();
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

#include "UI/FCBlockingWidgetBase.h"

#include "Core/UFCGameInstance.h"
#include "Core/FCUIManager.h"
#include "Core/FCUIBlockSubsystem.h"

void UFCBlockingWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	RegisterAsBlocker();

	// Backward-compat: keep your existing focused widget path until all gating is migrated.
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>())
			{
				UIManager->SetFocusedBlockingWidget(this);
			}
		}
	}
}

void UFCBlockingWidgetBase::NativeDestruct()
{
	UnregisterAsBlocker();

	// Backward-compat: clear focused widget only if we're still the focused one.
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>())
			{
				if (UIManager->FocusedBlockingWidget == this)
				{
					UIManager->SetFocusedBlockingWidget(nullptr);
				}
			}
		}
	}

	Super::NativeDestruct();
}

void UFCBlockingWidgetBase::RegisterAsBlocker()
{
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIBlockSubsystem* Blocker = GI->GetSubsystem<UFCUIBlockSubsystem>())
			{
				Blocker->RegisterBlocker(this, bBlocksWorldClick, bBlocksWorldInteract);
			}
		}
	}
}

void UFCBlockingWidgetBase::UnregisterAsBlocker()
{
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIBlockSubsystem* Blocker = GI->GetSubsystem<UFCUIBlockSubsystem>())
			{
				Blocker->UnregisterBlocker(this);
			}
		}
	}
}

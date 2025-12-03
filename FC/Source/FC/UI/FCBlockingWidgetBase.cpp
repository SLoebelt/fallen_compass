#include "UI/FCBlockingWidgetBase.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCUIManager.h"

void UFCBlockingWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

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
	if (UWorld* World = GetWorld())
	{
		if (UFCGameInstance* GI = Cast<UFCGameInstance>(World->GetGameInstance()))
		{
			if (UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>())
			{
				// Only clear if we're still the focused widget
				if (UIManager->FocusedBlockingWidget == this)
				{
					UIManager->SetFocusedBlockingWidget(nullptr);
				}
			}
		}
	}

	Super::NativeDestruct();
}

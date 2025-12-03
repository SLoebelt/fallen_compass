#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FCBlockingWidgetBase.generated.h"

/**
 * Base widget that automatically registers itself as a focused blocking
 * UI widget while it is on screen. Used for expedition planning and
 * overworld map so world interaction/click-to-move are disabled.
 */
UCLASS()
class FC_API UFCBlockingWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};

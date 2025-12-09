#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FCBlockingWidgetBase.generated.h"

/**
 * Base widget that automatically registers itself as a blocking UI widget
 * while it is on screen.
 *
 * New: You can choose whether it blocks World Click and/or World Interact.
 */
UCLASS()
class FC_API UFCBlockingWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** When true, blocks click-to-move / world clicking while this widget is visible */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI Blocking")
	bool bBlocksWorldClick = true;

	/** When true, blocks world interaction (e.g. E interact) while this widget is visible */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI Blocking")
	bool bBlocksWorldInteract = true;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void RegisterAsBlocker();
	void UnregisterAsBlocker();
};

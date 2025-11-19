// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FCExpeditionPlanningWidget.generated.h"

class UCanvasPanel;
class UImage;
class UVerticalBox;
class UHorizontalBox;
class UBorder;
class UButton;
class UTextBlock;

/**
 * Main expedition planning widget shown when clicking the map table object.
 * Displays supplies, map placeholder, and expedition start/back buttons.
 * Week 2 Task 3.4.0: WBP_TableMap implementation in C++
 */
UCLASS()
class FC_API UFCExpeditionPlanningWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    /** Button click handlers */
    UFUNCTION()
    void OnStartExpeditionClicked();

    UFUNCTION()
    void OnBackButtonClicked();

    /** Update supplies display from GameInstance */
    void UpdateSuppliesDisplay();

public:
    // UI Components (exposed for Blueprint access if needed)
    
    /** Root canvas panel */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UCanvasPanel> RootCanvas;

    /** Background image (parchment texture) */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UImage> BackgroundImage;

    /** Main container for all UI elements */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UVerticalBox> MainContainer;

    /** Title text "Expedition Planning" */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> TitleText;

    /** Supplies label "Supplies: " */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> SuppliesLabel;

    /** Supplies value (bound to GameInstance) */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> SuppliesValue;

    /** Map container border */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UBorder> MapContainer;

    /** Map placeholder image */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UImage> MapPlaceholder;

    /** Start expedition button */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UButton> StartExpeditionButton;

    /** Start button text */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> StartButtonText;

    /** Back button */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UButton> BackButton;

    /** Back button text */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> BackButtonText;
};

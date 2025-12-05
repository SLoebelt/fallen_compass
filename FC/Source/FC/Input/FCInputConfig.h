#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FCInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS(BlueprintType)
class FC_API UFCInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Actions (what PlayerController binds) ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
	TObjectPtr<UInputAction> QuickSaveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
	TObjectPtr<UInputAction> QuickLoadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
	TObjectPtr<UInputAction> EscapeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
	TObjectPtr<UInputAction> OverworldPanAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
	TObjectPtr<UInputAction> OverworldZoomAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Actions")
	TObjectPtr<UInputAction> ToggleOverworldMapAction;

	// --- Mapping Contexts (what UFCInputManager applies) ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Contexts")
	TObjectPtr<UInputMappingContext> FirstPersonMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Contexts")
	TObjectPtr<UInputMappingContext> TopDownMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Contexts")
	TObjectPtr<UInputMappingContext> POISceneMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Contexts")
	TObjectPtr<UInputMappingContext> StaticSceneMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input|Contexts")
	TObjectPtr<UInputMappingContext> FightMappingContext;
};

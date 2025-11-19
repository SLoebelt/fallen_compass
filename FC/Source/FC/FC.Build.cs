// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FC : ModuleRules
{
	public FC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"FC",
			"FC/Core",
			"FC/Expedition",
			"FC/Interaction",
			"FC/SaveGame",
			"FC/Variant_Strategy",
			"FC/Variant_Strategy/UI",
			"FC/Variant_TwinStick",
			"FC/Variant_TwinStick/AI",
			"FC/Variant_TwinStick/Gameplay",
			"FC/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

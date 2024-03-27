// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetworkManager : ModuleRules
{
	public NetworkManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"NetCore"
		});
	}
}

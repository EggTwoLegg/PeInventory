// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PeInventory : ModuleRules
{
	public PeInventory(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"NetCore",
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay", 
			"GameplayTags", 
		});
		
		//PrivateDependencyModuleNames.AddRange(new string[] {"ReplicationGraph"});
		PrivateDependencyModuleNames.AddRange(new string[]
		{
		    "NetCore",
			"Engine",
		});

		PublicIncludePaths.AddRange(new string[]
		{
			"PeInventory/Public",
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			"PeInventory/Private",
		});

		bUseUnity = false;
		MinFilesUsingPrecompiledHeaderOverride = 1;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProcGenMaze : ModuleRules
{
	public ProcGenMaze(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "AIModule", "NavigationSystem" });
	}
}

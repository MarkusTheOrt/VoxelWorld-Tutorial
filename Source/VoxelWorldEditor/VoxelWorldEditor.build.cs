// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class VoxelWorldEditor : ModuleRules
{
	public VoxelWorldEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add("VoxelWorldEditor/Public");
		PrivateIncludePaths.Add("VoxelWorldEditor/Private");

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UnrealEd",
			"DetailCustomizations",
			"PropertyEditor",
			"EditorStyle",
			"VoxelWorld"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Slate",
			"SlateCore"
		});

	}
}

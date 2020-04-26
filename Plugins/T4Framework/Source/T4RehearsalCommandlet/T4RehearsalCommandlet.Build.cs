// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class T4RehearsalCommandlet : ModuleRules
	{
		public T4RehearsalCommandlet(ReadOnlyTargetRules Target) : base(Target)
		{
			// http://api.unrealengine.com/KOR/Programming/UnrealBuildSystem/IWYUReferenceGuide/
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"Engine",
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					"T4RehearsalCommandlet/Private",
					"T4RehearsalCommandlet/Private/Commandlets",
					// ... add other private include paths required here ...
				});

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"T4Asset",
					"T4Engine",
					"T4Framework",
					"T4Gameplay",
				}
			);

			if (Target.bBuildEditor == true)
			{
				PrivateDependencyModuleNames.Add("UnrealEd");
				PrivateDependencyModuleNames.Add("SourceControl");
			}
		}
	}
}

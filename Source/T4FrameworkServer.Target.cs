// Copyright 2019 SoonBo Noh. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class T4FrameworkServerTarget : TargetRules
{
    // http://api.unrealengine.com/KOR/Programming/UnrealBuildSystem/TargetFiles/

    public T4FrameworkServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

        ExtraModuleNames.AddRange(
            new string[] {
				"T4Framework"
			}
        );
    }
}

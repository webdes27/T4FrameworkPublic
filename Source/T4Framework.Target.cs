// Copyright 2019 SoonBo Noh. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class T4FrameworkTarget : TargetRules
{
    // http://api.unrealengine.com/KOR/Programming/UnrealBuildSystem/TargetFiles/

    public T4FrameworkTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

        ExtraModuleNames.AddRange(
            new string[] {
				"T4Framework"
			} 
        );
	}
}

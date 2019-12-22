// Copyright 2019 SoonBo Noh. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class T4FrameworkEditorTarget : TargetRules
{
    // http://api.unrealengine.com/KOR/Programming/UnrealBuildSystem/TargetFiles/

    public T4FrameworkEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

        ExtraModuleNames.AddRange(
            new string[] {
				"T4Framework"
			}
        );
    }
}

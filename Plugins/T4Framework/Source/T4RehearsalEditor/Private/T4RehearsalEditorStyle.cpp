// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalEditorStyle.h"

#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4RehearsalEditorStyle"

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

/**
  * 
 */
TSharedPtr<FSlateStyleSet> FT4RehearsalEditorStyle::StyleInstance = nullptr;

void FT4RehearsalEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FT4RehearsalEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FT4RehearsalEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("T4RehearsalEditorStyle"));
	return StyleSetName;
}

T4REHEARSALEDITOR_API FString RelativePathToPluginPath(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("T4Framework"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( RelativePathToPluginPath(RelativePath, ".png" ), __VA_ARGS__)
#define IMAGE_BRUSH(RelativePath,...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png") ), __VA_ARGS__)

TSharedRef<FSlateStyleSet> FT4RehearsalEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("T4RehearsalEditorStyle"));
	Style->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));

	// Menu Icons
	Style->Set(
		"T4RehearsalEditorStyle.TabThumbnail_16px",
		new IMAGE_BRUSH("Icons/icon_file_switch_16px", Icon16x16)
	);

	Style->Set(
		"T4RehearsalEditorStyle.TabEntityBrowser_16px",
		new IMAGE_BRUSH("Icons/icon_file_switch_16px", Icon16x16)
	);

	Style->Set(
		"T4RehearsalEditorStyle.TabMainViewport_16px",
		new IMAGE_BRUSH("Icons/icon_file_switch_16px", Icon16x16)
	);

	Style->Set(
		"T4RehearsalEditorStyle.TabDetail_16px",
		new IMAGE_BRUSH("Icons/icon_file_switch_16px", Icon16x16)
	);

	Style->Set(
		"T4RehearsalEditorStyle.TabEditorPlaySettings_16px",
		new IMAGE_BRUSH("Icons/icon_file_switch_16px", Icon16x16)
	); // #60

	Style->Set(
		"T4RehearsalEditorStyle.TabAnimSetDetail_16px",
		new IMAGE_BRUSH("Icons/icon_file_switch_16px", Icon16x16)
	);

	Style->Set(
		"T4RehearsalEditorStyle.TabTimelie_16px",
		new IMAGE_BRUSH("Icons/icon_file_switch_16px", Icon16x16)
	);

	Style->Set(
		"T4RehearsalEditorStyle.TabContiBrowser_16px",
		new IMAGE_BRUSH("Icons/icon_file_switch_16px", Icon16x16)
	);

	// LevelEditor_Mode
	Style->Set(
		"T4RehearsalEditorStyle.LevelEditorMode_40x",
		new IMAGE_PLUGIN_BRUSH("Editor/Icons/T4EditorMode_40x", Icon40x40)
	);

	Style->Set(
		"T4RehearsalEditorStyle.LevelEditorMode_16x",
		new IMAGE_PLUGIN_BRUSH("Editor/Icons/T4EditorMode_16x", Icon16x16)
	);

	Style->Set(
		"T4RehearsalEditorStyle.ContiTrackActionIsolate_16x", 
		new IMAGE_PLUGIN_BRUSH("Icons/Isolate", Icon16x16)
	); // #54

	Style->Set(
		"T4RehearsalEditorStyle.ContiEditorMirrorToPIE_40x",
		new IMAGE_BRUSH("Icons/icon_findnode_40x", Icon40x40)
	); // #59

	// #68
	Style->Set(
		"T4RehearsalEditorStyle.ToolbarActionPlaybackRec_40x",
		new IMAGE_BRUSH("Icons/icon_Persona_StartRecord_40x", Icon40x40)
	);
	Style->Set(
		"T4RehearsalEditorStyle.ToolbarActionPlaybackPlay_40x",
		new IMAGE_BRUSH("Icons/icon_SCueEd_PlayCue_40x", Icon40x40)
	);
	Style->Set(
		"T4RehearsalEditorStyle.ToolbarActionPlaybackPause_40x",
		new IMAGE_BRUSH("Icons/icon_SCueEd_Pause_40x", Icon40x40)
	);
	Style->Set(
		"T4RehearsalEditorStyle.ToolbarActionPlaybackStop_40x",
		new IMAGE_BRUSH("Icons/icon_SCueEd_Stop_40x", Icon40x40)
	);
	Style->Set(
		"T4RehearsalEditorStyle.ToolbarDespawnAll_40x",
		new IMAGE_BRUSH("Icons/icon_DeviceRelease_40x", Icon40x40)
	);
	// ~#68

	Style->Set(
		"T4RehearsalEditorStyle.ToolbarGoPreviewScene_40x",
		new IMAGE_BRUSH("Icons/icon_Persona_PreviewAsset_40x", Icon40x40)
	); // #87

	Style->Set(
		"T4RehearsalEditorStyle.WorldEditorSaveAllModified_40x",
		new IMAGE_BRUSH("Icons/icon_file_saveall_40x", Icon40x40)
	); // #86
	return Style;
}

#undef IMAGE_BRUSH

void FT4RehearsalEditorStyle::ReloadTextures()
{
	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
}

const ISlateStyle& FT4RehearsalEditorStyle::Get()
{
	return *StyleInstance;
}

#undef LOCTEXT_NAMESPACE
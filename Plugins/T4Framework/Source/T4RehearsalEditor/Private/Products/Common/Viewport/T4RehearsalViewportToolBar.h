// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "SViewportToolBar.h"

/**
  * #94 : refer SCommonEditorViewportToolBarBase.h
 */
class ST4RehearsalViewport;
class FT4RehearsalViewportClient;
class ST4RehearsalViewportToolBar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS(ST4RehearsalViewportToolBar) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, ST4RehearsalViewport* InViewport);

protected:
	TSharedRef<SWidget> GenerateOptionsMenu() const;
	void ExtendOptionsMenu(FMenuBuilder& OptionsMenuBuilder) const;

	TSharedRef<SWidget> GenerateFOVMenu() const;
	float OnGetFOVValue() const;
	void OnFOVValueChanged(float NewValue) const;

	TSharedRef<SWidget> GenerateFarViewPlaneMenu() const;
	float OnGetFarViewPlaneValue() const;
	void OnFarViewPlaneValueChanged(float NewValue);

	TSharedRef<SWidget> GenerateScreenPercentageMenu() const;
	int32 OnGetScreenPercentageValue() const;
	bool OnScreenPercentageIsEnabled() const;
	void OnScreenPercentageValueChanged(int32 NewValue);

	FText GetCameraMenuLabel() const;
	const FSlateBrush* GetCameraMenuLabelIcon() const;
	TSharedRef<SWidget> GenerateCameraMenu() const;

	TSharedRef<class ST4RehearsalViewportViewMenu> MakeViewMenu();
	TSharedPtr<FExtender> GetViewMenuExtender() const;
	void CreateViewMenuExtensions(FMenuBuilder& MenuBuilder);
	TSharedPtr<FExtender> GetCombinedExtenderList(TSharedRef<FExtender> MenuExtender) const;

	TSharedRef<SWidget> GenerateShowMenu() const;

	EVisibility GetViewModeOptionsVisibility() const;
	TSharedRef<SWidget> GenerateViewModeOptionsMenu() const;

private:
	FT4RehearsalViewportClient* GetViewportClient() const;

private:
	ST4RehearsalViewport* ViewportRef;
};
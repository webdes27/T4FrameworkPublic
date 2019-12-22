// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Widgets/SWidget.h"
#include "SEditorViewport.h"
#include "SEditorViewportToolBarMenu.h"
#include "Styling/SlateTypes.h"

/**
  * #94 : refer SCommonEditorViewportToolBarBase.h
 */
struct FSlateBrush;
class ST4RehearsalViewport;
class ST4RehearsalViewportViewMenu : public SEditorViewportToolbarMenu
{
public:
	SLATE_BEGIN_ARGS(ST4RehearsalViewportViewMenu){}
		SLATE_ARGUMENT( TSharedPtr<class FExtender>, MenuExtenders )
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, ST4RehearsalViewport* InViewport, TSharedRef<class SViewportToolBar> InParentToolBar );

private:
	FText GetViewMenuLabel() const;
	const FSlateBrush* GetViewMenuLabelIcon() const;

protected:
	virtual TSharedRef<SWidget> GenerateViewMenuContent() const;
	ST4RehearsalViewport* ViewportRef;

private:
	TSharedPtr<class FExtender> MenuExtenders;
};

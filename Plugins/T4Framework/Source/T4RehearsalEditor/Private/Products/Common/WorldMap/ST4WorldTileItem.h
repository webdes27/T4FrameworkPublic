// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Layout/SlateRect.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Types/SlateStructs.h"
#include "Widgets/Images/SImage.h"
#include "SNodePanel.h"

class FT4LevelModel;
class FSlateWindowElementList;
class FT4TileThumbnailCollection;
class FT4WorldTileCollectionModel;
class FT4WorldTileModel;
class IToolTip;
class SToolTip;

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
class ST4WorldTileImage : public SImage
{
	SLATE_BEGIN_ARGS(ST4WorldTileImage)
		: _EditableTile(false) // #104
	{}
		SLATE_ATTRIBUTE(bool, EditableTile)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs)
	{
		EditableTile = InArgs._EditableTile; 
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TAttribute<bool> EditableTile;
};

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
class ST4WorldTileItem 
	: public SNodePanel::SNode
{
public:
	SLATE_BEGIN_ARGS(ST4WorldTileItem)
	{}
		/** The world data */
		SLATE_ARGUMENT(TSharedPtr<FT4WorldTileCollectionModel>, InWorldModel)
		/** Data for the asset this item represents */
		SLATE_ARGUMENT(TSharedPtr<FT4WorldTileModel>, InItemModel)
		/** Thumbnails management */
		SLATE_ARGUMENT(TSharedPtr<FT4TileThumbnailCollection>, InThumbnailCollection)
	SLATE_END_ARGS()

	ST4WorldTileItem();
	~ST4WorldTileItem();

	void Construct(const FArguments& InArgs);
		
	// SNodePanel::SNode interface start
	virtual FVector2D GetDesiredSizeForMarquee() const override;
	virtual UObject* GetObjectBeingDisplayed() const override;
	virtual FVector2D GetPosition() const override;
	virtual const FSlateBrush* GetShadowBrush(bool bSelected) const override;
	// SNodePanel::SNode interface end
	
	/** @return Deferred item refresh */
	void RequestRefresh();

	/** @return LevelModel associated with this item */
	TSharedPtr<FT4LevelModel>	GetLevelModel() const;
	
	/** @return Item width in world units */
	FOptionalSize GetItemWidth() const;
	
	/** @return Item height in world units */
	FOptionalSize GetItemHeight() const;
	
	/** @return Rectangle in world units for this item as FSlateRect*/
	FSlateRect GetItemRect() const;
	
	/** @return Whether this item can be edited (loaded and not locked) */
	bool IsItemEditable() const;

	/** @return Whether this item is selected */
	bool IsItemSelected() const;
	
	/** @return Whether this item is enabled */
	bool IsItemEnabled() const;

	/** @return Whether this item is editor selected */
	bool IsItemEditorSelected() const; // #90

	bool IsItemEditableEffect() const; // #104

private:
	// SWidget interface start
	virtual TSharedPtr<IToolTip> GetToolTip() override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	// SWidget interface end

	TSharedRef<SToolTip> CreateToolTipWidget();
	
	/** Tile tooltips fields */
	FText GetLevelNameText() const;
	FText GetPositionText() const;
	FText GetBoundsExtentText() const;
	FText GetLevelLayerNameText() const;
	FText GetLevelLayerDistanceText() const;
	
public:
	bool									bAffectedByMarquee;

private:
	/** The world data */
	TSharedPtr<FT4WorldTileCollectionModel>	WorldModel;
	/** The data for this item */
	TSharedPtr<FT4WorldTileModel>			TileModel;
	
	TSharedPtr<ST4WorldTileImage>			ThumbnailImageWidget;	
	TSharedPtr<FT4TileThumbnailCollection>	ThumbnailCollection;
	
	mutable bool							bNeedRefresh;
	bool									bIsDragging;
};


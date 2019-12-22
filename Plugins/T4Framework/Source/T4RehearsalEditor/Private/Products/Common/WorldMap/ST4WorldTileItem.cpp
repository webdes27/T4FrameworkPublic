// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4WorldTileItem.h"
#include "T4LevelModel.h"

#include "Rendering/DrawElements.h"
#include "Widgets/SBoxPanel.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SToolTip.h"

#include "T4WorldTileCollectionModel.h"
#include "T4WorldTileDetails.h"
#include "T4WorldTileThumbnails.h"

#define LOCTEXT_NAMESPACE "WorldMap"

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
int32 ST4WorldTileImage::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* ImageBrush = Image.Get();

	if ((ImageBrush != nullptr) && (ImageBrush->DrawAs != ESlateBrushDrawType::NoDrawType))
	{
		const bool bIsEnabled = EditableTile.Get() && ShouldBeEnabled(bParentEnabled); // #93 : 월드 섬네일의 회색 이팩트 처리
		const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			ImageBrush,
			DrawEffects | ESlateDrawEffect::IgnoreTextureAlpha, 
			FColor::White);
	}
	return LayerId;
}

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
ST4WorldTileItem::ST4WorldTileItem()
 : bAffectedByMarquee(false)
 , bNeedRefresh(false)
 , bIsDragging(false)
{
}

ST4WorldTileItem::~ST4WorldTileItem()
{
	ThumbnailImageWidget->SetImage(nullptr);
	ThumbnailCollection->UnregisterTile(*TileModel.Get());
	TileModel->ChangedEvent.RemoveAll(this);
}

void ST4WorldTileItem::Construct(const FArguments& InArgs)
{
	WorldModel = InArgs._InWorldModel;
	TileModel = InArgs._InItemModel;
	ThumbnailCollection = InArgs._InThumbnailCollection;
	
	TileModel->ChangedEvent.AddSP(this, &ST4WorldTileItem::RequestRefresh);
	
	GetOrAddSlot( ENodeZone::Center )
	[
		SAssignNew(ThumbnailImageWidget, ST4WorldTileImage)
		.EditableTile(this, &ST4WorldTileItem::IsItemEditableEffect) // #104
	];

	ThumbnailCollection->RegisterTile(*TileModel.Get());
	const FSlateBrush* TileBrush = ThumbnailCollection->GetTileBrush(*TileModel.Get());
	ThumbnailImageWidget->SetImage(TileBrush);

	SetToolTip(CreateToolTipWidget());

	bNeedRefresh = true;
}

void ST4WorldTileItem::RequestRefresh()
{
	bNeedRefresh = true;
}

UObject* ST4WorldTileItem::GetObjectBeingDisplayed() const
{
	return TileModel->GetNodeObject();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SToolTip> ST4WorldTileItem::CreateToolTipWidget()
{
	TSharedPtr<SToolTip> TooltipWidget;
	
	SAssignNew(TooltipWidget, SToolTip)
	.TextMargin(2)
	.BorderImage(FEditorStyle::GetBrush("ContentBrowser.TileViewTooltip.NonContentBorder"))
	[
		SNew(SVerticalBox)

		// Level name section
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0,0,0,4)
		[
			SNew(SBorder)
			.Padding(6)
			.BorderImage(FEditorStyle::GetBrush("ContentBrowser.TileViewTooltip.ContentBorder"))
			[
				SNew(SVerticalBox)
					
				+SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Level name
					+SHorizontalBox::Slot()
					.Padding(6,0,0,0)
					.HAlign(HAlign_Left)
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(this, &ST4WorldTileItem::GetLevelNameText) 
						.Font(FEditorStyle::GetFontStyle("ContentBrowser.TileViewTooltip.NameFont"))
					]
				]
			]
		]
			
		// Tile info section
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.Padding(6)
			.BorderImage(FEditorStyle::GetBrush("ContentBrowser.TileViewTooltip.ContentBorder"))
			[
				SNew(SUniformGridPanel)

				// Tile position
				+SUniformGridPanel::Slot(0, 0)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Item_OriginOffset", "Position:")) 
				]
					
				+SUniformGridPanel::Slot(1, 0)
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Text(this, &ST4WorldTileItem::GetPositionText) 
				]

				// Tile bounds
				+SUniformGridPanel::Slot(0, 1)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Item_BoundsExtent", "Extent:")) 
				]
					
				+SUniformGridPanel::Slot(1, 1)
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Text(this, &ST4WorldTileItem::GetBoundsExtentText) 
				]

				// Layer name
				+SUniformGridPanel::Slot(0, 2)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Item_Name", "Layer Name:")) 
				]
					
				+SUniformGridPanel::Slot(1, 2)
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Text(this, &ST4WorldTileItem::GetLevelLayerNameText) 
				]

				// Streaming distance
				+SUniformGridPanel::Slot(0, 3)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Item_Distance", "Streaming Distance:")) 
				]
					
				+SUniformGridPanel::Slot(1, 3)
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Text(this, &ST4WorldTileItem::GetLevelLayerDistanceText) 
				]
			]
		]
	];

	return TooltipWidget.ToSharedRef();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FVector2D ST4WorldTileItem::GetPosition() const
{
	return TileModel->GetLevelPosition2D();
}

TSharedPtr<FT4LevelModel> ST4WorldTileItem::GetLevelModel() const
{
	return StaticCastSharedPtr<FT4LevelModel>(TileModel);
}

const FSlateBrush* ST4WorldTileItem::GetShadowBrush(bool bSelected) const
{
	return bSelected ? FEditorStyle::GetBrush(TEXT("Graph.CompactNode.ShadowSelected")) : FEditorStyle::GetBrush(TEXT("Graph.Node.Shadow"));
}

FOptionalSize ST4WorldTileItem::GetItemWidth() const
{
	return FOptionalSize(TileModel->GetLevelSize2D().X);
}

FOptionalSize ST4WorldTileItem::GetItemHeight() const
{
	return FOptionalSize(TileModel->GetLevelSize2D().Y);
}

FSlateRect ST4WorldTileItem::GetItemRect() const
{
	FVector2D LevelSize = TileModel->GetLevelSize2D();
	FVector2D LevelPos = GetPosition();
	return FSlateRect(LevelPos, LevelPos + LevelSize);
}

TSharedPtr<IToolTip> ST4WorldTileItem::GetToolTip()
{
	// Hide tooltip in case item is being dragged now
	if (TileModel->GetLevelTranslationDelta().SizeSquared() > FMath::Square(KINDA_SMALL_NUMBER))
	{
		return NULL;
	}
	
	return SNodePanel::SNode::GetToolTip();
}

FVector2D ST4WorldTileItem::GetDesiredSizeForMarquee() const
{
	// we don't want to select items in non visible layers
	if (!WorldModel->PassesAllFilters(*TileModel))
	{
		return FVector2D::ZeroVector;
	}

	return SNodePanel::SNode::GetDesiredSizeForMarquee();
}

FVector2D ST4WorldTileItem::ComputeDesiredSize( float ) const
{
	return TileModel->GetLevelSize2D();
}

int32 ST4WorldTileItem::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const bool bIsVisible = FSlateRect::DoRectanglesIntersect(AllottedGeometry.GetLayoutBoundingRect(), ClippingRect);
	
	if (bIsVisible)
	{
		// Redraw thumbnail image if requested
		if (bNeedRefresh && !ThumbnailCollection->IsOnCooldown())
		{
			bNeedRefresh = false;
			const FSlateBrush* TileBrush = ThumbnailCollection->UpdateTileThumbnail(*TileModel.Get());
			ThumbnailImageWidget->SetImage(TileBrush);
		}
		
		LayerId = SNodePanel::SNode::OnPaint(Args, AllottedGeometry, ClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
		
		if (!WorldModel->IsSimulating()) // #92
		{
			// #104 : WorldBrowser 는 Editor world 에서 로드가 되면 회색톤을 풀어주었으나, WorldMap 에서는 PreviewWorld 를 기준으로 처리한다.
			//        대신 녹색 외곽 라인을 출력하는 것으로 대체! 단, Simulation 에서는 정상 동작됨
			const bool bEditorLoaded = IsItemEnabled();
			if (bEditorLoaded)
			{
				const FVector2D InflateAmount = FVector2D(2, 2);
				const float Scale = 0.5f; // Scale down image of the borders to make them thinner 
				FSlateLayoutTransform LayoutTransform(Scale, AllottedGeometry.GetAccumulatedLayoutTransform().GetTranslation() - InflateAmount);
				FSlateRenderTransform SlateRenderTransform(Scale, AllottedGeometry.GetAccumulatedRenderTransform().GetTranslation() - InflateAmount);
				FPaintGeometry SelectionGeometry(LayoutTransform, SlateRenderTransform, (AllottedGeometry.GetLocalSize() * AllottedGeometry.Scale + InflateAmount * 2) / Scale, !SlateRenderTransform.IsIdentity());
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 1,
					SelectionGeometry,
					GetShadowBrush(true),
					ESlateDrawEffect::None,
					FLinearColor::Green
				);
			}
		}

		// #90 : WorldBrowser 의 LevelHierarchy 에서 선택될 경우 선택된 Level 을 Preview 에 표시해준다. (특히, 로딩전 레벨 표시가 유효한 기능)
		const bool bEditorSelected = IsItemEditorSelected();
		if (bEditorSelected)
		{
			const FVector2D InflateAmount = FVector2D(2, 2);
			const float Scale = 0.5f; // Scale down image of the borders to make them thinner 
			FSlateLayoutTransform LayoutTransform(Scale, AllottedGeometry.GetAccumulatedLayoutTransform().GetTranslation() - InflateAmount);
			FSlateRenderTransform SlateRenderTransform(Scale, AllottedGeometry.GetAccumulatedRenderTransform().GetTranslation() - InflateAmount);
			FPaintGeometry SelectionGeometry(LayoutTransform, SlateRenderTransform, (AllottedGeometry.GetLocalSize() * AllottedGeometry.Scale + InflateAmount * 2) / Scale, !SlateRenderTransform.IsIdentity());
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 1,
				SelectionGeometry,
				GetShadowBrush(true),
				ESlateDrawEffect::None,
				FLinearColor::Blue
			);
		}

		const bool bSelected = (IsItemSelected() || bAffectedByMarquee);
		const int32* PreviewLODIndex = WorldModel->GetPreviewStreamingLevels().Find(TileModel->TileDetails->PackageName);
		const bool bHighlighted = (PreviewLODIndex != nullptr);

		// Draw the node's selection/highlight.
		if (bSelected || bHighlighted)
		{
			// Calculate selection box paint geometry 
			const FVector2D InflateAmount = FVector2D(4, 4);
			const float Scale = 0.5f; // Scale down image of the borders to make them thinner 
			FSlateLayoutTransform LayoutTransform(Scale, AllottedGeometry.GetAccumulatedLayoutTransform().GetTranslation() - InflateAmount);
			FSlateRenderTransform SlateRenderTransform(Scale, AllottedGeometry.GetAccumulatedRenderTransform().GetTranslation() - InflateAmount);
			FPaintGeometry SelectionGeometry(LayoutTransform, SlateRenderTransform, (AllottedGeometry.GetLocalSize()*AllottedGeometry.Scale + InflateAmount*2)/Scale, !SlateRenderTransform.IsIdentity());
			FLinearColor HighlightColor = FLinearColor::White;
			if (PreviewLODIndex)
			{
				// Highlight LOD tiles in different color to normal tiles
				HighlightColor = (*PreviewLODIndex == INDEX_NONE) ? FLinearColor::Green : FLinearColor(0.3f,1.0f,0.3f);
			}
			
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 2,
				SelectionGeometry,
				GetShadowBrush(bSelected || bHighlighted),
				ESlateDrawEffect::None,
				HighlightColor
			);
		}
	}
	
	return LayerId;
}

FReply ST4WorldTileItem::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	TileModel->MakeLevelCurrent();
	return FReply::Handled();
}

FText ST4WorldTileItem::GetLevelNameText() const
{
	return FText::FromString(TileModel->GetDisplayName());
}

FText ST4WorldTileItem::GetPositionText() const
{
	FIntVector Position = TileModel->GetRelativeLevelPosition();
	bool bLocked = WorldModel->IsLockTilesLocationEnabled();
	
	FTextFormat TextFormat;
	if (bLocked)
	{
		TextFormat = LOCTEXT("PositionXYZFmtLocked", "{0}, {1}, {2} (Locked)");
	}
	else
	{
		TextFormat = LOCTEXT("PositionXYZFmt", "{0}, {1}, {2}");
	}
		
	return FText::Format(TextFormat, FText::AsNumber(Position.X), FText::AsNumber(Position.Y), FText::AsNumber(Position.Z));
}

FText ST4WorldTileItem::GetBoundsExtentText() const
{
	FVector2D Size = TileModel->GetLevelSize2D();
	return FText::Format(LOCTEXT("PositionXYFmt", "{0}, {1}"), FText::AsNumber(FMath::RoundToInt(Size.X*0.5f)), FText::AsNumber(FMath::RoundToInt(Size.Y*0.5f)));
}

FText ST4WorldTileItem::GetLevelLayerNameText() const
{
	return FText::FromString(TileModel->TileDetails->Layer.Name);
}

FText ST4WorldTileItem::GetLevelLayerDistanceText() const
{
	if (TileModel->TileDetails->Layer.DistanceStreamingEnabled)
	{
		return FText::AsNumber(TileModel->TileDetails->Layer.StreamingDistance);
	}
	else
	{
		return FText(LOCTEXT("DistanceStreamingDisabled", "Distance Streaming Disabled"));
	}
}

bool ST4WorldTileItem::IsItemEditable() const
{
	return TileModel->IsEditable();
}

bool ST4WorldTileItem::IsItemSelected() const
{
	return TileModel->GetLevelSelectionFlag();
}

bool ST4WorldTileItem::IsItemEditorSelected() const
{
	return TileModel->GetEditorLevelSelectionFlag(); // #90
}

bool ST4WorldTileItem::IsItemEnabled() const
{
	return TileModel->IsEditable();
}

bool ST4WorldTileItem::IsItemEditableEffect() const
{
	if (WorldModel->IsSimulating()) // #92
	{
		return IsItemEnabled();
	}
	// #104 : WorldBrowser 는 Editor world 에서 로드가 되면 회색톤을 풀어주었으나, WorldMap 에서는 PreviewWorld 를 기준으로 처리한다.
	if (!TileModel->IsWorldCompositionEnabled())
	{
		return true; // World Composition 이 아니면 회색 처리를 하지 않는다.
	}
	return TileModel->GetPreviewLevelSelectionFlag();
}

#undef LOCTEXT_NAMESPACE

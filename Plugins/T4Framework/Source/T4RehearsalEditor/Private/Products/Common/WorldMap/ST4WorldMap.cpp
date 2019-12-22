// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4WorldMap.h"

#include "Products/Common/ViewModel/T4WorldMapViewModel.h" // #83

#include "T4Engine/Classes/World/T4MapZoneVolume.h" // #92
#include "T4Engine/Public/T4EngineUtility.h" // #92, #97

#include "Engine/BrushBuilder.h" // #92

#include "Layout/ArrangedChildren.h"
#include "Rendering/DrawElements.h"
#include "Widgets/SOverlay.h"
#include "Engine/GameViewportClient.h"
#include "Misc/PackageName.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Images/SImage.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Settings/LevelEditorViewportSettings.h"
#include "Editor.h"
#include "SNodePanel.h"
#include "Fonts/FontMeasure.h"

#include "ST4WorldTileItem.h"
#include "ST4WorldLayers.h"
#include "T4WorldTileThumbnails.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "WorldMap"

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
struct FWorldZoomLevelsContainer 
	: public FZoomLevelsContainer
{
	float	GetZoomAmount(int32 InZoomLevel) const override
	{
		return 1.f/FMath::Square(GetNumZoomLevels() - InZoomLevel + 1)*2.f;
	}

	int32 GetNearestZoomLevel( float InZoomAmount ) const override
	{
		for (int32 ZoomLevelIndex=0; ZoomLevelIndex < GetNumZoomLevels(); ++ZoomLevelIndex)
		{
			if (InZoomAmount <= GetZoomAmount(ZoomLevelIndex))
			{
				return ZoomLevelIndex;
			}
		}

		return GetDefaultZoomLevel();
	}

	FText GetZoomText(int32 InZoomLevel) const override
	{
		return FText::AsNumber(GetZoomAmount(InZoomLevel));
	}

	int32	GetNumZoomLevels() const override
	{
		return 300;
	}

	int32	GetDefaultZoomLevel() const override
	{
		return GetNumZoomLevels() - 7; // #90 : 기본 줌을 좀 더 당겨 놓는다.
	}

	EGraphRenderingLOD::Type GetLOD(int32 InZoomLevel) const override
	{
		return EGraphRenderingLOD::DefaultDetail;
	}
};

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
class ST4WorldMapGrid 
	: public SNodePanel
{
public:
	SLATE_BEGIN_ARGS(ST4WorldMapGrid) {}
		SLATE_ARGUMENT(TSharedPtr<FT4WorldTileCollectionModel>, InWorldModel)
	SLATE_END_ARGS()

	ST4WorldMapGrid()
		: CommandList(MakeShareable(new FUICommandList))
		, bHasScrollToRequest(false)
		, bHasScrollByRequest(false)
		, bIsFirstTickCall(true)
		, bHasNodeInteraction(true)
		, BoundsSnappingDistance(20.f)
		, WorldMouseDownPosition(FVector::ZeroVector) // #90
		, bActorDraw(false) // #104
		, ActorDrawLocation(FVector::ZeroVector) // #104
		, ActorDrawArea(FBox()) // #104
	{
	}

	~ST4WorldMapGrid()
	{
		WorldModel->SelectionChanged.RemoveAll(this);
		WorldModel->CollectionChanged.RemoveAll(this);

		FCoreDelegates::PreWorldOriginOffset.RemoveAll(this);
	}

	void Construct(const FArguments& InArgs)
	{
		ZoomLevels = MakeUnique<FWorldZoomLevelsContainer>();

		SNodePanel::Construct();

		// otherwise tiles will be drawn outside of this widget area
		SetClipping(EWidgetClipping::ClipToBounds);

		//
		WorldModel = InArgs._InWorldModel;

		bUpdatingSelection = false;
	
		WorldModel->SelectionChanged.AddSP(this, &ST4WorldMapGrid::UpdateSelection);
		WorldModel->CollectionChanged.AddSP(this, &ST4WorldMapGrid::RefreshView);
		SelectionManager.OnSelectionChanged.BindSP(this, &ST4WorldMapGrid::OnSelectionChanged);

		FCoreDelegates::PreWorldOriginOffset.AddSP(this, &ST4WorldMapGrid::PreWorldOriginOffset);

		int32 TileThumbnailSize = DefaultTileThumbnailSize; // #91
		int32 TileThumbnailAtlasSize = DefaultTileThumbnailAtlasSize;
		WorldModel->GetTileThumbnailSize(TileThumbnailSize, TileThumbnailAtlasSize);
		ThumbnailCollection = MakeShareable(new FT4TileThumbnailCollection(TileThumbnailSize, TileThumbnailAtlasSize));
	
		RefreshView();
	}
	
	/**  Add specified item to the grid view */
	void AddItem(TSharedPtr<FT4WorldTileModel> LevelModel)
	{
		auto NewNode = SNew(ST4WorldTileItem)
							.InWorldModel(WorldModel)
							.InItemModel(LevelModel)
							.InThumbnailCollection(ThumbnailCollection);
	
		AddGraphNode(NewNode);
	}
	
	/**  Remove specified item from the grid view */
	void RemoveItem(TSharedPtr<FT4LevelModel> LevelModel)
	{
		TSharedRef<SNode>* pItem = NodeToWidgetLookup.Find(LevelModel->GetNodeObject());
		if (pItem == NULL)
		{
			return;
		}

		Children.Remove(*pItem);
		VisibleChildren.Remove(*pItem);
		NodeToWidgetLookup.Remove(LevelModel->GetNodeObject());
	}
		
	/**  Updates all the items in the grid view */
	void RefreshView()
	{
		RemoveAllNodes();

		FT4LevelModelList AllLevels = WorldModel->GetAllLevels();
		for (auto It = AllLevels.CreateConstIterator(); It; ++It)
		{
			AddItem(StaticCastSharedPtr<FT4WorldTileModel>(*It));
		}
	}
		
	void RefreshThumbnails() // #93
	{
		for (int32 ChildIndex = 0; ChildIndex < VisibleChildren.Num(); ++ChildIndex)
		{
			const auto Child = StaticCastSharedRef<ST4WorldTileItem>(VisibleChildren[ChildIndex]);
			Child->RequestRefresh();
		}
	}

	/**  SWidget interface */
	void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override
	{
		SNodePanel::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

		// scroll to world center on first open
		if (bIsFirstTickCall)
		{
			bIsFirstTickCall = false;
			ViewOffset-= AllottedGeometry.GetLocalSize()*0.5f/GetZoomAmount();
		}

		FVector2D CursorPosition = FSlateApplication::Get().GetCursorPos();

		// Update cached variables
		WorldMouseLocation = CursorToWorldPosition(AllottedGeometry, CursorPosition);
		WorldMarqueeSize = Marquee.Rect.GetSize()/AllottedGeometry.Scale;
			
		// Update streaming preview data
		const bool bShowPotentiallyVisibleLevels = FSlateApplication::Get().GetModifierKeys().IsAltDown() && 
													AllottedGeometry.IsUnderLocation(CursorPosition);
	
		WorldModel->UpdateStreamingPreview(WorldMouseLocation, bShowPotentiallyVisibleLevels);
			
		// deferred scroll and zooming requests
		if (bHasScrollToRequest || bHasScrollByRequest)
		{
			// zoom to
			if (RequestedAllowZoomIn)
			{
				RequestedAllowZoomIn = false;
				
				FVector2D SizeWithZoom = RequestedZoomArea*ZoomLevels->GetZoomAmount(ZoomLevel);
				
				if (SizeWithZoom.X >= AllottedGeometry.GetLocalSize().X ||
					SizeWithZoom.Y >= AllottedGeometry.GetLocalSize().Y)
				{
					// maximum zoom out by default
					ZoomLevel = ZoomLevels->GetDefaultZoomLevel();
					// expand zoom area little bit, so zooming will fit original area not so tight
					RequestedZoomArea*= 1.2f;
					// find more suitable zoom value
					for (int32 Zoom = 0; Zoom < ZoomLevels->GetDefaultZoomLevel(); ++Zoom)
					{
						SizeWithZoom = RequestedZoomArea*ZoomLevels->GetZoomAmount(Zoom);
						if (SizeWithZoom.X >= AllottedGeometry.GetLocalSize().X || SizeWithZoom.Y >= AllottedGeometry.GetLocalSize().Y)
						{
							ZoomLevel = Zoom;
							break;
						}
					}
				}
			}

			// scroll to
			if (bHasScrollToRequest)
			{
				bHasScrollToRequest = false;
				ViewOffset = RequestedScrollToValue - AllottedGeometry.GetLocalSize() * 0.5f / GetZoomAmount();
			}

			// scroll by
			if (bHasScrollByRequest)
			{
				bHasScrollByRequest = false;
				ViewOffset += RequestedScrollByValue;
			}
		}
	}
	
	/**  SWidget interface */
	virtual void OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const override
	{
		for (int32 ChildIndex=0; ChildIndex < VisibleChildren.Num(); ++ChildIndex)
		{
			const auto Child = StaticCastSharedRef<ST4WorldTileItem>(VisibleChildren[ChildIndex]);
			const EVisibility ChildVisibility = Child->GetVisibility();

			if (ArrangedChildren.Accepts(ChildVisibility))
			{
				FVector2D ChildPos = Child->GetPosition();
					
				ArrangedChildren.AddWidget(ChildVisibility,
					AllottedGeometry.MakeChild(Child,
					ChildPos - GetViewOffset(),
					Child->GetDesiredSize(), GetZoomAmount()
				));
			}
		}
	}

	/**  SWidget interface */
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		// First paint the background
		{
			LayerId = PaintBackground(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
			LayerId++;
		}

		FArrangedChildren ArrangedChildren(EVisibility::Visible);
		ArrangeChildren(AllottedGeometry, ArrangedChildren);

		// Draw the child nodes

		// When drawing a marquee, need a preview of what the selection will be.
		const auto* SelectionToVisualize = &(SelectionManager.SelectedNodes);
		FGraphPanelSelectionSet SelectionPreview;
		if (Marquee.IsValid())
		{
			ApplyMarqueeSelection(Marquee, SelectionManager.SelectedNodes, SelectionPreview);
			SelectionToVisualize = &SelectionPreview;
		}

		int32 NodesLayerId = LayerId;

		for (int32 ChildIndex = 0; ChildIndex < ArrangedChildren.Num(); ++ChildIndex)
		{
			FArrangedWidget& CurWidget = ArrangedChildren[ChildIndex];
			TSharedRef<ST4WorldTileItem> ChildNode = StaticCastSharedRef<ST4WorldTileItem>(CurWidget.Widget);

			ChildNode->bAffectedByMarquee = SelectionToVisualize->Contains(ChildNode->GetObjectBeingDisplayed());
			LayerId = CurWidget.Widget->Paint(Args.WithNewParent(this), CurWidget.Geometry, MyCullingRect, OutDrawElements, NodesLayerId, InWidgetStyle, ShouldBeEnabled(bParentEnabled));
			ChildNode->bAffectedByMarquee = false;
		}

		// Draw editable world bounds
		if (!WorldModel->IsSimulating())
		{
			float ScreenSpaceSize = FT4LevelCollectionModel::EditableAxisLength() * GetZoomAmount() * 2.f;
			FVector2D PaintSize = FVector2D(ScreenSpaceSize, ScreenSpaceSize);
			FVector2D PaintPosition = GraphCoordToPanelCoord(FVector2D::ZeroVector) - (PaintSize * 0.5f);
			float Scale = 0.2f; // Scale down drawing border
			FSlateLayoutTransform LayoutTransform(Scale, AllottedGeometry.GetAccumulatedLayoutTransform().GetTranslation() + PaintPosition);
			FSlateRenderTransform SlateRenderTransform(Scale, AllottedGeometry.GetAccumulatedRenderTransform().GetTranslation() + PaintPosition);
			FPaintGeometry EditableArea(LayoutTransform, SlateRenderTransform, PaintSize / Scale, !SlateRenderTransform.IsIdentity());

			FLinearColor PaintColor = FLinearColor::Yellow;
			PaintColor.A = 0.4f;

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				++LayerId,
				EditableArea,
				FEditorStyle::GetBrush(TEXT("Graph.CompactNode.ShadowSelected")),
				ESlateDrawEffect::None,
				PaintColor
			);
		}

		// Draw the marquee selection rectangle
		PaintMarquee(AllottedGeometry, MyCullingRect, OutDrawElements, ++LayerId);

		// Draw the software cursor
		PaintSoftwareCursor(AllottedGeometry, MyCullingRect, OutDrawElements, ++LayerId);

		if (bActorDraw) // #104
		{
			const FVector AreaExtent = ActorDrawArea.GetExtent();
			const FVector2D AreaSize = FVector2D(AreaExtent.X, AreaExtent.Y) * GetZoomAmount() * 2.f;
			FVector2D PaintPosition = GraphCoordToPanelCoord(FVector2D(ActorDrawLocation.X, ActorDrawLocation.Y)) - (AreaSize * 0.5f);
			float Scale = 0.5f; // Scale down drawing border
			FSlateLayoutTransform LayoutTransform(Scale, AllottedGeometry.GetAccumulatedLayoutTransform().GetTranslation() + PaintPosition);
			FSlateRenderTransform SlateRenderTransform(Scale, AllottedGeometry.GetAccumulatedRenderTransform().GetTranslation() + PaintPosition);
			FPaintGeometry EditableArea(LayoutTransform, SlateRenderTransform, AreaSize / Scale, !SlateRenderTransform.IsIdentity());

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				++LayerId,
				EditableArea,
				FEditorStyle::GetBrush(TEXT("Graph.CompactNode.ShadowSelected")),
				ESlateDrawEffect::None,
				FLinearColor::Red
			);
		}

		{
			// #92 : MapZoneVolume 을 표시해준다.
			if (WorldModel->IsSimulating()) // #92
			{
				UWorld* PreviewWorld = WorldModel->GetSimulationWorld();
				check(nullptr != PreviewWorld);
				TArray<AT4MapZoneVolume*> MapZoneVolumes;
				T4EngineUtility::GetMapZomeVolumesOnWorld(PreviewWorld, MapZoneVolumes);
				for (AT4MapZoneVolume* ZoneVolume : MapZoneVolumes)
				{
					check(nullptr != ZoneVolume);
					if (0.0f < ZoneVolume->GetBlendWeight())
					{
						PaintMapZoneVolume(AllottedGeometry, OutDrawElements, LayerId + ZoneVolume->BlendPriority, ZoneVolume); // #92
					}
				}
			}
			else if (0 < WorldMapZoneNames.Num()) // #92
			{
				UWorld* EditorWorld = WorldModel->GetWorld();
				check(nullptr != EditorWorld);
				TArray<AT4MapZoneVolume*> MapZoneVolumes;
				T4EngineUtility::GetMapZomeVolumesOnWorld(EditorWorld, MapZoneVolumes);
				for (AT4MapZoneVolume* ZoneVolume : MapZoneVolumes)
				{
					check(nullptr != ZoneVolume);
					auto Predicate = [&](FName InMapZoneName)
					{
						return (ZoneVolume->ZoneName == InMapZoneName);
					};
					if (const FName* const FoundMapZoneName = WorldMapZoneNames.FindByPredicate(Predicate))
					{
						PaintMapZoneVolume(AllottedGeometry, OutDrawElements, LayerId + ZoneVolume->BlendPriority, ZoneVolume); // #92
					}
				}
			}
		}

		LayerId += 5; // #93 : Max ZoneVolume->BlendPriority 위에 찍기 위한 조치

		if(WorldModel->IsSimulating())
		{
			// Draw a surrounding indicator when PIE is active, to make it clear that the graph is read-only, etc...
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				FEditorStyle::GetBrush(TEXT("Graph.PlayInEditor"))
			);
		}

		// Draw observer location
		{
			FVector ObserverPosition;
			FRotator ObserverRotation;
			if (WorldModel->GetObserverView(ObserverPosition, ObserverRotation))
			{
				FVector2D ObserverPositionScreen = GraphCoordToPanelCoord(FVector2D(ObserverPosition.X, ObserverPosition.Y));
				const FSlateBrush* CameraImage = FEditorStyle::GetBrush(TEXT("WorldBrowser.SimulationViewPositon"));

				FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(
					ObserverPositionScreen - CameraImage->ImageSize*0.5f, 
					CameraImage->ImageSize
				);

				FSlateDrawElement::MakeRotatedBox(
					OutDrawElements,
					++LayerId,
					PaintGeometry,
					CameraImage,
					ESlateDrawEffect::None,
					FMath::DegreesToRadians(ObserverRotation.Yaw),
					CameraImage->ImageSize*0.5f,
					FSlateDrawElement::RelativeToElement
				);
			}

			const FSlateBrush* TargetPointImage = FEditorStyle::GetBrush(TEXT("WorldBrowser.AddLayer"));

			// #104
			{
				TArray<FVector2D> GameObjectLocations;
				if (WorldModel->GetGameObjectLocations(GameObjectLocations))
				{
					for (const FVector2D& TargetLocation : GameObjectLocations)
					{
						FVector2D TargetPositionScreen = GraphCoordToPanelCoord(TargetLocation);
						FPaintGeometry PaintAtGeometry = AllottedGeometry.ToPaintGeometry(
							TargetPositionScreen - TargetPointImage->ImageSize * 0.5f,
							TargetPointImage->ImageSize
						);
						FSlateDrawElement::MakeRotatedBox(
							OutDrawElements,
							++LayerId,
							PaintAtGeometry,
							TargetPointImage,
							ESlateDrawEffect::None,
							FMath::DegreesToRadians(45.0f),
							TargetPointImage->ImageSize * 0.5f,
							FSlateDrawElement::RelativeToElement,
							FLinearColor(FColorList::YellowGreen)
						);
					}
				}
			}

			FVector CameraAtPosition = FVector::ZeroVector; // #93
			FVector CameraEyePosition;
			FRotator CameraRotation;
			if (WorldModel->GetPlayerView(CameraEyePosition, CameraRotation, CameraAtPosition))
			{
				FVector2D CameraPositionScreen = GraphCoordToPanelCoord(FVector2D(CameraEyePosition.X, CameraEyePosition.Y));
				const FSlateBrush* CameraImage = FEditorStyle::GetBrush(TEXT("WorldBrowser.SimulationViewPositon"));
	
				FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(
					CameraPositionScreen - CameraImage->ImageSize*0.5f,
					CameraImage->ImageSize
				);

				FSlateDrawElement::MakeRotatedBox(
					OutDrawElements,
					++LayerId,
					PaintGeometry,
					CameraImage,
					ESlateDrawEffect::None,
					FMath::DegreesToRadians(CameraRotation.Yaw),
					CameraImage->ImageSize*0.5f,
					FSlateDrawElement::RelativeToElement,
					FLinearColor(FColorList::Orange)
				);

				if (!CameraAtPosition.IsNearlyZero())
				{
					// #93 : Env 는 Player 위치로 변함으로 위치를 찍어준다.
					FVector2D PlayerPositionScreen = GraphCoordToPanelCoord(FVector2D(CameraAtPosition.X, CameraAtPosition.Y));
					FPaintGeometry PaintAtGeometry = AllottedGeometry.ToPaintGeometry(
						PlayerPositionScreen - TargetPointImage->ImageSize * 0.5f,
						TargetPointImage->ImageSize
					);
					FSlateDrawElement::MakeRotatedBox(
						OutDrawElements,
						++LayerId,
						PaintAtGeometry,
						TargetPointImage,
						ESlateDrawEffect::None,
						FMath::DegreesToRadians(45.0f),
						TargetPointImage->ImageSize * 0.5f,
						FSlateDrawElement::RelativeToElement,
						FLinearColor(FColorList::OrangeRed)
					);
				}
			}
		}

		LayerId = PaintScaleRuler(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
		return LayerId;
	}
		
	/** SWidget interface */
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown( EKeys::RightMouseButton );
		const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown( EKeys::LeftMouseButton );
		const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);

		PastePosition = PanelCoordToGraphCoord( MyGeometry.AbsoluteToLocal( MouseEvent.GetScreenSpacePosition() ) );

		if ( this->HasMouseCapture() )
		{
			const FVector2D CursorDelta = MouseEvent.GetCursorDelta();
			// Track how much the mouse moved since the mouse down.
			TotalMouseDelta += CursorDelta.Size();

			// Zooming - drop through to default behaviour
			if(bIsRightMouseButtonDown && (bIsLeftMouseButtonDown || bIsMiddleMouseButtonDown || FSlateApplication::Get().GetModifierKeys().IsAltDown() || FSlateApplication::Get().IsUsingTrackpad()))
			{
				return SNodePanel::OnMouseMove(MyGeometry, MouseEvent);
			}
			else if (bIsRightMouseButtonDown || bIsMiddleMouseButtonDown)
			{
				FReply ReplyState = FReply::Handled();

				if( !CursorDelta.IsZero() )
				{
					bShowSoftwareCursor = true;
				}

				// Panning and mouse is outside of panel? Pasting should just go to the screen center.
				PastePosition = PanelCoordToGraphCoord( 0.5 * MyGeometry.GetLocalSize() );

				this->bIsPanning = true;
				ViewOffset -= CursorDelta / GetZoomAmount();

				return ReplyState;
			}
			else if (bIsLeftMouseButtonDown)
			{
				TSharedPtr<SNode> NodeBeingDragged = NodeUnderMousePtr.Pin();

				if ( IsEditable.Get() )
				{
					// Update the amount to pan panel
					UpdateViewOffset(MyGeometry, MouseEvent.GetScreenSpacePosition());

					const bool bCursorInDeadZone = TotalMouseDelta <= FSlateApplication::Get().GetDragTriggerDistance();

					if ( NodeBeingDragged.IsValid() )
					{
						if ( !bCursorInDeadZone )
						{
							// Note, NodeGrabOffset() comes from the node itself, so it's already scaled correctly.
							FVector2D AnchorNodeNewPos = PanelCoordToGraphCoord( MyGeometry.AbsoluteToLocal( MouseEvent.GetScreenSpacePosition() ) ) - NodeGrabOffset;

							// Dragging an unselected node automatically selects it.
							SelectionManager.StartDraggingNode(NodeBeingDragged->GetObjectBeingDisplayed(), MouseEvent);

							// Move all the selected nodes.
							{
								const FVector2D AnchorNodeOldPos = NodeBeingDragged->GetPosition();
								const FVector2D DeltaPos = AnchorNodeNewPos - AnchorNodeOldPos;
								if (DeltaPos.SizeSquared() > FMath::Square(KINDA_SMALL_NUMBER))
								{
									MoveSelectedNodes(NodeBeingDragged, AnchorNodeNewPos);
								}
							}
						}

						return FReply::Handled();
					}
				}

				if ( !NodeBeingDragged.IsValid() )
				{
					// We are marquee selecting
					const FVector2D GraphMousePos = PanelCoordToGraphCoord( MyGeometry.AbsoluteToLocal( MouseEvent.GetScreenSpacePosition() ) );
					Marquee.Rect.UpdateEndPoint(GraphMousePos);

					FindNodesAffectedByMarquee( /*out*/ Marquee.AffectedNodes );
					return FReply::Handled();
				}
			}
		}

		return FReply::Unhandled();
	}

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override // #90
	{
		WorldMouseDownPosition = PanelCoordToGraphCoord(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
		return SNodePanel::OnMouseButtonDown(MyGeometry, MouseEvent);
	}

	FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		// We want to zoom into this point; i.e. keep it the same fraction offset into the panel
		const FVector2D WidgetSpaceCursorPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		const int32 ZoomLevelDelta = FMath::TruncToInt(FMath::RoundFromZero(MouseEvent.GetWheelDelta()));
		ChangeZoomLevel(ZoomLevelDelta, WidgetSpaceCursorPos, true); // #90 : Control 에 관계없이 최대 줌 인으로 갈 수 있도록 처리. MouseEvent.IsControlDown())

		// Stop the zoom-to-fit in favor of user control
		CancelZoomToFit();

		return FReply::Handled();
	}

	void OnUpdateSelection() // #86
	{
		UpdateSelection(); // #86&
	}

	/** @return Size of a marquee rectangle in world space */
	FVector2D GetMarqueeWorldSize() const
	{
		return WorldMarqueeSize;
	}

	/** @return Mouse cursor position in world space */
	FVector2D GetMouseWorldLocation() const
	{
		return WorldMouseLocation;
	}
	
	FVector2D GetMouseDownWorldLocation() const // #90
	{
		return WorldMouseDownPosition;
	}

	void RequestScrollTo(const FVector& InLocation, const FBox& InArea) // #90
	{
		const FVector TargetExtent = InArea.GetExtent();
		FVector2D TargetPosition = FVector2D(InLocation.X, InLocation.Y);
		FVector2D TargetArea = FVector2D(TargetExtent.X, TargetExtent.Y);
		FSlateRect SelectionRect = FSlateRect(GraphCoordToPanelCoord(TargetPosition - TargetArea), GraphCoordToPanelCoord(TargetPosition + TargetArea));
		FSlateRect PanelRect = FSlateRect(FVector2D::ZeroVector, CachedGeometry.GetLocalSize());
		bool bIsVisible = FSlateRect::DoRectanglesIntersect(PanelRect, SelectionRect);
		if (!bIsVisible)
		{
			RequestScrollTo(TargetPosition, TargetArea); // #90 : 화면에서 보이지 않을 경우에만 Grid 이동!
		}
	}

	void SetActorSelected(const FVector& InLocation, const FBox& InArea) // #104
	{
		bActorDraw = true;
		ActorDrawLocation = InLocation;
		ActorDrawArea = InArea;
	}

	void SetMapZoneSelected(FName InMapZoneName) // #92
	{
		WorldMapZoneNames.Empty();
		WorldMapZoneNames.Add(InMapZoneName);
	}

protected:
	/**  Draws background for grid view */
	uint32 PaintBackground(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, uint32 LayerId) const
	{
		FVector2D ScreenWorldOrigin = GraphCoordToPanelCoord(FVector2D(0, 0));
		FSlateRect ScreenRect(FVector2D(0, 0), AllottedGeometry.GetLocalSize());
	
		// World Y-axis
		if (ScreenWorldOrigin.X > ScreenRect.Left &&
			ScreenWorldOrigin.X < ScreenRect.Right)
		{
			TArray<FVector2D> LinePoints;
			LinePoints.Add(FVector2D(ScreenWorldOrigin.X, ScreenRect.Top));
			LinePoints.Add(FVector2D(ScreenWorldOrigin.X, ScreenRect.Bottom));

			FLinearColor YAxisColor = FLinearColor::Green;
			YAxisColor.A = 0.4f;
		
			FSlateDrawElement::MakeLines( 
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				LinePoints,
				ESlateDrawEffect::None,
				YAxisColor);
		}

		// World X-axis
		if (ScreenWorldOrigin.Y > ScreenRect.Top &&
			ScreenWorldOrigin.Y < ScreenRect.Bottom)
		{
			TArray<FVector2D> LinePoints;
			LinePoints.Add(FVector2D(ScreenRect.Left, ScreenWorldOrigin.Y));
			LinePoints.Add(FVector2D(ScreenRect.Right, ScreenWorldOrigin.Y));

			FLinearColor XAxisColor = FLinearColor::Red;
			XAxisColor.A = 0.4f;
		
			FSlateDrawElement::MakeLines( 
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				LinePoints,
				ESlateDrawEffect::None,
				XAxisColor);
		}

		return LayerId + 1;
	}

	/**  Draws current scale */
	uint32 PaintScaleRuler(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, uint32 LayerId) const
	{
		const float	ScaleRulerLength = 100.f; // pixels
		TArray<FVector2D> LinePoints;
		LinePoints.Add(FVector2D::ZeroVector);
		LinePoints.Add(FVector2D::ZeroVector + FVector2D(ScaleRulerLength, 0.f));
	
		FSlateDrawElement::MakeLines( 
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToOffsetPaintGeometry(FVector2D(10, 40)),
			LinePoints,
			ESlateDrawEffect::None,
			FColor(200, 200, 200));

		const float UnitsInRuler = ScaleRulerLength/GetZoomAmount();// Pixels to world units
		const int32 UnitsInMeter = 100;
		const int32 UnitsInKilometer = UnitsInMeter*1000;
	
		FString RulerText;
		if (UnitsInRuler > UnitsInKilometer) // in kilometers
		{
			RulerText = FString::Printf(TEXT("%.2f km"), UnitsInRuler/UnitsInKilometer);
		}
		else // in meters
		{
			RulerText = FString::Printf(TEXT("%.2f m"), UnitsInRuler/UnitsInMeter);
		}
	
		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToOffsetPaintGeometry(FVector2D(10, 27)),
			RulerText,
			FEditorStyle::GetFontStyle("NormalFont"),
			ESlateDrawEffect::None,
			FColor(200, 200, 200)
		);
		
		return LayerId + 1;
	}
	
	// #92
	void PaintMapZoneVolume(
		const FGeometry& AllottedGeometry, 
		FSlateWindowElementList& OutDrawElements, 
		int32 LayerId, 
		AT4MapZoneVolume* InZoneVolume
	) const
	{
		check(nullptr != InZoneVolume);

		const FColor FillColor = InZoneVolume->GetPaintColor();
		const FTransform& ZoneTransform = InZoneVolume->GetTransform();

		UModel* BrushModel = InZoneVolume->Brush;
		if (nullptr != BrushModel)
		{
			int32 NumBspNodes = BrushModel->Nodes.Num();
			if (0 < NumBspNodes)
			{
				const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

				FSlateResourceHandle ResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*WhiteBrush); // #4.24
				const FSlateShaderResourceProxy* ResourceProxy = ResourceHandle.GetResourceProxy(); // #4.24

				FVector2D AtlasOffset = ResourceProxy ? ResourceProxy->StartUV : FVector2D(0.0f, 0.0f);
				FVector2D AtlasUVSize = ResourceProxy ? ResourceProxy->SizeUV : FVector2D(1.0f, 1.0f);

				FSlateRenderTransform DrawRenderTransform;

				const FVector2D Pos = AllottedGeometry.GetAbsolutePosition();
				const float Scale = AllottedGeometry.Scale;

				FVector2D TestCenterScreen(0.0f, 0.0f);
				TArray<FSlateVertex> Verts;
				for (int32 Idx = 0; Idx < BrushModel->Points.Num(); ++Idx)
				{
					const FVector& Vertex = BrushModel->Points[Idx];
					const FVector WorldVertex = ZoneTransform.TransformPosition(Vertex);
					FVector2D PositionScreen = GraphCoordToPanelCoord(FVector2D(WorldVertex.X, WorldVertex.Y));
					FVector2D LocalPosition = (Pos + FVector2D(PositionScreen.X, PositionScreen.Y)) * Scale;
					Verts.Add(
						FSlateVertex::Make<ESlateVertexRounding::Disabled>(
							DrawRenderTransform,
							LocalPosition,
							AtlasOffset + FVector2D(0.5f, 0.5f) * AtlasUVSize,
							FillColor
						)
					);
					TestCenterScreen += PositionScreen;
				}

				TestCenterScreen = TestCenterScreen / BrushModel->Points.Num();

				TArray<SlateIndex> Indices;

				int FaceCount = 0;
				for (FBspNode& WorldNode : BrushModel->Nodes)
				{
					if (WorldNode.NumVertices <= 2)
					{
						continue;
					}

					int32 Index0 = BrushModel->Verts[WorldNode.iVertPool + 0].pVertex;
					int32 Index1 = BrushModel->Verts[WorldNode.iVertPool + 1].pVertex;
					int32 Index2;

					for (auto v = 2; v < WorldNode.NumVertices; ++v)
					{
						Index2 = BrushModel->Verts[WorldNode.iVertPool + v].pVertex;
						Indices.Add(Index0);
						Indices.Add(Index2);
						Indices.Add(Index1);
						Index1 = Index2;

						++FaceCount;
					}
				}

				FSlateDrawElement::MakeCustomVerts(
					OutDrawElements,
					LayerId,
					ResourceHandle,
					Verts,
					Indices,
					nullptr,
					0,
					0
				);

				if (!InZoneVolume->IsGlobalZone())
				{
					FSlateFontInfo FontStyle = FEditorStyle::GetFontStyle("NormalFont");
					const FVector2D TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(InZoneVolume->ZoneName.ToString(), FontStyle);

					FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(
						FVector2D(TestCenterScreen) - (TextSize * 0.5f),
						TextSize
					);

					FSlateDrawElement::MakeText(
						OutDrawElements,
						LayerId,
						PaintGeometry,
						InZoneVolume->ZoneName.ToString(),
						FontStyle,
						ESlateDrawEffect::None,
						FColor::Black
					);
				}
			}
		}
	}

	/**  SWidget interface */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (CommandList->ProcessCommandBindings(InKeyEvent))
		{
			return FReply::Handled();
		}
	
		if (WorldModel->GetCommandList()->ProcessCommandBindings(InKeyEvent))
		{
			return FReply::Handled();
		}

		return SNodePanel::OnKeyDown(MyGeometry, InKeyEvent);
	}

	/**  SWidget interface */
	bool SupportsKeyboardFocus() const override
	{
		return true;
	}
	
	/**  SNodePanel interface */
	TSharedPtr<SWidget> OnSummonContextMenu(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (WorldModel->IsReadOnly())
		{
			return SNullWidget::NullWidget;
		}

		FArrangedChildren ArrangedChildren(EVisibility::Visible);
		ArrangeChildren(MyGeometry, ArrangedChildren);

		const int32 NodeUnderMouseIndex = SWidget::FindChildUnderMouse( ArrangedChildren, MouseEvent );
		if (NodeUnderMouseIndex != INDEX_NONE)
		{
			// PRESSING ON A NODE!
			const FArrangedWidget& NodeGeometry = ArrangedChildren[NodeUnderMouseIndex];
			const FVector2D MousePositionInNode = NodeGeometry.Geometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			TSharedRef<SNode> NodeWidgetUnderMouse = StaticCastSharedRef<SNode>( NodeGeometry.Widget );

			if (NodeWidgetUnderMouse->CanBeSelected(MousePositionInNode))
			{
				if (!SelectionManager.IsNodeSelected(NodeWidgetUnderMouse->GetObjectBeingDisplayed()))
				{
					SelectionManager.SelectSingleNode(NodeWidgetUnderMouse->GetObjectBeingDisplayed());
				}

			}
		}
		else
		{
			SelectionManager.ClearSelectionSet();
		}
	
		// Summon context menu
		FMenuBuilder MenuBuilder(true, WorldModel->GetCommandList());
		WorldModel->BuildWorldMapMenu(MenuBuilder);
		TSharedPtr<SWidget> MenuWidget = MenuBuilder.MakeWidget();

		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();

		FSlateApplication::Get().PushMenu(
			AsShared(),
			WidgetPath,
			MenuWidget.ToSharedRef(),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect( FPopupTransitionEffect::ContextMenu )
			);

		return MenuWidget;
	}
	
	/**  SNodePanel interface */
	virtual void PopulateVisibleChildren(const FGeometry& AllottedGeometry) override
	{
		VisibleChildren.Empty();

		FSlateRect PanelRect(FVector2D(0, 0), AllottedGeometry.GetLocalSize());
		FVector2D ViewStartPos = PanelCoordToGraphCoord(FVector2D(PanelRect.Left, PanelRect.Top));
		FVector2D ViewEndPos = PanelCoordToGraphCoord(FVector2D(PanelRect.Right, PanelRect.Bottom));
		FSlateRect ViewRect(ViewStartPos, ViewEndPos);

		for (int32 ChildIndex=0; ChildIndex < Children.Num(); ++ChildIndex)
		{
			const auto Child = StaticCastSharedRef<ST4WorldTileItem>(Children[ChildIndex]);
			const auto LevelModel = Child->GetLevelModel();
			if (LevelModel->IsVisibleInCompositionView())
			{
				FSlateRect ChildRect = Child->GetItemRect();
				FVector2D ChildSize = ChildRect.GetSize();

				if (ChildSize.X > 0.f && 
					ChildSize.Y > 0.f && 
					FSlateRect::DoRectanglesIntersect(ChildRect, ViewRect))
				{
					VisibleChildren.Add(Child);
				}
			}
		}

		// Sort tiles such that smaller and selected tiles will be drawn on top of other tiles
		struct FVisibleTilesSorter
		{
			FVisibleTilesSorter(const FT4LevelCollectionModel& InWorldModel) : WorldModel(InWorldModel)
			{}
			bool operator()(const TSharedRef<SNodePanel::SNode>& A,
							const TSharedRef<SNodePanel::SNode>& B) const
			{
				TSharedRef<ST4WorldTileItem> ItemA = StaticCastSharedRef<ST4WorldTileItem>(A);
				TSharedRef<ST4WorldTileItem> ItemB = StaticCastSharedRef<ST4WorldTileItem>(B);
				return WorldModel.CompareLevelsZOrder(ItemA->GetLevelModel(), ItemB->GetLevelModel());
			}
			const FT4LevelCollectionModel& WorldModel;
		};

		VisibleChildren.Sort(FVisibleTilesSorter(*WorldModel.Get()));
	}

	/**  SNodePanel interface */
	virtual void OnBeginNodeInteraction(const TSharedRef<SNode>& InNodeToDrag, const FVector2D& GrabOffset) override
	{
		bHasNodeInteraction = true;
		SNodePanel::OnBeginNodeInteraction(InNodeToDrag, GrabOffset);
	}
	
	/**  SNodePanel interface */
	virtual void OnEndNodeInteraction(const TSharedRef<SNode>& InNodeDragged) override
	{
		const ST4WorldTileItem& Item = static_cast<const ST4WorldTileItem&>(InNodeDragged.Get());
		if (Item.IsItemEditable() && !WorldModel->IsLockTilesLocationEnabled())
		{
			FVector2D AbsoluteDelta = Item.GetLevelModel()->GetLevelTranslationDelta();
			FIntPoint IntAbsoluteDelta = FIntPoint(AbsoluteDelta.X, AbsoluteDelta.Y);

			// Reset stored translation delta to 0
			WorldModel->UpdateTranslationDelta(WorldModel->GetSelectedLevels(), FVector2D::ZeroVector, false, 0.f);
	
			// In case we have non zero dragging delta, translate selected levels 
			if (IntAbsoluteDelta != FIntPoint::ZeroValue)
			{
				WorldModel->TranslateLevels(WorldModel->GetSelectedLevels(), IntAbsoluteDelta);
			}
		}
	
		bHasNodeInteraction = false;

		SNodePanel::OnEndNodeInteraction(InNodeDragged);
	}

	/** Handles selection changes in the grid view */
	void OnSelectionChanged(const FGraphPanelSelectionSet& SelectedNodes)
	{
		if (bUpdatingSelection)
		{
			return;
		}
	
		bUpdatingSelection = true;
		FT4LevelModelList SelectedLevels;
	
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			TSharedRef<SNode>* pWidget = NodeToWidgetLookup.Find(*NodeIt);
			if (pWidget != NULL)
			{
				TSharedRef<ST4WorldTileItem> Item = StaticCastSharedRef<ST4WorldTileItem>(*pWidget);
				SelectedLevels.Add(Item->GetLevelModel());
			}
		}
	
		WorldModel->SetSelectedLevels(SelectedLevels);
		bUpdatingSelection = false;
	}
	
	/** Handles selection changes in data source */
	void UpdateSelection()
	{
		if (bUpdatingSelection)
		{
			return;
		}

		bUpdatingSelection = true;

		SelectionManager.ClearSelectionSet();
		FT4LevelModelList SelectedLevels = WorldModel->GetSelectedLevels();
		for (auto It = SelectedLevels.CreateConstIterator(); It; ++It)
		{
			SelectionManager.SetNodeSelection((*It)->GetNodeObject(), true);
		}

		if (SelectionManager.AreAnyNodesSelected())
		{
			FVector2D MinCorner, MaxCorner;
			if (GetBoundsForNodes(true, MinCorner, MaxCorner, 0.f))
			{
				FSlateRect SelectionRect = FSlateRect(GraphCoordToPanelCoord(MinCorner), GraphCoordToPanelCoord(MaxCorner));
				FSlateRect PanelRect = FSlateRect(FVector2D::ZeroVector, CachedGeometry.GetLocalSize());
				bool bIsVisible = FSlateRect::DoRectanglesIntersect(PanelRect, SelectionRect);
				if (!bIsVisible)
				{
					FVector2D TargetPosition = MaxCorner/2.f + MinCorner/2.f;
					RequestScrollTo(TargetPosition, MaxCorner - MinCorner);
				}
			}
		}
		bUpdatingSelection = false;
	}

	/** Delegate callback: world origin is going to be moved. */
	void PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
	{
		if (InWorld && (WorldModel->GetWorld() == InWorld || WorldModel->GetSimulationWorld() == InWorld))		
		{
			FIntVector Offset = InDstOrigin - InSrcOrigin;
			RequestScrollBy(-FVector2D(Offset.X, Offset.Y));
		}
	}
	
	/** Handles new item added to data source */
	void OnNewItemAdded(TSharedPtr<FT4LevelModel> NewItem)
	{
		RefreshView();
	}

	/**  FitToSelection command handler */
	void FitToSelection_Executed()
	{
		FVector2D MinCorner, MaxCorner;
		if (GetBoundsForNodes(true, MinCorner, MaxCorner, 0.f))
		{
			RequestScrollTo((MaxCorner + MinCorner)*0.5f, MaxCorner - MinCorner, true);
		}
	}
		
	/**  @returns Whether any of the levels are selected */
	bool AreAnyItemsSelected() const
	{
		return SelectionManager.AreAnyNodesSelected();
	}

	/**  Requests view scroll to specified position and fit to specified area 
	 *   @param	InLocation		The location to scroll to
	 *   @param	InArea			The area to fit in view
	 *   @param	bAllowZoomIn	Is zoom in allowed during fit to area calculations
	 */
	void RequestScrollTo(FVector2D InLocation, FVector2D InArea, bool bAllowZoomIn = false)
	{
		bHasScrollToRequest = true;
		RequestedScrollToValue = InLocation;
		RequestedZoomArea = InArea;
		RequestedAllowZoomIn = bAllowZoomIn;
	}

	void RequestScrollBy(FVector2D InDelta)
	{
		bHasScrollByRequest = true;
		RequestedScrollByValue = InDelta;
	}
	
	/** Handlers for moving items using arrow keys */
	void MoveLevelLeft_Executed()
	{
		if (!bHasNodeInteraction)
		{
			WorldModel->TranslateLevels(WorldModel->GetSelectedLevels(), FIntPoint(-1, 0));
		}
	}

	void MoveLevelRight_Executed()
	{
		if (!bHasNodeInteraction)
		{
			WorldModel->TranslateLevels(WorldModel->GetSelectedLevels(), FIntPoint(+1, 0));
		}
	}

	void MoveLevelUp_Executed()
	{
		if (!bHasNodeInteraction)
		{
			WorldModel->TranslateLevels(WorldModel->GetSelectedLevels(), FIntPoint(0, -1));
		}
	}

	void MoveLevelDown_Executed()
	{
		if (!bHasNodeInteraction)
		{
			WorldModel->TranslateLevels(WorldModel->GetSelectedLevels(), FIntPoint(0, +1));
		}
	}	
	
	/** Moves selected nodes by specified offset */
	void MoveSelectedNodes(const TSharedPtr<SNode>& InNodeToDrag, FVector2D NewPosition)
	{
		auto ItemDragged = StaticCastSharedPtr<ST4WorldTileItem>(InNodeToDrag);
	
		if (ItemDragged->IsItemEditable() && !WorldModel->IsLockTilesLocationEnabled())
		{
			// Current translation snapping value
			float SnappingDistanceWorld = 0.f;
			const bool bBoundsSnapping = (FSlateApplication::Get().GetModifierKeys().IsControlDown() == false);
			if (bBoundsSnapping)
			{
				SnappingDistanceWorld = BoundsSnappingDistance/GetZoomAmount();
			}
			else if (GetDefault<ULevelEditorViewportSettings>()->GridEnabled)
			{
				SnappingDistanceWorld = GEditor->GetGridSize();
			}
		
			FVector2D StartPosition = ItemDragged->GetPosition() - ItemDragged->GetLevelModel()->GetLevelTranslationDelta();
			FVector2D AbsoluteDelta = NewPosition - StartPosition;

			WorldModel->UpdateTranslationDelta(WorldModel->GetSelectedLevels(), AbsoluteDelta, bBoundsSnapping, SnappingDistanceWorld);
		}
	}

	/**  Converts cursor absolute position to the world position */
	FVector2D CursorToWorldPosition(const FGeometry& InGeometry, FVector2D InAbsoluteCursorPosition)
	{
		FVector2D ViewSpacePosition = (InAbsoluteCursorPosition - InGeometry.AbsolutePosition)/InGeometry.Scale;
		return PanelCoordToGraphCoord(ViewSpacePosition);
	}

private:
	/** Levels data list to display*/
	TSharedPtr<FT4WorldTileCollectionModel>	WorldModel;

	/** Geometry cache */
	mutable FVector2D						CachedAllottedGeometryScaledSize;

	bool									bUpdatingSelection;
	TArray<FIntRect>						OccupiedCells;
	const TSharedRef<FUICommandList>		CommandList;

	bool									bHasScrollToRequest;
	bool									bHasScrollByRequest;
	FVector2D								RequestedScrollToValue;
	FVector2D								RequestedScrollByValue;
	FVector2D								RequestedZoomArea;
	bool									RequestedAllowZoomIn;

	bool									bIsFirstTickCall;
	// Is user interacting with a node now
	bool									bHasNodeInteraction;

	// Snapping distance in screen units for a tile bounds
	float									BoundsSnappingDistance;

	//
	// Mouse location in the world
	FVector2D								WorldMouseLocation;
	// Current marquee rectangle size in world units
	FVector2D								WorldMarqueeSize;
	// Thumbnail managment for tile items
	TSharedPtr<FT4TileThumbnailCollection>	ThumbnailCollection;

	FVector2D								WorldMouseDownPosition; // #90

	bool									bActorDraw; // #104
	FVector									ActorDrawLocation; // #104
	FBox									ActorDrawArea; // #104

	TArray<FName>							WorldMapZoneNames; // #92
};

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
ST4WorldMap::ST4WorldMap()
{
}

ST4WorldMap::~ST4WorldMap()
{
	Reset(); // #83
	WorldMapViewModelRef = nullptr; // #83
}

void ST4WorldMap::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(ContentParent, SBorder)
		.BorderImage(FEditorStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
	];
}

void ST4WorldMap::OnInitialize(FT4WorldMapViewModel* InWorldViewModel) // #90
{
	WorldMapViewModelRef = InWorldViewModel; // #83
	BrowseWorld();
}

void ST4WorldMap::Reset() // #83
{
	// Remove old world bindings
	ContentParent->SetContent(SNullWidget::NullWidget);
	LayersListWrapBox = nullptr;
	NewLayerButton = nullptr;
	NewLayerMenu.Reset();
	GridView = nullptr;
	TileWorldModel = nullptr;
}

void ST4WorldMap::OnRefresh() // #90
{
	if (!GridView.IsValid())
	{
		return;
	}
	BrowseWorld();
}

void ST4WorldMap::OnRefreshSelection() // #86
{
	if (GridView.IsValid())
	{
		GridView->OnUpdateSelection();
	}
}

void ST4WorldMap::OnUpdateThumbnails() // #93
{
	if (GridView.IsValid())
	{
		GridView->RefreshThumbnails();
	}
}

void ST4WorldMap::BrowseWorld()
{
	Reset(); // #83
			
	check(nullptr != WorldMapViewModelRef);
	TSharedPtr<FT4LevelCollectionModel> SharedWorldModel = WorldMapViewModelRef->GetWorldModelPtr(); // #83

	// double check we have a tile world
	//if (SharedWorldModel.IsValid() && SharedWorldModel->IsTileWorld())
	if (SharedWorldModel.IsValid()) // #91 : 단일 월드 지원!
	{
		TileWorldModel = StaticCastSharedPtr<FT4WorldTileCollectionModel>(SharedWorldModel);
		ContentParent->SetContent(ConstructContentWidget());
		PopulateLayersList();
	}
}

void ST4WorldMap::OnRequestScrollTo(const FVector& InLocation, const FBox& InArea) // #90
{
	if (GridView.IsValid())
	{
		GridView->RequestScrollTo(InLocation, InArea);
	}
}

void ST4WorldMap::OnSubLevelSelected(const TArray<FName>& InSubLevelNames) // #104
{
	FT4LevelModelList NewList;
	for (FName InLevelPackageName : InSubLevelNames)
	{
		TSharedPtr<FT4LevelModel> FoundLevelModelPtr = TileWorldModel->FindLevelModel(InLevelPackageName);
		if (FoundLevelModelPtr.IsValid())
		{
			NewList.Add(FoundLevelModelPtr);
		}
	}
	TileWorldModel->SetSelectedLevels(NewList);
}

void ST4WorldMap::OnEditorSubLevelLoad(const TArray<FName>& InSubLevelNames) // #104
{
	TileWorldModel->LoadEditorSubLevel(InSubLevelNames);
}

void ST4WorldMap::OnPreviewSubLevelLoad(const TArray<FName>& InSubLevelNames) // #104
{
	TileWorldModel->LoadPreviewSubLevel(InSubLevelNames);
}

void ST4WorldMap::OnPreviewSubLevelUnload(const TArray<FName>& InSubLevelNames) // #104
{
	TileWorldModel->UnloadPreviewSubLevel(InSubLevelNames);
}

void ST4WorldMap::OnActorSelected(
	const FVector& InLocation, 
	const FBox& InBoundingBox
) // #104
{
	if (GridView.IsValid())
	{
		GridView->SetActorSelected(InLocation, InBoundingBox);
	}
}

void ST4WorldMap::OnMapZoneSelected(FName InMapZoneName) // #92
{
	if (GridView.IsValid())
	{
		GridView->SetMapZoneSelected(InMapZoneName);
	}
}

FVector2D ST4WorldMap::GetMouseDownLocationInWorldSpace() const // #90
{
	if (!GridView.IsValid())
	{
		return FVector2D(0.0f, 0.0f);
	}
	return GridView->GetMouseDownWorldLocation();
}

TSharedRef<SWidget> ST4WorldMap::ConstructContentWidget()
{
	return 	
		SNew(SVerticalBox)

		// Layers list
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(LayersListWrapBox, SWrapBox)
			.UseAllottedWidth(true)
		]
				
		+SVerticalBox::Slot()
		.FillHeight(1)
		[
			SNew( SOverlay )

			// Grid view
			+SOverlay::Slot()
			[
				SAssignNew(GridView, ST4WorldMapGrid)
					.InWorldModel(TileWorldModel)
			]

			// Grid view top status bar
			+SOverlay::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush(TEXT("Graph.TitleBackground")))
				[
					SNew(SVerticalBox)

					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
							
						// Current world view scale
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNullWidget::NullWidget
						]
						+SHorizontalBox::Slot()
						.Padding(5,0,0,0)
						[
							SNullWidget::NullWidget
						]
							
						// World origin position
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush( "WorldBrowser.WorldOrigin" ))
						]
						+SHorizontalBox::Slot()
						.Padding(5,0,0,0)
						[
							SNew(STextBlock)
							.TextStyle( FEditorStyle::Get(), "WorldBrowser.StatusBarText" )
							.Text(this, &ST4WorldMap::GetCurrentOriginText)
						]

						// Current level
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Right)
						.Padding(0,0,5,0)
						[
							SNew(STextBlock)
							.TextStyle( FEditorStyle::Get(), "WorldBrowser.StatusBarText" )
							.Text(this, &ST4WorldMap::GetCurrentLevelText)
						]											
					]
				]
			]
			// Grid view bottom status bar
			+SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush(TEXT("Graph.TitleBackground")))
				[
					SNew(SVerticalBox)

					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Mouse location
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush( "WorldBrowser.MouseLocation" ))
						]
						+SHorizontalBox::Slot()
						.Padding(5,0,0,0)
						[
							SNew(STextBlock)
							.TextStyle( FEditorStyle::Get(), "WorldBrowser.StatusBarText" )
							.Text(this, &ST4WorldMap::GetMouseLocationText)
						]

						// Selection marquee rectangle size
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush( "WorldBrowser.MarqueeRectSize" ))
						]
						+SHorizontalBox::Slot()
						.Padding(5,0,0,0)
						[
							SNew(STextBlock)
							.TextStyle( FEditorStyle::Get(), "WorldBrowser.StatusBarText" )
							.Text(this, &ST4WorldMap::GetMarqueeSelectionSizeText)
						]

						// World size
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Right)
						[
							SNew(SHorizontalBox)

							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush( "WorldBrowser.WorldSize" ))
							]

							+SHorizontalBox::Slot()
							.Padding(5,0,5,0)
							[
								SNew(STextBlock)
								.TextStyle( FEditorStyle::Get(), "WorldBrowser.StatusBarText" )
								.Text(this, &ST4WorldMap::GetWorldSizeText)
							]
						]											
					]
				]
			]

			// Top-right corner text indicating that simulation is active
			+SOverlay::Slot()
			.Padding(20)
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.Visibility(this, &ST4WorldMap::IsSimulationVisible)
				.TextStyle( FEditorStyle::Get(), "Graph.SimulatingText" )
				.Text(LOCTEXT("SimulatingNotification", "SIMULATING"))
			]
		];
}

void ST4WorldMap::PopulateLayersList()
{
	TArray<FWorldTileLayer>& AllLayers = TileWorldModel->GetLayers();
	
	LayersListWrapBox->ClearChildren();
	for (auto WorldLayer : AllLayers)
	{
		LayersListWrapBox->AddSlot()
		.Padding(1,1,0,0)
		[
			SNew(ST4WorldLayerButton)
				.WorldLayer(WorldLayer)
				.InWorldModel(TileWorldModel)
				.OnSelectLayer(this, &ST4WorldMap::HandleOnSelectLayerClicked)
				.OnEditLayer(this, &ST4WorldMap::HandleOnEditLayerClicked)
		];
	}

	// Add new layer button
	LayersListWrapBox->AddSlot()
	.Padding(1,1,0,0)
	[
		SAssignNew(NewLayerButton, SButton)
		.OnClicked(this, &ST4WorldMap::NewLayer_Clicked)
		.ButtonColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.2f, 0.2f))
		.Content()
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("WorldBrowser.AddLayer"))
		]
	];
}

void ST4WorldMap::HandleOnSelectLayerClicked() // #86
{
	check(nullptr != WorldMapViewModelRef);
	WorldMapViewModelRef->UpdatePreviewWorldSubLevel();
}

void ST4WorldMap::HandleOnEditLayerClicked(
	const FWorldTileLayer& InOldWorldTileLayer,
	const FWorldTileLayer& InNewWorldTileLayer
) // #86
{
	// #86 : Editor World 편집 이슈로 Preview World 에서 로드한 SubLevel 을 리로드 처리해준다.
	check(nullptr != WorldMapViewModelRef);
	WorldMapViewModelRef->HandleOnSubLevelChanged();
	bool bResult = TileWorldModel->ChangeManagedLayer(InOldWorldTileLayer, InNewWorldTileLayer);
	if (bResult)
	{
		PopulateLayersList();
	}
}

FReply ST4WorldMap::NewLayer_Clicked()
{
	if (TileWorldModel->IsReadOnly())
	{
		return FReply::Handled();
	}
	
	TSharedRef<ST4NewWorldLayerPopup> CreateLayerWidget = 
		SNew(ST4NewWorldLayerPopup)
		.OnCreateLayer(this, &ST4WorldMap::CreateNewLayer)
		.DefaultName(LOCTEXT("Layer_DefaultName", "MyLayer").ToString())
		.InWorldModel(TileWorldModel);

	NewLayerMenu = FSlateApplication::Get().PushMenu(
		this->AsShared(),
		FWidgetPath(),
		CreateLayerWidget,
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect( FPopupTransitionEffect::TypeInPopup )
		);

	return FReply::Handled();
}

FReply ST4WorldMap::CreateNewLayer(const FWorldTileLayer& NewLayer)
{
	TileWorldModel->AddManagedLayer(NewLayer);
	PopulateLayersList();
	
	if (NewLayerMenu.IsValid())
	{
		NewLayerMenu.Pin()->Dismiss();
	}
		
	return FReply::Handled();
}

FText ST4WorldMap::GetZoomText() const
{
	return GridView->GetZoomText();
}

FText ST4WorldMap::GetCurrentOriginText() const
{
	UWorld* CurrentWorld = (TileWorldModel->IsSimulating() ? TileWorldModel->GetSimulationWorld() : TileWorldModel->GetWorld());
	return FText::Format(LOCTEXT("PositionXYFmt", "{0}, {1}"), FText::AsNumber(CurrentWorld->OriginLocation.X), FText::AsNumber(CurrentWorld->OriginLocation.Y));
}

FText ST4WorldMap::GetCurrentLevelText() const
{
#if 1
	UWorld* CurrentWorld = TileWorldModel->GetWorld();
#else
	UWorld* CurrentWorld = (TileWorldModel->IsSimulating() ? TileWorldModel->GetSimulationWorld() : TileWorldModel->GetWorld());
#endif

	if (CurrentWorld->GetCurrentLevel())
	{
		UPackage* Package = CurrentWorld->GetCurrentLevel()->GetOutermost();
		return FText::FromString(FPackageName::GetShortName(Package->GetName()));
	}
	
	return LOCTEXT("None", "None");
}

FText ST4WorldMap::GetMouseLocationText() const
{
	FVector2D MouseLocation = GridView->GetMouseWorldLocation();
	return FText::Format(LOCTEXT("PositionXYFmt", "{0}, {1}"), FText::AsNumber(FMath::RoundToInt(MouseLocation.X)), FText::AsNumber(FMath::RoundToInt(MouseLocation.Y)));
}

FText ST4WorldMap::GetMarqueeSelectionSizeText() const
{
	FVector2D MarqueeSize = GridView->GetMarqueeWorldSize();
	
	if (MarqueeSize.X > 0 && 
		MarqueeSize.Y > 0)
	{
		return FText::Format(LOCTEXT("SizeXYFmt", "{0} x {1}"), FText::AsNumber(FMath::RoundToInt(MarqueeSize.X)), FText::AsNumber(FMath::RoundToInt(MarqueeSize.Y)));
	}
	else
	{
		return FText::GetEmpty();
	}
}

FText ST4WorldMap::GetWorldSizeText() const
{
	FIntPoint WorldSize = TileWorldModel->GetWorldSize();
	
	if (WorldSize.X > 0 && 
		WorldSize.Y > 0)
	{
		return FText::Format(LOCTEXT("SizeXYFmt", "{0} x {1}"), FText::AsNumber(WorldSize.X), FText::AsNumber(WorldSize.Y));
	}
	else
	{
		return FText::GetEmpty();
	}
}

EVisibility ST4WorldMap::IsSimulationVisible() const
{
	return (TileWorldModel->IsSimulating() ? EVisibility::Visible : EVisibility::Hidden);
}

#undef LOCTEXT_NAMESPACE

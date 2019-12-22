// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EditorObjectSelectionEditMode.h"

#include "Products/Common/Viewport/T4RehearsalViewportClient.h" // #94
#include "Products/Common/ViewModel/T4RehearsalViewModel.h" // #94

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "AssetEditorModeManager.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "SkeletonSelectionEditMode"

/**
  * #94
 */

const FEditorModeID FT4EditorObjectSelectionEditMode::EM_T4EditorObjectSelectionEditMode = TEXT("EM_T4EditorObjectSelectionEditMode");

FT4EditorObjectSelectionEditMode::FT4EditorObjectSelectionEditMode()
	: bManipulating(false)
	, bInTransaction(false)
	, ViewModelRef(nullptr) // #94
{
	// Disable grid drawing for this mode as the viewport handles this
	bDrawGrid = false;
}

bool FT4EditorObjectSelectionEditMode::StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	EAxisList::Type CurrentAxis = InViewportClient->GetCurrentWidgetAxis();
	FWidget::EWidgetMode WidgetMode = InViewportClient->GetWidgetMode();

	check(nullptr != ViewModelRef);
	AActor* SelectedActor = ViewModelRef->GetEditWidgetModeTarget();

	if (nullptr != SelectedActor)
	{
		bool bValidAxis = false;
		FVector WorldAxisDir;

		bManipulating = true;
		return true;
	}

	return false;
}

bool FT4EditorObjectSelectionEditMode::EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	if (bManipulating)
	{
		// Socket movement is transactional - we want undo/redo and saving of it
		if (bInTransaction)
		{
			GEditor->EndTransaction();
			bInTransaction = false;
		}

		bManipulating = false;
		return true;
	}

	return false;
}

bool FT4EditorObjectSelectionEditMode::InputDelta(
	FEditorViewportClient* InViewportClient, 
	FViewport* InViewport, 
	FVector& InDrag, 
	FRotator& InRot, 
	FVector& InScale
)
{
	const EAxisList::Type CurrentAxis = InViewportClient->GetCurrentWidgetAxis();
	const FWidget::EWidgetMode WidgetMode = InViewportClient->GetWidgetMode();
	const ECoordSystem CoordSystem = InViewportClient->GetWidgetCoordSystemSpace();

	// Get some useful info about buttons being held down
	const bool bCtrlDown = InViewport->KeyState(EKeys::LeftControl) || InViewport->KeyState(EKeys::RightControl);
	const bool bShiftDown = InViewport->KeyState(EKeys::LeftShift) || InViewport->KeyState(EKeys::RightShift);
	const bool bMouseButtonDown = InViewport->KeyState( EKeys::LeftMouseButton ) || InViewport->KeyState( EKeys::MiddleMouseButton ) || InViewport->KeyState( EKeys::RightMouseButton );

	bool bHandled = false;

	//UDebugSkelMeshComponent* PreviewMeshComponent = GetAnimPreviewScene().GetPreviewMeshComponent();

	if ( bManipulating && CurrentAxis != EAxisList::None )
	{
		bHandled = true;

		check(nullptr != ViewModelRef);
		AActor* SelectedActor = ViewModelRef->GetEditWidgetModeTarget();
		if (nullptr != SelectedActor)
		{
			if (WidgetMode == FWidget::WM_Rotate)
			{
				FTransform Transform = SelectedActor->GetTransform();
				FRotator NewRotation = (Transform * FTransform( InRot ) ).Rotator();

				SelectedActor->SetActorRotation( NewRotation );
			}
			else
			{
				FVector Location = SelectedActor->GetActorLocation();
				Location += InDrag;
				SelectedActor->SetActorLocation(Location);
			}
		}

		InViewport->Invalidate();
	}

	return bHandled;
}

void FT4EditorObjectSelectionEditMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
#if 0
	// If we have a socket of interest, draw the widget
	if (GetAnimPreviewScene().GetSelectedSocket().IsValid())
	{
		TArray<USkeletalMeshSocket*> SocketAsArray;
		SocketAsArray.Add(GetAnimPreviewScene().GetSelectedSocket().Socket);
		FAnimationViewportClient::DrawSockets(GetAnimPreviewScene().GetPreviewMeshComponent(), SocketAsArray, GetAnimPreviewScene().GetSelectedSocket(), PDI, false);
	}
#endif
}

void FT4EditorObjectSelectionEditMode::DrawHUD(
	FEditorViewportClient* ViewportClient, 
	FViewport* Viewport, 
	const FSceneView* View, 
	FCanvas* Canvas
)
{
#if 0
	UDebugSkelMeshComponent* PreviewMeshComponent = GetAnimPreviewScene().GetPreviewMeshComponent();

	// Draw name of selected bone
	if (IsSelectedBoneRequired())
	{
		const FIntPoint ViewPortSize = Viewport->GetSizeXY();
		const int32 HalfX = ViewPortSize.X / 2;
		const int32 HalfY = ViewPortSize.Y / 2;

		int32 BoneIndex = GetAnimPreviewScene().GetSelectedBoneIndex();
		const FName BoneName = PreviewMeshComponent->SkeletalMesh->RefSkeleton.GetBoneName(BoneIndex);

		FMatrix BoneMatrix = PreviewMeshComponent->GetBoneMatrix(BoneIndex);
		const FPlane Proj = View->Project(BoneMatrix.GetOrigin());
		if (Proj.W > 0.f)
		{
			const int32 XPos = HalfX + (HalfX * Proj.X);
			const int32 YPos = HalfY + (HalfY * (Proj.Y * -1));

			FQuat BoneQuat = PreviewMeshComponent->GetBoneQuaternion(BoneName);
			FVector Loc = PreviewMeshComponent->GetBoneLocation(BoneName);
			FCanvasTextItem TextItem(FVector2D(XPos, YPos), FText::FromString(BoneName.ToString()), GEngine->GetSmallFont(), FLinearColor::White);
			Canvas->DrawItem(TextItem);
		}
	}
#endif
}

bool FT4EditorObjectSelectionEditMode::AllowWidgetMove()
{
	return ShouldDrawWidget();
}

bool FT4EditorObjectSelectionEditMode::ShouldDrawWidget() const
{
	check(nullptr != ViewModelRef);
	AActor* SelectedActor = ViewModelRef->GetEditWidgetModeTarget();
	return (nullptr != SelectedActor) ? true : false;
}

bool FT4EditorObjectSelectionEditMode::UsesTransformWidget() const
{
	return true;
}

bool FT4EditorObjectSelectionEditMode::UsesTransformWidget(FWidget::EWidgetMode CheckMode) const
{
	return ShouldDrawWidget() && (CheckMode == FWidget::WM_Scale || CheckMode == FWidget::WM_Translate || CheckMode == FWidget::WM_Rotate);
}

bool FT4EditorObjectSelectionEditMode::GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData)
{
	check(nullptr != ViewModelRef);
	AActor* SelectedActor = ViewModelRef->GetEditWidgetModeTarget();
	if (nullptr != SelectedActor)
	{
		InMatrix = SelectedActor->GetTransform().ToMatrixNoScale().RemoveTranslation();
		return true;
	}

	return false;
}

bool FT4EditorObjectSelectionEditMode::GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData)
{
	return GetCustomDrawingCoordinateSystem(InMatrix, InData);
}

FVector FT4EditorObjectSelectionEditMode::GetWidgetLocation() const
{
	check(nullptr != ViewModelRef);
	AActor* SelectedActor = ViewModelRef->GetEditWidgetModeTarget();
	if (nullptr != SelectedActor)
	{
		return SelectedActor->GetActorLocation();
	}
	return FVector::ZeroVector;
}

bool FT4EditorObjectSelectionEditMode::HandleClick(
	FEditorViewportClient* InViewportClient, 
	HHitProxy *HitProxy, 
	const FViewportClick &Click
)
{
	bool bHandled = false;
#if 0
	const bool bSelectingSections = GetAnimPreviewScene().AllowMeshHitProxies();

	USkeletalMeshComponent* MeshComponent = GetAnimPreviewScene().GetPreviewMeshComponent();

	if ( HitProxy )
	{
		if (!HitProxy->IsA(HActor::StaticGetType()) && MeshComponent)
		{
			MeshComponent->SetSelectedEditorSection(INDEX_NONE);
		}

		if ( HitProxy->IsA( HPersonaSocketProxy::StaticGetType() ) )
		{
			// Tell the preview scene that the socket has been selected - this will sort out the skeleton tree, etc.
			GetAnimPreviewScene().DeselectAll();
			GetAnimPreviewScene().SetSelectedSocket(static_cast<HPersonaSocketProxy*>(HitProxy)->SocketInfo);
			bHandled = true;
		}
		else if ( HitProxy->IsA( HPersonaBoneProxy::StaticGetType() ) )
		{			
			// Tell the preview scene that the bone has been selected - this will sort out the skeleton tree, etc.
			GetAnimPreviewScene().DeselectAll();
			GetAnimPreviewScene().SetSelectedBone(static_cast<HPersonaBoneProxy*>(HitProxy)->BoneName);
			bHandled = true;
		}
		else if ( HitProxy->IsA( HActor::StaticGetType() ) && bSelectingSections)
		{
			HActor* ActorHitProxy = static_cast<HActor*>(HitProxy);
			GetAnimPreviewScene().BroadcastMeshClick(ActorHitProxy, Click); // This can pop up menu which redraws viewport and invalidates HitProxy!
			bHandled = true;
		}
	}
	else
	{
		// Deselect mesh sections
		if (MeshComponent)
		{
			MeshComponent->SetSelectedEditorSection(INDEX_NONE);
		}
	}
	
	if ( !bHandled && !bSelectingSections )
	{
		// Cast for phys bodies if we didn't get any hit proxies
		FHitResult Result(1.0f);
		UDebugSkelMeshComponent* PreviewMeshComponent = GetAnimPreviewScene().GetPreviewMeshComponent();
		bool bHit = PreviewMeshComponent->LineTraceComponent(Result, Click.GetOrigin(), Click.GetOrigin() + Click.GetDirection() * SkeletonSelectionModeConstants::BodyTraceDistance, FCollisionQueryParams(NAME_None, FCollisionQueryParams::GetUnknownStatId(),true));
		
		if(bHit)
		{
			GetAnimPreviewScene().DeselectAll();
			GetAnimPreviewScene().SetSelectedBone(Result.BoneName);
			bHandled = true;
		}
		else
		{
			// We didn't hit a proxy or a physics object, so deselect all objects
			GetAnimPreviewScene().DeselectAll();
		}
	}
#endif

	return bHandled;
}

bool FT4EditorObjectSelectionEditMode::CanCycleWidgetMode() const
{
	return false;
}

#undef LOCTEXT_NAMESPACE

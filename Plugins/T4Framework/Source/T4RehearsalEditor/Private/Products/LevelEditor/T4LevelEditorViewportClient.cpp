// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4LevelEditorViewportClient.h"

#if WITH_EDITOR
#include "SceneView.h"
#include "LevelEditorViewport.h"
#include "Editor.h"
#endif

#include "T4RehearsalEditorInternal.h"

/**
  * 
 */
FT4LevelEditorViewportClient::FT4LevelEditorViewportClient()
{
}

FT4LevelEditorViewportClient::~FT4LevelEditorViewportClient()
{
}

FViewport* FT4LevelEditorViewportClient::GetViewport() const // #68
{
	if (GCurrentLevelEditingViewportClient)
	{
		return GCurrentLevelEditingViewportClient->Viewport;
	}
	return nullptr;
}

void FT4LevelEditorViewportClient::SetMouseLocation(const int InX, const int InY)
{
	// #17
	if (GCurrentLevelEditingViewportClient)
	{
		if (GCurrentLevelEditingViewportClient->Viewport)
		{
			GCurrentLevelEditingViewportClient->Viewport->SetMouse(InX, InY);
		}
	}
}

bool FT4LevelEditorViewportClient::GetMousePosition(float& InLocationX, float& InLocationY)
{
	// #17
	if (GCurrentLevelEditingViewportClient)
	{
		FViewportCursorLocation CusorLocation = GCurrentLevelEditingViewportClient->GetCursorWorldLocationFromMousePos();
		InLocationX = CusorLocation.GetCursorPos().X;
		InLocationY = CusorLocation.GetCursorPos().Y;
	}
	return true;
}

bool FT4LevelEditorViewportClient::GetMousePositionToWorldRay(
	FVector& OutStartPosition,
	FVector& OutStartDirection
)
{
	// #17
	if (GCurrentLevelEditingViewportClient)
	{
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			GCurrentLevelEditingViewportClient->Viewport,
			GCurrentLevelEditingViewportClient->GetWorld()->Scene,
			GCurrentLevelEditingViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));

		FSceneView* SceneView = GCurrentLevelEditingViewportClient->CalcSceneView(&ViewFamily);
		if (nullptr != SceneView)
		{
			FIntPoint MousePosition;
			FVector WorldOrigin;
			FVector WorldDirection;
			GCurrentLevelEditingViewportClient->Viewport->GetMousePos(MousePosition);
			SceneView->DeprojectFVector2D(MousePosition, OutStartPosition, OutStartDirection);
			return true;
		}
	}
	return false;
}

void FT4LevelEditorViewportClient::ShowMouseCursor(bool InShow)
{
	// #17
	/*
	if (GCurrentLevelEditingViewportClient)
	{
		if (GCurrentLevelEditingViewportClient->Viewport)
		{
			GCurrentLevelEditingViewportClient->Viewport->ShowCursor(InShow);
			GCurrentLevelEditingViewportClient->Viewport->ShowSoftwareCursor(false);
		}
	}
	*/
}

void FT4LevelEditorViewportClient::SetMouseCursorType(EMouseCursor::Type InMouseCursorType)
{
	// #17
	/*
	if (GCurrentLevelEditingViewportClient)
	{
		GCurrentLevelEditingViewportClient->SetRequiredCursorOverride(true, InMouseCursorType);
	}
	*/
}

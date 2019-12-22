// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Frame/Public/T4Frame.h"

/**
  *
 */
class FT4LevelEditorViewportClient : public IT4EditorViewportClient
{
public:
	FT4LevelEditorViewportClient();
	~FT4LevelEditorViewportClient();

	// IT4EditorViewportClient
	FViewport* GetViewport() const override; // #68

	bool IsPreviewMode() const override { return false; }

	void SetUpdateCameraForPlayer(bool bEnable) override {} // #79

	void SetMouseLocation(const int InX, const int InY) override;
	bool GetMousePosition(float& InLocationX, float& InLocationY) override;
	bool GetMousePositionToWorldRay(FVector& OutStartPosition, FVector& OutStartDirection) override;

	void ShowMouseCursor(bool InShow) override;
	void SetMouseCursorType(EMouseCursor::Type InMouseCursorType) override;

	void SetInitialLocationAndRotation(const FVector& InLocation, const FRotator& InRotation) override {} // #86
};

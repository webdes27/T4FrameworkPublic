// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/Camera/T4CameraComponent.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Camera/index.html
 */
UT4CameraComponent::UT4CameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4CameraComponent::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
bool UT4CameraComponent::GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut)
{
	bool bResult = Super::GetEditorPreviewInfo(DeltaTime, ViewOut);
	return bResult;
}
#endif

void UT4CameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	Super::GetCameraView(DeltaTime, DesiredView);
}
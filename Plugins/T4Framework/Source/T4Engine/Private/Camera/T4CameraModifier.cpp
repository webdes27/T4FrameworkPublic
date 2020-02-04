// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/Camera/T4CameraModifier.h"
#include "Camera/PlayerCameraManager.h"

#include "T4EngineInternal.h"

/**
  * #100
*/
UT4CameraModifier::UT4CameraModifier(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CameraBlendWeight(0.0f) // #58
	, CameraBlendFOVDegree(0.0f) // #58
	, CameraBlendLocation(FVector::ZeroVector) // #58
	, CameraBlendRotation(FRotator::ZeroRotator) // #58
{
}

void UT4CameraModifier::Reset()
{
	CameraBlendWeight = 0.0f; // #58
	PostProcessBlendWeights.Empty();
	PostProcessSettings.Empty();
}

bool UT4CameraModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	if (0.0f < CameraBlendWeight) // #58 : Shake 보다 먼저!!
	{
		InOutPOV.Location = FMath::Lerp(InOutPOV.Location, CameraBlendLocation, CameraBlendWeight);
		InOutPOV.Rotation = FMath::Lerp(InOutPOV.Rotation, CameraBlendRotation, CameraBlendWeight);
		if (0.0f < CameraBlendFOVDegree)
		{
			InOutPOV.FOV = FMath::Lerp(InOutPOV.FOV, CameraBlendFOVDegree, CameraBlendWeight);
		}
	}

	Super::ModifyCamera(DeltaTime, InOutPOV);

	if (nullptr != CameraOwner)
	{
		int32 NumPostProcessSettings = PostProcessSettings.Num();
		for (int32 i = 0; i < NumPostProcessSettings; ++i)
		{
			CameraOwner->AddCachedPPBlend(*PostProcessSettings[i], PostProcessBlendWeights[i]);
		}
	}

	return true;
}

void UT4CameraModifier::UpdateCameraAnimBlend(
	const FVector& InLocation,
	const FRotator& InRotation,
	float InFOVDegree,
	float InBlendWeight
) // #58
{
	CameraBlendLocation = InLocation;
	CameraBlendRotation = InRotation;
	CameraBlendFOVDegree = InFOVDegree;
	CameraBlendWeight = InBlendWeight;
}
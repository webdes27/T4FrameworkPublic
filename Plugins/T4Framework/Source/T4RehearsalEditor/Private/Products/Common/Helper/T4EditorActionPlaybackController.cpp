// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EditorActionPlaybackController.h"

#include "T4RehearsalEditorSettings.h"
#include "Products/T4RehearsalEditorUtils.h"

#include "T4Engine/Public/Playback/T4ActionPlaybackAPI.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #68
 */
UT4EditorActionPlaybackController::UT4EditorActionPlaybackController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bPlayRepeat(false)
	, bPlayerPossessed(false)
	, LayerType(ET4LayerType::Max)
{
	UT4RehearsalEditorSettings* RehearsalSettings = GetMutableDefault<UT4RehearsalEditorSettings>();
	check(nullptr != RehearsalSettings);
	bPlayRepeat = RehearsalSettings->bDefaultActionPlaybackRepeat;
	bPlayerPossessed = RehearsalSettings->bDefaultActionPlaybackPlayerPossessed;
}

#if WITH_EDITOR
void UT4EditorActionPlaybackController::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent
)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	check(ET4LayerType::Max != LayerType);
	T4ActionPlaybackAPI::SetPlayRepeat(
		LayerType,
		bPlayRepeat
	);
	T4ActionPlaybackAPI::SetPlayerPossessed(
		LayerType,
		bPlayerPossessed
	);
}
#endif

void UT4EditorActionPlaybackController::Set(
	ET4LayerType InLayerType,
	const FString& InAssetName,
	const FString& InFolderName
)
{
	LayerType = InLayerType;
	AssetName = InAssetName;
	FolderName = InFolderName;
	FString ActionPlaybackLongPackageName = T4ActionPlaybackAPI::GetLongPackageName(
		InAssetName,
		InFolderName
	); // #68
	SelectActionPlaybackAsset = FSoftObjectPath(ActionPlaybackLongPackageName);
	if (!SelectActionPlaybackAsset.IsNull())
	{
		UT4ActionPlaybackAsset* ActionPlaybackAsset = SelectActionPlaybackAsset.LoadSynchronous();
		if (nullptr == ActionPlaybackAsset)
		{
			SelectActionPlaybackAsset.Reset();
		}
	}
}

bool UT4EditorActionPlaybackController::IsPlayed() // #104
{
	const bool bIsPlaying = T4ActionPlaybackAPI::IsPlaying(LayerType);
	const bool bIsPlayPaused = T4ActionPlaybackAPI::IsPaused(LayerType);
	if (bIsPlaying || bIsPlayPaused)
	{
		return true;
	}
	return false;
}

bool UT4EditorActionPlaybackController::CanRec()
{
	check(ET4LayerType::Max != LayerType);
	const bool bIsPlaying = T4ActionPlaybackAPI::IsPlaying(LayerType);
	const bool bIsRecording = T4ActionPlaybackAPI::IsRecording(LayerType);
	return (bIsPlaying || bIsRecording) ? false : true;
}

bool UT4EditorActionPlaybackController::DoRec()
{
	check(ET4LayerType::Max != LayerType);
	bool bResult = true;
	if (SelectActionPlaybackAsset.IsNull())
	{
		bResult = T4ActionPlaybackAPI::DoRec(
			LayerType,
			AssetName,
			FolderName
		);
		if (!bResult)
		{
			return false;
		}
		FString ActionPlaybackLongPackageName = T4ActionPlaybackAPI::GetLongPackageName(
			AssetName,
			FolderName
		); // #68
		SelectActionPlaybackAsset = FSoftObjectPath(ActionPlaybackLongPackageName);
		if (!SelectActionPlaybackAsset.IsNull())
		{
			UT4ActionPlaybackAsset* ActionPlaybackAsset = SelectActionPlaybackAsset.LoadSynchronous();
			if (nullptr == ActionPlaybackAsset)
			{
				SelectActionPlaybackAsset.Reset();
			}
		}
	}
	else
	{
		bResult = T4ActionPlaybackAPI::DoRec(
			LayerType,
			SelectActionPlaybackAsset.ToSoftObjectPath()
		);
	}
	return bResult;
}

bool UT4EditorActionPlaybackController::CanPlay()
{
	check(ET4LayerType::Max != LayerType);
	const bool bIsRecording = T4ActionPlaybackAPI::IsRecording(LayerType);
	if (bIsRecording)
	{
		return false;
	}
	const bool bIsPlaying = T4ActionPlaybackAPI::IsPlaying(LayerType);
	const bool bIsPlayPaused = T4ActionPlaybackAPI::IsPaused(LayerType);
	if (bIsPlaying && bIsPlayPaused)
	{
		return true;
	}
	else if (bIsPlaying)
	{
		return false;
	}
	return true;
}

bool UT4EditorActionPlaybackController::DoPlay()
{
	check(ET4LayerType::Max != LayerType);
	bool bResult = true;
	if (SelectActionPlaybackAsset.IsNull())
	{
		T4ActionPlaybackAPI::DoPlay(
			LayerType,
			AssetName,
			FolderName
		);
	}
	else
	{
		T4ActionPlaybackAPI::DoPlay(
			LayerType,
			SelectActionPlaybackAsset.ToSoftObjectPath()
		);
	}
	return true;
}

bool UT4EditorActionPlaybackController::CanPause()
{
	check(ET4LayerType::Max != LayerType);
	return T4ActionPlaybackAPI::IsPlaying(LayerType);
}

bool UT4EditorActionPlaybackController::DoPause()
{
	check(ET4LayerType::Max != LayerType);
	if (!T4ActionPlaybackAPI::IsPlaying(LayerType))
	{
		return false;
	}
	bool bPause = !T4ActionPlaybackAPI::IsPaused(LayerType);
	T4ActionPlaybackAPI::SetPlayPause(LayerType, bPause);
	return true;
}

bool UT4EditorActionPlaybackController::CanStop()
{
	check(ET4LayerType::Max != LayerType);
	const bool bIsPlaying = T4ActionPlaybackAPI::IsPlaying(LayerType);
	const bool bIsRecording = T4ActionPlaybackAPI::IsRecording(LayerType);
	return (bIsPlaying || bIsRecording) ? true : false;
}

bool UT4EditorActionPlaybackController::DoStop()
{
	check(ET4LayerType::Max != LayerType);
	if (T4ActionPlaybackAPI::IsPlaying(LayerType))
	{
		T4ActionPlaybackAPI::StopPlaying(LayerType);
	}
	if (T4ActionPlaybackAPI::IsRecording(LayerType))
	{
		T4ActionPlaybackAPI::StopRecording(LayerType);
	}
	return true;
}

void UT4EditorActionPlaybackController::TooglePlayRepeat()
{
	check(ET4LayerType::Max != LayerType);
	bPlayRepeat = !bPlayRepeat;
	T4ActionPlaybackAPI::SetPlayRepeat(
		LayerType,
		bPlayRepeat
	);
}

void UT4EditorActionPlaybackController::TooglePlayerPossessed()
{
	check(ET4LayerType::Max != LayerType);
	bPlayerPossessed = !bPlayerPossessed;
	T4ActionPlaybackAPI::SetPlayerPossessed(
		LayerType,
		bPlayerPossessed
	);
}
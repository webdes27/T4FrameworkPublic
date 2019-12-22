// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Public/Playback/T4ActionPlaybackAPI.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"
#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/T4Engine.h"

#if !UE_BUILD_SHIPPING
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#endif

#include "T4EngineInternal.h"

/**
  * #68
 */
namespace T4ActionPlaybackAPI
{

#if !UE_BUILD_SHIPPING

static const TCHAR* DeveloperActionPlaybackFilePath = TEXT("ActionPlayback/");

// #68
inline IT4ActionPlaybackController* GetActionPlaybackController(ET4LayerType InLayerType)
{
	IT4GameWorld* GameWorld = T4EngineWorldGet(InLayerType);
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	return GameWorld->GetActionPlaybackController();
}

bool IsPlaying(
	ET4LayerType InLayerType
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	return ActionPlaybackController->IsPlaying();
}

bool DoPlay(
	ET4LayerType InLayerType,
	const FString& InPlayAssetName,
	const FString& InPlayFolderName
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	if (ActionPlaybackController->IsRecording())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("ActionPlaybackPlay Action playback Play failed. Current state Recording.")
		);
		return false;
	}
	if (ActionPlaybackController->IsPlaying())
	{
		if (IsPaused(InLayerType))
		{
			SetPlayPause(InLayerType, false);
		}
		return true;
	}
	bool bResult = ActionPlaybackController->DoPlay(InPlayAssetName, InPlayFolderName);
	return bResult;
}

bool DoPlay(
	ET4LayerType InLayerType,
	const FSoftObjectPath& InPlayPath
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	if (ActionPlaybackController->IsRecording())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("ActionPlaybackPlay Action playback Play failed. Current state Recording.")
		);
		return false;
	}
	if (ActionPlaybackController->IsPlaying())
	{
		if (IsPaused(InLayerType))
		{
			SetPlayPause(InLayerType, false);
		}
		return true;
	}
	bool bResult = ActionPlaybackController->DoPlay(InPlayPath);
	return bResult;
}

void StopPlaying(
	ET4LayerType InLayerType
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return;
	}
	ActionPlaybackController->DoStopPlaying();
}

bool IsPaused(
	ET4LayerType InLayerType
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	if (!ActionPlaybackController->IsPlaying())
	{
		return false;
	}
	return ActionPlaybackController->GetPlayer()->IsPaused();
}

void SetPlayPause(
	ET4LayerType InLayerType,
	bool bPause
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return;
	}
	if (!ActionPlaybackController->IsPlaying())
	{
		return;
	}
	return ActionPlaybackController->GetPlayer()->SetPause(bPause);
}

bool IsRepeat(
	ET4LayerType InLayerType
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	return ActionPlaybackController->IsPlayRepeat();
}

void SetPlayRepeat(
	ET4LayerType InLayerType,
	bool bEnable
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return;
	}
	ActionPlaybackController->SetPlayRepeat(bEnable);
}

bool IsPlayerPossessed(
	ET4LayerType InLayerType
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	return ActionPlaybackController->IsPlayerPossessed();
}

void SetPlayerPossessed(
	ET4LayerType InLayerType,
	bool bPossess
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return;
	}
	ActionPlaybackController->SetPlayerPossessed(bPossess);
}

bool IsRecording(
	ET4LayerType InLayerType
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	return ActionPlaybackController->IsRecording();
}

bool DoRec(
	ET4LayerType InLayerType,
	const FString& InRecAssetName,
	const FString& InRecFolderName
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	if (ActionPlaybackController->IsPlaying())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("ActionPlaybackRec Action playback Rec failed. Current state Playing.")
		);
		return false;
	}
	if (ActionPlaybackController->IsRecording())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("ActionPlaybackRec Action playback Rec failed. Current state Recording.")
		);
		return false;
	}
	bool bResult = ActionPlaybackController->DoRec(InRecAssetName, InRecFolderName);
	return bResult;
}

bool DoRec(
	ET4LayerType InLayerType,
	const FSoftObjectPath& InRecPath
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return false;
	}
	if (ActionPlaybackController->IsPlaying())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("ActionPlaybackRec Action playback Rec failed. Current state Playing.")
		);
		return false;
	}
	if (ActionPlaybackController->IsRecording())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("ActionPlaybackRec Action playback Rec failed. Current state Recording.")
		);
		return false;
	}
	bool bResult = ActionPlaybackController->DoRec(InRecPath);
	return bResult;
}

void StopRecording(
	ET4LayerType InLayerType
)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return;
	}
	ActionPlaybackController->DoStopRecording();
}

const TCHAR* GetRecFileName(ET4LayerType InLayerType)
{
	IT4ActionPlaybackController* ActionPlaybackController = GetActionPlaybackController(InLayerType);
	if (nullptr == ActionPlaybackController)
	{
		return nullptr;
	}
	if (!ActionPlaybackController->IsRecording())
	{
		return nullptr;
	}
	IT4ActionPlaybackRecorder* ActionPlaybackRecorder = ActionPlaybackController->GetRecorder();
	check(nullptr != ActionPlaybackRecorder);
	return ActionPlaybackRecorder->GetRecFile();
}

const FString GetLongPackagePath(
	const FString& InFolderName
)
{
	FString UserDeveloperPath = FPackageName::FilenameToLongPackageName(FPaths::GameUserDeveloperDir());
	UserDeveloperPath += DeveloperActionPlaybackFilePath;
	UserDeveloperPath += InFolderName;
	return UserDeveloperPath;
}

const FString GetAssetNameFromObjectPath(
	const FString& InObjectPathString
)
{
	FString AssetName = FPackageName::GetLongPackageAssetName(InObjectPathString);
	int32 StartIdx = 0;
	if (AssetName.FindChar(TEXT('.'), StartIdx))
	{
		int32 LenString = AssetName.Len();
		AssetName.RemoveAt(StartIdx - 1, LenString - StartIdx);
	}
	return AssetName;
}

const FString GetLongPackageName(
	const FString& InAssetName,
	const FString& InFolderName
)
{
	FString LongPackageName = GetLongPackagePath(InFolderName) + TEXT("/") + InAssetName + TEXT(".") + InAssetName;
	return LongPackageName;
}

// ~#68
#endif

}
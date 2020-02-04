// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Public/Replay/T4ActionReplayUtility.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"
#include "T4Engine/Public/Action/T4ActionCodeCommandIncludes.h"
#include "T4Engine/Public/T4Engine.h"

#if !UE_BUILD_SHIPPING
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#endif

#include "T4EngineInternal.h"

/**
  * #68
 */
namespace T4ActionReplayUtility
{

#if !UE_BUILD_SHIPPING

static const TCHAR* DeveloperActionReplayFilePath = TEXT("ActionReplay/");

// #68
inline IT4ActionReplaySystem* GetActionReplaySystem(ET4LayerType InLayerType)
{
	IT4GameWorld* GameWorld = T4EngineWorldGet(InLayerType);
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	return GameWorld->GetActionReplaySystem();
}

bool IsPlaying(
	ET4LayerType InLayerType
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	return ActionReplaySystem->IsPlaying();
}

bool DoPlay(
	ET4LayerType InLayerType,
	const FString& InPlayAssetName,
	const FString& InPlayFolderName
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	if (ActionReplaySystem->IsRecording())
	{
		T4_LOG(
			Error,
			TEXT("Action Replay failed. Current state Recording.")
		);
		return false;
	}
	if (ActionReplaySystem->IsPlaying())
	{
		if (IsPaused(InLayerType))
		{
			SetPlayPause(InLayerType, false);
		}
		return true;
	}
	bool bResult = ActionReplaySystem->DoPlay(InPlayAssetName, InPlayFolderName);
	return bResult;
}

bool DoPlay(
	ET4LayerType InLayerType,
	const FSoftObjectPath& InPlayPath
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	if (ActionReplaySystem->IsRecording())
	{
		T4_LOG(
			Error,
			TEXT("Action Replay failed. Current state Recording.")
		);
		return false;
	}
	if (ActionReplaySystem->IsPlaying())
	{
		if (IsPaused(InLayerType))
		{
			SetPlayPause(InLayerType, false);
		}
		return true;
	}
	bool bResult = ActionReplaySystem->DoPlay(InPlayPath);
	return bResult;
}

void StopPlaying(
	ET4LayerType InLayerType
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return;
	}
	ActionReplaySystem->DoStopPlaying();
}

bool IsPaused(
	ET4LayerType InLayerType
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	if (!ActionReplaySystem->IsPlaying())
	{
		return false;
	}
	return ActionReplaySystem->GetPlayer()->IsPaused();
}

void SetPlayPause(
	ET4LayerType InLayerType,
	bool bPause
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return;
	}
	if (!ActionReplaySystem->IsPlaying())
	{
		return;
	}
	return ActionReplaySystem->GetPlayer()->SetPause(bPause);
}

bool IsRepeat(
	ET4LayerType InLayerType
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	return ActionReplaySystem->IsPlayRepeat();
}

void SetPlayRepeat(
	ET4LayerType InLayerType,
	bool bEnable
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return;
	}
	ActionReplaySystem->SetPlayRepeat(bEnable);
}

bool IsPlayerPossessed(
	ET4LayerType InLayerType
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	return ActionReplaySystem->IsPlayerPossessed();
}

void SetPlayerPossessed(
	ET4LayerType InLayerType,
	bool bPossess
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return;
	}
	ActionReplaySystem->SetPlayerPossessed(bPossess);
}

bool IsRecording(
	ET4LayerType InLayerType
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	return ActionReplaySystem->IsRecording();
}

bool DoRec(
	ET4LayerType InLayerType,
	const FString& InRecAssetName,
	const FString& InRecFolderName
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	if (ActionReplaySystem->IsPlaying())
	{
		T4_LOG(
			Error,
			TEXT("Action Replay Rec failed. Current state Playing.")
		);
		return false;
	}
	if (ActionReplaySystem->IsRecording())
	{
		T4_LOG(
			Error,
			TEXT("Action Replay Rec failed. Current state Recording.")
		);
		return false;
	}
	bool bResult = ActionReplaySystem->DoRec(InRecAssetName, InRecFolderName);
	return bResult;
}

bool DoRec(
	ET4LayerType InLayerType,
	const FSoftObjectPath& InRecPath
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return false;
	}
	if (ActionReplaySystem->IsPlaying())
	{
		T4_LOG(
			Error,
			TEXT("Action Replay Rec failed. Current state Playing.")
		);
		return false;
	}
	if (ActionReplaySystem->IsRecording())
	{
		T4_LOG(
			Error,
			TEXT("Action Replay Rec failed. Current state Recording.")
		);
		return false;
	}
	bool bResult = ActionReplaySystem->DoRec(InRecPath);
	return bResult;
}

void StopRecording(
	ET4LayerType InLayerType
)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return;
	}
	ActionReplaySystem->DoStopRecording();
}

const TCHAR* GetRecFileName(ET4LayerType InLayerType)
{
	IT4ActionReplaySystem* ActionReplaySystem = GetActionReplaySystem(InLayerType);
	if (nullptr == ActionReplaySystem)
	{
		return nullptr;
	}
	if (!ActionReplaySystem->IsRecording())
	{
		return nullptr;
	}
	IT4ActionReplayRecorder* ActionReplayRecorder = ActionReplaySystem->GetRecorder();
	check(nullptr != ActionReplayRecorder);
	return ActionReplayRecorder->GetRecFile();
}

const FString GetLongPackagePath(
	const FString& InFolderName
)
{
	FString UserDeveloperPath = FPackageName::FilenameToLongPackageName(FPaths::GameUserDeveloperDir());
	UserDeveloperPath += DeveloperActionReplayFilePath;
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
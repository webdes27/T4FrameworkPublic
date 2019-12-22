// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionPlaybackBase.h"
#include "Public/Playback/T4ActionPlaybackAPI.h"

#if !UE_BUILD_SHIPPING

#include "World/T4GameWorld.h"

#include "Public/T4Engine.h"
#include "T4Asset/Public/T4AssetUtils.h"

#include "Serialization/LargeMemoryData.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/BufferArchive.h"
#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"

#include "T4EngineInternal.h"

/**
  * #68
 */
#if 0 // TODO : Saved 폴더에 저장해야 할 시점에 처리. 저장까지는 문제없는데, 로드에서 문제가 있음
class FT4ActionPlaybackDataWriter : public FArchiveUObject
{
public:
	explicit FT4ActionPlaybackDataWriter(FLargeMemoryData& InObjectData)
		: ObjectData(InObjectData)
		, Offset(0)
	{
		this->SetIsSaving(true);
		this->SetIsPersistent(true);
		this->SetWantBinaryPropertySerialization(true);
	}

	FString GetArchiveName() const override { return TEXT("FT4ActionPlaybackDataWriter"); }

	void Serialize(void* Data, int64 Num) override
	{
		if (ObjectData.Write(Data, Offset, Num))
		{
			Offset += Num;
		}
	}

	void Seek(int64 InPos) override
	{
		Offset = InPos;
	}

	int64 Tell() override
	{
		return Offset;
	}

	int64 TotalSize() override
	{
		return ObjectData.GetSize();
	}

private:
	FLargeMemoryData& ObjectData;
	int64 Offset;
};

class FT4ActionPlaybackDataReader : public FArchiveUObject
{
public:
	explicit FT4ActionPlaybackDataReader(FLargeMemoryData& InObjectData)
		: ObjectData(InObjectData)
		, Offset(0)
	{
		this->SetIsLoading(true);
		this->SetIsPersistent(true);
		this->SetWantBinaryPropertySerialization(true);
	}

	FString GetArchiveName() const override { return TEXT("FT4ActionPlaybackDataReader"); }

	void Serialize(void* Data, int64 Num) override
	{
		if (ObjectData.Read(Data, Offset, Num))
		{
			Offset += Num;
		}
	}

	void Seek(int64 InPos) override
	{
		Offset = InPos;
	}

	int64 Tell() override
	{
		return Offset;
	}

	int64 TotalSize() override
	{
		return ObjectData.GetSize();
	}

private:
	const FLargeMemoryData& ObjectData;
	int64 Offset;
};
#endif

FT4ActionPlaybackBase::FT4ActionPlaybackBase(ET4LayerType InLayerType)
	: LayerType(InLayerType)
	, PlaybackAsset(nullptr)
{
}

FT4ActionPlaybackBase::~FT4ActionPlaybackBase()
{
}

void FT4ActionPlaybackBase::OnReset()
{
	Reset();
	ResetAsset();
}

void FT4ActionPlaybackBase::OnStop()
{
	Stop();
	OnReset();
}

#if 0
bool FT4ActionPlaybackBase::DoLoad(const FString& InLoadFileName)
{
	check(nullptr != PlaybackAsset);
	
	// TODO : Saved 폴더에 저장해야 할 시점에 처리. 저장까지는 문제없는데, 로드에서 문제가 있음

	//const FString LoadFullPath = FPaths::ProjectSavedDir() / + ActionPlaybackFilePath + InLoadFileName + TEXT(".t4playback");
	const FString LoadFullPath = FPaths::ProjectSavedDir() / + TEXT("T4Framework/ActionPlayback") + InLoadFileName + TEXT(".t4playback");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*LoadFullPath, false);
	if (nullptr == FileHandle)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionPlaybackBase : Failed to load. '%s'"),
			*LoadFullPath
		);
		return false;
	}
	FLargeMemoryData ObjectData(FileHandle->Size());
	bool bResult = FileHandle->Read(ObjectData.GetData(), ObjectData.GetSize());
	delete FileHandle;
	if (bResult)
	{
		FT4ActionPlaybackDataReader ReadArchive(ObjectData);
		FObjectAndNameAsStringProxyArchive SafeAr(ReadArchive, false);
		PlaybackAsset->Serialize(ReadArchive);
		SafeAr.Close();
		ReadArchive.Close();
	}
	return bResult;
}

bool FT4ActionPlaybackBase::DoSave(const FString& InSaveFileName)
{
	check(nullptr != PlaybackAsset);

	// TODO : Saved 폴더에 저장해야 할 시점에 처리. 저장까지는 문제없는데, 로드에서 문제가 있음

	const FString SaveFilePath = FPaths::ProjectSavedDir() / + TEXT("T4Framework/ActionPlayback");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*SaveFilePath))
	{
		PlatformFile.CreateDirectoryTree(*SaveFilePath);
	}
	const FString SaveFullPath = SaveFilePath / InSaveFileName + TEXT(".t4playback");
	IFileHandle* FileHandle = PlatformFile.OpenWrite(
		*SaveFullPath,
		false,
		false
	);
	if (nullptr == FileHandle)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionPlaybackBase : Failed to save. '%s'"),
			*SaveFullPath
		);
		return false;
	}
	FLargeMemoryData ObjectData;
	{
		FT4ActionPlaybackDataWriter WriteArchive(ObjectData);
		FObjectAndNameAsStringProxyArchive SafeAr(WriteArchive, false);
		PlaybackAsset->Serialize(WriteArchive);
		SafeAr.Close();
		WriteArchive.Close();
	}
	FileHandle->Write(ObjectData.GetData(), ObjectData.GetSize());
	delete FileHandle;
	FileHandle = nullptr;

#if 1
	{
		UT4ActionPlaybackAsset* TestPlaybackAsset = NewObject<UT4ActionPlaybackAsset>();
		check(nullptr != TestPlaybackAsset);
		TestPlaybackAsset->AddToRoot();
		FT4ActionPlaybackDataReader ReadArchive(ObjectData);
		FObjectAndNameAsStringProxyArchive SafeAr(ReadArchive, false);
		TestPlaybackAsset->Serialize(ReadArchive);
		SafeAr.Close();
		ReadArchive.Close();
	}
#endif
	return true;
}
#endif

void FT4ActionPlaybackBase::ResetAsset()
{
	if (nullptr == PlaybackAsset)
	{
		return;
	}
	PlaybackAsset->RemoveFromRoot();
	PlaybackAsset = nullptr;
}

bool FT4ActionPlaybackBase::NewAsset()
{
	check(nullptr == PlaybackAsset);
#if WITH_EDITOR
	PlaybackAsset = NewObject<UT4ActionPlaybackAsset>();
	check(nullptr != PlaybackAsset);
	PlaybackAsset->AddToRoot();
	return true;
#else
	return false;
#endif
}

bool FT4ActionPlaybackBase::LoadAsset(
	const FSoftObjectPath& InLoadPath
)
{
	check(nullptr == PlaybackAsset);
	FSoftObjectPath LoadObjectPath(InLoadPath);
	PlaybackAsset = Cast<UT4ActionPlaybackAsset>(LoadObjectPath.TryLoad());
	if (nullptr == PlaybackAsset)
	{
		return false;
	}
	check(nullptr != PlaybackAsset);
	PlaybackAsset->AddToRoot();
	return true;
}

bool FT4ActionPlaybackBase::SaveAsset(
	const FString& InAssetName, 
	const FString& InPackagePath
)
{
	check(nullptr != PlaybackAsset);
#if WITH_EDITOR
	UT4ActionPlaybackAsset* SavePlaybackAsset = nullptr;
	FSoftObjectPath CheckAsset(InPackagePath / InAssetName + TEXT(".") + InAssetName);
	UObject* ExistAsset = CheckAsset.TryLoad();
	if (nullptr == ExistAsset)
	{
		SavePlaybackAsset = CreateAsset(InAssetName, InPackagePath);
	}
	else
	{
		SavePlaybackAsset = Cast<UT4ActionPlaybackAsset>(ExistAsset);
	}
	if (nullptr == SavePlaybackAsset)
	{
		return false;
	}
	SavePlaybackAsset->PlaybackData = PlaybackAsset->PlaybackData; // Data Copy
	bool bResult = T4AssetUtil::SaveAsset(SavePlaybackAsset, false);
	return bResult;
#else
	return false;
#endif
}

UT4ActionPlaybackAsset* FT4ActionPlaybackBase::CreateAsset(
	const FString& InAssetName,
	const FString& InPackagePath
)
{
#if WITH_EDITOR
	UObject* NewAsset = T4AssetUtil::NewAsset(
		UT4ActionPlaybackAsset::StaticClass(),
		InAssetName,
		InPackagePath + TEXT("/")
	);
	if (nullptr == NewAsset)
	{
		return false;
	}
	UT4ActionPlaybackAsset* NewPlaybackAsset = Cast<UT4ActionPlaybackAsset>(NewAsset);
	check(nullptr != NewPlaybackAsset);
	NewPlaybackAsset->AddToRoot();
	return NewPlaybackAsset;
#else
	return nullptr;
#endif
}

FT4ActionPlaybackData& FT4ActionPlaybackBase::GetPlaybackData()
{
	check(nullptr != PlaybackAsset);
	return PlaybackAsset->PlaybackData;
}

FT4GameWorld* FT4ActionPlaybackBase::GetGameWorld() const
{
	return static_cast<FT4GameWorld*>(T4EngineWorldGet(LayerType));
}

IT4GameObject* FT4ActionPlaybackBase::FindGameObject(const FT4ObjectID& InObjectID) const
{
	FT4GameWorld* GameWorld = GetGameWorld();
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	return GameWorld->GetContainer()->FindGameObject(InObjectID);
}

#endif

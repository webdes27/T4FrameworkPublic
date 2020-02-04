// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionReplayBase.h"
#include "Public/Replay/T4ActionReplayUtility.h"

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
class FT4ActionReplayDataWriter : public FArchiveUObject
{
public:
	explicit FT4ActionReplayDataWriter(FLargeMemoryData& InObjectData)
		: ObjectData(InObjectData)
		, Offset(0)
	{
		this->SetIsSaving(true);
		this->SetIsPersistent(true);
		this->SetWantBinaryPropertySerialization(true);
	}

	FString GetArchiveName() const override { return TEXT("FT4ActionReplayDataWriter"); }

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

class FT4ActionReplayDataReader : public FArchiveUObject
{
public:
	explicit FT4ActionReplayDataReader(FLargeMemoryData& InObjectData)
		: ObjectData(InObjectData)
		, Offset(0)
	{
		this->SetIsLoading(true);
		this->SetIsPersistent(true);
		this->SetWantBinaryPropertySerialization(true);
	}

	FString GetArchiveName() const override { return TEXT("FT4ActionReplayDataReader"); }

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

FT4ActionReplayBase::FT4ActionReplayBase(ET4LayerType InLayerType)
	: LayerType(InLayerType)
	, ActionReplayAsset(nullptr)
{
}

FT4ActionReplayBase::~FT4ActionReplayBase()
{
}

void FT4ActionReplayBase::OnReset()
{
	Reset();
	ResetAsset();
}

void FT4ActionReplayBase::OnStop()
{
	Stop();
	OnReset();
}

#if 0
bool FT4ActionReplayBase::DoLoad(const FString& InLoadFileName)
{
	check(nullptr != ActionReplayAsset);
	
	// TODO : Saved 폴더에 저장해야 할 시점에 처리. 저장까지는 문제없는데, 로드에서 문제가 있음

	//const FString LoadFullPath = FPaths::ProjectSavedDir() / + ActionReplayFilePath + InLoadFileName + TEXT(".t4playback");
	const FString LoadFullPath = FPaths::ProjectSavedDir() / + TEXT("T4Framework/ActionReplay") + InLoadFileName + TEXT(".t4playback");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenRead(*LoadFullPath, false);
	if (nullptr == FileHandle)
	{
		T4_LOG(
			Error,
			TEXT("Failed to load. '%s'"),
			*LoadFullPath
		);
		return false;
	}
	FLargeMemoryData ObjectData(FileHandle->Size());
	bool bResult = FileHandle->Read(ObjectData.GetData(), ObjectData.GetSize());
	delete FileHandle;
	if (bResult)
	{
		FT4ActionReplayDataReader ReadArchive(ObjectData);
		FObjectAndNameAsStringProxyArchive SafeAr(ReadArchive, false);
		ActionReplayAsset->Serialize(ReadArchive);
		SafeAr.Close();
		ReadArchive.Close();
	}
	return bResult;
}

bool FT4ActionReplayBase::OnSave(const FString& InSaveFileName)
{
	check(nullptr != ActionReplayAsset);

	// TODO : Saved 폴더에 저장해야 할 시점에 처리. 저장까지는 문제없는데, 로드에서 문제가 있음

	const FString SaveFilePath = FPaths::ProjectSavedDir() / + TEXT("T4Framework/ActionReplay");
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
		T4_LOG(
			Error,
			TEXT("Failed to save. '%s'"),
			*SaveFullPath
		);
		return false;
	}
	FLargeMemoryData ObjectData;
	{
		FT4ActionReplayDataWriter WriteArchive(ObjectData);
		FObjectAndNameAsStringProxyArchive SafeAr(WriteArchive, false);
		ActionReplayAsset->Serialize(WriteArchive);
		SafeAr.Close();
		WriteArchive.Close();
	}
	FileHandle->Write(ObjectData.GetData(), ObjectData.GetSize());
	delete FileHandle;
	FileHandle = nullptr;

#if 1
	{
		UT4ActionReplayAsset* TestActionReplayAsset = NewObject<UT4ActionReplayAsset>();
		check(nullptr != TestActionReplayAsset);
		TestActionReplayAsset->AddToRoot();
		FT4ActionReplayDataReader ReadArchive(ObjectData);
		FObjectAndNameAsStringProxyArchive SafeAr(ReadArchive, false);
		TestActionReplayAsset->Serialize(ReadArchive);
		SafeAr.Close();
		ReadArchive.Close();
	}
#endif
	return true;
}
#endif

void FT4ActionReplayBase::ResetAsset()
{
	if (nullptr == ActionReplayAsset)
	{
		return;
	}
	ActionReplayAsset->RemoveFromRoot();
	ActionReplayAsset = nullptr;
}

bool FT4ActionReplayBase::NewAsset()
{
	check(nullptr == ActionReplayAsset);
#if WITH_EDITOR
	ActionReplayAsset = NewObject<UT4ActionReplayAsset>();
	check(nullptr != ActionReplayAsset);
	ActionReplayAsset->AddToRoot();
	return true;
#else
	return false;
#endif
}

bool FT4ActionReplayBase::LoadAsset(
	const FSoftObjectPath& InLoadPath
)
{
	check(nullptr == ActionReplayAsset);
	FSoftObjectPath LoadObjectPath(InLoadPath);
	ActionReplayAsset = Cast<UT4ActionReplayAsset>(LoadObjectPath.TryLoad());
	if (nullptr == ActionReplayAsset)
	{
		return false;
	}
	check(nullptr != ActionReplayAsset);
	ActionReplayAsset->AddToRoot();
	return true;
}

bool FT4ActionReplayBase::SaveAsset(
	const FString& InAssetName, 
	const FString& InPackagePath
)
{
	check(nullptr != ActionReplayAsset);
#if WITH_EDITOR
	UT4ActionReplayAsset* SaveActionReplayAsset = nullptr;
	FSoftObjectPath CheckAsset(InPackagePath / InAssetName + TEXT(".") + InAssetName);
	UObject* ExistAsset = CheckAsset.TryLoad();
	if (nullptr == ExistAsset)
	{
		SaveActionReplayAsset = CreateAsset(InAssetName, InPackagePath);
	}
	else
	{
		SaveActionReplayAsset = Cast<UT4ActionReplayAsset>(ExistAsset);
	}
	if (nullptr == SaveActionReplayAsset)
	{
		return false;
	}
	SaveActionReplayAsset->ReplayData = ActionReplayAsset->ReplayData; // Data Copy
	bool bResult = T4AssetUtil::SaveAsset(SaveActionReplayAsset, false);
	return bResult;
#else
	return false;
#endif
}

UT4ActionReplayAsset* FT4ActionReplayBase::CreateAsset(
	const FString& InAssetName,
	const FString& InPackagePath
)
{
#if WITH_EDITOR
	UObject* NewAsset = T4AssetUtil::NewAsset(
		UT4ActionReplayAsset::StaticClass(),
		InAssetName,
		InPackagePath + TEXT("/")
	);
	if (nullptr == NewAsset)
	{
		return false;
	}
	UT4ActionReplayAsset* NewActionReplayAsset = Cast<UT4ActionReplayAsset>(NewAsset);
	check(nullptr != NewActionReplayAsset);
	NewActionReplayAsset->AddToRoot();
	return NewActionReplayAsset;
#else
	return nullptr;
#endif
}

FT4ActionReplayData& FT4ActionReplayBase::GetReplayData()
{
	check(nullptr != ActionReplayAsset);
	return ActionReplayAsset->ReplayData;
}

FT4GameWorld* FT4ActionReplayBase::GetGameWorld() const
{
	return static_cast<FT4GameWorld*>(T4EngineWorldGet(LayerType));
}

IT4GameObject* FT4ActionReplayBase::FindGameObject(const FT4ObjectID& InObjectID) const
{
	FT4GameWorld* GameWorld = GetGameWorld();
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	return GameWorld->GetContainer()->FindGameObject(InObjectID);
}

#endif

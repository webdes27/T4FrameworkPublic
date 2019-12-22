// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Public/T4AssetUtils.h"

#if WITH_EDITOR

#include "Classes/AnimSet/T4AnimSetAsset.h"
#include "Classes/Entity/T4EntityAsset.h"
#include "Classes/Conti/T4ContiAsset.h"
#include "Classes/World/T4WorldAsset.h" // #84

#include "Classes/Common/T4CommonAssetStructs.h" // #103

#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "FileHelpers.h"
#include "IImageWrapper.h" // #84
#include "IImageWrapperModule.h" // #84

#include "T4AssetInternal.h"

namespace T4AssetUtil
{

	/**
	  * #39
	 */
	UObject* NewAsset(
		UClass* InAssetClass,
		const FString& InAssetName,
		const FString& InPackagePath
	)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

		const FString PackagePath = FPackageName::GetLongPackagePath(InPackagePath);
		UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
			InAssetName,
			PackagePath,
			InAssetClass,
			nullptr
		);
		if (nullptr == NewAsset)
		{
			return nullptr;
		}

		TArray<UObject*> Objects;
		Objects.Add(NewAsset);
		ContentBrowserModule.Get().SyncBrowserToAssets(Objects);
		return NewAsset;
	}

	bool HasDirtyAsset(UObject* InCheckObject)
	{
		check(nullptr != InCheckObject);
		UPackage* ObjectPackage = InCheckObject->GetOutermost();
		check(nullptr != ObjectPackage);
		return ObjectPackage->IsDirty();
	}

	void SetDirtyAsset(UObject* InCheckObject, bool bDirtyFlag)
	{
		check(nullptr != InCheckObject);
		UPackage* ObjectPackage = InCheckObject->GetOutermost();
		check(nullptr != ObjectPackage);
		ObjectPackage->SetDirtyFlag(bDirtyFlag);
	}

	bool SaveAsset(
		UObject* InSaveObject,
		bool bInCheckDirty
	)
	{
		check(nullptr != InSaveObject);
		UPackage* ObjectPackage = InSaveObject->GetOutermost();
		check(nullptr != ObjectPackage);
		TArray<UPackage*> PackagesToSave;
		if (!bInCheckDirty || ObjectPackage->IsDirty())
		{
			PackagesToSave.AddUnique(ObjectPackage);
		}
		else
		{
			return false;
		}
		if (0 >= PackagesToSave.Num())
		{
			return false;
		}
		const bool bPromptToSave = false;
		const FEditorFileUtils::EPromptReturnCode Return = FEditorFileUtils::PromptForCheckoutAndSave(
			PackagesToSave,
			bInCheckDirty,
			bPromptToSave
		);
		return (PackagesToSave.Num() > 0) && Return == FEditorFileUtils::EPromptReturnCode::PR_Success;
	}

	bool SaveThumbnailImage(
		UObject* InSaveObject,
		UTexture2D* InThumbnail
	)
	{
		UT4EntityAsset* CheckEntityAsset = Cast<UT4EntityAsset>(InSaveObject);
		if (nullptr != CheckEntityAsset)
		{
			CheckEntityAsset->MarkPackageDirty();
			CheckEntityAsset->ThumbnailImage = InThumbnail;
			return true;
		}
		UT4ContiAsset* CheckContiAsset = Cast<UT4ContiAsset>(InSaveObject);
		if (nullptr != CheckContiAsset)
		{
			CheckContiAsset->MarkPackageDirty();
			CheckContiAsset->ThumbnailImage = InThumbnail;
			return true;
		}
		UT4AnimSetAsset* CheckAnimSetAsset = Cast<UT4AnimSetAsset>(InSaveObject);
		if (nullptr != CheckAnimSetAsset)
		{
			CheckAnimSetAsset->MarkPackageDirty();
			CheckAnimSetAsset->ThumbnailImage = InThumbnail;
			return true;
		}
		UT4WorldAsset* CheckWorldAsset = Cast<UT4WorldAsset>(InSaveObject); // #54
		if (nullptr != CheckWorldAsset)
		{
			CheckWorldAsset->MarkPackageDirty();
			CheckWorldAsset->ThumbnailImage = InThumbnail;
			return true;
		}
		return false;
	}

	bool GetPointOfInterest(
		FT4EditorTestAutomation* InTestAutomation,
		int32 InSelectIndex,
		FT4EditorPointOfInterest* OutData
	) // #103
	{
		if (InSelectIndex >= InTestAutomation->PointOfInterests.Num())
		{
			return false;
		}
		*OutData = InTestAutomation->PointOfInterests[InSelectIndex];
		return true;
	}

	bool UpdatePointOfInterest(
		UObject* InSaveObject,
		FT4EditorTestAutomation* InTestAutomation,
		int32 InSelectIndex,
		FT4EditorPointOfInterest* InUpdateData
	) // #103
	{
		check(nullptr != InSaveObject);
		if (InSelectIndex >= InTestAutomation->PointOfInterests.Num())
		{
			return false;
		}
		InSaveObject->MarkPackageDirty();
		FT4EditorPointOfInterest& Data = InTestAutomation->PointOfInterests[InSelectIndex];
		Data = *InUpdateData;
		Data.Name = InTestAutomation->TransientName;
		return true;
	}

	bool AddPointOfInterest(
		UObject* InSaveObject,
		FT4EditorTestAutomation* InTestAutomation,
		FT4EditorPointOfInterest* InNewData
	) // #103
	{
		check(nullptr != InSaveObject);
		InSaveObject->MarkPackageDirty();
		FT4EditorPointOfInterest& NewData = InTestAutomation->PointOfInterests.AddDefaulted_GetRef();
		NewData = *InNewData;
		return true;
	}

	bool RemovePointOfInterest(
		UObject* InSaveObject,
		FT4EditorTestAutomation* InTestAutomation,
		int32 InRemoveIndex
	) // #103
	{
		check(nullptr != InSaveObject);
		if (InRemoveIndex >= InTestAutomation->PointOfInterests.Num())
		{
			return false;
		}
		InSaveObject->MarkPackageDirty();
		InTestAutomation->PointOfInterests.RemoveAt(InRemoveIndex);
		return true;
	}

}
#endif
// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetEnvironmentUtils.h"

#include "T4Asset/Classes/World/T4EnvironmentAsset.h" // #92
#include "T4Asset/Public/T4AssetUtils.h"

#include "JsonObjectConverter.h" // #104

#include "T4RehearsalEditorInternal.h"

/**
  * #94
 */

#if WITH_EDITOR

namespace T4AssetUtil
{

	bool EnviromentTimeTagUpdate(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		const FT4EnvTimeTagData* InData,
		FString& OutErrorMessage
	) // #90, #95
	{
		if (nullptr == InEnvironmentAsset)
		{
			OutErrorMessage = TEXT("No Set EnvironmentAsset");
			return false;
		}
		FT4EnvTimeTagSetData& TimeTagSetData = InEnvironmentAsset->TimeTagSetData;
		if (!TimeTagSetData.TimeTagMap.Contains(InTimeTagName))
		{
			OutErrorMessage = TEXT("TimeTag Not found");
			return false;
		}
		InEnvironmentAsset->MarkPackageDirty();
		TimeTagSetData.TimeTagMap[InTimeTagName] = *InData;
		return true;
	}

	bool EnviromentTimeTagAdd(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		const FT4EnvTimeTagData* InData,
		FString& OutErrorMessage
	) // #90
	{
		if (nullptr == InEnvironmentAsset)
		{
			OutErrorMessage = TEXT("No Set EnvironmentAsset");
			return false;
		}
		FT4EnvTimeTagSetData& TimeTagSetData = InEnvironmentAsset->TimeTagSetData;
		if (TimeTagSetData.TimeTagMap.Contains(InTimeTagName))
		{
			OutErrorMessage = TEXT("TimeTag Already exists");
			return false;
		}
		InEnvironmentAsset->MarkPackageDirty();
		TimeTagSetData.TimeTagMap.Add(InTimeTagName, *InData);
		return true;
	}

	bool EnviromentTimeTagRemove(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		FString& OutErrorMessage
	) // #90
	{
		if (nullptr == InEnvironmentAsset)
		{
			OutErrorMessage = TEXT("No Set EnvironmentAsset");
			return false;
		}
		FT4EnvTimeTagSetData& TimeTagSetData = InEnvironmentAsset->TimeTagSetData;
		if (!TimeTagSetData.TimeTagMap.Contains(InTimeTagName))
		{
			OutErrorMessage = TEXT("TimeTag Not found");
			return false;
		}
		InEnvironmentAsset->MarkPackageDirty();
		TimeTagSetData.TimeTagMap.Remove(InTimeTagName);
		return true;
	}

	bool EnviromentTimeTagCopyToClipboard(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		FString& OutJsonTimeTagDataString,
		FString& OutErrorMessage
	) // #104
	{
		if (nullptr == InEnvironmentAsset)
		{
			OutErrorMessage = TEXT("No Set EnvironmentAsset");
			return false;
		}
		const FT4EnvTimeTagSetData& TimeTagSetData = InEnvironmentAsset->TimeTagSetData;
		if (!TimeTagSetData.TimeTagMap.Contains(InTimeTagName))
		{
			OutErrorMessage = TEXT("TimeTag Not found");
			return false;
		}
		const FT4EnvTimeTagData& EnvTimeTagData = TimeTagSetData.TimeTagMap[InTimeTagName];
		bool bResult = FJsonObjectConverter::UStructToJsonObjectString(EnvTimeTagData, OutJsonTimeTagDataString);
		if (!bResult)
		{
			OutErrorMessage = TEXT("Unable to write out json");
		}
		return bResult;
	}

	bool EnviromentTimeTagPastToClipboard(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		const FString& InJsonTimeTagDataString,
		FString& OutErrorMessage
	) // #104
	{
		if (nullptr == InEnvironmentAsset)
		{
			OutErrorMessage = TEXT("No Set EnvironmentAsset");
			return false;
		}
		FT4EnvTimeTagSetData& TimeTagSetData = InEnvironmentAsset->TimeTagSetData;
		if (!TimeTagSetData.TimeTagMap.Contains(InTimeTagName))
		{
			OutErrorMessage = TEXT("TimeTag Not found");
			return false;
		}
		FT4EnvTimeTagData& EnvTimeTagData = TimeTagSetData.TimeTagMap[InTimeTagName];
		InEnvironmentAsset->MarkPackageDirty();
		if (!FJsonObjectConverter::JsonObjectStringToUStruct(InJsonTimeTagDataString, &EnvTimeTagData, 0, 0))
		{
			OutErrorMessage = TEXT("Struct Missmatched");
			return false;
		}
		return true;
	}

	bool EnviromentAssetSave(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FString& OutErrorMessage
	) // #90
	{
		if (nullptr == InEnvironmentAsset)
		{
			OutErrorMessage = TEXT("No Set EnvironmentAsset");
			return false;
		}
		bool bResult = true;
		UPackage* Package = InEnvironmentAsset->GetOutermost();
		check(nullptr != Package);
		bool bWasAnimSetPackageDirty = Package->IsDirty();
		if (bWasAnimSetPackageDirty)
		{
			bResult = T4AssetUtil::SaveAsset(InEnvironmentAsset, true);
		}
		return bResult;
	}

}
#endif
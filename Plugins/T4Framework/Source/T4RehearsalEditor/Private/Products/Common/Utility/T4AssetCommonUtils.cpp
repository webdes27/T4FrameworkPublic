// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetCommonUtils.h"

#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #104
#include "T4Asset/Public/T4AssetUtils.h"

#include "ObjectTools.h"
#include "IImageWrapper.h" // #84
#include "IImageWrapperModule.h" // #84
#include "Misc/ObjectThumbnail.h"
#include "Modules/ModuleManager.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #94
 */

namespace T4AssetUtil
{

	// #84
	const FT4LevelThumbnailData* MapEntityGetSubLevelThumbnail(
		UT4MapEntityAsset* InMapEntityAsset,
		const FName InLevelAssetName
	)
	{
		check(nullptr != InMapEntityAsset);
		if (!InMapEntityAsset->LevelThumbnailDatas.Contains(InLevelAssetName))
		{
			return nullptr;
		}
		FT4LevelThumbnailData& SubLevelThumbnail = InMapEntityAsset->LevelThumbnailDatas[InLevelAssetName];
		if (0 >= SubLevelThumbnail.RawImageData.Num())
		{
			if (0 >= SubLevelThumbnail.CompressedImageData.Num())
			{
				return nullptr;
			}
			SubLevelThumbnail.RawImageData.Empty();
			// Assign thumbnail compressor/decompressor
			// FObjectThumbnail::SetThumbnailCompressor(new FPNGThumbnailCompressor());
			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
			if (ImageWrapper.IsValid())
			{
				if (ImageWrapper->SetCompressed(
					&SubLevelThumbnail.CompressedImageData[0],
					SubLevelThumbnail.CompressedImageData.Num()
				))
				{
					if (ImageWrapper->GetWidth() != SubLevelThumbnail.ImageWidth ||
						ImageWrapper->GetHeight() != SubLevelThumbnail.ImageHeight)
					{
						return nullptr;
					}
					const TArray<uint8>* RawData = NULL;
					if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData))	// @todo CB: Eliminate image copy here? (decompress straight to buffer)
					{
						SubLevelThumbnail.RawImageData = *RawData;
					}
				}
			}
		}
		if (0 >= SubLevelThumbnail.RawImageData.Num())
		{
			return nullptr;
		}
		return &SubLevelThumbnail;
	}

	bool MapEntityAddOrUpdateSubLevelThumbnail(
		UT4MapEntityAsset* InMapEntityAsset,
		const FName InLevelAssetName,
		FObjectThumbnail* InObjectThumbnail
	)
	{
		check(nullptr != InMapEntityAsset);
		if (0 >= InObjectThumbnail->AccessImageData().Num())
		{
			return false;
		}
		InMapEntityAsset->MarkPackageDirty();

		{
			UPackage* ThumbnailPackage = InMapEntityAsset->GetOutermost(); // #84
			check(nullptr != ThumbnailPackage);
			ThumbnailPackage->MarkPackageDirty();
			ThumbnailTools::CacheThumbnail(InLevelAssetName.ToString(), InObjectThumbnail, ThumbnailPackage);
		}

		FT4LevelThumbnailData& LevelSubLevelThumbnail = InMapEntityAsset->LevelThumbnailDatas.FindOrAdd(InLevelAssetName);
		LevelSubLevelThumbnail.CompressedImageData.Empty();
		LevelSubLevelThumbnail.RawImageData.Empty();

		LevelSubLevelThumbnail.ImageWidth = InObjectThumbnail->GetImageWidth();
		LevelSubLevelThumbnail.ImageHeight = InObjectThumbnail->GetImageHeight();
		LevelSubLevelThumbnail.RawImageData = InObjectThumbnail->GetUncompressedImageData();
		if (0 >= LevelSubLevelThumbnail.RawImageData.Num())
		{
			return false;
		}
		bool bResult = false;
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		if (ImageWrapper.IsValid())
		{
			if (ImageWrapper->SetRaw(
				&LevelSubLevelThumbnail.RawImageData[0],
				LevelSubLevelThumbnail.RawImageData.Num(),
				LevelSubLevelThumbnail.ImageWidth,
				LevelSubLevelThumbnail.ImageHeight,
				ERGBFormat::RGBA,
				8
			))
			{
				LevelSubLevelThumbnail.CompressedImageData = ImageWrapper->GetCompressed();
				bResult = true;
			}
		}
		return bResult;
	}
	// ~#84

}

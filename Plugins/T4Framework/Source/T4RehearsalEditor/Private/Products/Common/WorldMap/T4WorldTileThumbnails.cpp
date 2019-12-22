// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldTileThumbnails.h"
#include "T4LevelCollectionModel.h"

#include "Products/Common/Utility/T4AssetCommonUtils.h" // #84, #88

#include "T4Asset/Classes/World/T4WorldAsset.h" // #84

#include "Misc/ObjectThumbnail.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "UObject/Package.h"
#include "Brushes/SlateColorBrush.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "Engine/Texture2DDynamic.h"
#include "ObjectTools.h"
#include "Slate/SlateTextures.h"

#include "T4WorldTileModel.h"

static const double TileThumbnailUpdateCooldown = 0.005f;
static const FSlateColorBrush ThumbnailDefaultBrush(FLinearColor::White);

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
FT4TileThumbnail::FT4TileThumbnail(
	FSlateTextureRenderTarget2DResource* InThumbnailRenderTarget, 
	const FT4WorldTileModel& InTileModel, 
	const FIntPoint& InSlotAllocation
)
	: TileModel(InTileModel)
	, ThumbnailRenderTarget(InThumbnailRenderTarget)
	, SlotAllocation(InSlotAllocation)
{
}

FIntPoint FT4TileThumbnail::GetThumbnailSlotAllocation() const
{
	return SlotAllocation;
}

FSlateTextureDataPtr FT4TileThumbnail::UpdateThumbnail()
{
	// #84 : refer => FTileThumbnail::UpdateThumbnail
	if (TileModel.IsPersistent())
	{
		return ToSlateTextureDataEx(nullptr);
	}

	UT4MapEntityAsset* MapEntityAsset = TileModel.GetThumbnailTargetAsset(); // #84
	if (nullptr == MapEntityAsset)
	{
		return ToSlateTextureDataEx(nullptr);
	}

	const bool bWorldCompositionEnabled = TileModel.IsWorldCompositionEnabled(); // #91
	if (!bWorldCompositionEnabled)
	{
		// #91 : World Single 이기 때문에 항상 로드되어 있기 때문에 Thumbnail 은 런타임에만 사용하고
		//       World Asset 에 저장하지 않는다. 대신, 높은 해상도를 사용한다.
		ULevel* LoadedLevel = TileModel.GetThumbnailLoadedLevel();
		check(nullptr != LoadedLevel);

		// Editor UWorld 를 넣어서 WorldThumbnailRenderer 를 통해 Thumbnail 을 생성하도록 처리한다.
		UWorld* EditorWorld = TileModel.GetEditorWorld();
		check(nullptr != EditorWorld);

		// Set persistent world package as transient to avoid package dirtying during thumbnail rendering
		FT4UnmodifiableObject ImmuneWorld(EditorWorld);

		FIntPoint RTSize = ThumbnailRenderTarget->GetSizeXY();
		FObjectThumbnail NewThumbnail;
		// Generate the thumbnail
		ThumbnailTools::RenderThumbnail(
			EditorWorld, // #91 : UT4WorldCustomThumbnailRenderer
			RTSize.X,
			RTSize.Y,
			ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush,
			ThumbnailRenderTarget,
			&NewThumbnail
		);
		return ToSlateTextureData(&NewThumbnail);
	}

	// #84 : World Composition 을 사용하기 때문에 SubLevel 수만큼 Thumbnamil 이 생기고, (대신 작은 해상도 사용)
	//       맵을 로드해 런타임 렌더링하지 Thumbnail 을 볼 수 있도록 World Asset 에 Thumbnail 을 저장한다.
	ULevel* LoadedLevel = TileModel.GetThumbnailLoadedLevel();
	if (nullptr == LoadedLevel)
	{
		// #84 : PreviewWorld 에 Level 이 로드되지 않았다면 WorldAsset 에 저장된 Thumbnail 을 로드해서 출력해준다.
		const FName LevelAssetName = TileModel.GetAssetName();
		const FT4LevelThumbnailData* WorldSubLevelThumbnail = T4AssetUtil::MapEntityGetSubLevelThumbnail(
			MapEntityAsset, 
			LevelAssetName
		); // #84
		if (nullptr != WorldSubLevelThumbnail)
		{
			return ToSlateTextureDataEx(WorldSubLevelThumbnail);
		}
	}
	else
	{
		FIntPoint RTSize = ThumbnailRenderTarget->GetSizeXY();
		FObjectThumbnail NewThumbnail;

		// Set persistent world package as transient to avoid package dirtying during thumbnail rendering
		FT4UnmodifiableObject ImmuneWorld(LoadedLevel->OwningWorld);

		ThumbnailTools::RenderThumbnail(
			LoadedLevel, // ULevelThumbnailRenderer
			RTSize.X,
			RTSize.Y,
			ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush,
			ThumbnailRenderTarget,
			&NewThumbnail
		);

		const FName LevelAssetName = TileModel.GetAssetName();
		T4AssetUtil::MapEntityAddOrUpdateSubLevelThumbnail(
			MapEntityAsset, 
			LevelAssetName, 
			&NewThumbnail
		); // #84

		return ToSlateTextureData(&NewThumbnail);
	}

	return ToSlateTextureData(nullptr);
}

FSlateTextureDataPtr FT4TileThumbnail::ToSlateTextureData(const FObjectThumbnail* ObjectThumbnail) const
{
	FSlateTextureDataPtr Result;
	
	if (ObjectThumbnail)
	{
		FIntPoint ImageSize(ObjectThumbnail->GetImageWidth(), ObjectThumbnail->GetImageHeight());
		FIntPoint TargetSize = ThumbnailRenderTarget->GetSizeXY();
		if (TargetSize == ImageSize)
		{
			const TArray<uint8>& ImageData = ObjectThumbnail->GetUncompressedImageData();
			if (ImageData.Num())
			{
				Result = MakeShareable(new FSlateTextureData(ImageData.GetData(), ImageSize.X, ImageSize.Y, 4));
			}
		}
	}

	return Result;
}

FSlateTextureDataPtr FT4TileThumbnail::ToSlateTextureDataEx(const FT4LevelThumbnailData* InWorldSubLevelThumbnail) const // #84
{
	FSlateTextureDataPtr Result;

	if (InWorldSubLevelThumbnail)
	{
		FIntPoint ImageSize(InWorldSubLevelThumbnail->ImageWidth, InWorldSubLevelThumbnail->ImageHeight);
		FIntPoint TargetSize = ThumbnailRenderTarget->GetSizeXY();
		if (TargetSize == ImageSize)
		{
			const TArray<uint8>& ImageData = InWorldSubLevelThumbnail->RawImageData;
			if (ImageData.Num())
			{
				Result = MakeShareable(new FSlateTextureData(ImageData.GetData(), ImageSize.X, ImageSize.Y, 4));
			}
		}
	}

	return Result;
}

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------

FT4TileAtlasPage::FT4TileAtlasPage(int32 InTileThumbnailSize, int32 InTileThumbnailAtlasSize)
	: TileThumbnailSize(InTileThumbnailSize) // #91: per World
	, TileThumbnailAtlasSize(InTileThumbnailAtlasSize) // #91: per World
	, TileThumbnailAtlasDim(InTileThumbnailAtlasSize / InTileThumbnailSize) // #91: per World
{
	AtlasTexture = UTexture2DDynamic::Create(
		TileThumbnailAtlasSize, 
		TileThumbnailAtlasSize, 
		FTexture2DDynamicCreateInfo(PF_B8G8R8A8, false)
	);
	AtlasTexture->AddToRoot();

	static int32 NextPageUniqueID = 1;
	FName AtlasPageName = *FString::Printf(TEXT("WorldMapAtlasPage_%d"), NextPageUniqueID++);
	
	int32 NumAtlasSlots = TileThumbnailAtlasDim * TileThumbnailAtlasDim; // #91: per World
	AtlasSlotArrays.SetNum(NumAtlasSlots);
	for (int32 i = 0; i < NumAtlasSlots; ++i)
	{
		FTileAtlasSlot& Slot = AtlasSlotArrays[i];
		
		Slot.bOccupied = false;
		Slot.SlotBrush = new FSlateDynamicImageBrush(
			AtlasTexture, 
			FVector2D(TileThumbnailAtlasSize, TileThumbnailAtlasSize),
			AtlasPageName
		);

		int32 SlotOffsetX = i % TileThumbnailAtlasDim;
		int32 SlotOffsetY = i / TileThumbnailAtlasDim;
		FVector2D StartUV = FVector2D(SlotOffsetX, SlotOffsetY)/TileThumbnailAtlasDim;
		FVector2D SizeUV = FVector2D(1.0f, 1.0f)/TileThumbnailAtlasDim;

		Slot.SlotBrush->SetUVRegion(FBox2D(StartUV, StartUV+SizeUV));
	}
}

FT4TileAtlasPage::~FT4TileAtlasPage()
{
	for (int32 i = 0; i < AtlasSlotArrays.Num(); ++i)
	{
		FTileAtlasSlot& Slot = AtlasSlotArrays[i];
		delete Slot.SlotBrush;
		Slot.SlotBrush = nullptr;
	}
	
	AtlasTexture->RemoveFromRoot();
	AtlasTexture->MarkPendingKill();
	AtlasTexture = nullptr;
}

void FT4TileAtlasPage::SetOccupied(int32 SlotIdx, bool bOccupied)
{
	check(SlotIdx < AtlasSlotArrays.Num());
	AtlasSlotArrays[SlotIdx].bOccupied = bOccupied;
}

bool FT4TileAtlasPage::HasOccupiedSlots() const
{
	for (int32 i = 0; i < AtlasSlotArrays.Num(); ++i)
	{
		if (AtlasSlotArrays[i].bOccupied)
		{
			return true;
		}
	}

	return false;
}

int32 FT4TileAtlasPage::GetFreeSlotIndex() const
{
	int32 Result = INDEX_NONE;
	for (int32 i = 0; i < AtlasSlotArrays.Num(); ++i)
	{
		if (!AtlasSlotArrays[i].bOccupied)
		{
			Result = i;
		}
	}
	return Result;
}

const FSlateBrush* FT4TileAtlasPage::GetSlotBrush(int32 SlotIdx) const
{
	check(SlotIdx < AtlasSlotArrays.Num());
	return AtlasSlotArrays[SlotIdx].SlotBrush;
}

void FT4TileAtlasPage::UpdateSlotImageData(int32 SlotIdx, FSlateTextureDataPtr ImageData)
{
	if (AtlasTexture && AtlasTexture->Resource)
	{
		int32 SlotX = (SlotIdx % TileThumbnailAtlasDim)*TileThumbnailSize;
		int32 SlotY = (SlotIdx / TileThumbnailAtlasDim)*TileThumbnailSize;
		
		const FUpdateTextureRegion2D UpdateRegion(
			SlotX, SlotY,		// Dest X, Y
			0, 0,				// Source X, Y
			TileThumbnailSize,	// Width
			TileThumbnailSize	// Height
		);
	
		struct FSlotUpdateContext
		{
			FTextureRHIRef			TextureRHI;
			FSlateTextureDataPtr	ImageData;
			uint32					SourcePitch;
			FUpdateTextureRegion2D	Region;
		} 
		Context = 
		{
			AtlasTexture->Resource->TextureRHI,
			ImageData,
			TileThumbnailSize*4,
			UpdateRegion
		};
			
		ENQUEUE_RENDER_COMMAND(UpdateSlotImageData)(
			[Context](FRHICommandList& RHICmdList)
			{
				FRHITexture2D* RHITexture2D = (FRHITexture2D*)Context.TextureRHI.GetReference();
				RHIUpdateTexture2D(RHITexture2D, 0, Context.Region, Context.SourcePitch, Context.ImageData->GetRawBytesPtr());
			});
	}
}

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
FT4TileThumbnailCollection::FT4TileThumbnailCollection(int32 InTileThumbnailSize, int32 InTileThumbnailAtlasSize)
	: LastThumbnailUpdateTime(0.0)
	, TileThumbnailSize(InTileThumbnailSize) // #91: per World
	, TileThumbnailAtlasSize(InTileThumbnailAtlasSize) // #91: per World
{
	SharedThumbnailRT = new FSlateTextureRenderTarget2DResource(
		FLinearColor::Black, 
		TileThumbnailSize, 
		TileThumbnailSize, 
		PF_B8G8R8A8, 
		SF_Bilinear, 
		TA_Wrap,
		TA_Wrap, 
		0.0f
	);

	BeginInitResource(SharedThumbnailRT);
}

FT4TileThumbnailCollection::~FT4TileThumbnailCollection()
{
	for (int32 i = 0; i < AtlasPages.Num(); ++i)
	{
		delete AtlasPages[i];
		AtlasPages[i] = nullptr;
	}

	TileThumbnailsMap.Reset();
	
	BeginReleaseResource(SharedThumbnailRT);
	FlushRenderingCommands();
	delete SharedThumbnailRT;
}

void FT4TileThumbnailCollection::RegisterTile(const FT4WorldTileModel& InTileModel)
{
	FName TileName = InTileModel.GetLongPackageName();
	FIntPoint SlotAllocation = AllocateSlot();

	TileThumbnailsMap.Add(TileName, FT4TileThumbnail(SharedThumbnailRT, InTileModel, SlotAllocation));
}

void FT4TileThumbnailCollection::UnregisterTile(const FT4WorldTileModel& InTileModel)
{
	FName TileName = InTileModel.GetLongPackageName();
	
	const FT4TileThumbnail* TileThumbnail = TileThumbnailsMap.Find(TileName);
	if (TileThumbnail)
	{
		ReleaseSlot(TileThumbnail->GetThumbnailSlotAllocation());
		TileThumbnailsMap.Remove(TileName);
	}
}

const FSlateBrush* FT4TileThumbnailCollection::UpdateTileThumbnail(const FT4WorldTileModel& InTileModel)
{
	FName TileName = InTileModel.GetLongPackageName();
	FT4TileThumbnail* TileThumbnail = TileThumbnailsMap.Find(TileName);
	
	if (TileThumbnail)
	{
		auto ImageData = TileThumbnail->UpdateThumbnail();
		if (ImageData.IsValid())
		{
			LastThumbnailUpdateTime = FPlatformTime::Seconds();
				
			FIntPoint SlotAllocation = TileThumbnail->GetThumbnailSlotAllocation();
			AtlasPages[SlotAllocation.X]->UpdateSlotImageData(SlotAllocation.Y, ImageData);
			return AtlasPages[SlotAllocation.X]->GetSlotBrush(SlotAllocation.Y);
		}
	}

	return &ThumbnailDefaultBrush;
}

const FSlateBrush* FT4TileThumbnailCollection::GetTileBrush(const FT4WorldTileModel& InTileModel) const
{
	FName TileName = InTileModel.GetLongPackageName();
	const FT4TileThumbnail* TileThumbnail = TileThumbnailsMap.Find(TileName);
	
	if (TileThumbnail)
	{
		FIntPoint SlotAllocation = TileThumbnail->GetThumbnailSlotAllocation();
		return AtlasPages[SlotAllocation.X]->GetSlotBrush(SlotAllocation.Y);
	}

	return &ThumbnailDefaultBrush;
}

bool FT4TileThumbnailCollection::IsOnCooldown() const
{
	CA_SUPPRESS(6326);
	if (TileThumbnailUpdateCooldown > 0.0)
	{
		const double CurrentTime = FPlatformTime::Seconds();
		return ((CurrentTime - LastThumbnailUpdateTime) < TileThumbnailUpdateCooldown);
	}
	
	return false;
}

FIntPoint FT4TileThumbnailCollection::AllocateSlot()
{
	int32 PageIndex = INDEX_NONE;
	int32 SlotIndex = INDEX_NONE;

	for (int32 i = 0; i < AtlasPages.Num(); ++i)
	{
		SlotIndex = AtlasPages[i]->GetFreeSlotIndex();
		if (SlotIndex != INDEX_NONE)
		{
			PageIndex = i;
			break;
		}
	}

	// Add new page
	if (SlotIndex == INDEX_NONE)
	{
		PageIndex = AtlasPages.Add(new FT4TileAtlasPage(TileThumbnailSize, TileThumbnailAtlasSize));
		SlotIndex = AtlasPages[PageIndex]->GetFreeSlotIndex();
	}
	
	check(PageIndex!= INDEX_NONE && SlotIndex != INDEX_NONE);
	AtlasPages[PageIndex]->SetOccupied(SlotIndex, true);

	return FIntPoint(PageIndex, SlotIndex);
}

void FT4TileThumbnailCollection::ReleaseSlot(const FIntPoint& InSlotAllocation)
{
	AtlasPages[InSlotAllocation.X]->SetOccupied(InSlotAllocation.Y, false);
}

// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Textures/SlateTextureData.h"

class FObjectThumbnail;
struct FT4LevelThumbnailData;
class FSlateTextureRenderTarget2DResource;
class FT4WorldTileModel;
class UTexture2DDynamic;
struct FSlateBrush;
struct FSlateDynamicImageBrush;

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
const int32 DefaultTileThumbnailSize = 512; // #84: 256;
const int32 DefaultTileThumbnailAtlasSize = 2048; // #84: 1024;

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
class FT4TileThumbnail
{
public:
	FT4TileThumbnail(
		FSlateTextureRenderTarget2DResource* InThumbnailRenderTarget, 
		const FT4WorldTileModel& InTileModel, 
		const FIntPoint& InSlotAllocation
	);

	/** Redraw thumbnail */
	FSlateTextureDataPtr UpdateThumbnail();
	FIntPoint GetThumbnailSlotAllocation() const;

private:
	FSlateTextureDataPtr ToSlateTextureData(const FObjectThumbnail* ObjectThumbnail) const;
	FSlateTextureDataPtr ToSlateTextureDataEx(const FT4LevelThumbnailData* InWorldSubLevelThumbnail) const; // #84

private:
	const FT4WorldTileModel&					TileModel;
	/** Shared render target for slate */
	FSlateTextureRenderTarget2DResource*	ThumbnailRenderTarget;
	FIntPoint								SlotAllocation;
};

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
class FT4TileAtlasPage
{
public:
	FT4TileAtlasPage(int32 InTileThumbnailSize, int32 InTileThumbnailAtlasSize);
	~FT4TileAtlasPage();
	
	void SetOccupied(int32 SlotIdx, bool bOccupied);
	bool HasOccupiedSlots() const;
	int32 GetFreeSlotIndex() const;
	const FSlateBrush* GetSlotBrush(int32 SlotIdx) const;
	void UpdateSlotImageData(int32 SlotIdx, FSlateTextureDataPtr ImageData);
		
private:
	struct FTileAtlasSlot
	{
		FSlateDynamicImageBrush*	SlotBrush;
		bool						bOccupied;
	};
	
	TArray<FTileAtlasSlot>		AtlasSlotArrays; // #91 : FTileAtlasSlot		AtlasSlots[TileThumbnailAtlasDim*TileThumbnailAtlasDim];
	UTexture2DDynamic*			AtlasTexture;

	int32 TileThumbnailSize; // #91: per World
	int32 TileThumbnailAtlasSize; // #91: per World
	int32 TileThumbnailAtlasDim; // #91: per World
};

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
class FT4TileThumbnailCollection
{
public:
	FT4TileThumbnailCollection(int32 InTileThumbnailSize, int32 InTileThumbnailAtlasSize);
	~FT4TileThumbnailCollection();

	void RegisterTile(const FT4WorldTileModel& InTileModel);
	void UnregisterTile(const FT4WorldTileModel& InTileModel);
	const FSlateBrush* UpdateTileThumbnail(const FT4WorldTileModel& InTileModel);
	const FSlateBrush* GetTileBrush(const FT4WorldTileModel& InTileModel) const;

	bool IsOnCooldown() const;

private:
	FIntPoint AllocateSlot();
	void ReleaseSlot(const FIntPoint& InSlotAllocation);

private:
	FSlateTextureRenderTarget2DResource*	SharedThumbnailRT;
	TMap<FName, FT4TileThumbnail>			TileThumbnailsMap;
	double									LastThumbnailUpdateTime;

	TArray<FT4TileAtlasPage*>				AtlasPages;

	int32 TileThumbnailSize; // #91: per World
	int32 TileThumbnailAtlasSize; // #91: per World
};

// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "T4ThumbnailDefaultRenderer.generated.h"

/**
  * 
 */

// WARN : #84 SaveThumbnailImage 에 새로운 포멧을 추가해주어야 저장이 된다.

class UTexture2D;

UCLASS()
class UT4ThumbnailDefaultRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

public:
	static void RegisterCustomRenderer();
	static void UnregisterCustomRenderer();

public:
	// UThumbnailRenderer interface.
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas) override;

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const { return nullptr; }
};

UCLASS()
class UT4ContiThumbnailRenderer : public UT4ThumbnailDefaultRenderer
{
	GENERATED_BODY()

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const override;
};

UCLASS()
class UT4MapEntityThumbnailRenderer : public UT4ThumbnailDefaultRenderer
{
	GENERATED_BODY()

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const override;
};

UCLASS()
class UT4ActorEntityThumbnailRenderer : public UT4ThumbnailDefaultRenderer
{
	GENERATED_BODY()

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const override;
};


UCLASS()
class UT4PropEntityThumbnailRenderer : public UT4ThumbnailDefaultRenderer
{
	GENERATED_BODY()

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const override;
};

UCLASS()
class UT4WeaponEntityThumbnailRenderer : public UT4ThumbnailDefaultRenderer
{
	GENERATED_BODY()

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const override;
};

UCLASS()
class UT4CostumeEntityThumbnailRenderer : public UT4ThumbnailDefaultRenderer
{
	GENERATED_BODY()

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const override;
};

UCLASS()
class UT4AnimSetThumbnailRenderer : public UT4ThumbnailDefaultRenderer
{
	GENERATED_BODY()

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const override;
};

UCLASS()
class UT4WorldThumbnailRenderer : public UT4ThumbnailDefaultRenderer // #86
{
	GENERATED_BODY()

protected:
	virtual UTexture2D* GetThumbnailTextureFromObject(UObject* Object) const override;
};

// #91 : refer WorldThumbnailRenderer.h
UCLASS()
class UT4WorldCustomThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_UCLASS_BODY()

	// Begin UThumbnailRenderer Object
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas) override;
	// End UThumbnailRenderer Object

private:
	void GetView(UWorld* World, FSceneViewFamily* ViewFamily, int32 X, int32 Y, uint32 SizeX, uint32 SizeY) const;

private:
	float GlobalOrbitPitchOffset;
	float GlobalOrbitYawOffset;
	bool bUseUnlitScene;
	bool bAllowWorldThumbnails;
};

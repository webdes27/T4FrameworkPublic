// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ThumbnailDefaultRenderer.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h" // #39
#include "T4Asset/Classes/Conti/T4ContiAsset.h"
#include "T4Asset/Classes/Entity/T4EntityAssetMinimal.h"
#include "T4Asset/Classes/World/T4WorldAsset.h" // #86

#include "CanvasTypes.h"
#include "ThumbnailRendering/ThumbnailManager.h"

#include "T4RehearsalEditorInternal.h"

/**
  *
 */
 // #T4_ADD_ENTITY_TAG
void UT4ThumbnailDefaultRenderer::RegisterCustomRenderer()
{
	UThumbnailManager::Get().RegisterCustomRenderer(
		UT4ContiAsset::StaticClass(),
		UT4ContiThumbnailRenderer::StaticClass()
	);

	UThumbnailManager::Get().RegisterCustomRenderer(
		UT4MapEntityAsset::StaticClass(),
		UT4MapEntityThumbnailRenderer::StaticClass()
	);

	UThumbnailManager::Get().RegisterCustomRenderer(
		UT4CharacterEntityAsset::StaticClass(),
		UT4ActorEntityThumbnailRenderer::StaticClass()
	);

	UThumbnailManager::Get().RegisterCustomRenderer(
		UT4PropEntityAsset::StaticClass(),
		UT4PropEntityThumbnailRenderer::StaticClass()
	);

	UThumbnailManager::Get().RegisterCustomRenderer(
		UT4CostumeEntityAsset::StaticClass(),
		UT4CostumeEntityThumbnailRenderer::StaticClass()
	);

	UThumbnailManager::Get().RegisterCustomRenderer(
		UT4WeaponEntityAsset::StaticClass(),
		UT4WeaponEntityThumbnailRenderer::StaticClass()
	);

	UThumbnailManager::Get().RegisterCustomRenderer(
		UT4AnimSetAsset::StaticClass(),
		UT4AnimSetThumbnailRenderer::StaticClass()
	); // #39

	UThumbnailManager::Get().RegisterCustomRenderer(
		UT4WorldAsset::StaticClass(),
		UT4WorldThumbnailRenderer::StaticClass()
	); // #86

	UThumbnailManager::Get().UnregisterCustomRenderer(UWorld::StaticClass()); // #91 : 기존 Renderer 대체
	UThumbnailManager::Get().RegisterCustomRenderer(
		UWorld::StaticClass(),
		UT4WorldCustomThumbnailRenderer::StaticClass()
	); // #91
}

void UT4ThumbnailDefaultRenderer::UnregisterCustomRenderer()
{
	UThumbnailManager::Get().UnregisterCustomRenderer(UWorld::StaticClass()); // #91
	UThumbnailManager::Get().UnregisterCustomRenderer(UT4WorldAsset::StaticClass()); // #86
	UThumbnailManager::Get().UnregisterCustomRenderer(UT4AnimSetAsset::StaticClass());
	UThumbnailManager::Get().UnregisterCustomRenderer(UT4WeaponEntityAsset::StaticClass());
	UThumbnailManager::Get().UnregisterCustomRenderer(UT4CostumeEntityAsset::StaticClass());
	UThumbnailManager::Get().UnregisterCustomRenderer(UT4PropEntityAsset::StaticClass());
	UThumbnailManager::Get().UnregisterCustomRenderer(UT4CharacterEntityAsset::StaticClass());
	UThumbnailManager::Get().UnregisterCustomRenderer(UT4MapEntityAsset::StaticClass());
	UThumbnailManager::Get().UnregisterCustomRenderer(UT4ContiAsset::StaticClass());
}

bool UT4ThumbnailDefaultRenderer::CanVisualizeAsset(UObject* Object)
{
	return GetThumbnailTextureFromObject(Object) != nullptr;
}

void UT4ThumbnailDefaultRenderer::GetThumbnailSize(
	UObject* Object, 
	float Zoom, 
	uint32& OutWidth, 
	uint32& OutHeight
) const
{
	UTexture2D* ObjectTexture = GetThumbnailTextureFromObject(Object);
	if (ObjectTexture != nullptr)
	{
		OutWidth = Zoom * ObjectTexture->GetSizeX();
		OutHeight = Zoom * ObjectTexture->GetSizeY();
	}
	else
	{
		OutWidth = 0;
		OutHeight = 0;
	}
}

void UT4ThumbnailDefaultRenderer::Draw(
	UObject* Object, 
	int32 X, 
	int32 Y, 
	uint32 Width, 
	uint32 Height, 
	FRenderTarget*, 
	FCanvas* Canvas
)
{
	UTexture2D* ObjectTexture = GetThumbnailTextureFromObject(Object);
	if (ObjectTexture != nullptr)
	{
		Canvas->DrawTile(
			X, 
			Y, 
			Width, 
			Height, 
			0.0f,
			0.0f, 
			1.0f, 
			1.0f, 
			FLinearColor::White, 
			ObjectTexture->Resource, 
			false
		);
	}
}

UTexture2D* UT4ContiThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object) const
{
	UT4ContiAsset* ContiAsset = Cast<UT4ContiAsset>(Object);
	if (nullptr != ContiAsset)
	{
		return ContiAsset->ThumbnailImage;
	}
	return nullptr;
}

UTexture2D* UT4MapEntityThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object) const
{
	UT4MapEntityAsset* EntityAsset = Cast<UT4MapEntityAsset>(Object);
	if (nullptr != EntityAsset)
	{
		return EntityAsset->ThumbnailImage;
	}
	return nullptr;
}

UTexture2D* UT4ActorEntityThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object) const
{
	UT4CharacterEntityAsset* EntityAsset = Cast<UT4CharacterEntityAsset>(Object);
	if (nullptr != EntityAsset)
	{
		return EntityAsset->ThumbnailImage;
	}
	return nullptr;
}

UTexture2D* UT4PropEntityThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object) const
{
	UT4PropEntityAsset* EntityAsset = Cast<UT4PropEntityAsset>(Object);
	if (nullptr != EntityAsset)
	{
		return EntityAsset->ThumbnailImage;
	}
	return nullptr;
}

UTexture2D* UT4WeaponEntityThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object) const
{
	UT4WeaponEntityAsset* EntityAsset = Cast<UT4WeaponEntityAsset>(Object);
	if (nullptr != EntityAsset)
	{
		return EntityAsset->ThumbnailImage;
	}
	return nullptr;
}

UTexture2D* UT4CostumeEntityThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object) const
{
	UT4CostumeEntityAsset* EntityAsset = Cast<UT4CostumeEntityAsset>(Object);
	if (nullptr != EntityAsset)
	{
		return EntityAsset->ThumbnailImage;
	}
	return nullptr;
}

UTexture2D* UT4AnimSetThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object) const
{
	UT4AnimSetAsset* AnimSetAsset = Cast<UT4AnimSetAsset>(Object);
	if (nullptr != AnimSetAsset)
	{
		return AnimSetAsset->ThumbnailImage;
	}
	return nullptr;
}

UTexture2D* UT4WorldThumbnailRenderer::GetThumbnailTextureFromObject(UObject* Object) const // #86
{
	UT4WorldAsset* WorldAsset = Cast<UT4WorldAsset>(Object);
	if (nullptr != WorldAsset)
	{
		return WorldAsset->ThumbnailImage;
	}
	return nullptr;
}

// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Entity/T4EntityAsset.h"

#include "Public/Entity/T4EntityKey.h" // #87

#include "Misc/PackageName.h"

#include "T4AssetInternal.h"

/**
  * #24
 */
UT4EntityAsset::UT4EntityAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
#if WITH_EDITORONLY_DATA
	, ThumbnailImage(nullptr)
#endif
{
}

void UT4EntityAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

void UT4EntityAsset::PostLoad()
{
	Super::PostLoad();
}

FName UT4EntityAsset::GetEntityKeyPath() const
{
	// #37 : Make FT4EntityKey
	FName KeyValue = *GetPathName();
	return KeyValue;
}

FString UT4EntityAsset::GetEntityDisplayName() const
{
	return FPackageName::ObjectPathToObjectName(GetPathName());
}

const TCHAR* UT4EntityAsset::GetEntityTypeString() const // #87
{
	FT4EntityKey TempKey(GetEntityType(), GetEntityKeyPath());
	return TempKey.ToTypeString();
}

#if WITH_EDITOR
void UT4EntityAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (nullptr == PropertyChangedEvent.Property) // #77
	{
		return;
	}
	if (PropertyChangedEvent.Property->HasAnyPropertyFlags(CPF_Transient))
	{
		return; // #71 : Transient Property 는 Changed 이벤트를 보내지 않도록 조치
	}
	OnPropertiesChanged().Broadcast();
}
#endif

void UT4EntityAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
}

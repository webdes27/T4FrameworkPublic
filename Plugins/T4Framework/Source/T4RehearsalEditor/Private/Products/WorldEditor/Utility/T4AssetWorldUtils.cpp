// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetWorldUtils.h"

#include "T4Asset/Classes/World/T4WorldAsset.h" // #84
#include "T4Asset/Public/T4AssetUtils.h"

#include "T4Engine/Classes/World/T4MapZoneVolume.h" // #92
#include "T4Engine/Public/T4EngineUtility.h" // #94

#include "AssetToolsModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "FileHelpers.h"

#include "ObjectTools.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "T4RehearsalEditorInternal.h"

#if WITH_EDITOR

namespace T4AssetUtil
{

	bool WorldAddNewMapZoneVolume(
		UT4WorldAsset* InWorldAsset,
		const FName InMapZoneName,
		UWorld* InWorld
	) // #92
	{
		check(nullptr != InWorldAsset);
		check(nullptr != InWorld);
		AT4MapZoneVolume* NewMapZoneVolume = T4EngineUtility::SpawnMapZomeVolumeOnWorld(
			InWorld,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector::OneVector,
			ET4EntityZoneBrushType::Cube,
			false
		);
		if (nullptr == NewMapZoneVolume)
		{
			return false;
		}
		const FT4WorldEditorTransientData& EditorTransientData = InWorldAsset->EditorTransientData;
		NewMapZoneVolume->ZoneName = InMapZoneName;
		NewMapZoneVolume->EnvironmentAsset = EditorTransientData.TransientEnvironmentAsset;
		NewMapZoneVolume->BlendPriority = EditorTransientData.TransientBlendPriority;
		NewMapZoneVolume->BlendInTimeSec = EditorTransientData.TransientBlendInTimeSec;
		NewMapZoneVolume->BlendOutTimeSec = EditorTransientData.TransientBlendOutTimeSec;
		NewMapZoneVolume->DebugColor = EditorTransientData.TransientDebugColor;
		NewMapZoneVolume->SetActorTransform(EditorTransientData.TransientTransform);			
		NewMapZoneVolume->Modify();
		return true;
	}

	bool WorldSelectMapZoneVolumeByName(
		UT4WorldAsset* InWorldAsset,
		const AT4MapZoneVolume* InMapZoneVolume
	) // #92
	{
		check(nullptr != InWorldAsset);
		FT4ScopedTransientTransaction TransientTransaction(InWorldAsset); // #88
		FT4WorldEditorTransientData& EditorTransientData = InWorldAsset->EditorTransientData;
		if (nullptr == InMapZoneVolume)
		{
			EditorTransientData.Reset();
			return true;
		}
		EditorTransientData.TransientMapZoneName = InMapZoneVolume->ZoneName;
		EditorTransientData.TransientEnvironmentAsset = InMapZoneVolume->EnvironmentAsset;
		EditorTransientData.TransientBlendPriority = InMapZoneVolume->BlendPriority;
		EditorTransientData.TransientBlendInTimeSec = InMapZoneVolume->BlendInTimeSec;
		EditorTransientData.TransientBlendOutTimeSec = InMapZoneVolume->BlendOutTimeSec;
		EditorTransientData.TransientTransform = InMapZoneVolume->GetTransform();
		EditorTransientData.TransientDebugColor = InMapZoneVolume->DebugColor;
		return true;
	}

	bool WorldUpdateMapZoneVolume(
		UT4WorldAsset* InWorldAsset,
		AT4MapZoneVolume* OutMapZoneVolume
	) // #92
	{
		check(nullptr != OutMapZoneVolume);
		if (!OutMapZoneVolume->HasAnyFlags(RF_Transient))
		{
			OutMapZoneVolume->MarkPackageDirty();
		}

		const FT4WorldEditorTransientData& EditorTransientData = InWorldAsset->EditorTransientData;
		OutMapZoneVolume->ZoneName = EditorTransientData.TransientMapZoneName;
		OutMapZoneVolume->EnvironmentAsset = EditorTransientData.TransientEnvironmentAsset;
		OutMapZoneVolume->BlendPriority = EditorTransientData.TransientBlendPriority;
		OutMapZoneVolume->BlendInTimeSec = EditorTransientData.TransientBlendInTimeSec;
		OutMapZoneVolume->BlendOutTimeSec = EditorTransientData.TransientBlendOutTimeSec;
		OutMapZoneVolume->SetActorTransform(EditorTransientData.TransientTransform);
		OutMapZoneVolume->DebugColor = EditorTransientData.TransientDebugColor;

		OutMapZoneVolume->CheckGlobalPostProcessSettings(); // #104 : 만약 GlobalZone 으로 리네임 하였다면 PostProcessingSettings instance 생성을 체크해준다.

		OutMapZoneVolume->MarkComponentsRenderStateDirty();
		return true;
	}

}
#endif
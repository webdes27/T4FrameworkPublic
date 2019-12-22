// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldViewModel.h"

#include "Products/Common/ViewModel/T4WorldPreviewViewModel.h"
#include "Products/Common/DetailView/T4EnvironmentDetailObject.h" // #90

#include "Products/WorldEditor/DetailView/T4WorldPreviewLevelDetailObject.h" // #85
#include "Products/WorldEditor/Utility/T4AssetWorldUtils.h" // #90

#include "Products/T4RehearsalEditorUtils.h" // #104

#include "T4Asset/Classes/World/T4WorldAsset.h"
#include "T4Engine/Classes/World/T4MapZoneVolume.h" // #92

#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineUtility.h" // #92

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "FT4WorldViewModel"

/**
  * #83, #104
 */
FT4WorldViewModelOptions::FT4WorldViewModelOptions()
	: WorldAsset(nullptr)
{
}

FT4WorldViewModel::FT4WorldViewModel(const FT4WorldViewModelOptions& InOptions)
	: FT4WorldMapViewModel(InOptions.PreviewViewModelPtr)
	, WorldAssetOwner(InOptions.WorldAsset)
	, WorldPreviewLevelDetailObjectOwner(NewObject<UT4WorldPreviewLevelDetailObject>()) // #85
	, EnvironmentDetailObjectOwner(NewObject<UT4EnvironmentDetailObject>()) // #90
	, MapZoneSelectedOnEditorWorld(NAME_None) // #92
	, TimeTagNameSelected(NAME_None) // #93
{
	check(nullptr != InOptions.WorldAsset);
	if (!InOptions.WorldAsset->MapEntityAsset.IsNull())
	{
		WorldMapSetEntity(InOptions.WorldAsset->MapEntityAsset.LoadSynchronous());
	}
	WorldMapInitialize();
	SetPropertiesChangedDelegate(true);
	GEditor->RegisterForUndo(this);
}

FT4WorldViewModel::~FT4WorldViewModel()
{
}

void FT4WorldViewModel::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(WorldAssetOwner);
	Collector.AddReferencedObject(WorldPreviewLevelDetailObjectOwner); // #85
	Collector.AddReferencedObject(EnvironmentDetailObjectOwner); // #90
}

void FT4WorldViewModel::PostUndo(bool bSuccess)
{
}

void FT4WorldViewModel::Tick(float DeltaTime)
{
	WorldMapTick(DeltaTime);
}

TStatId FT4WorldViewModel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FT4WorldViewModel, STATGROUP_Tickables);
}

void FT4WorldViewModel::Cleanup()
{
	if (MapZoneSelectedOnEditorWorld != NAME_None) // #92
	{
		AT4MapZoneVolume* MapZoneVolume = GetMapZoneSelectedOnEditorWorld();
		if (nullptr != MapZoneVolume)
		{
			MapZoneVolume->OnPropertiesChanged().RemoveAll(this);
		}
		MapZoneSelectedOnEditorWorld = NAME_None;
	}
	WorldMapCleanup();
	SetPropertiesChangedDelegate(false);
	GEditor->UnregisterForUndo(this);
	WorldAssetOwner = nullptr;
	WorldPreviewLevelDetailObjectOwner = nullptr;
	EnvironmentDetailObjectOwner = nullptr; // #90
}

void FT4WorldViewModel::Reset() // #79
{
}

void FT4WorldViewModel::StartPlay()
{
	check(false); // #86 : WorldViewModel 은 Viewport 가 없음으로 여기가 호출되면 안된다.
}

const FString FT4WorldViewModel::GetAssetPath() // #79
{
	if (nullptr == WorldAssetOwner)
	{
		return FString();
	}
	return FSoftObjectPath(WorldAssetOwner).ToString();
}

void FT4WorldViewModel::ChangeWorldEnvironment(FName InTimeTagName) // #94
{
	TimeTagNameSelected = InTimeTagName; // #93
	UpdateWorldEnvironment();
}

AT4MapZoneVolume* FT4WorldViewModel::GetMapZoneSelectedOnEditorWorld() // #92
{
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	check(nullptr != EditorWorld);
	return T4EngineUtility::FindMapZomeVolumeOnWorld(EditorWorld, MapZoneSelectedOnEditorWorld);
}

AT4MapZoneVolume* FT4WorldViewModel::GetMapZoneSelectedOnPreviewWorld() // #92
{
	UWorld* PreviewWorld = GetPreviewWorld();
	check(nullptr != PreviewWorld);
	return T4EngineUtility::FindMapZomeVolumeOnWorld(PreviewWorld, MapZoneSelectedOnEditorWorld);
}

void FT4WorldViewModel::ToggleSimulation() // #86
{
	bSimulating = !bSimulating;

	if (!bSimulating)
	{
		if (nullptr != EnvironmentDetailObjectOwner)
		{
			// #92 : 시뮬레이션이 꺼지면 툴에서 선택된 환경존을 다시 보여준다.
			FString ErrorMessage;
			bool bResult = EnvironmentDetailObjectOwner->ApplyTo(
				GetPreviewWorld(),
				TimeTagNameSelected,
				ErrorMessage
			);
			if (!bResult)
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Warning,
					TEXT("ToggleSimulation : ApplyTo 'PreviewWorld' failed. '%s'"),
					*ErrorMessage
				);
			}
		}
	}
	if (PreviewViewModelPtr.IsValid()) // #93 : 시뮬레이션 모드시 TimeName 변경시 WorldMap 을 함께 갱신해준다.
	{
		if (bSimulating)
		{
			FT4EngineDelegates::OnGameWorldTimeTransition.AddRaw(
				this, &FT4WorldViewModel::HandleOnGameWorldTimeTransition
			);
		}
		else
		{
			FT4EngineDelegates::OnGameWorldTimeTransition.RemoveAll(this);
		}
	}
}

void FT4WorldViewModel::UpdateWorldEnvironment() // #90
{
	if (nullptr == EnvironmentDetailObjectOwner)
	{
		return;
	}
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	if (nullptr != EditorWorld)
	{
		// #92 : 시뮬레이션이 꺼지면 툴에서 선택된 환경존을 다시 보여준다.
		FString ErrorMessage;
		bool bResult = EnvironmentDetailObjectOwner->ApplyTo(
			EditorWorld,
			TimeTagNameSelected,
			ErrorMessage
		);
		if (!bResult)
		{
			UE_LOG(
				LogT4RehearsalEditor,
				Warning,
				TEXT("UpdateWorldEnvironment : ApplyTo 'EditorWorld' failed. '%s'"),
				*ErrorMessage
			);
		}
	}
	if (!IsSimulating())
	{
		// 시뮬레이션 중에는 Preview 는 변경하지 않는다.
		UWorld* PreviewWorld = GetPreviewWorld();
		if (nullptr != PreviewWorld)
		{
			FString ErrorMessage;
			bool bResult = EnvironmentDetailObjectOwner->ApplyTo(
				PreviewWorld,
				TimeTagNameSelected,
				ErrorMessage
			);
			if (!bResult)
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Warning,
					TEXT("UpdateWorldEnvironment : ApplyTo 'PreviewWorld' failed. '%s'"),
					*ErrorMessage
				);
			}
		}
		if (WorldMapPtr.IsValid())
		{
			WorldMapPtr->OnUpdateThumbnails();
		}
	}
}

void FT4WorldViewModel::ChangeMapEntityAsset()
{
	if (nullptr == WorldAssetOwner)
	{
		return;
	}
	if (!WorldAssetOwner->MapEntityAsset.IsNull())
	{
		WorldMapSetEntity(WorldAssetOwner->MapEntityAsset.LoadSynchronous());
	}
	GetOnWorldEditorRefresh().ExecuteIfBound(); // #90
}

void FT4WorldViewModel::SelectEditorWorldMapZone(FName InMapZoneName) // #92
{
	if (MapZoneSelectedOnEditorWorld != NAME_None)
	{
		AT4MapZoneVolume* MapZoneVolume = GetMapZoneSelectedOnEditorWorld();
		if (nullptr != MapZoneVolume)
		{
			MapZoneVolume->OnPropertiesChanged().RemoveAll(this);
		}
	}
	MapZoneSelectedOnEditorWorld = InMapZoneName;
	if (MapZoneSelectedOnEditorWorld != NAME_None)
	{
		AT4MapZoneVolume* MapZoneVolume = GetMapZoneSelectedOnEditorWorld();
		if (nullptr != MapZoneVolume)
		{
			MapZoneVolume->OnPropertiesChanged().AddRaw(this, &FT4WorldViewModel::HandleOnMapZonePropertyChanged);
		}
	}
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnMapZoneSelected(InMapZoneName);
	}
}

void FT4WorldViewModel::HandleOnMapZonePropertyChanged() // #92
{
	if (nullptr == WorldAssetOwner)
	{
		return;
	}
	AT4MapZoneVolume* MapZoneVolume = GetMapZoneSelectedOnEditorWorld();
	if (nullptr != MapZoneVolume)
	{
		T4AssetUtil::WorldSelectMapZoneVolumeByName(WorldAssetOwner, MapZoneVolume);
	}
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnMapZoneSelected(MapZoneSelectedOnEditorWorld);
	}
}

UObject* FT4WorldViewModel::GetEditObject() const // #103
{
	return Cast<UObject>(WorldAssetOwner);
}

FT4EditorTestAutomation* FT4WorldViewModel::GetTestAutomation() const // #103
{
	if (nullptr == WorldAssetOwner)
	{
		return nullptr;
	}
	if (WorldAssetOwner->MapEntityAsset.IsNull())
	{
		return nullptr;
	}
	return &WorldAssetOwner->TestAutomation;
}

void FT4WorldViewModel::TravelPOI(FT4EditorPointOfInterest* InPOIData) // #100, #103
{
	check(nullptr != InPOIData);
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->SetCameraLocationAndRotation(InPOIData->SpawnLocation, InPOIData->SpawnRotation);
		PreviewViewModelPtr->GetGameWorld()->GetController()->SetGameTimeHour(InPOIData->GameTimeHour);
	}
	if (WorldMapPtr.IsValid()) // #90
	{
		WorldMapPtr->OnRequestScrollTo(InPOIData->SpawnLocation, FBox(FVector(-500.0f), FVector(500.0f)));
	}
	SetEditorWorldUpdateCamera(InPOIData->SpawnLocation, InPOIData->SpawnRotation);
}

bool FT4WorldViewModel::GetPOIInfo(FT4EditorPointOfInterest* OutPOIData) // #100, #103
{
	if (!PreviewViewModelPtr.IsValid())
	{
		return false;
	}
	FVector CameraLocation;
	FRotator CameraRotation;
	FVector PlayerLocation;
	if (!PreviewViewModelPtr->GetPlayerViewPoint(CameraLocation, CameraRotation, PlayerLocation))
	{
		return false;
	}
	OutPOIData->MapEntityName = *WorldAssetOwner->MapEntityAsset->GetEntityDisplayName();
	OutPOIData->GameTimeHour = PreviewViewModelPtr->GetGameWorld()->GetController()->GetGameTimeHour();
	OutPOIData->SpawnLocation = (PlayerLocation.IsNearlyZero()) ? CameraLocation : PlayerLocation;
	OutPOIData->SpawnRotation = CameraRotation;
	return true;
}

void FT4WorldViewModel::SetPropertiesChangedDelegate(bool bRegister)
{
	if (nullptr == WorldAssetOwner)
	{
		return;
	}
	if (bRegister)
	{
		WorldAssetOwner->OnPropertiesChanged().AddRaw(
			this,
			&FT4WorldMapViewModel::HandleOnWorldPropertiesChanged
		);
	}
	else
	{
		WorldAssetOwner->OnPropertiesChanged().RemoveAll(this);
	}
}

void FT4WorldViewModel::HandleOnGameWorldTimeTransition(
	IT4GameWorld* InGameWorld,
	const FName InTimeName
) // #93
{
	if (!PreviewViewModelPtr.IsValid())
	{
		return;
	}
	IT4GameWorld* PreviewGameWorld = PreviewViewModelPtr->GetGameWorld();
	if (nullptr == PreviewGameWorld || InGameWorld != PreviewViewModelPtr->GetGameWorld())
	{
		return;
	}
	if (nullptr != EnvironmentDetailObjectOwner)
	{
		// Preview 월드의 설정을 Editor 월드로 세팅해준다.
		// 현재는 미니맵을 찍기 위한 용도인데, 다른 용도로 얼마든지 활용할 수 있을 듯함
		EnvironmentDetailObjectOwner->ApplyTo(GetPreviewWorld(), T4EditorUtil::GetWorld(ET4LayerType::LevelEditor));
	}
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnUpdateThumbnails();
	}
}

#undef LOCTEXT_NAMESPACE
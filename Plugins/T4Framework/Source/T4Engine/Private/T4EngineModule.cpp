// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4EngineModule.h"

#include "Public/T4EngineSettings.h" // #40
#include "Public/T4EngineConstants.h" // #90

#include "World/T4ClientGameWorld.h"
#if (WITH_EDITOR || WITH_SERVER_CODE)
#include "World/T4ServerGameWorld.h"
#endif

#include "T4EngineInternal.h"

#define LOCTEXT_NAMESPACE "T4Engine"

// #87 : 월드 이동 playback 지원
FT4OnGameWorldTravel FT4EngineDelegates::OnGameWorldTravelPre;
FT4OnGameWorldTravel FT4EngineDelegates::OnGameWorldTravelPost;
FT4OnGameWorldTimeTransition FT4EngineDelegates::OnGameWorldTimeTransition; // #93 : 월드 TimeName 변경 알림

/**
  *
 */
class FT4EngineModule : public IT4EngineModule
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

	void OnPreLoadMap(const FString& InURL);
};

IMPLEMENT_MODULE(FT4EngineModule, T4Engine)
DEFINE_LOG_CATEGORY(LogT4Engine)

void InitializeT4GameWorldInstances();
void FT4EngineModule::StartupModule()
{
	UT4EngineSettings* EngineSettings = GetMutableDefault<UT4EngineSettings>();
	check(nullptr != EngineSettings);

	InitializeT4GameWorldInstances();

	{
		FCoreUObjectDelegates::PreLoadMap.AddRaw(this, &FT4EngineModule::OnPreLoadMap); // #115
	}
	{
		// #115
		T4EngineConstant::Initailize();
	}
	{
		// #90
		IT4ConstantTableManager* ConstantTableManager = T4EngineConstant::GetTableManager();
		check(nullptr != ConstantTableManager);

		bool bResult = false;

		bResult = ConstantTableManager->LoadEngineTable(
			ET4EngineConstantTable::WorldZone,
			EngineSettings->WorldZoneConstantTablePath
		); // #92
		ensureMsgf(bResult, TEXT("FT4EngineModule : Failed to load WorldZoneConstantTable. (DefaultT4Framework.ini)"));

		bResult = ConstantTableManager->LoadEngineTable(
			ET4EngineConstantTable::TimeTag,
			EngineSettings->TimeTagConstantTablePath
		); // #90
		ensureMsgf(bResult, TEXT("FT4EngineModule : Failed to load TimeTagConstantTable. (DefaultT4Framework.ini)"));
	}
}

void FT4EngineModule::ShutdownModule()
{
	T4EngineConstant::Finalize(); // #115
}

void FT4EngineModule::OnPreLoadMap(const FString& InURL)
{

}

static IT4GameWorld* GGameWorldInstances[(uint32)ET4LayerType::Max];

void InitializeT4GameWorldInstances()
{
	for (uint32 Itr = 0; Itr < (uint32)ET4LayerType::Max; ++Itr)
	{
		GGameWorldInstances[Itr] = nullptr;
	}
}

IT4GameWorld* T4EngineWorldCreate(
	ET4WorldType InWorldType,
	const FT4WorldConstructionValues& InWorldConstructionValues // #87
)
{
	FT4GameWorld* NewGameWorld = nullptr;
	switch (InWorldType)
	{
		case ET4WorldType::Client:
			NewGameWorld = new FT4ClientGameWorld;
			break;

#if (WITH_EDITOR || WITH_SERVER_CODE)
		case ET4WorldType::Server:
			NewGameWorld = new FT4ServerGameWorld;
			break;
#endif

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Type '%u'"),
					uint32(InWorldType)
				);
				return nullptr;
			}
			break;
	}

	check(nullptr != NewGameWorld);
	bool bResult = NewGameWorld->OnInitialize(InWorldConstructionValues);
	if (!bResult)
	{
		delete NewGameWorld;
		return nullptr;
	}

	// #87 : GameWorld class 내에서 World 관리를 위해 InWorldContext 가 nullptr 일 수도 있도록 디자인을 변경함
	ET4LayerType LayerType = NewGameWorld->GetLayerType();
	check(LayerType < ET4LayerType::Max);
	check(nullptr == GGameWorldInstances[uint8(LayerType)]);
	GGameWorldInstances[uint8(LayerType)] = NewGameWorld;
	return NewGameWorld;
}

void T4EngineWorldDestroy(IT4GameWorld* InGameWorld)
{
	check(nullptr != InGameWorld);
	const ET4LayerType CurrLayerType = InGameWorld->GetLayerType();
	check(nullptr != GGameWorldInstances[uint8(CurrLayerType)]);
	FT4GameWorld* T4GameWorld = static_cast<FT4GameWorld*>(InGameWorld);
	T4GameWorld->OnFinalize();
	delete T4GameWorld;
	GGameWorldInstances[uint8(CurrLayerType)] = nullptr;
}

IT4GameWorld* T4EngineWorldGet(ET4LayerType InLayerType)
{
	return GGameWorldInstances[uint8(InLayerType)];
}

#undef LOCTEXT_NAMESPACE
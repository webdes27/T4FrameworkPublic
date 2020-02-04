// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4FrameModule.h"
#include "T4FrameHandler.h"

#include "Frame/Client/T4ClientGameFrame.h"
#if (WITH_EDITOR || WITH_SERVER_CODE)
#include "Frame/Server/T4ServerGameFrame.h"
#endif

#include "T4FrameInternal.h"

/**
  *
 */
class AHUD;
class UCanvas;
class FT4FrameModule : public IT4FrameModule
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};

IMPLEMENT_MODULE(FT4FrameModule, T4Frame)
DEFINE_LOG_CATEGORY(LogT4Frame)

static FT4GameFrame* GFrameInstances[(uint32)ET4LayerType::Max];

void InitializeFrameInstances()
{
	for (uint32 Itr = 0; Itr < (uint32)ET4LayerType::Max; ++Itr)
	{
		GFrameInstances[Itr] = nullptr;
	}
}

IT4GameFrame* T4FrameCreate(
	ET4FrameType InFrameType,
	const FT4WorldConstructionValues& InWorldConstructionValues // #87
)
{
	FT4GameFrame* NewFrame = nullptr;
	switch (InFrameType)
	{
		case ET4FrameType::Frame_Client:
			NewFrame = new FT4ClientGameFrame;
			break;

#if (WITH_EDITOR || WITH_SERVER_CODE)
		case ET4FrameType::Frame_Server:
			NewFrame = new FT4ServerGameFrame;
			break;
#endif

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Type '%u'"),
					uint32(InFrameType)
				);
				return nullptr;
			}
			break;
	}
	check(nullptr != NewFrame);
	{
		FT4FrameDelegates::OnRegisterGameplayLayerInstancce.ExecuteIfBound(NewFrame); // #42
	}
	bool bInitialized = NewFrame->OnInitialize(InWorldConstructionValues);
	if (!bInitialized)
	{
		delete NewFrame;
		return nullptr;
	}

	ET4LayerType LayerType = NewFrame->GetLayerType();
	check(nullptr == GFrameInstances[uint8(LayerType)]);
	GFrameInstances[uint8(LayerType)] = NewFrame;
	return NewFrame;
}

void T4FrameDestroy(IT4GameFrame* InFrame)
{
	check(nullptr != InFrame);
	const ET4LayerType LayerType = InFrame->GetLayerType();
	FT4GameFrame* DeleteFrame = GFrameInstances[uint8(LayerType)];
	check(nullptr != DeleteFrame);
	DeleteFrame->OnFinalize();
	delete DeleteFrame;
	GFrameInstances[uint8(LayerType)] = nullptr;
}

IT4GameFrame* T4FrameGet(ET4LayerType InLayerType)
{
	return static_cast<IT4GameFrame*>(GFrameInstances[uint8(InLayerType)]);
}

void FT4FrameModule::StartupModule()
{
	InitializeFrameInstances();
	GetFrameHandler().Initialize();
}

void FT4FrameModule::ShutdownModule()
{
	GetFrameHandler().Finalize();
}
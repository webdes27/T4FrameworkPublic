// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayInstance.h"
#include "T4GameplayConsole.h"
#include "T4GameplayHUD.h" // #68

#include "T4GameplaySettings.h" // #45

#include "Mode/T4GameplayModeHandler.h"

#include "GameDB/T4GameDB.h" // #25

#include "PacketHandler/T4PacketHandler_SC.h"
#include "PacketHandler/T4PacketHandler_CS.h"

#include "T4Engine/Public/T4Engine.h"

#include "GameFramework/Controller.h"

#include "T4GameplayInternal.h"

/**
  *
 */
FT4GameplayInstance::FT4GameplayInstance()
	: LayerType(ET4LayerType::Max)
	, GameplayConsole(nullptr)
	, GameplayHUD(nullptr) // #68
	, PacketHandlerSC(nullptr) // #27
	, PacketHandlerCS(nullptr) // #27
#if WITH_EDITOR
	, bInputControlLocked(false)
	, bPlayerChangeDisabled(false) // #72
#endif
{
}

FT4GameplayInstance::~FT4GameplayInstance()
{
	// #42 : GameFramework 소멸 시 인스턴스가 소멸된다!!
	check(nullptr == GameplayConsole);
	check(nullptr == GameplayHUD); // #68
	check(nullptr == PacketHandlerSC);
	check(nullptr == PacketHandlerCS);
	check(!GameplayModeHandler.IsValid());
}

bool FT4GameplayInstance::OnInitialize(ET4LayerType InLayerType)
{
	check(ET4LayerType::Max == LayerType);
	LayerType = InLayerType;
	check(ET4LayerType::Max != LayerType);

	{
		FT4GameDB& GameDB = GetGameDB();

		// #25
		const FSoftObjectPath ClientGameMasterTablePath 
			= GetDefault<UT4GameplaySettings>()->ClientGameMasterTablePath.ToSoftObjectPath(); // #48
		if (!GameDB.Initialize(ClientGameMasterTablePath))
		{
			//check(false);
			return false;
		}
#if 0
		const FSoftObjectPath ServerGameMasterTablePath
			= GetDefault<UT4GameplaySettings>()->ServerGameMasterTablePath.ToSoftObjectPath(); // #48
		if (!GameDB.Initialize(ServerGameMasterTablePath))
		{
			//check(false);
			return false;
		}
#endif
	}

	PacketHandlerSC = new FT4PacketHandlerSC(LayerType);
	PacketHandlerCS = new FT4PacketHandlerCS(LayerType);

#if (WITH_EDITOR || WITH_SERVER_CODE)
	ServerEventManager.Initialize(LayerType); // #63
#endif

#if WITH_EDITOR
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		EditorGameData.Initialize(LayerType); // #60
	}
#endif
	return true;
}

void FT4GameplayInstance::OnFinalize()
{
#if WITH_EDITOR
	if (T4EngineLayer::IsPreview(LayerType))
	{
		EditorGameData.Finalize(); // #60
	}
#endif
#if (WITH_EDITOR || WITH_SERVER_CODE)
	ServerEventManager.Finalize(); // #63
#endif
	if (nullptr != GameplayConsole)
	{
		GameplayConsole->Finalize();
		delete GameplayConsole;
		GameplayConsole = nullptr;
	}
	if (nullptr != GameplayHUD) // #68
	{
		GameplayHUD->Finalize();
		delete GameplayHUD;
		GameplayHUD = nullptr;
	}
	OnReset();
	GetGameDB().Finalize(); // #25
	if (nullptr != PacketHandlerSC)
	{
		delete PacketHandlerSC;
		PacketHandlerSC = nullptr;
	}
	if (nullptr != PacketHandlerCS)
	{
		delete PacketHandlerCS;
		PacketHandlerCS = nullptr;
	}
}

void FT4GameplayInstance::OnReset()
{
	if (GameplayModeHandler.IsValid())
	{
		GameplayModeHandler->Finalize();
		GameplayModeHandler->RemoveFromRoot();
		GameplayModeHandler.Reset();
	}
}

void FT4GameplayInstance::OnStartPlay()
{
	check(ET4LayerType::Max != LayerType);
	if (nullptr == GameplayConsole)
	{
		GameplayConsole = new FT4GameplayConsole(LayerType);
		GameplayConsole->Initialize();
	}
	if (!T4EngineLayer::IsServer(LayerType))
	{
		// #68 서버가 아닐 경우만...
		if (nullptr == GameplayHUD) // #68
		{
			GameplayHUD = new FT4GameplayHUD(LayerType);
			GameplayHUD->Initialize();
		}
	}
}

void FT4GameplayInstance::OnPlayerSpawned(IT4PlayerController* InOwnerPC)
{
	check(nullptr != InOwnerPC);
	if (!T4EngineLayer::IsServer(LayerType))
	{
		check(!GameplayModeHandler.IsValid());
		GameplayModeHandler = NewObject<UT4GameplayModeHandler>();
		check(GameplayModeHandler.IsValid());
		GameplayModeHandler->Initialize(LayerType);
		GameplayModeHandler->AddToRoot();
	}
}

void FT4GameplayInstance::OnProcess(float InDeltaTime)
{
	if (GameplayModeHandler.IsValid())
	{
		GameplayModeHandler->Process(InDeltaTime);
	}
#if (WITH_EDITOR || WITH_SERVER_CODE)
	ServerEventManager.Process(InDeltaTime); // #63
#endif
}

void FT4GameplayInstance::OnDrawHUD(
	FViewport* InViewport, 
	FCanvas* InCanvas,
	FT4HUDDrawInfo& InOutDrawInfo
)
{
	// #68 : Only Client
	check(!T4EngineLayer::IsServer(LayerType));
	if (nullptr == GameplayHUD)
	{
		return;
	}
	GameplayHUD->Draw(InViewport, InCanvas, InOutDrawInfo);
}

FT4GameplayInstance* FT4GameplayInstance::CastFrom(
	IT4GameplayInstance* InGameplayInstance
)
{
	FT4GameplayInstance* GameplayInstance 
		= static_cast<FT4GameplayInstance*>(InGameplayInstance);
	return GameplayInstance;
}

IT4PacketHandlerSC* FT4GameplayInstance::GetPacketHandlerSC()
{
	return static_cast<IT4PacketHandlerSC*>(PacketHandlerSC); // #27
}

IT4PacketHandlerCS* FT4GameplayInstance::GetPacketHandlerCS()
{
	return static_cast<IT4PacketHandlerCS*>(PacketHandlerCS); // #27
}

#if WITH_EDITOR
void FT4GameplayInstance::SetInputControlLock(bool bLock)
{
	// #30
	bInputControlLocked = bLock;
	if (GameplayModeHandler.IsValid())
	{
		GameplayModeHandler->SetInputControlLock(bInputControlLocked);
	}
}

void FT4GameplayInstance::SetPlayerChangeDisable(bool bDisable) // #72
{
	bPlayerChangeDisabled = bDisable;
	if (GameplayModeHandler.IsValid())
	{
		GameplayModeHandler->SetPlayerChangeDisable(bPlayerChangeDisabled);
	}
}
#endif
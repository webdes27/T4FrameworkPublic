// Copyright 2019 Tech4 Labs, Inc. All Rights Reserved.

#include "T4GameplayInstance.h"
#include "T4GameplayConsole.h"

#include "T4GameplaySettings.h" // #45

#include "Mode/T4GameplayModeHandler.h"

#include "GameDB/T4GameDB.h" // #25

#include "PacketHandler/T4PacketHandler_SC.h"
#include "PacketHandler/T4PacketHandler_CS.h"

#include "T4Core/Public/T4CoreMinimal.h"
#include "T4Engine/Public/T4Engine.h"

#include "GameFramework//Controller.h"

#include "T4GameplayInternal.h"

/**
  *
 */
FT4GameplayInstance::FT4GameplayInstance()
	: LayerType(ET4LayerType::Max)
	, GameplayConsole(nullptr)
	, PacketHandlerSC(nullptr) // #27
	, PacketHandlerCS(nullptr) // #27
#if WITH_EDITOR
	, bInputControlLocked(false)
#endif
{
}

FT4GameplayInstance::~FT4GameplayInstance()
{
	// #42 : GameFramework 소멸 시 인스턴스가 소멸된다!!
	check(nullptr == GameplayConsole);
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
		// #25
		const FSoftObjectPath ClientGameMasterTablePath 
			= GetDefault<UT4GameplaySettings>()->ClientGameMasterTablePath.ToSoftObjectPath(); // #48
		if (!GetGameDB().Initialize(ClientGameMasterTablePath))
		{
			//check(false);
			return false;
		}
#if 0
		const FSoftObjectPath ServerGameMasterTablePath
			= GetDefault<UT4GameplaySettings>()->ServerGameMasterTablePath.ToSoftObjectPath(); // #48
		if (!GetGameDB().Initialize(ServerGameMasterTablePath))
		{
			//check(false);
			return false;
		}
#endif
	}

	PacketHandlerSC = new FT4PacketHandlerSC(LayerType);
	PacketHandlerCS = new FT4PacketHandlerCS(LayerType);
	return true;
}

void FT4GameplayInstance::OnFinalize()
{
	if (nullptr != GameplayConsole)
	{
		GameplayConsole->Finalize();
		delete GameplayConsole;
		GameplayConsole = nullptr;
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
	check(nullptr == GameplayConsole);
	GameplayConsole = new FT4GameplayConsole(LayerType);
	GameplayConsole->Initialize();
}

void FT4GameplayInstance::OnPlayerSpawned(IT4PlayerController* InOwnerPC)
{
	check(nullptr != InOwnerPC);
	if (!T4CoreLayer::IsServer(LayerType))
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
}

FT4GameplayInstance* FT4GameplayInstance::CastGameplayInstance(
	IT4GameplayHandler* InGameplayHandler
)
{
	FT4GameplayInstance* GameplayInstance = static_cast<FT4GameplayInstance*>(InGameplayHandler);
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
#endif
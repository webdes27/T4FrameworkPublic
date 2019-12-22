// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameplayEditorGameData.h" // #60
#include "Server/T4ServerEventManager.h" // #63
#include "T4Frame/Public/T4Frame.h"

/**
  * 
 */
class UT4GameplayModeHandler;
class FT4GameplayConsole;
class FT4GameplayHUD;
class IT4PacketHandlerSC;
class IT4PacketHandlerCS;
class FT4PacketHandlerSC;
class FT4PacketHandlerCS;
class FT4GameplayInstance : public IT4GameplayInstance
{
public:
	explicit FT4GameplayInstance();
	~FT4GameplayInstance();

	// IT4GameplayInstance
	bool OnInitialize(ET4LayerType InLayerType) override;
	void OnFinalize() override;

	void OnReset() override;
	void OnStartPlay() override;
	void OnPlayerSpawned(IT4PlayerController* InOwnerPC) override;

	void OnProcess(float InDeltaTime) override;
	void OnDrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo& InOutDrawInfo) override; // #68 : Only Client

#if WITH_EDITOR
	IT4EditorGameData* GetEditorGameData() override 
	{ 
		return static_cast<IT4EditorGameData*>(&EditorGameData); 
	} // #60

	void SetInputControlLock(bool bLock) override; // #30
	void SetPlayerChangeDisable(bool bDisable) override; // #72
#endif

public:
	static FT4GameplayInstance* CastFrom(IT4GameplayInstance* InGameplayInstance);

	IT4PacketHandlerSC* GetPacketHandlerSC(); // #27
	IT4PacketHandlerCS* GetPacketHandlerCS(); // #27

#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerEventManager* GetServerEventManager() { return &ServerEventManager; } // #63
#endif

private:
	ET4LayerType LayerType;

	FT4GameplayConsole* GameplayConsole;
	FT4GameplayHUD* GameplayHUD; // #68
	TWeakObjectPtr<UT4GameplayModeHandler> GameplayModeHandler;

	FT4PacketHandlerSC* PacketHandlerSC; // #27
	FT4PacketHandlerCS* PacketHandlerCS; // #27
	   
#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerEventManager ServerEventManager; // #63
#endif

#if WITH_EDITOR
	FT4GameplayEditorGameData EditorGameData; // #60

	bool bInputControlLocked; // #30
	bool bPlayerChangeDisabled; // #72
#endif
};

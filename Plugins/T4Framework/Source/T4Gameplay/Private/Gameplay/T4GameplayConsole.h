// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Engine/Public/T4EngineTypes.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Development/Tools/ConsoleManager/
 */
class IConsoleVariable;
class IT4PlayerController;
class IT4PacketHandlerCS;
class FT4GameplayConsole
{
public:
	explicit FT4GameplayConsole(ET4LayerType InLayerType);
	~FT4GameplayConsole();

	bool Initialize();
	void Finalize();

private:
	void HandleOnDespawnAll(const TArray<FString>& InArgs); // #104

	void HandleOnActionPlaybackPlay(IConsoleVariable* InVariable); // #68
	void HandleOnActionPlaybackPlayFromEntity(IConsoleVariable* InVariable); // #87
	void HandleOnActionPlaybackStopPlaying(const TArray<FString>& InArgs); // #68

	void HandleOnActionPlaybackRec(IConsoleVariable* InVariable); // #68
	void HandleOnActionPlaybackStopRecording(const TArray<FString>& InArgs); // #68

	void HandleOnChangeToWorld(IConsoleVariable* InVariable);

	void HandleOnSpawnProp(IConsoleVariable* InVariable);
	void HandleOnSpawnCharacter(IConsoleVariable* InVariable);
	
	void HandleOnQuickSpawn(IConsoleVariable* InVariable);

	void HandleOnTakeSnapshotFrom(IConsoleVariable* InVariable);

private:
	IT4PlayerController* GetPlayerController() const;
	IT4PacketHandlerCS* GetPacketHandlerCS() const;

	bool GetSpawnLocation(FVector& OutLocation);

private:
	ET4LayerType LayerType;
	
	IConsoleCommand* ConsoleVarDespawnAllRef; // #104

	// #68
	IConsoleVariable* ConsoleVarActionPlaybackPlayRef; 
	IConsoleCommand* ConsoleVarActionPlaybackStopPlayingRef;
	IConsoleVariable* ConsoleVarActionPlaybackRecRef;
	IConsoleCommand* ConsoleVarActionPlaybackStopRecordingRef;
	// ~#68

	IConsoleVariable* ConsoleVarChangeToWorldRef;
	IConsoleVariable* ConsoleVarSpawnPropRef;
	IConsoleVariable* ConsoleVarSpawnCharacterRef;
	IConsoleVariable* ConsoleVarQuickSpawnRef;
	IConsoleVariable* ConsoleVarTakeSnapshotInLevelEditorRef;
};

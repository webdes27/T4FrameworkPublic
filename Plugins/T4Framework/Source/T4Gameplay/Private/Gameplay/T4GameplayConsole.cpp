// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayConsole.h"
#include "T4GameplayUtils.h"

#include "Public/Protocol/T4PacketCS_Command.h"
#include "GameDB/T4GameDB.h"

#include "Gameplay/T4GameplayInstance.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

static const TCHAR* DefaultActionPlaybackFileName = TEXT("TestPlayback");
static const TCHAR* DefaultActionPlaybackFolderName = TEXT("Gameplay");
static const TCHAR* DefaultActionPlaybackEntityFolderName = TEXT("Entity"); // #87

static const TCHAR* DefaultWorldTravelDBKey = TEXT("DefaultMap");
static const TCHAR* DefaultSpawnPropDBKey = TEXT("Rock");
static const TCHAR* DefaultSpawnCharacterDBKey = TEXT("Mannequin");

/**
  * http://api.unrealengine.com/KOR/Programming/Development/Tools/ConsoleManager/
 */
FT4GameplayConsole::FT4GameplayConsole(ET4LayerType InLayerType)
	: LayerType(InLayerType)
{
}

FT4GameplayConsole::~FT4GameplayConsole()
{
}

bool FT4GameplayConsole::Initialize()
{
	check(LayerType < ET4LayerType::Max);

#if WITH_EDITOR
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr == GameFrame)
	{
		return false;
	}
	if (GameFrame->IsPreviewMode())
	{
		return false; //#68 : PreviewMode 에서는 콘솔 출력을 제외한다.
	}
#endif

	// #15 : per Multiple PIE

	FString CommandName;
	FString LayerPrefix = T4EngineLayer::ToString(LayerType);

	IConsoleManager& ConsoleManager = IConsoleManager::Get();

	// #68
	CommandName = FString::Printf(TEXT("t4.%s.Despawn.All"), *LayerPrefix);
	ConsoleVarDespawnAllRef = ConsoleManager.RegisterConsoleCommand
	(
		*CommandName,
		TEXT("Despawn All")
		TEXT(""),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnDespawnAll),
		ECVF_Default
	); // #104

	// #68
	CommandName = FString::Printf(TEXT("t4.%s.ActionPlayback.Play"), *LayerPrefix);
	ConsoleVarActionPlaybackPlayRef = ConsoleManager.RegisterConsoleVariable
	(
		*CommandName,
		DefaultActionPlaybackFileName,
		TEXT("Action playback Play")
		TEXT(" // Saved/T4Playback/<PlayAssetName>.dat"),
		ECVF_Default
	);
	ConsoleVarActionPlaybackPlayRef->AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnActionPlaybackPlay)
	);

	// #87
	CommandName = FString::Printf(TEXT("t4.%s.ActionPlayback.Entity.Play"), *LayerPrefix);
	ConsoleVarActionPlaybackPlayRef = ConsoleManager.RegisterConsoleVariable
	(
		*CommandName,
		DefaultActionPlaybackFileName,
		TEXT("Action playback Play")
		TEXT(" // Saved/T4Playback/<PlayAssetName>.dat"),
		ECVF_Default
	);
	ConsoleVarActionPlaybackPlayRef->AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnActionPlaybackPlayFromEntity)
	);

	CommandName = FString::Printf(TEXT("t4.%s.ActionPlayback.Play.Stop"), *LayerPrefix);
	ConsoleVarActionPlaybackStopPlayingRef = ConsoleManager.RegisterConsoleCommand
	(
		*CommandName,
		TEXT("Action playback Play")
		TEXT(" // Saved/T4Playback/<PlayAssetName>.dat"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnActionPlaybackStopPlaying),
		ECVF_Default
	);

	CommandName = FString::Printf(TEXT("t4.%s.ActionPlayback.Rec"), *LayerPrefix);
	ConsoleVarActionPlaybackRecRef = ConsoleManager.RegisterConsoleVariable
	(
		*CommandName,
		DefaultActionPlaybackFileName,
		TEXT("Action playback Record")
		TEXT(" // Saved/T4Playback/<RecAssetName>.dat"),
		ECVF_Default
	);
	ConsoleVarActionPlaybackRecRef->AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnActionPlaybackRec)
	);

	CommandName = FString::Printf(TEXT("t4.%s.ActionPlayback.Rec.Stop"), *LayerPrefix);
	ConsoleVarActionPlaybackStopRecordingRef = ConsoleManager.RegisterConsoleCommand
	(
		*CommandName,
		TEXT("Action playback Record")
		TEXT(" // Saved/T4Playback/<RecAssetName>.dat"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnActionPlaybackStopRecording),
		ECVF_Default
	);
	// ~#68

#if 0

	CommandName = FString::Printf(TEXT("t4.%s.World.Change"), *LayerPrefix);
	ConsoleVarChangeToWorldRef = ConsoleManager.RegisterConsoleVariable
	(
		*CommandName,
		DefaultWorldTravelDBKey,
		TEXT("Do Change to World DBKey")
		TEXT(" <= DefaultMap\n")
		TEXT(" <= TestMap\n"),
		ECVF_Default
	);
	ConsoleVarChangeToWorldRef->AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnChangeToWorld)
	);

	CommandName = FString::Printf(TEXT("t4.%s.Spawn.Prop"), *LayerPrefix);
	ConsoleVarSpawnPropRef = ConsoleManager.RegisterConsoleVariable
	(
		*CommandName,
		DefaultSpawnCharacterDBKey,
		TEXT("Do Spawn a Prop DBKey\n")
		TEXT(" <= Rock\n")
		TEXT(" <= Mannequin\n")
		TEXT(" <= Fire\n"),
		ECVF_Default
	);
	ConsoleVarSpawnPropRef->AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnSpawnProp)
	);

	CommandName = FString::Printf(TEXT("t4.%s.Spawn.Char"), *LayerPrefix);
	ConsoleVarSpawnCharacterRef = ConsoleManager.RegisterConsoleVariable
	(
		*CommandName,
		DefaultSpawnCharacterDBKey,
		TEXT("Do Spawn a Character DBKey\n")
		TEXT(" <= Knight\n")
		TEXT(" <= Mannequin"),
		ECVF_Default
	);
	ConsoleVarSpawnCharacterRef->AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnSpawnCharacter)
	);

	CommandName = FString::Printf(TEXT("t4.%s.tt"), *LayerPrefix);
	ConsoleVarQuickSpawnRef = ConsoleManager.RegisterConsoleVariable
	(
		*CommandName,
		DefaultSpawnCharacterDBKey,
		TEXT("Quick Spawn Test (1/2/3/4...)\n"),
		ECVF_Default
	);
	ConsoleVarQuickSpawnRef->AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnQuickSpawn)
	);

	CommandName = FString::Printf(TEXT("t4.%s.Snapshot.Take"), *LayerPrefix);
	ConsoleVarTakeSnapshotInLevelEditorRef = ConsoleManager.RegisterConsoleVariable
	(
		*CommandName,
		DefaultSpawnCharacterDBKey,
		TEXT("Take Snapshot From <Client/LevelEditor/Preview>\n"),
		ECVF_Default
	);
	ConsoleVarTakeSnapshotInLevelEditorRef->AsVariable()->SetOnChangedCallback(
		FConsoleVariableDelegate::CreateRaw(this, &FT4GameplayConsole::HandleOnTakeSnapshotFrom)
	);

#endif

	return true;
}

void FT4GameplayConsole::Finalize()
{
	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	if (nullptr != ConsoleVarDespawnAllRef) // #104
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarDespawnAllRef);
		ConsoleVarDespawnAllRef = nullptr;
	}
	// #68
	if (nullptr != ConsoleVarActionPlaybackRecRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarActionPlaybackPlayRef);
		ConsoleVarActionPlaybackPlayRef = nullptr;
	}
	if (nullptr != ConsoleVarActionPlaybackRecRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarActionPlaybackStopPlayingRef);
		ConsoleVarActionPlaybackStopPlayingRef = nullptr;
	}
	if (nullptr != ConsoleVarActionPlaybackRecRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarActionPlaybackRecRef);
		ConsoleVarActionPlaybackRecRef = nullptr;
	}
	if (nullptr != ConsoleVarActionPlaybackRecRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarActionPlaybackStopRecordingRef);
		ConsoleVarActionPlaybackStopRecordingRef = nullptr;
	}
	// ~#68
	if (nullptr != ConsoleVarChangeToWorldRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarChangeToWorldRef);
		ConsoleVarChangeToWorldRef = nullptr;
	}
	if (nullptr != ConsoleVarSpawnPropRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarSpawnPropRef);
		ConsoleVarSpawnPropRef = nullptr;
	}
	if (nullptr != ConsoleVarSpawnCharacterRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarSpawnCharacterRef);
		ConsoleVarSpawnCharacterRef = nullptr;
	}
	if (nullptr != ConsoleVarQuickSpawnRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarQuickSpawnRef);
		ConsoleVarQuickSpawnRef = nullptr;
	}
	if (nullptr != ConsoleVarTakeSnapshotInLevelEditorRef)
	{
		ConsoleManager.UnregisterConsoleObject(ConsoleVarTakeSnapshotInLevelEditorRef);
		ConsoleVarTakeSnapshotInLevelEditorRef = nullptr;
	}
}

void FT4GameplayConsole::HandleOnDespawnAll(const TArray<FString>& InArgs) // #104
{
	T4GameplayUtil::DoDespawnAll(LayerType, false);
}

void FT4GameplayConsole::HandleOnActionPlaybackPlay(IConsoleVariable* InVariable)
{
	const FName RecAssetName = *(InVariable->GetString());
	if (RecAssetName == NAME_None)
	{
		return;
	}
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	check(nullptr != GameWorld);
	IT4ActionPlaybackController* ActionPlaybackContoller = GameWorld->GetActionPlaybackController();
	if (nullptr == ActionPlaybackContoller)
	{
		return;
	}
	bool bResult = ActionPlaybackContoller->DoPlay(RecAssetName.ToString(), DefaultActionPlaybackFolderName);
	if (!bResult)
	{

	}
}

void FT4GameplayConsole::HandleOnActionPlaybackPlayFromEntity(IConsoleVariable* InVariable) // #87
{
	const FName RecAssetName = *(InVariable->GetString());
	if (RecAssetName == NAME_None)
	{
		return;
	}
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	check(nullptr != GameWorld);
	IT4ActionPlaybackController* ActionPlaybackContoller = GameWorld->GetActionPlaybackController();
	if (nullptr == ActionPlaybackContoller)
	{
		return;
	}
	bool bResult = ActionPlaybackContoller->DoPlay(RecAssetName.ToString(), DefaultActionPlaybackEntityFolderName);
	if (!bResult)
	{

	}
}

void FT4GameplayConsole::HandleOnActionPlaybackStopPlaying(const TArray<FString>& InArgs)
{
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	check(nullptr != GameWorld);
	IT4ActionPlaybackController* ActionPlaybackContoller = GameWorld->GetActionPlaybackController();
	if (nullptr == ActionPlaybackContoller)
	{
		return;
	}
	ActionPlaybackContoller->DoStopPlaying();
}

void FT4GameplayConsole::HandleOnActionPlaybackRec(IConsoleVariable* InVariable)
{
	const FName RecAssetName = *(InVariable->GetString());
	if (RecAssetName == NAME_None)
	{
		return;
	}
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	check(nullptr != GameWorld);
	IT4ActionPlaybackController* ActionPlaybackContoller = GameWorld->GetActionPlaybackController();
	if (nullptr == ActionPlaybackContoller)
	{
		return;
	}
	bool bResult = ActionPlaybackContoller->DoRec(RecAssetName.ToString(), DefaultActionPlaybackFolderName);
	if (!bResult)
	{

	}
}

void FT4GameplayConsole::HandleOnActionPlaybackStopRecording(const TArray<FString>& InArgs)
{
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	check(nullptr != GameWorld);
	IT4ActionPlaybackController* ActionPlaybackContoller = GameWorld->GetActionPlaybackController();
	if (nullptr == ActionPlaybackContoller)
	{
		return;
	}
	ActionPlaybackContoller->DoStopRecording();
}

void FT4GameplayConsole::HandleOnChangeToWorld(IConsoleVariable* InVariable)
{
	const FName WorldName = *(InVariable->GetString());
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameDataID WorldDataID = FT4GameDataID(ET4GameDataType::World, WorldName);
	const FT4GameWorldData* WorldData = GameDB.GetGameData<FT4GameWorldData>(WorldDataID);
	if (nullptr == WorldData)
	{
		return;
	}
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);

	FT4PacketCmdWorldTravelCS NewPacketCS; // #27
	NewPacketCS.WorldDataID = WorldDataID;
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
}

bool FT4GameplayConsole::GetSpawnLocation(FVector& OutLocation)
{
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr == GameFrame)
	{
		return false;
	}

	bool bResult = GameFrame->GetMousePickingLocation(OutLocation);
	if (bResult)
	{
		return true;
	}

	// fallback
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	check(nullptr != GameWorld);

	FVector CameraLocation = GameWorld->GetCameraLocation();
	CameraLocation.Z += 5000.0f;

	FT4HitSingleResult HitResult;
	bResult = GameWorld->GetCollisionSystem()->QueryLineTraceSingle(
		ET4CollisionChannel::WorldStatic,
		CameraLocation,
		-FVector::UpVector,
		DefaultLineTraceMaxDistance,
		FCollisionQueryParams::DefaultQueryParam,
		HitResult
	);
	if (!bResult)
	{
		return false;
	}
	OutLocation = HitResult.ResultLocation;
	return true;
}

void FT4GameplayConsole::HandleOnSpawnProp(IConsoleVariable* InVariable)
{
	FVector SpawnLocation;
	if (!GetSpawnLocation(SpawnLocation))
	{
		return;
	}
	const FName FOName = *InVariable->GetString();
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameDataID FODataID = FT4GameDataID(ET4GameDataType::FO, FOName);
	const FT4GameFOData* FOData = GameDB.GetGameData<FT4GameFOData>(FODataID);
	if (nullptr == FOData)
	{
		return;
	}
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);

	FT4PacketCmdFOEnterCS NewPacketCS; // #27
	NewPacketCS.FODataID = FODataID;
	NewPacketCS.SpawnLocation = SpawnLocation;
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
}

static void DoSpawnCharacter(
	IT4PacketHandlerCS* PacketHandlerCS,
	const FName& InPlayerDataID,
	const FVector& InSpawnLocation
)
{
	check(nullptr != PacketHandlerCS);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameDataID PlayerDataID = FT4GameDataID(ET4GameDataType::Player, InPlayerDataID);
	const FT4GamePlayerData* PlayerData = GameDB.GetGameData<FT4GamePlayerData>(PlayerDataID);
	if (nullptr == PlayerData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4GameplayConsole : failed to player spawn. PlayerDataID '%s' Not Found."),
			*(PlayerDataID.ToString())
		);
		return;
	}
	FT4PacketCmdPCEnterCS NewPacketCS; // #27
	NewPacketCS.PlayerDataID = PlayerDataID;
	NewPacketCS.SpawnLocation = InSpawnLocation;
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
}

void FT4GameplayConsole::HandleOnSpawnCharacter(IConsoleVariable* InVariable)
{
	FVector SpawnLocation;
	if (!GetSpawnLocation(SpawnLocation))
	{
		return;
	}
	DoSpawnCharacter(GetPacketHandlerCS(), *InVariable->GetString(), SpawnLocation);
}

void FT4GameplayConsole::HandleOnQuickSpawn(IConsoleVariable* InVariable)
{
	FVector SpawnLocation;
	if (!GetSpawnLocation(SpawnLocation))
	{
		return;
	}
	FName NewSpawnTableName = NAME_None;
	FString OptionString = InVariable->GetString();
	if (OptionString.Compare(TEXT("1")) == 0)
	{
		NewSpawnTableName = TEXT("DefaultCharacter");
	}
	if (NewSpawnTableName != NAME_None)
	{
		DoSpawnCharacter(GetPacketHandlerCS(), NewSpawnTableName, SpawnLocation);
	}
}

void FT4GameplayConsole::HandleOnTakeSnapshotFrom(IConsoleVariable* InVariable)
{
	const FString LayerNameString = InVariable->GetString();
	ET4LayerType SourceLayerType = T4EngineLayer::FromString(LayerNameString);
	if (ET4LayerType::Max == SourceLayerType)
	{
		return;
	}
	IT4GameWorld* SourceGameWorld = T4EngineWorldGet(SourceLayerType);
	if (nullptr == SourceGameWorld)
	{
		return;
	}
	const uint32 NumSourceSpawnedGameObjects = SourceGameWorld->GetContainer()->GetNumGameObjects();
	if (0 >= NumSourceSpawnedGameObjects)
	{
		return;
	}
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);

#if 0 // #54 : GameWorld:::GetObjectIterator 를 삭제하여 일단 주석 처리, 내부 자료 구조를 IT4GameObject => AT4GameObject 로 바꿨다.
	FT4GameDB& GameDB = GetGameDB();
	for (FConstGameObjectIterator It = SourceGameWorld->GetObjectIterator(); It; ++It)
	{
		IT4GameObject* GameObject = *It;
		const FName GameDataIDName = GameObject->GetGameDataIDName();
		const FT4EntityKey& EntityKey = GameObject->GetEntityKey();
		const FVector RootLocation = GameObject->GetRootLocation();
		const FRotator Rotation = GameObject->GetRotation();
		if (EntityKey.CheckType(ET4EntityType::Character))
		{
			FT4GameDataID TestGameDataID = FT4GameDataID(ET4GameDataType::Player, GameDataIDName);
			const FT4GamePlayerData* PlayerData = GameDB.GetGameData<FT4GamePlayerData>(TestGameDataID);
			if (nullptr != PlayerData)
			{
				FT4PacketCmdPCEnterCS NewPacketCS; // #27
				NewPacketCS.PlayerDataID = TestGameDataID;
				NewPacketCS.SpawnLocation = RootLocation;
				NewPacketCS.SpawnRotation = Rotation;
				PacketHandlerCS->DoSendPacket(&NewPacketCS);
				continue;
			}
			TestGameDataID = FT4GameDataID(ET4GameDataType::NPC, GameDataIDName);
			const FT4GameNPCData* NPCData = GameDB.GetGameData<FT4GameNPCData>(TestGameDataID);
			if (nullptr != NPCData)
			{
				FT4PacketCmdNPCEnterCS NewPacketCS; // #27
				NewPacketCS.NPCDataID = TestGameDataID;
				NewPacketCS.SpawnLocation = RootLocation;
				NewPacketCS.SpawnRotation = Rotation;
				PacketHandlerCS->DoSendPacket(&NewPacketCS);
				continue;
			}
		}
		else if (EntityKey.CheckType(ET4EntityType::Prop))
		{
			FT4GameDataID TestGameDataID = FT4GameDataID(ET4GameDataType::FO, GameDataIDName);
			const FT4GameFOData* FOData = GameDB.GetGameData<FT4GameFOData>(TestGameDataID);
			if (nullptr != FOData)
			{
				FT4PacketCmdFOEnterCS NewPacketCS; // #27
				NewPacketCS.FODataID = TestGameDataID;
				NewPacketCS.SpawnLocation = RootLocation;
				NewPacketCS.SpawnRotation = Rotation;
				PacketHandlerCS->DoSendPacket(&NewPacketCS);
			}
		}
	}
#endif
}

IT4PlayerController* FT4GameplayConsole::GetPlayerController() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	return GameFrame->GetPlayerController();
}

IT4PacketHandlerCS* FT4GameplayConsole::GetPacketHandlerCS() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	FT4GameplayInstance* GameplayInstance = FT4GameplayInstance::CastFrom(
		GameFrame->GetGameplayInstance()
	);
	if (nullptr == GameplayInstance)
	{
		return nullptr;
	}
	return GameplayInstance->GetPacketHandlerCS();
}
// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ServerEventManager.h"

#include "GameDB/T4GameDB.h"

#include "Public/Protocol/T4PacketSC_Action.h" // #50
#include "Public/Protocol/T4PacketSC_Move.h" // #76
#include "Public/Protocol/T4PacketSC_Status.h" // #76

#include "T4Engine/Public/T4Engine.h" // #63
#include "T4Frame/Public/T4Frame.h" // #63

#include "T4GameplayInternal.h"

/**
  *
 */

#if (WITH_EDITOR || WITH_SERVER_CODE)

FT4ServerEventManager::FT4ServerEventManager()
	: LayerType(ET4LayerType::Max)
{
}

FT4ServerEventManager::~FT4ServerEventManager()
{
}

bool FT4ServerEventManager::Initialize(ET4LayerType InLayerType)
{
	LayerType = InLayerType;
	return true;
}

void FT4ServerEventManager::Finalize()
{
	EffectDamages.Empty();
}

void FT4ServerEventManager::Process(float InDeltaTime)
{
	// TODO : temp
	if (0 < EffectDamages.Num())
	{
		for (TArray<FEffectDamageInfo>::TIterator It(EffectDamages); It; ++It)
		{
			FEffectDamageInfo& EventInfo = *It;
			EventInfo.Time -= InDeltaTime;
			if (0.0f < EventInfo.Time)
			{
				continue;
			}
			ProcessEffectDamage(EventInfo.Data);
			EffectDamages.RemoveAt(It.GetIndex());
		}
	}
}

void FT4ServerEventManager::AddEffectDamage(
	float InProcessTimeSec,
	const FEffectDamageData& InEffectData
) // #63 : temp
{
	FEffectDamageInfo& NewEvent = EffectDamages.AddDefaulted_GetRef();
	NewEvent.Time = InProcessTimeSec;
	NewEvent.Data = InEffectData;
}

void FT4ServerEventManager::ProcessEffectDamage(
	const FEffectDamageData& InEffectData
) // #50, #63
{
	FT4GameEffectDataID ResultEffectDataID = InEffectData.EffectDataID;

#if WITH_EDITOR
	IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
	if (nullptr != EditorGameplayHandler) // #60
	{
		if (EditorGameplayHandler->IsUsedGameplaySettings()) // #104
		{
			if (EditorGameplayHandler->IsOverrideEffectData())
			{
				// #63 : 에디터용 스킬을 사용하면, Effect 도 에디터용으로 출력해준다.
				ResultEffectDataID.RowName = EditorGameplayHandler->GetOverrideEffectDataNameID();
			}
		}
	}
#endif

	FT4GameDB& GameDB = GetGameDB();
	const FT4GameEffectData* EffectData = GameDB.GetGameData<FT4GameEffectData>(ResultEffectDataID);
	if (nullptr == EffectData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("ProcessEffectDamage failed. EffectData '%s' not found."),
			*(ResultEffectDataID.ToString())
		);
		return;
	}

	float ApplyAreaRange = EffectData->RawData.AreaRange;
#if WITH_EDITOR
	if (nullptr != EditorGameplayHandler) // #60
	{
		if (EditorGameplayHandler->IsOverrideEffectData())
		{
			const FT4EditorEffectDataInfo& EditorEffectData = EditorGameplayHandler->GetOverrideEffectDataInfo();
			ApplyAreaRange = EditorEffectData.AreaRange;
		}
	}
#endif

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC(LayerType);
	check(nullptr != PacketHandlerSC);

	if (ET4GameEffectType::Direct == EffectData->RawData.EffectType)
	{
		IT4ObjectController* TargetController = GetObjectController(InEffectData.TargetObjectID);
		if (nullptr == TargetController)
		{
			return; // 타겟이 없으면 더 진행할 필요가 없다! Die or Leave ??
		}

		FT4PacketEffectDirectSC NewPacketSC;
		NewPacketSC.ObjectID = InEffectData.TargetObjectID;
		NewPacketSC.EffectDataID = ResultEffectDataID; // #48, #60
		NewPacketSC.AttackerObjectID = InEffectData.AttackerObjectID;
		check(NewPacketSC.ObjectID.IsValid());
		PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);

		TargetController->OnNotifyAIEvent(InEffectData.EventName, InEffectData.AttackerObjectID); // #63
	}
	else if (ET4GameEffectType::Area == EffectData->RawData.EffectType)
	{
		FT4PacketEffectAreaSC NewPacketSC;
		NewPacketSC.TargetLocation = InEffectData.TargetLocation; // #68
		NewPacketSC.EffectDataID = ResultEffectDataID; // #48, #60
		NewPacketSC.AttackerObjectID = InEffectData.AttackerObjectID;
		check(!NewPacketSC.TargetLocation.IsNearlyZero());
		PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);

		const FT4GameEffectData* DamageEffectData = GameDB.GetGameData<FT4GameEffectData>(
			EffectData->RawData.DamageEffectDataID
		);
		if (nullptr == DamageEffectData)
		{
			return;
		}

		TArray<IT4GameObject*> TargetObjects;
		bool bResult = GetTargetObjects(InEffectData.TargetLocation, ApplyAreaRange, TargetObjects);
		if (!bResult)
		{
			return;
		}
		for (IT4GameObject* TargetObject : TargetObjects)
		{
			// 데미지 전달!
			if (TargetObject->GetObjectID() == InEffectData.AttackerObjectID)
			{
				continue;
			}
			IT4ObjectController* TargetController = TargetObject->GetObjectController();
			check(nullptr != TargetController);
			{
				FT4PacketEffectDirectSC NewDamagePacketSC;
				NewDamagePacketSC.ObjectID = TargetObject->GetObjectID();
				NewDamagePacketSC.EffectDataID = EffectData->RawData.DamageEffectDataID;
				NewDamagePacketSC.AttackerObjectID = InEffectData.AttackerObjectID;
				check(NewDamagePacketSC.ObjectID.IsValid());
				PacketHandlerSC->DoBroadcastPacketForServer(&NewDamagePacketSC, true);
			}
			TargetController->OnNotifyAIEvent(InEffectData.EventName, InEffectData.AttackerObjectID); // #63
		}
	}
#if WITH_EDITOR
	if (nullptr != EditorGameplayHandler) // #60
	{
		if (EditorGameplayHandler->IsSandbackOneHitDie())
		{
			FT4PacketDieSC NewPacketSC;
			NewPacketSC.ObjectID = InEffectData.TargetObjectID;
			NewPacketSC.ReactionName = EditorGameplayHandler->GetOverrideDieReactionNameID();
			if (ET4GameEffectType::Direct == EffectData->RawData.EffectType)
			{
				NewPacketSC.AttackerObjectID = InEffectData.AttackerObjectID;
			}
			PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);
		}
	}
#endif
}

IT4ObjectController* FT4ServerEventManager::GetObjectController(const FT4ObjectID& InObjectID) // #63
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* ServerFramework = T4FrameGet(LayerType);
	check(nullptr != ServerFramework);
	IT4GameWorld* ServerGameWorld = ServerFramework->GetGameWorld();
	check(nullptr != ServerGameWorld);
	IT4GameObject* FoundObject = ServerGameWorld->GetContainer()->FindGameObject(InObjectID);
	if (nullptr == FoundObject)
	{
		return nullptr;
	}
	IT4ObjectController* ObjectController = FoundObject->GetObjectController();
	if (nullptr == ObjectController)
	{
		return nullptr;
	}
	return ObjectController;
}

IT4EditorGameplayHandler* FT4ServerEventManager::GetEditorGameplayCustomHandler() const // #60, #63
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* ServerFramework = T4FrameGet(LayerType);
	check(nullptr != ServerFramework);
#if WITH_EDITOR
	IT4EditorGameplayHandler* EditorGameplayHandler = ServerFramework->GetEditorGameplayCustomHandler();
	return EditorGameplayHandler;
#else
	return nullptr;
#endif
}

bool FT4ServerEventManager::GetTargetObjects(
	const FVector& InCenterLocation,
	const float InAreaRange,
	TArray<IT4GameObject*>& OutTargetObjects
)
{
	check(ET4LayerType::Max > LayerType);
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	check(nullptr != GameWorld);
	bool bResult = GameWorld->GetContainer()->QueryNearestGameObjects(
		InCenterLocation,
		InAreaRange, // #50
		OutTargetObjects
	);
	if (!bResult)
	{
		return false;
	}
	return true;
}

#endif
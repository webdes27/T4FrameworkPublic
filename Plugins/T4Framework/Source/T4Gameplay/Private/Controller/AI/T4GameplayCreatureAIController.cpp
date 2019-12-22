// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/T4GameplayCreatureAIController.h"

#include "Public/Protocol/T4PacketSC_Action.h" // #50
#include "Public/Protocol/T4PacketSC_Status.h"
#include "Public/Protocol/T4PacketSC_Move.h"
#include "Public/Protocol/T4PacketSC.h"

#include "T4GameplayUtils.h" // #63
#include "T4GameplayDefinitions.h" // #63

#include "GameDB/T4GameDB.h"

#include "Gameplay/Server/T4ServerEventManager.h" // #49
#include "Gameplay/T4GameplayInstance.h"

#include "T4GameplaySettings.h" // #52

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"
#include "T4Frame/Classes/Controller/AI/Component/T4PathFollowingComponent.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"

#include "T4GameplayInternal.h"

const float WeaponAttackRangeReduce = 0.7f; // #104 : StopDistance 에 관여. 무기 길이를 일정 기준으로 줄여서 다가가도록 처리

/**
  * WARN : AI Controller 는 서버에서만 사용하고, 클라리언트에서는 사용하지 않음에 유의할 것!
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Controller/AIController/
 */
AT4GameplayCreatureAIController::AT4GameplayCreatureAIController(
	const FObjectInitializer& ObjectInitializer
)	: Super(ObjectInitializer)
	, NPCGameData(nullptr)
	, MainWeaponGameData(nullptr) // #50
	, AIDataLoadState(ET4AIDataLoadState::AIDataLoad_Ready) // #50
#if (WITH_EDITOR || WITH_SERVER_CODE)
	, bHitOverlapEventStarted(false)
	, HitOverlapEventDelayTimeSec(0.0f)
	, HitOverlapEventClearTimeLeft(0.0f)
#endif
{
	check(nullptr != OverridePathFollowingComponent); // #50
	OverridePathFollowingComponent->GetOnCallbackMoveTo().BindUObject(
		this, 
		&AT4GameplayCreatureAIController::HandleOnCallbackMoveTo
	);
	OverridePathFollowingComponent->GetOnCallbackMoveStop().BindUObject(
		this, 
		&AT4GameplayCreatureAIController::HandleOnCallbackMoveStop
	); // #52
}

void AT4GameplayCreatureAIController::TickActor(
	float InDeltaTime,
	enum ELevelTick InTickType,
	FActorTickFunction& InThisTickFunction
)
{
	Super::TickActor(InDeltaTime, InTickType, InThisTickFunction);

	if (!CheckAsyncLoading())
	{
		return;
	}

	// TODO : Used Timer
	if (AIMemory.bActiveSkill) // #50
	{
		AIMemory.SkillPlayTimeLeft -= InDeltaTime;
		if (0.0f >= AIMemory.SkillPlayTimeLeft)
		{
			AIMemory.bActiveSkill = false;
		}
	}
	if (AIMemory.bActiveAggressive)
	{
		if (IsMoving() && AIMemory.AttackTargetObjectID.IsValid()) 
		{
			// #104 : 만약 타겟을 향해 이동중이라면 제자리에 세우고 BTTree 가 공격하도록 처리
			if (CheckValidAttackByTarget())
			{
				DoMoveStop(false);
			}
		}
		AIMemory.AggressiveClearTimeLeft -= InDeltaTime;
		if (0.0f >= AIMemory.AggressiveClearTimeLeft)
		{
			AIMemory.bActiveAggressive = false;
			AIMemory.AttackTargetObjectID.SetNone(); // #104 : 공격 대상도 Clear 해준다.
		}
	}


#if (WITH_EDITOR || WITH_SERVER_CODE)
	if (bHitOverlapEventStarted)
	{
		IT4GameObject* OwnerGameObject = GetGameObject();
		if (nullptr != OwnerGameObject && OwnerGameObject->IsLoaded())
		{
			HitOverlapEventDelayTimeSec -= InDeltaTime;
			if (!OwnerGameObject->IsWeaponHitOverlapEventEnabled() && HitOverlapEventDelayTimeSec <= 0.0f)
			{
				OwnerGameObject->BeginWeaponHitOverlapEvent(UseSkillDataID.RowName);
			}
			HitOverlapEventClearTimeLeft -= InDeltaTime;
			if (HitOverlapEventClearTimeLeft <= 0.0f)
			{
				ClearHitOverlapEvent();
			}
		}
		else
		{
			bHitOverlapEventStarted = false;
		}
	}
#endif
}

bool AT4GameplayCreatureAIController::CheckAsyncLoading()
{
	if (ET4AIDataLoadState::AIDataLoad_Loading != AIDataLoadState)
	{
		return true;
	}
	if (!GameObjectID.IsValid())
	{
		return false;
	}
	if (!BehaviorTreeAssetLoader.IsBinded())
	{
		if (!BehaviorTreeAssetLoader.IsLoadCompleted())
		{
			return false;
		}
		BehaviorTreeAsset = BehaviorTreeAssetLoader.GetBehaviorTree();
		BehaviorTreeAssetLoader.SetBinded();
	}
	check(nullptr != BehaviorTreeAsset);
	{
		IT4GameObject* OwnerGameObject = GetGameObject();
		check(nullptr != OwnerGameObject);
		if (!OwnerGameObject->IsLoaded())
		{
			return false;
		}
		AIMemory.InitSpawnLocation = OwnerGameObject->GetNavPoint();
	}
	if (!RunBehaviorTree(BehaviorTreeAsset))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("AT4GameplayCreatureAIController : failed to run behavior tree!")
		);
	}
	else
	{
#if WITH_EDITOR
		if (T4EngineLayer::IsLevelEditor(LayerType))
		{
			// #34 : 게임 모드일 경우만 InitializeComponent 가 되며 Blackboard 가 세팅되어 별도 처리함
			GetBrainComponent()->InitializeComponent();
		}
#endif
	}
	AIDataLoadState = ET4AIDataLoadState::AIDataLoad_Loaded;
	return true;
}

void AT4GameplayCreatureAIController::NotifyAIReady() // #50
{
	check(nullptr != OverridePathFollowingComponent);
}

void AT4GameplayCreatureAIController::NotifyAIStart() // #50
{
	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC(LayerType);
	check(nullptr != PacketHandlerSC);
	check(nullptr != NPCGameData);
	FT4GameDB& GameDB = GetGameDB();
	if (NPCGameData->RawData.MainWeaponDataID.IsValid())
	{
		check(nullptr == MainWeaponGameData);
		MainWeaponGameData = GameDB.GetGameData<FT4GameItemWeaponData>(NPCGameData->RawData.MainWeaponDataID);
		if (nullptr != MainWeaponGameData)
		{
			FT4PacketEquipSC NewPacketSC;
			NewPacketSC.ObjectID = GetGameObjectID();
			NewPacketSC.ItemWeaponDataID = NPCGameData->RawData.MainWeaponDataID;
			NewPacketSC.bMainWeapon = true; // #48

			check(NewPacketSC.ObjectID.IsValid());

			PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);
		}
		else
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("AT4GameplayCreatureAIController : Failed to MainWeapon Equip. MainWeaponDataID '%s' Not Found."),
				*(NPCGameData->RawData.MainWeaponDataID.ToString())
			);
		}
	}
#if (WITH_EDITOR || WITH_SERVER_CODE)
	// #49
	FT4ServerGameObjectDelegates& GameObjectDelegates = GetGameObject()->GetServerDelegates();
	HitOverlapDelegateHandle = GameObjectDelegates.OnHitOverlap.AddUObject(
		this,
		&AT4GameplayCreatureAIController::HandleOnHitOverlap
	);
#endif
}

void AT4GameplayCreatureAIController::NotifyAIEnd() // #50
{
	// WARN : AsyncLoad 가 걸렸을 수 있음으로 종료 시 명시적으로 Reset 을 호출해야 한다.
	BehaviorTreeAssetLoader.Reset();

#if (WITH_EDITOR || WITH_SERVER_CODE)
	if (HitOverlapDelegateHandle.IsValid())
	{
		HitOverlapDelegateHandle.Reset();
	}
#endif
}

void AT4GameplayCreatureAIController::HandleOnHitOverlap(
	const FName& InEventName,
	IT4GameObject* InHitGameObject,
	const FHitResult& InSweepResult
) // #49
{
#if (WITH_EDITOR || WITH_SERVER_CODE)
	if (nullptr == InHitGameObject)
	{
		return;
	}

	if (!IsServerRunning()) // #104
	{
		return; 
	}

	bool bCheckValidTarget = true;
	FT4GameEffectDataID ResultEffectDataID;

#if WITH_EDITOR
	if (InEventName == EditorSkillDataNameID)
	{
		if (HasServerGameplayCustomSettings()) // #104
		{
			IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
			check(nullptr != EditorGameplayHandler);
			const FT4EditorSkillDataInfo& EditorSkillData = EditorGameplayHandler->GetOverrideSkillDataInfo();
			ResultEffectDataID.RowName = EditorSkillData.ResultEffectDataID; // #63 : Effect 설정
			bCheckValidTarget = false; // Conti 용이라 제외
		}
	}
#endif

	// #104 : tribe 와 enemy 설정을 보고 hit 전달 여부를 결정해야 한다.
	if (bCheckValidTarget && !CheckValidAttackTarget(InHitGameObject))
	{
		return;
	}

	if (!ResultEffectDataID.IsValid())
	{
		FT4GameSkillDataID SkillDataID(InEventName);
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameSkillData* SkillData = GameDB.GetGameData<FT4GameSkillData>(SkillDataID);
		if (nullptr == SkillData)
		{
			return;
		}
		ResultEffectDataID = SkillData->RawData.ResultEffectDataID;
	}

	if (!ResultEffectDataID.IsValid())
	{
		return;
	}

	FT4ServerEventManager* ServerEventManager = T4GameplayUtil::GetServerEventManager(GetLayerType()); // #63
	if (nullptr != ServerEventManager)
	{
		FEffectDamageData NewEffectDamageData;
		NewEffectDamageData.EventName = T4GameplayAIEventEffectDamage; // #63
		NewEffectDamageData.TargetObjectID = InHitGameObject->GetObjectID();
		NewEffectDamageData.EffectDataID = ResultEffectDataID;
		NewEffectDamageData.AttackerObjectID = GetGameObjectID();
		ServerEventManager->AddEffectDamage(0.0f, NewEffectDamageData);
	}
#endif
}

void AT4GameplayCreatureAIController::ClearHitOverlapEvent() // #49
{
	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		bHitOverlapEventStarted = false;
		return;
	}
	if (OwnerGameObject->IsWeaponHitOverlapEventEnabled())
	{
		OwnerGameObject->EndWeaponHitOverlapEvent();
		bHitOverlapEventStarted = false;
	}
}

void AT4GameplayCreatureAIController::HandleOnCallbackMoveTo(
	const FVector& InMoveVelocity,
	bool bForceMaxSpeed
) // #42, #34
{
	DoMoveTo(InMoveVelocity, bForceMaxSpeed); // #52
}

void AT4GameplayCreatureAIController::HandleOnCallbackMoveStop()
{
	if (!HasGameObject())
	{
		return;
	}
	DoMoveStop(false); // #52
}

#if (WITH_EDITOR || WITH_SERVER_CODE)
void AT4GameplayCreatureAIController::OnNotifyAIEvent(
	const FName& InEventName,
	const FT4ObjectID& InSenderObjectID
) // #63
{
	if (T4GameplayAIEventEffectDamage == InEventName)
	{
		// 피격 되었다.

		// 이동중이면 멈춘다.
		DoMoveStop(false); // #52

		// Waiting 중이면 UBTTask_T4Wait::TickTask 에서 어그로를 체크해 Waiting 을 해제한다.

		// Aggressive 를 갱신한다.
		if (InSenderObjectID.IsValid())
		{
			SetAttackTargetObjectID(InSenderObjectID); // #104 : 날 때렸다면 AttackTarget 으로 설정해준다.
		}
		UpdateAggressive();
	}
}
#endif

ET4GameTribeType AT4GameplayCreatureAIController::GetTribeType() const // #104 : TODO
{
	if (nullptr == NPCGameData)
	{
		return ET4GameTribeType::Neutral;
	}
	return NPCGameData->RawData.TribeType;
}

ET4GameEnemyType AT4GameplayCreatureAIController::GetEnemyType() const // #104 : TODO
{
	if (nullptr == NPCGameData)
	{
		return ET4GameEnemyType::NoEnemy;
	}
	return NPCGameData->RawData.EnemyType;
}

bool AT4GameplayCreatureAIController::Bind(
	const FT4GameDataID& InNPCGameDataID
)
{
	check(nullptr == NPCGameData);

	FT4GameDB& GameDB = GetGameDB();
	NPCGameData = GameDB.GetGameData<FT4GameNPCData>(InNPCGameDataID);
	if (nullptr == NPCGameData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("AT4GameplayCreatureAIController : Failed to NPCBind. NPCGameDataID '%s' Not Found."),
			*(InNPCGameDataID.ToString())
		);
		return false;
	}

	AIDataLoadState = ET4AIDataLoadState::AIDataLoad_NoData; // #50

	for (;;)
	{
		const FSoftObjectPath BehaviorTreePath = NPCGameData->RawData.BehaviorTreePath.ToSoftObjectPath();
		if (!BehaviorTreePath.IsValid())
		{
			// TODO : 없어도 스폰이 되도록 수정 필요!
			UE_LOG(
				LogT4Gameplay,
				Error,
				TEXT("FT4PacketHandlerCS : Failed to NPCBind. BehaviorTree Not Found."),
				*(InNPCGameDataID.ToString())
			);
			break;
		}

		// #31
		const TCHAR* DebugTableName = *(InNPCGameDataID.ToString());
		BehaviorTreeAssetLoader.Load(BehaviorTreePath, false, DebugTableName);
		AIDataLoadState = ET4AIDataLoadState::AIDataLoad_Loading; // #50
		break;
	}

	NPCGameDataID = InNPCGameDataID;
	return true;
}

bool AT4GameplayCreatureAIController::IsMoving() const // #50
{
	if (!HasGameObject())
	{
		return false;
	}
	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return false;
	}
	if (OwnerGameObject->GetMovementVelocity().IsNearlyZero())
	{
		return false;
	}
	return true;
}

bool AT4GameplayCreatureAIController::IsAttacking() const // #50
{
	return AIMemory.bActiveSkill;
}

bool AT4GameplayCreatureAIController::IsCurrentAggressive() const // #50
{
	check(nullptr != NPCGameData);

	if (!IsServerRunning()) // #104
	{
		return false;
	}

#if WITH_EDITOR
	if (HasServerGameplayCustomSettings())
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
		check(nullptr != EditorGameplayHandler);
		// #63 : AI 가 꺼지면 동작하지 않는다.
		if (EditorGameplayHandler->IsAISystemDisabled())
		{
			return false;
		}
		// #60 : Editor AI Handler 가 있다면 AI를 별도로 제어함. 공격 가능여부를 처리해준다.
		if (!EditorGameplayHandler->IsSandbackAttackable())
		{
			return false;
		}
		return true;
	}
#endif

	return AIMemory.bActiveAggressive;
}

bool AT4GameplayCreatureAIController::IsOriginAggressive() const // #104
{
	check(nullptr != NPCGameData);

	if (!IsServerRunning()) // #104
	{
		return false;
	}

#if WITH_EDITOR
	if (HasServerGameplayCustomSettings())
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
		check(nullptr != EditorGameplayHandler);
		// #63 : AI 가 꺼지면 동작하지 않는다.
		if (EditorGameplayHandler->IsAISystemDisabled())
		{
			return false;
		}
		// #60 : Editor AI Handler 가 있다면 AI를 별도로 제어함. 공격 가능여부를 처리해준다.
		if (!EditorGameplayHandler->IsSandbackAttackable())
		{
			return false;
		}
		return true;
	}
#endif

	if (NPCGameData->RawData.bAggressive)
	{
		return true;
	}
	return false;
}

IT4GameObject* AT4GameplayCreatureAIController::FindNearestObject(float InMaxDistance) // #50
{
	TArray<IT4GameObject*> TargetObjects;
	bool bResult = FindNearestGameObjects(InMaxDistance, TargetObjects); // #50
	if (!bResult)
	{
		return nullptr;
	}
	IT4GameObject* NearestObject = nullptr;
	const ET4GameEnemyType MyEnemyType = GetEnemyType();
	if (ET4GameEnemyType::NoEnemy != MyEnemyType)
	{
		for (IT4GameObject* TargetObject : TargetObjects)
		{
			if (CheckValidAttackTarget(TargetObject))
			{
				NearestObject = TargetObject;
				break;
			}
		}
	}
	return NearestObject;
}

bool AT4GameplayCreatureAIController::CheckValidAttackTarget(
	IT4GameObject* InTargetObject
)
{
	// #104 : tribe 와 enemy 설정을 보고 hit 전달 여부를 결정해야 한다.
	check(nullptr != InTargetObject);
	const ET4GameTribeType MyTribeType = GetTribeType();
	const ET4GameEnemyType MyEnemyType = GetEnemyType();
	if (ET4GameEnemyType::NoEnemy == MyEnemyType)
	{
		return false;
	}
	else if (ET4GameEnemyType::All == MyEnemyType)
	{
		return true;
	}
	IT4ObjectController* ObjectController = InTargetObject->GetObjectController();
	if (nullptr == ObjectController)
	{
		return false;
	}
	const FName ObjectClassTypeName = ObjectController->GetClassTypeName();
	if (ET4GameEnemyType::Player == MyEnemyType)
	{
		if (ObjectClassTypeName == DefaultPlayerClassName)
		{
			return true;
		}
	}
	else if (ET4GameEnemyType::Hostile == MyEnemyType) // #104
	{
#if (WITH_EDITOR || WITH_SERVER_CODE) // #68 : 클라에서는 GameplayControl 은 오직 MyPC 밖에 없다.
		if (ObjectClassTypeName != DefaultPlayerClassName) // #104
		{
			IT4GameAIController* NPCAIController = static_cast<IT4GameAIController*>(ObjectController);
			if (nullptr != NPCAIController && NPCAIController->GetTribeType() != MyTribeType)
			{
				return true;
			}
		}
#endif
	}
	else if (ET4GameEnemyType::PlayerAndHostile == MyEnemyType) // #104
	{
		if (ObjectClassTypeName == DefaultPlayerClassName)
		{
			return true;
		}
#if (WITH_EDITOR || WITH_SERVER_CODE) // #68 : 클라에서는 GameplayControl 은 오직 MyPC 밖에 없다.
		else // #104
		{
			IT4GameAIController* NPCAIController = static_cast<IT4GameAIController*>(ObjectController);
			if (nullptr != NPCAIController && NPCAIController->GetTribeType() != MyTribeType)
			{
				return true; // NPC 간 Tribe 를 보고 전투 판단!
			}
		}
#endif
	}
	return false;
}

bool AT4GameplayCreatureAIController::CheckValidAttackByTarget() // #104 : AttackTarget 이 Normal Attack 이 가능한 거리인지 체크!
{
	check(nullptr != NPCGameData);

	if (!IsServerRunning()) // #104
	{
		return false;
	}

	FT4ObjectID AttackTargetObjectID = AIMemory.AttackTargetObjectID;
	if (!AttackTargetObjectID.IsValid())
	{
		return false;
	}

#if WITH_EDITOR
	if (HasServerGameplayCustomSettings())
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
		check(nullptr != EditorGameplayHandler);
		if (EditorGameplayHandler->IsAISystemDisabled()) // #63 : Ai 가 꺼져있다면 AttackRange 를 계산할 필요가 없다.
		{
			return false;
		}
	}
#endif
	if (nullptr == MainWeaponGameData)
	{
		return false;
	}
	if (0.0f >= MainWeaponGameData->RawData.AttackRange)
	{
		return false;
	}
	IT4GameObject* AttackTargetObject = FindGameObject(AIMemory.AttackTargetObjectID); // #104
	if (nullptr == AttackTargetObject)
	{
		return false;
	}
	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return false;
	}
	const FT4GameObjectProperty& OwnerObjectProperty = OwnerGameObject->GetPropertyConst();
	// #50 : 대략적인 공격 가능한 거리는 나의 Radius 에 무기의 공격거리, 나중에는 더 추가해주어야 할 수도 있다.
	const float TryAttackDistance = OwnerObjectProperty.CapsuleRadius
								  + MainWeaponGameData->RawData.AttackRange;
								 // + AttackTargetObject->GetPropertyConst().CapsuleRadius; // 타겟 Radius 포함
	float CurrentDistance = FVector(AttackTargetObject->GetRootLocation() - OwnerGameObject->GetRootLocation()).Size();
	if (TryAttackDistance < CurrentDistance)
	{
		return false;
	}
	return true;
}

IT4GameObject* AT4GameplayCreatureAIController::FindNearestEnemyByAttackRange() // #34
{
	check(nullptr != NPCGameData);

	if (!IsServerRunning()) // #104
	{
		return nullptr;
	}

#if WITH_EDITOR
	if (HasServerGameplayCustomSettings())
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
		check(nullptr != EditorGameplayHandler);
		if (EditorGameplayHandler->IsAISystemDisabled()) // #63 : Ai 가 꺼져있다면 AttackRange 를 계산할 필요가 없다.
		{
			return nullptr;
		}
	}
#endif
	if (nullptr == MainWeaponGameData)
	{
		return nullptr;
	}
	if (0.0f >= MainWeaponGameData->RawData.AttackRange)
	{
		return nullptr;
	}
	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return nullptr;
	}
	const FT4GameObjectProperty& OwnerObjectProperty = OwnerGameObject->GetPropertyConst();
	// #50 : 대략적인 공격 가능한 거리는 나의 Radius 에 무기의 공격거리, 나중에는 더 추가해주어야 할 수도 있다.
	const float TryAttackDistance = OwnerObjectProperty.CapsuleRadius
								  + (MainWeaponGameData->RawData.AttackRange * WeaponAttackRangeReduce);
	IT4GameObject* AttackTargetObject = FindNearestObject(TryAttackDistance); // TODO : #104 : 타겟 Radius 도 감안해야 한다. 체크 필요
	if (nullptr != AttackTargetObject)
	{
		UpdateAggressive(); // 공격 거리에 타겟이 잇다면 Aggressive 를 유지시켜 준다.
	}
	return AttackTargetObject;
}

IT4GameObject* AT4GameplayCreatureAIController::FindNearestEnemyBySensoryRange() // #34
{
	check(nullptr != NPCGameData);
	if (!IsServerRunning()) // #104
	{
		return nullptr;
	}
#if WITH_EDITOR
	if (HasServerGameplayCustomSettings())
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
		check(nullptr != EditorGameplayHandler);
		if (EditorGameplayHandler->IsAISystemDisabled()) // #63 : Ai 가 꺼져있다면 AttackRange 를 계산할 필요가 없다.
		{
			return nullptr;
		}
	}
#endif
	if (0.0f >= NPCGameData->RawData.SensoryRange)
	{
		return nullptr;
	}
	IT4GameObject* TargetObject = FindNearestObject(NPCGameData->RawData.SensoryRange);
	if (nullptr != TargetObject)
	{
		UpdateAggressive(); // #104 : 어그로가 있는 녀석이 타겟을 찾은 것이라 Aggressive 를 켜준다.
	}
	return TargetObject;
}

bool AT4GameplayCreatureAIController::GetMoveTargetLocationByAttackRange(
	const FVector& InStartLocation,
	const FVector& InEndLocation,
	float InTargetCapsuleRadius,
	FVector& OutTargetLocation
) // #50
{
	check(nullptr != NPCGameData);
	if (nullptr == MainWeaponGameData)
	{
		return false;
	}
	if (0.0f >= MainWeaponGameData->RawData.AttackRange)
	{
		return false;
	}
	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return false;
	}
	const FT4GameObjectProperty& OwnerObjectProperty = OwnerGameObject->GetPropertyConst();
	IT4GameFrame* ServerFramework = T4FrameGet(LayerType);
	check(nullptr != ServerFramework);
	IT4GameWorld* GameWorld = ServerFramework->GetGameWorld();
	check(nullptr != GameWorld);
	IT4WorldNavigationSystem* NavigationSystem = GameWorld->GetNavigationSystem();
	if (nullptr == NavigationSystem)
	{
		return false;
	}
	// #50 : 공격 가능한 거리로 다가가야 한다. 일단, 무기의 AttachRange 로 임시 설정
	const float MaxRandomRadius = 10.0f;
	const float TryAttackDistance = (MainWeaponGameData->RawData.AttackRange * WeaponAttackRangeReduce) - MaxRandomRadius;
	FVector MoveDirection = InEndLocation - InStartLocation;
	float CurrentDistance = MoveDirection.Size();
	if (0.0f < CurrentDistance && CurrentDistance >= TryAttackDistance)
	{
		MoveDirection = MoveDirection / CurrentDistance;
		const float MaxMoveDistancePerSec = 300.0f; // NPC 마다 다르게? 너무 멀면 바보처럼 움직인다.
		FVector OriginLocation = InStartLocation + (MaxMoveDistancePerSec * MoveDirection);
		if (!NavigationSystem->GetRandomLocation(
			OriginLocation,
			MaxRandomRadius,
			OutTargetLocation
		))
		{
			return false;
		}
	}
	else
	{
		OutTargetLocation = InStartLocation;
	}
	//UpdateAggressive(); // #104 : 대상에게 다가가는 것도 시간을 소비해야 한다.
	return true;
}

bool AT4GameplayCreatureAIController::DoRoaming(
	FVector& OutTargetLocation
) // #50
{
	check(nullptr != NPCGameData);

	if (!IsServerRunning()) // #104
	{
		return false;
	}

	if (HasServerGameplayCustomSettings()) // #104
	{
		return false; // #60 : Editor AI Handler 가 있다면 AI를 별도로 제어함
	}

	IT4GameFrame* ServerFramework = T4FrameGet(LayerType);
	check(nullptr != ServerFramework);

	if (FMath::RandRange(0.0f, 100.0f) > NPCGameData->RawData.RoamingRateRatio)
	{
		return false; // CHECK : Rand
	}
	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return false;
	}
	const FVector OriginNavPoint = OwnerGameObject->GetNavPoint();
	const FT4GameObjectProperty& OwnerObjectProperty = OwnerGameObject->GetPropertyConst();
	IT4GameWorld* GameWorld = ServerFramework->GetGameWorld();
	check(nullptr != GameWorld);
	IT4WorldNavigationSystem* NavigationSystem = GameWorld->GetNavigationSystem();
	if (nullptr == NavigationSystem)
	{
		return false;
	}
	if (!NavigationSystem->GetRandomLocation(
		OriginNavPoint,
		NPCGameData->RawData.RomaingRange,
		OutTargetLocation
	))
	{
		return false;
	}
	return true;
}

bool AT4GameplayCreatureAIController::DoAttack(
	const FT4ObjectID& InTargetGameObjectID
) // #50
{
	check(nullptr != NPCGameData);

	if (!IsServerRunning()) // #104
	{
		return false;
	}

	IT4GameObject* TargetGameObject = FindGameObjectForServer(InTargetGameObjectID);
	if (nullptr == TargetGameObject)
	{
		return false;
	}

	// #60
	FT4GameSkillDataID SelectSkillDataID; // #50

	bool bUpdateSkillData = true;

	bool bMoveable = false;
	float SkillDataHitDelayTimeSec = 0.0f;
	float SkillDataDurationSec = 0.0f;

	ET4GameAttackType AttackType = ET4GameAttackType::Melee; // #63
	float ProjectileSpeed = 0.0f; // #63
	FT4GameEffectDataID ResultEffectDataID; // #63

#if WITH_EDITOR
	if (HasServerGameplayCustomSettings()) // #104
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
		check(nullptr != EditorGameplayHandler);
		if (EditorGameplayHandler->IsAISystemDisabled())
		{
			return false;
		}

		// #63 : Sandback 이 공격 Role 일 경우만 실행
		if (EditorGameplayHandler->IsSandbackAttackable())
		{
			// #60 : Editor AI Handler 가 있다면 AI를 별도로 제어함
			SelectSkillDataID.RowName = EditorGameplayHandler->GetOverrideSkillDataNameID();
			if (SelectSkillDataID.RowName == NAME_None)
			{
				return false;
			}
			if (EditorGameplayHandler->IsOverrideSkillData())
			{
				const FT4EditorSkillDataInfo& EditorSkillData = EditorGameplayHandler->GetOverrideSkillDataInfo();
				AttackType = EditorSkillData.AttackType; // #63
				SkillDataHitDelayTimeSec = EditorSkillData.HitDelayTimeSec;
				SkillDataDurationSec = EditorSkillData.DurationSec;
				ProjectileSpeed = EditorSkillData.ProjectileSpeed; // #63
				bMoveable = EditorSkillData.bMoveable;
				SelectSkillDataID.RowName = EditorSkillDataNameID; // #63 ; 에디터용 스킬이 성절되도록 처리
				ResultEffectDataID.RowName = EditorSkillData.ResultEffectDataID; // #63 : Effect 설정
				if (SelectSkillDataID.RowName == EditorSkillDataNameID && 0.0f >= SkillDataDurationSec)
				{
					return false;
				}
				bUpdateSkillData = false;
			}
		}
		else
		{
			return false; // #63 : 툴 동작에 오해가 없도록 공격 체크가 없으면 동작하지 않도록 처리한다.
		}
	}
#endif

	FT4GameDB& GameDB = GetGameDB();
	if (!SelectSkillDataID.IsValid())
	{
		if (nullptr == MainWeaponGameData)
		{
			return false;
		}
		if (!MainWeaponGameData->RawData.SkillSetNameID.IsValid())
		{
			return false;
		}

		const FT4GameSkillSetData* SkillSetData = GameDB.GetGameData<FT4GameSkillSetData>(
			MainWeaponGameData->RawData.SkillSetNameID
		);
		if (nullptr == SkillSetData)
		{
			return false;
		}

		// TODO : CombaAttack
		uint32 AttackIndex = FMath::RandRange(0, 10);
		if (0 == AttackIndex)
		{
			SelectSkillDataID = SkillSetData->RawData.FinishAttackNameID;
		}
		else if (1 <= AttackIndex && 3 >= AttackIndex)
		{
			SelectSkillDataID = SkillSetData->RawData.ComboSecondaryAttackNameID;
		}
		else if (4 <= AttackIndex && 6 >= AttackIndex)
		{
			SelectSkillDataID = SkillSetData->RawData.ComboTertiaryAttackNameID;
		}
		else
		{
			SelectSkillDataID = SkillSetData->RawData.ComboPrimaryAttackNameID;
		}
	}

	if (bUpdateSkillData)
	{
		const FT4GameSkillData* SkillData = GameDB.GetGameData<FT4GameSkillData>(SelectSkillDataID);
		if (nullptr == SkillData)
		{
			return false;
		}
		AttackType = SkillData->RawData.AttackType; // #63
		SkillDataHitDelayTimeSec = SkillData->RawData.HitDelayTimeSec;
		SkillDataDurationSec = SkillData->RawData.DurationSec;
		ProjectileSpeed = SkillData->RawData.ProjectileSpeed; // #63
		bMoveable = SkillData->RawData.bMoveable;
		ResultEffectDataID = SkillData->RawData.ResultEffectDataID; // #63
	}

	if (!SelectSkillDataID.IsValid())
	{
		return false;
	}

	if (!bMoveable)
	{
		if (IsMoving())
		{
			DoMoveStop(false); // #52 : 공격전 이동을 명시적으로 멈춘다.
		}
	}

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC(LayerType);
	check(nullptr != PacketHandlerSC);

	float TargetDistance = 0.0f;
	FVector TargetDirection = GetGameObject()->GetFrontVector();

	if (ET4GameAttackType::Ranged == AttackType) // #63
	{
		FVector TargetVector = TargetGameObject->GetNavPoint() - GetGameObject()->GetNavPoint();
		TargetDistance = TargetVector.Size();
		TargetDirection = (FMath::Abs(TargetDistance) > KINDA_SMALL_NUMBER)
			? TargetVector / TargetDistance : GetGameObject()->GetFrontVector();
	}

	FT4PacketAttackSC NewPacketSC;
	NewPacketSC.ObjectID = GetGameObjectID();
	NewPacketSC.SkillDataID = SelectSkillDataID;
	NewPacketSC.TargetObjectID = TargetGameObject->GetObjectID();
	NewPacketSC.TargetDirection = TargetDirection;
	NewPacketSC.TargetDirection.Normalize();

	if (ET4GameAttackType::Ranged == AttackType) // #63
	{
		if (0.0f < ProjectileSpeed && 0.1f < TargetDistance)
		{
			NewPacketSC.ProjectileDurationSec = TargetDistance / ProjectileSpeed;

			FT4ServerEventManager* ServerEventManager = T4GameplayUtil::GetServerEventManager(GetLayerType()); // #63
			if (nullptr != ServerEventManager)
			{
				FEffectDamageData NewDamageData;
				NewDamageData.EventName = T4GameplayAIEventEffectDamage; // #63
				NewDamageData.TargetObjectID = InTargetGameObjectID;
				NewDamageData.EffectDataID = ResultEffectDataID;
				NewDamageData.AttackerObjectID = GetGameObjectID();
				ServerEventManager->AddEffectDamage(
					NewPacketSC.ProjectileDurationSec + SkillDataHitDelayTimeSec, // HitDelay 를 더해줘야 한다!
					NewDamageData
				);
			}
		}
	}
	else if (ET4GameAttackType::Area == AttackType) // #68
	{
		FT4ServerEventManager* ServerEventManager = T4GameplayUtil::GetServerEventManager(GetLayerType()); // #68
		if (nullptr != ServerEventManager)
		{
			FEffectDamageData NewDamageData;
			NewDamageData.EventName = T4GameplayAIEventEffectDamage; // #68
			NewDamageData.EffectDataID = ResultEffectDataID;
			NewDamageData.TargetLocation = TargetGameObject->GetNavPoint();
			NewDamageData.AttackerObjectID = GetGameObjectID();
			ServerEventManager->AddEffectDamage(
				SkillDataHitDelayTimeSec, // HitDelay 를 더해줘야 한다!
				NewDamageData
			);
		}
	}

	check(NewPacketSC.ObjectID.IsValid());
	PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);

	AIMemory.bActiveSkill = true;
	AIMemory.SkillPlayTimeLeft = SkillDataDurationSec;

#if (WITH_EDITOR || WITH_SERVER_CODE)
	ClearHitOverlapEvent(); // #49

	if (ET4GameAttackType::Melee == AttackType) // #63
	{
		bHitOverlapEventStarted = true;
		UseSkillDataID = NewPacketSC.SkillDataID;
		HitOverlapEventDelayTimeSec = SkillDataHitDelayTimeSec;
		HitOverlapEventClearTimeLeft = SkillDataDurationSec;
		if (0.0f >= HitOverlapEventDelayTimeSec)
		{
			GetGameObject()->BeginWeaponHitOverlapEvent(UseSkillDataID.RowName);
		}
	}
#endif

	UpdateAggressive();
	return true;
}

bool AT4GameplayCreatureAIController::DoMoveTo(
	const FVector& InMoveVelocity,
	bool bForceMaxSpeed
) // #42, #34
{
	if (!HasGameObject())
	{
		return false;
	}

	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return false;
	}

	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);

	// #52 : MoveToLocation 을 레이턴시를 감안한 거리로 보낸다. 클라이언트에서 레이턴시를 감안한 거리로 복원해 사용한다.
	const float DefaultNetworkLatencySec = GetDefault<UT4GameplaySettings>()->GameplayDefaultNetworkLatencySec; // #52 : 200ms
	float CurrentMoveSpeed = InMoveVelocity.Size();
	FVector MoveDirection = InMoveVelocity / CurrentMoveSpeed;

	FVector MoveToLocation = FVector::ZeroVector;
	const FVector MoveStartLocation = OwnerGameObject->GetNavPoint();
	FVector NewTargetLocation = MoveStartLocation;
	NewTargetLocation += MoveDirection * (CurrentMoveSpeed * DefaultNetworkLatencySec);

	IT4WorldNavigationSystem* WorldNavigationSystem = GameWorld->GetNavigationSystem();
	check(nullptr != WorldNavigationSystem);

	if (!WorldNavigationSystem->ProjectPoint(NewTargetLocation, T4_INVALID_NAVEXTENT, MoveToLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] DoMoveTo failed. ProjectPoint to Navigation."),
			uint32(LayerType)
		);
		return false;
	}
	if (!WorldNavigationSystem->HasReached(MoveStartLocation, MoveToLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] DoMoveTo failed. has not reached. (%s => %s)"),
			uint32(LayerType),
			*(MoveStartLocation.ToString()),
			*(MoveToLocation.ToString())
		);
		return false;
	}

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC(LayerType);
	check(nullptr != PacketHandlerSC);

	FT4PacketMoveToSC NewPacketSC;
	NewPacketSC.ObjectID = GetGameObjectID();
	NewPacketSC.MoveToLocation = MoveToLocation; // #50
	NewPacketSC.HeadYawAngle = MoveDirection.Rotation().Yaw; // #50 : 이동 방향으로 방향 수정
	NewPacketSC.bForceMaxSpeed = bForceMaxSpeed; // #52 : MovementComponet::MaxSpeed 를 사용할지에 대한 Flag, 기본값이 false 로 Velocity 에서 Speed 를 얻는다. 동기화 이슈!!
#if WITH_EDITOR
	NewPacketSC.ServerNavPoint = OwnerGameObject->GetNavPoint(); // #52
	NewPacketSC.ServerDirection = OwnerGameObject->GetFrontVector(); // #52
#endif
	PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);
	return true;
}

bool AT4GameplayCreatureAIController::DoMoveStop(bool bSyncLocation) // #52
{
	if (!HasGameObject())
	{
		return false;
	}

	StopMovement();

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC(LayerType);
	check(nullptr != PacketHandlerSC);

	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return false;
	}

	FT4PacketMoveStopSC NewPacketSC;
	NewPacketSC.ObjectID = GetGameObjectID();
	NewPacketSC.StopLocation = OwnerGameObject->GetNavPoint();
	NewPacketSC.HeadYawAngle = OwnerGameObject->GetRotation().Yaw; // #50 : 이동 방향으로 방향 수정
	NewPacketSC.bSyncLocation = bSyncLocation;
	PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);
	return true;
}

bool AT4GameplayCreatureAIController::DoUpdateMoveSpeed(
	ET4MoveSpeed InMoveSpeedType
) // #52
{
	if (!HasGameObject())
	{
		return false;
	}

	AIMemory.MoveSpeedType = InMoveSpeedType;
	float UpdateMoveSpeed = GetMoveSpeedSelected();
	if (UpdateMoveSpeed == AIMemory.CurrentMoveSpeed)
	{
		return false;
	}
	AIMemory.CurrentMoveSpeed = UpdateMoveSpeed;

	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return false;
	}

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC(LayerType);
	check(nullptr != PacketHandlerSC);

	FT4PacketMoveSpeedSyncSC NewPacketSC;
	NewPacketSC.ObjectID = GetGameObjectID();
	NewPacketSC.MoveSpeed = UpdateMoveSpeed;
	PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);
	return true;
}

float AT4GameplayCreatureAIController::GetMoveSpeedSelected() const // #50
{
	check(nullptr != NPCGameData);
	switch (AIMemory.MoveSpeedType)
	{
		case ET4MoveSpeed::MoveSpeed_Walk:
			return NPCGameData->RawData.WalkSpeed;

		case ET4MoveSpeed::MoveSpeed_Run:
			return NPCGameData->RawData.RunSpeed;

		case ET4MoveSpeed::MoveSpeed_FastRun:
			return (0.0f < NPCGameData->RawData.FastRunSpeed)
				? NPCGameData->RawData.FastRunSpeed : NPCGameData->RawData.RunSpeed; // Temp
	}
	return 0.0f;
}

void AT4GameplayCreatureAIController::UpdateAggressive()
{
	check(nullptr != NPCGameData);
	AIMemory.bActiveAggressive = true;
	AIMemory.AggressiveClearTimeLeft = NPCGameData->RawData.PassiveApproachTimeSec;
}
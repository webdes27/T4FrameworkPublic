// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionEnvironmentNode.h"

#include "Object/Component/T4EnvironmentZoneComponent.h" // #99
#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "T4Asset/Classes/Entity/T4ZoneEntityAsset.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

#include "T4EngineInternal.h"

/**
  * #99
 */
FT4ActionEnvironmentNode::FT4ActionEnvironmentNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionAttachedNodeBase(InControl, InKey)
	, PlayTarget(ET4PlayTarget::Default)
	, bOverrideBlendTime(false)
	, OverrideBlendInTimeSec(0.0f)
	, OverrideBlendOutTimeSec(0.0f)
	, bBlendStart(false)
{
}

FT4ActionEnvironmentNode::~FT4ActionEnvironmentNode()
{
}

FT4ActionEnvironmentNode* FT4ActionEnvironmentNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4EnvironmentAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::Environment == InAction.ActionType);
	if (ET4PlayTarget::Player == InAction.PlayTarget || ET4PlayTarget::Default == InAction.PlayTarget) // #100
	{
		AT4GameObject* OwnerGameObject = InControl->GetGameObject();
		if (nullptr == OwnerGameObject)
		{
			return nullptr;
		}
		if (!OwnerGameObject->HasPlayer()) // #100 : 옵션에 따라 플레이어가 아니면 플레이하지 않는다.
		{
			return nullptr;
		}
	}
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionEnvironmentNode* NewNode = new FT4ActionEnvironmentNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionEnvironmentNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::Environment == InAction->ActionType);
	const FT4EnvironmentAction& ConvAction = *(static_cast<const FT4EnvironmentAction*>(InAction));
	EntityAssetPtr = ConvAction.ZoneEntityAsset.Get(); // Entity 는 초기화 때 로딩이 되어 있다!
	check(EntityAssetPtr.IsValid());
	PlayTarget = ConvAction.PlayTarget; // #100
	bOverrideBlendTime = ConvAction.bOverrideBlendTime; // #99
	if (bOverrideBlendTime)
	{
		OverrideBlendInTimeSec = ConvAction.OverrideBlendInTimeSec;
		OverrideBlendOutTimeSec = ConvAction.OverrideBlendOutTimeSec;
	}
	else
	{
		OverrideBlendInTimeSec = EntityAssetPtr->ZoneData.BlendInTimeSec;
		OverrideBlendOutTimeSec = EntityAssetPtr->ZoneData.BlendOutTimeSec;
	}
	SetAttachInfo(
		ConvAction.AttachParent, // #54
		ConvAction.bParentInheritPoint, // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...
		ConvAction.ActionPoint, // #57 : BoneOrSocketName
		ConvAction.ZoneEntityAsset.ToSoftObjectPath(), // Entity 는 초기화 때 로딩이 되어 있다!
		ET4LoadingPolicy::Default
	);

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionEnvironmentNode::Destroy()
{
	ensure(!EnvironmentZoneComponentPtr.IsValid());
}

void FT4ActionEnvironmentNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (!IsPlayed())
	{
		return;
	}
	if (EnvironmentZoneComponentPtr.IsValid())
	{
		EnvironmentZoneComponentPtr->SetPlayRate(InUpdateTime.TimeScale); // #102

		if (!bBlendStart && 0.0f < OverrideBlendOutTimeSec && GetPlayTimeLeft() <= OverrideBlendOutTimeSec)
		{
			// #99 : 강제로 Leave 한다. Action 연계한 FadeOut 처리
			//       환경은 민감함으로 타이밍을 맞추기 위해 넘어간 시간을 넘겨준다.
			EnvironmentZoneComponentPtr->LeaveOut(OverrideBlendOutTimeSec - GetPlayTimeLeft());
			bBlendStart = true;
		}

#if WITH_EDITOR
		if (EntityAssetPtr.IsValid())
		{
			UWorld* UnrealWorld = EnvironmentZoneComponentPtr->GetWorld();
			if (nullptr != UnrealWorld)
			{
				const FT4EntityZoneData& ZoneBrushData = EntityAssetPtr->ZoneData;
				const FVector CenterLocation = EnvironmentZoneComponentPtr->GetComponentLocation();
				const FVector HalfHeight = FVector(0.0f, 0.0f, ZoneBrushData.HalfHeight);
				DrawDebugCylinder(
					UnrealWorld,
					CenterLocation + HalfHeight,
					CenterLocation - HalfHeight,
					ZoneBrushData.Radius,
					ZoneBrushData.DebugData.DebugSegments,
					ZoneBrushData.DebugData.DebugColor,
					false
				);
			}
		}
#endif
	}
}

bool FT4ActionEnvironmentNode::Play()
{
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	check(!EnvironmentZoneComponentPtr.IsValid());
	EnvironmentZoneComponentPtr = NewComponentTemplate<UT4EnvironmentZoneComponent>(OwnerGameObject->GetPawn(), true);
	{
		EnvironmentZoneComponentPtr->PrimaryComponentTick.bCanEverTick = true;
	}
	OnAttachToParent(Cast<USceneComponent>(EnvironmentZoneComponentPtr.Get()), true);
	PlayInternal(GetOffsetTimeSec()); // Entity 는 Sync 로드하였기 때문에 바로 플레이 처리 해준다.
	return true;
}

void FT4ActionEnvironmentNode::Stop()
{
	if (!EnvironmentZoneComponentPtr.IsValid())
	{
		return;
	}
	OnDetachFromParent(Cast<USceneComponent>(EnvironmentZoneComponentPtr.Get()), true);
	EnvironmentZoneComponentPtr->DestroyComponent();
	EnvironmentZoneComponentPtr.Reset();
}

bool FT4ActionEnvironmentNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}

bool FT4ActionEnvironmentNode::PlayInternal(float InOffsetTimeSec) // #56
{
	check(ET4LoadingPolicy::Async == LoadingPolicy);
	check(EntityAssetPtr.IsValid());
	check(EnvironmentZoneComponentPtr.IsValid());
	EnvironmentZoneComponentPtr->Initialize(
		EntityAssetPtr->GetEntityKeyPath(),
		ET4ZoneType::Dynamic,
		EntityAssetPtr,
		InOffsetTimeSec
	);
	if (bOverrideBlendTime)
	{
		EnvironmentZoneComponentPtr->SetOverrideBlendTime(OverrideBlendInTimeSec, OverrideBlendOutTimeSec);
	}
	SetPlayState(APS_Playing);
	return true;
}


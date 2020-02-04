// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionMeshNode.h"

#include "Object/Component/T4StaticMeshComponent.h"
#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionMeshNode::FT4ActionMeshNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionAttachedNodeBase(InControl, InKey)
{
	WorldObjectType = ET4ObjectType::World_Default; // #54, #63
}

FT4ActionMeshNode::~FT4ActionMeshNode()
{
	check(AssetLoader.CheckReset());
}

FT4ActionMeshNode* FT4ActionMeshNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4MeshAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::Mesh == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionMeshNode* NewNode = new FT4ActionMeshNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionMeshNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::Mesh == InAction->ActionType);
	const FT4MeshAction& ConvAction = *(static_cast<const FT4MeshAction*>(InAction));
	SetAttachInfo(
		ConvAction.AttachParent, // #54
		ConvAction.bParentInheritPoint, // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...
		ConvAction.ActionPoint, // #57 : BoneOrSocketName
		ConvAction.StaticMeshAsset.ToSoftObjectPath(),
		ConvAction.LoadingPolicy
	);
	SetRelativeTransform(ConvAction.LocalOffset, ConvAction.LocalRotation, ConvAction.LocalScale); // #112
	if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		OnStartLoading();
	}
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionMeshNode::Destroy()
{
	ensure(!MeshComponentPtr.IsValid());
	AssetLoader.Reset();
}

void FT4ActionMeshNode::AdvanceLoading(const FT4UpdateTime& InUpdateTime)
{
	if (AssetLoader.IsLoadFailed()) // Failed 먼저 체크!!
	{
		SetLoadState(ALS_Failed);
	}
	else if (AssetLoader.IsLoadCompleted())
	{
		SetLoadState(ALS_Completed);
	}
}

void FT4ActionMeshNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (CheckPlayState(APS_Ready))
	{
		// #56
		check(ET4LoadingPolicy::Async == LoadingPolicy);
		AddOffsetTimeSec(InUpdateTime.ScaledTimeSec); // #54 : Case-2
		if (CheckLoadState(ALS_Failed))
		{
			SetPlayState(APS_Stopped);
		}
		else if (CheckLoadState(ALS_Completed))
		{
			PlayInternal(GetOffsetTimeSec());
		}
	}
	if (IsPlayed())
	{
		if (MeshComponentPtr.IsValid())
		{
#if 0
			if (0.0f < FadeOutTimeSec && GetPlayTimeLeft() <= FadeOutTimeSec)
			{
				DecalComponentPtr->SetFadeOut(0.0f, GetPlayTimeLeft(), false);
				FadeOutTimeSec = 0.0f;
			}
#endif
		}
	}
}

bool FT4ActionMeshNode::Play()
{
	check(!AssetLoader.IsBinded());

	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);

	check(!MeshComponentPtr.IsValid());
	MeshComponentPtr = NewComponentTemplate<UT4StaticMeshComponent>(OwnerGameObject->GetPawn(), true);
	{
		MeshComponentPtr->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	OnAttachToParent(Cast<USceneComponent>(MeshComponentPtr.Get()), false);
	if (ET4LoadingPolicy::Sync == LoadingPolicy)
	{
		// #8, #56 : 사용 제한 필요!!! 만약을 대비해 준비는 해둔 것!
		UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetPath.TryLoad());
		if (nullptr != StaticMesh)
		{
			MeshComponentPtr->SetStaticMesh(StaticMesh);
		}
	}
	else if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		if (CheckLoadState(ALS_Completed))
		{
			PlayInternal(GetOffsetTimeSec());
		}
		else if (CheckLoadState(ALS_Failed))
		{
			T4_LOG(
				Error,
				TEXT("Failed to load StaticMesh '%s'"),
				*(AssetPath.ToString())
			);
			OnStop();
			return false;
		}
		else
		{
			// #54 : 리소스가 로드가 안되어 Offset Time 적용이 필요할 경우,
			//       PlayState 를 APS_Ready 로 바꾼 후 로딩 완료 후 OffsetTimeSec 로 플레이 되도록 처리
			SetPlayState(APS_Ready);
			return false;
		}
	}
	return true;
}

void FT4ActionMeshNode::Stop()
{
	AssetLoader.Reset();
	if (!MeshComponentPtr.IsValid())
	{
		return;
	}
	OnDetachFromParent(Cast<USceneComponent>(MeshComponentPtr.Get()), false);
	MeshComponentPtr->DestroyComponent();
	MeshComponentPtr.Reset();
}

void FT4ActionMeshNode::StartLoading()
{
	AssetLoader.Load(AssetPath, false, TEXT("MeshNode"));
}

bool FT4ActionMeshNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}

bool FT4ActionMeshNode::PlayInternal(float InOffsetTimeSec) // #56
{
	check(ET4LoadingPolicy::Async == LoadingPolicy);
	check(!AssetLoader.IsBinded());
	check(AssetLoader.IsLoadCompleted());
	check(MeshComponentPtr.IsValid());
	AssetLoader.Process(MeshComponentPtr.Get());
	AssetLoader.SetBinded();
	SetPlayState(APS_Playing);
	return true;
}

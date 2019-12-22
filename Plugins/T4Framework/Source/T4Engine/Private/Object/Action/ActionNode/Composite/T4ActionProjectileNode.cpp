// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionProjectileNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

/**
  * #63
 */
FT4ActionProjectileNode::FT4ActionProjectileNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionCompositeNode(InControl, InKey)
	, ActionPoint(NAME_None) // #57 : BoneOrSocketName
	, bThrowed(false)
	, ThrowDelayTimeSec(0.0f)
	, CastingStopDelayTimeSec(0.0f)
	, CastingStopTimeSec(0.0f)
	, ProjectileSpeed(0.0f)
	, ProjectileDurationSec(0.0f) // #63 : 서버에서 계산된 Hit 시간까지의 ProjectileDurationSec, Hit 가 없으면 0이다.
{
}

FT4ActionProjectileNode::~FT4ActionProjectileNode()
{
}

FT4ActionProjectileNode* FT4ActionProjectileNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4ProjectileAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::Projectile == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionProjectileNode* NewNode = new FT4ActionProjectileNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionProjectileNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::Projectile == InAction->ActionType);
	const FT4ProjectileAction& ConvAction = *(static_cast<const FT4ProjectileAction*>(InAction));
	ActionPoint = ConvAction.ActionPoint;
	ThrowDelayTimeSec = ConvAction.ThrowDelayTimeSec;
	CastingStopDelayTimeSec = ConvAction.CastingStopDelayTimeSec;
	LoadingPolicy = ConvAction.LoadingPolicy;
	if (ET4LoadingPolicy::Default == LoadingPolicy)
	{
		LoadingPolicy = ET4LoadingPolicy::Async;
	}
	CastingAssetPath = ConvAction.CastingContiAsset.ToSoftObjectPath();
	if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		OnStartLoading(); // #8, #56 : 생성 시점에 무조건 비동기 로딩을 걸어준다.
	}
	if (!ConvAction.HeadContiAsset.IsNull())
	{
		HeadAssetPath = ConvAction.HeadContiAsset.ToSoftObjectPath();
		HeadAssetLoader.Load(
			ConvAction.HeadContiAsset.ToSoftObjectPath(), 
			false, 
			TEXT("ActionProjectileNode:Head")
		);
	}
	if (!ConvAction.EndContiAsset.IsNull())
	{
		EndAssetPath = ConvAction.EndContiAsset.ToSoftObjectPath();
	}
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionProjectileNode::Destroy()
{
	CastingAssetLoader.Reset();
	HeadAssetLoader.Reset();
}

void FT4ActionProjectileNode::AdvanceLoading(const FT4UpdateTime& InUpdateTime)
{
	if (CastingAssetLoader.IsLoadFailed()) // Failed 먼저 체크!!
	{
		SetLoadState(ALS_Failed);
	}
	else if (CastingAssetLoader.IsLoadCompleted())
	{
		SetLoadState(ALS_Completed);
	}
}

void FT4ActionProjectileNode::Advance(const FT4UpdateTime& InUpdateTime)
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
	if (CheckPlayState(APS_Playing))
	{
		if (!bThrowed)
		{
			if (GetPlayingTime() >= ThrowDelayTimeSec)
			{
				ThrowProjectile(FMath::Max(0.0f, GetPlayingTime() - ThrowDelayTimeSec));
			}
		}
		if (bThrowed)
		{
			if (0.0f < CastingStopTimeSec && GetPlayingTime() >= CastingStopTimeSec)
			{
				ActionNodeGraph.OnStopAllChildren(); // Casting Action 삭제!
				CastingStopTimeSec = 0.0f;
			}
		}
	}
}

bool FT4ActionProjectileNode::Play()
{
	check(!CastingAssetLoader.IsBinded());

	if (ET4LoadingPolicy::Sync == LoadingPolicy)
	{
		// #8, #56 : 사용 제한 필요!!! 만약을 대비해 준비는 해둔 것!
		UT4ContiAsset* ContiAsset = Cast<UT4ContiAsset>(CastingAssetPath.TryLoad());
		if (nullptr != ContiAsset)
		{
			ProcessCompositeData(
				ContiAsset->CompositeData,
				ContiAsset->TotalPlayTimeSec, // #54
				GetOffsetTimeSec()
			); // #54, #56 : OffsetTime
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
			UE_LOG(
				LogT4Engine,
				Error,
				TEXT("FT4ActionProjectileNode::Play Failed to load CastingAssetPath '%s'."),
				*(CastingAssetPath.ToString())
			);
			OnStop();
			return false;
		}
		else
		{
			// #56 : 리소스가 로드가 안되어 Offset Time 적용이 필요할 경우,
			//       PlayState 를 APS_Ready 로 바꾼 후 로딩 완료 후 OffsetTimeSec 로 플레이 되도록 처리
			SetPlayState(APS_Ready);
			return false;
		}
	}
	else
	{
		check(false); // ??
	}

	return true;
}

void FT4ActionProjectileNode::Stop()
{
#if WITH_EDITOR
	if (IsDebugPaused())
	{
		// #63 : World 에 Projectile 을 던졌다면 나는 소유권이 없다. 다만, 툴에서의 작업 지원을 위해
		//       만약 Pause 상태라면 Projectile Object 의 Pause 를 풀어주고 종료한다.
		AT4GameObject* ProjectileObject = GetClientObject();
		if (nullptr != ProjectileObject)
		{
			ProjectileObject->SetDebugPause(false);
		}
	}
#endif
}

void FT4ActionProjectileNode::StartLoading()
{
	CastingAssetLoader.Load(CastingAssetPath, false, TEXT("ActionProjectileNode:Casting"));
}

bool FT4ActionProjectileNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}

bool FT4ActionProjectileNode::PlayInternal(float InOffsetTimeSec) // #56
{
	check(ET4LoadingPolicy::Async == LoadingPolicy);
	check(!CastingAssetLoader.IsBinded());
	check(CastingAssetLoader.IsLoadCompleted());
	UT4ContiAsset* ContiAsset = CastingAssetLoader.GetContiAsset();
	check(nullptr != ContiAsset);
	ProcessCompositeData(
		ContiAsset->CompositeData,
		ContiAsset->TotalPlayTimeSec, // #54
		InOffsetTimeSec // #54, #56 : OffsetTime
	);
	CastingAssetLoader.SetBinded();
	SetPlayState(APS_Playing);

	if (ActionParameterPtr.IsValid())
	{
		if (ActionParameterPtr->CheckBits(ET4TimeParamBits::ProjectileSpeedBit))
		{
			ProjectileSpeed = ActionParameterPtr->GetTimeParams().ProjectileSpeed;
		}
		if (ActionParameterPtr->CheckBits(ET4TimeParamBits::ProjectileDurationBit))
		{
			ProjectileDurationSec = ActionParameterPtr->GetTimeParams().ProjectileDurationSec;
		}
	}
	if (0.0f >= ProjectileSpeed)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionProjectileNode::PlayInternal: Projectile Speed is Zero!! '%s'"),
			*(CastingAssetPath.ToString())
		);
		return true;
	}

	if (0.0f >= ThrowDelayTimeSec)
	{
		ThrowProjectile(0.0f);
	}
	return true;
}

void FT4ActionProjectileNode::ThrowProjectile(
	float ThrowOffsetTimeSec
) // #63
{
	AT4GameObject* ParentGameObject = GetGameObject();
	check(nullptr != ParentGameObject);
	FVector StartLocation = FVector::ZeroVector;
	ParentGameObject->GetSocketLocation(ActionPoint, StartLocation);

	AT4GameObject* WorldProjectileObject = NewClientObject(
		ET4ObjectType::World_Projectile,
		TEXT("FT4ActionProjectileNode"),
		StartLocation,
		ParentGameObject->GetRotation(),
		FVector::OneVector
	);

	if (nullptr != WorldProjectileObject)
	{
		FT4LaunchAction NewAction;
		NewAction.MoveSpeed = ProjectileSpeed;
		NewAction.HeadContiAsset = HeadAssetPath;
		NewAction.EndContiAsset = EndAssetPath;
		NewAction.LoadingPolicy = LoadingPolicy;
		NewAction.ThrowOffsetTimeSec = ThrowOffsetTimeSec;
		NewAction.MaxPlayTimeSec = GetGlobalMaxPlayTimeSec();
		if (0.0f < ProjectileDurationSec)
		{
			// #63 : 서버에서 계산된 Hit 시간까지의 ProjectileDurationSec, Hit 가 없으면 0이다.
			NewAction.MaxPlayTimeSec = ProjectileDurationSec;
		}
		if (ActionParameterPtr.IsValid())
		{
			// #63 : 타겟이 있을 경우 타겟을 무조건 맞춰야 한다.
			if (ActionParameterPtr->CheckBits(ET4TargetParamBits::ObjectIDBit))
			{
				NewAction.TargetObjectID = ActionParameterPtr->GetTargetParams().TargetObjectID;
			}
		}
		WorldProjectileObject->DoExecuteAction(&NewAction, ActionParameterPtr.Get());
	}

	CastingStopTimeSec = GetPlayingTime() + CastingStopDelayTimeSec;

	bThrowed = true;
}

#if !UE_BUILD_SHIPPING
void FT4ActionProjectileNode::NotifyDebugPaused(bool bInPause) // #54
{
	// #63 : World 에 Projectile 을 던졌다면 나는 소유권이 없다. 다만, 툴에서의 작업 지원을 위해 처리해주었다.
	AT4GameObject* ProjectileObject = GetClientObject();
	if (nullptr != ProjectileObject)
	{
		ProjectileObject->SetDebugPause(bInPause);
	}
}
#endif

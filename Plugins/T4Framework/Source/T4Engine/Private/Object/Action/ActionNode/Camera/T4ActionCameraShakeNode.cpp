// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionCameraShakeNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Classes/Camera/T4CameraModifier.h" // #100, #101
#include "Camera/CameraShake.h"

#include "T4EngineInternal.h"

/**
  * #101
 */
FT4ActionCameraShakeNode::FT4ActionCameraShakeNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionCameraNode(InControl, InKey)
	, PlayScale(1.0f)
	, PlaySpace(ECameraAnimPlaySpace::CameraLocal)
	, UserDefinedPlaySpace(ForceInitToZero)
{
}

FT4ActionCameraShakeNode::~FT4ActionCameraShakeNode()
{
	check(!CameraShakePtr.IsValid());
}

FT4ActionCameraShakeNode* FT4ActionCameraShakeNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4CameraShakeAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::CameraShake == InAction.ActionType);
	if (!T4ActionNodeUtility::CheckPlayTarget(InAction.PlayTarget, InControl, &InAction, InParameters))
	{
		return nullptr; // #101 : PlayTarget 옵션에 따라 스스로 처리할지 Player 에게 전달할지를 결정!
	}
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionCameraShakeNode* NewNode = new FT4ActionCameraShakeNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionCameraShakeNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::CameraShake == InAction->ActionType);
	const FT4CameraShakeAction& ConvAction = *(static_cast<const FT4CameraShakeAction*>(InAction));

	PlayScale = ConvAction.PlayScale;
	PlaySpace = ConvAction.PlaySpace;
	UserDefinedPlaySpace = ConvAction.UserDefinedPlaySpace;
	OscillationData = ConvAction.OscillationData;
	AnimData = ConvAction.AnimData;

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionCameraShakeNode::Destroy()
{
}

void FT4ActionCameraShakeNode::Advance(const FT4UpdateTime& InUpdateTime)
{
}

bool FT4ActionCameraShakeNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionCameraShakeNode::Stop()
{
	if (CameraShakePtr.IsValid())
	{
		UT4CameraModifier* CameraModifier = GetCameraModifier(); // #101
		if (nullptr != CameraModifier)
		{
			CameraModifier->RemoveCameraShake(CameraShakePtr.Get());
		}
		CameraShakePtr.Reset();
	}
}

bool FT4ActionCameraShakeNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}

bool FT4ActionCameraShakeNode::PlayInternal(float InOffsetTimeSec)
{
	AT4PlayerCameraManager* CameraManager = GetPlayerCameraManager();
	if (nullptr == CameraManager)
	{
		return false;
	}
	UT4CameraModifier* CameraModifier = GetCameraModifier(); // #101
	if (nullptr == CameraModifier)
	{
		return false;
	}
	// #101 : BP 로 CameraShake 를 만들지 않기 때문에 Property 설정이 되지 않아 플레이가 실패했을 것이기 때문에
	//        프로퍼티 설정 후 PlayShake 를 다시 호출해준다.
	CameraShakePtr = CameraModifier->AddCameraShake(
		UCameraShake::StaticClass(), 
		PlayScale,
		PlaySpace,
		UserDefinedPlaySpace
	);
	check(CameraShakePtr.IsValid());
	{
		// Duration 이 있으면 Duration 시간동안, 없다면 Conti 의 MaxPlayTime 을 사용
		// PlayTimeLeft 와 Max - Elapsed 를 빼주는 것은 OffsetTime 을 감안해 종료 시간을 맞추기 위함
		if (0.0f < DurationSec)
		{
			CameraShakePtr->OscillationDuration = GetPlayTimeLeft();
		}
		else
		{
			CameraShakePtr->OscillationDuration = FMath::Max(0.0f, GetGlobalMaxPlayTimeSec() - GetElapsedTimeSec());
		}
		CameraShakePtr->OscillationBlendInTime = OscillationData.BlendInTimeSec;
		CameraShakePtr->OscillationBlendOutTime = OscillationData.BlendOutTimeSec;
		CameraShakePtr->RotOscillation = OscillationData.RotOscillation;
		CameraShakePtr->LocOscillation = OscillationData.LocOscillation;
		CameraShakePtr->FOVOscillation = OscillationData.FOVOscillation;
	}
	{
		if (nullptr != AnimData.CameraAnim)
		{
			CameraShakePtr->AnimPlayRate = AnimData.AnimPlayRate;
			CameraShakePtr->AnimScale = AnimData.AnimScale;
			CameraShakePtr->AnimBlendInTime = AnimData.AnimBlendInTime;
			CameraShakePtr->AnimBlendOutTime = AnimData.AnimBlendOutTime;
			CameraShakePtr->bRandomAnimSegment = AnimData.bRandomAnimSegment;
			CameraShakePtr->RandomAnimSegmentDuration = AnimData.RandomAnimSegmentDuration;
			CameraShakePtr->Anim = AnimData.CameraAnim;
		}
	}
	CameraShakePtr->PlayShake(CameraManager, PlayScale, PlaySpace, UserDefinedPlaySpace);
	return true;
}
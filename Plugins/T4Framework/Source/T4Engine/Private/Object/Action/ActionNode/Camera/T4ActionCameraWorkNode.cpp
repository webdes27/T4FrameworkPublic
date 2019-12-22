// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionCameraWorkNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Classes/Camera/T4CameraModifier.h" // #58
#include "Classes/Camera/T4PlayerCameraManager.h" // #100, #101

#include "Public/T4EngineUtility.h"
#if WITH_EDITOR
#include "T4Engine/Classes/Camera/T4EditorCameraActor.h" // #58
#endif

#include "T4EngineInternal.h"

/**
  * #58
 */
FT4ActionCameraWorkNode::FT4ActionCameraWorkNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionCameraNode(InControl, InKey)
#if WITH_EDITOR
	, bShowEditorCameraActor(false)
#endif
{
}

FT4ActionCameraWorkNode::~FT4ActionCameraWorkNode()
{
}

FT4ActionCameraWorkNode* FT4ActionCameraWorkNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4CameraWorkAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::CameraWork == InAction.ActionType);
	if (!T4ActionNodeUtility::CheckPlayTarget(InAction.PlayTarget, InControl, &InAction, InParameters))
	{
		return nullptr; // #101 : PlayTarget 옵션에 따라 스스로 처리할지 Player 에게 전달할지를 결정!
	}
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionCameraWorkNode* NewNode = new FT4ActionCameraWorkNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionCameraWorkNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::CameraWork == InAction->ActionType);
	const FT4CameraWorkAction& ConvAction = *(static_cast<const FT4CameraWorkAction*>(InAction));

	EasingCurveBlender.Initialize(
		ConvAction.BlendInCurve,
		ConvAction.BlendInTimeSec,
		ConvAction.BlendOutCurve,
		ConvAction.BlendOutTimeSec
	);

	for (const FT4CameraWorkSectionKeyData& KeyData : ConvAction.SectionData.KeyDatas)
	{
		FT4PlayCameraWorkSectionKeyData& NewKeyData = PlayKeyDatas.AddDefaulted_GetRef();
		NewKeyData.KeyData = KeyData;
	}

#if WITH_EDITOR
	const FT4EditorParameters& EditorParam = ActionParameterPtr->EditorParams;
	if (EditorParam.InvisibleActionSet.Contains(ConvAction.HeaderKey))
	{
		bShowEditorCameraActor = true; // #56, #58 : Invisible 이면 카메라 모델로 대체!
	}
#endif

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionCameraWorkNode::Destroy()
{
}

void FT4ActionCameraWorkNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (!IsPlayed())
	{
		return;
	}
	AT4PlayerCameraManager* CameraManager = GetPlayerCameraManager();
	if (nullptr == CameraManager)
	{
		return;
	}
	UT4CameraModifier* CameraModifier = GetCameraModifier();
	if (nullptr == CameraModifier)
	{
		return;
	}
	AT4GameObject* GameObject = GetGameObject();
	if (nullptr == GameObject)
	{
		return;
	}
	int32 PlayKeyIndex = INDEX_NONE;
	int32 NumPlayKeyDatas = PlayKeyDatas.Num();
	for (int32 i = 0; i < NumPlayKeyDatas; ++i)
	{
		const FT4PlayCameraWorkSectionKeyData& CurrKeyData = PlayKeyDatas[i];
		if (GetPlayingTime() >= CurrKeyData.StartTimeSec && GetPlayingTime() < CurrKeyData.EndTimeSec)
		{
			PlayKeyIndex = i;
			break;
		}
	}
	if (INDEX_NONE == PlayKeyIndex || PlayKeyIndex == (NumPlayKeyDatas - 1))
	{
		return;
	}
	const FT4PlayCameraWorkSectionKeyData& SourcePlayKeyData = PlayKeyDatas[PlayKeyIndex];
	check(SourcePlayKeyData.NextKeyIndex < PlayKeyDatas.Num());
	const FT4PlayCameraWorkSectionKeyData& TargetPlayKeyData = PlayKeyDatas[SourcePlayKeyData.NextKeyIndex];

	float SegmentWeight = 1.0f;
	if (0.0f < SourcePlayKeyData.RangeTimeSec)
	{
		SegmentWeight = (GetPlayingTime() - SourcePlayKeyData.StartTimeSec) / SourcePlayKeyData.RangeTimeSec;
	}

	float LocalBlendWeight = SegmentWeight;
	if (ET4BuiltInEasing::Linear != SourcePlayKeyData.KeyData.EasingCurve)
	{
		LocalBlendWeight = T4EngineUtility::Evaluate(
			SourcePlayKeyData.KeyData.EasingCurve,
			SegmentWeight
		);
	}

	FVector SourceLocation = FVector::ZeroVector;
	FRotator SourceRotation = FRotator::ZeroRotator;
	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;
	float SourceFOVDegree = (0.0f < SourcePlayKeyData.KeyData.FOVDegree) 
		? SourcePlayKeyData.KeyData.FOVDegree : CameraManager->GetFOVAngle();
	float TargetFOVDegree = (0.0f < TargetPlayKeyData.KeyData.FOVDegree) 
		? TargetPlayKeyData.KeyData.FOVDegree : CameraManager->GetFOVAngle();

	const FVector OwnerForward = -GameObject->GetFrontVector();
	const FRotator OwnerRotation = GameObject->GetRotation();

	if (SourcePlayKeyData.KeyData.LookAtPoint == NAME_None || // 본래 카메라!
		!GameObject->GetSocketLocation(SourcePlayKeyData.KeyData.LookAtPoint, SourceLocation))
	{
		SourceLocation = CameraManager->GetCameraLocation();
		SourceRotation = CameraManager->GetCameraRotation();
	}
	else
	{
		FVector WorldViewDirection = OwnerRotation.RotateVector(SourcePlayKeyData.KeyData.ViewDirection);
		SourceLocation += (-WorldViewDirection * SourcePlayKeyData.KeyData.Distance); // Distance 카메라 방향 반대로 적용
		if (SourcePlayKeyData.KeyData.bInverse)
		{
			SourceRotation = (-WorldViewDirection).ToOrientationRotator();
		}
		else
		{
			SourceRotation = WorldViewDirection.ToOrientationRotator();
		}
	}

	if (TargetPlayKeyData.KeyData.LookAtPoint == NAME_None || // 본래 카메라!
		!GameObject->GetSocketLocation(TargetPlayKeyData.KeyData.LookAtPoint, TargetLocation))
	{
		TargetLocation = CameraManager->GetCameraLocation();
		TargetRotation = CameraManager->GetCameraRotation();
	}
	else
	{
		FVector WorldViewDirection = OwnerRotation.RotateVector(TargetPlayKeyData.KeyData.ViewDirection);
		TargetLocation += (-WorldViewDirection * TargetPlayKeyData.KeyData.Distance); // Distance 카메라 방향 반대로 적용
		if (TargetPlayKeyData.KeyData.bInverse)
		{
			TargetRotation = (-WorldViewDirection).ToOrientationRotator();
		}
		else
		{
			TargetRotation = WorldViewDirection.ToOrientationRotator();
		}
	}

	FVector FinalLocation = FMath::Lerp(SourceLocation, TargetLocation, LocalBlendWeight);
	FRotator FinalRotation = FMath::Lerp(SourceRotation, TargetRotation, LocalBlendWeight);
	float FinalFOVDegree = FMath::Lerp(SourceFOVDegree, TargetFOVDegree, LocalBlendWeight);

	const float BlendWeight = GetBlendWeight();

#if WITH_EDITOR
	if (bShowEditorCameraActor)
	{
		FVector Location = FMath::Lerp(CameraManager->GetCameraLocation(), FinalLocation, BlendWeight);
		FRotator Rotation = FMath::Lerp(CameraManager->GetCameraRotation(), FinalRotation, BlendWeight);
		AT4EditorCameraActor* EditorCameraActor = GetGameWorld()->FindOrCreateEditorCameraActor(
			GameObject->GetObjectID().Value,
			true, 
			true
		);
		check(nullptr != EditorCameraActor);
		EditorCameraActor->SetActorLocationAndRotation(Location, Rotation);
		EditorCameraActor->SetActorScale3D(FVector(0.75f));
		return;
	}
#endif

	// Blend !! Camera At 
	GameObject->SetCameraTargetBlend(
		LocalBlendWeight,
		&SourcePlayKeyData.KeyData,
		&TargetPlayKeyData.KeyData,
		BlendWeight
	);

	CameraModifier->UpdateCameraAnimBlend(
		FinalLocation,
		FinalRotation,
		FinalFOVDegree,
		BlendWeight
	);

	// UE_LOG(LogT4Engine, Display, TEXT("BlendWeight = %.2f"), BlendWeight);
}

bool FT4ActionCameraWorkNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionCameraWorkNode::Stop()
{
	AT4GameObject* GameObject = GetGameObject();
	if (nullptr != GameObject)
	{
		GameObject->SetCameraTargetBlend(
			0.0f,
			nullptr,
			nullptr,
			0.0f
		);
	}
	UT4CameraModifier* CameraModifier = GetCameraModifier();
	if (nullptr != CameraModifier)
	{
		CameraModifier->UpdateCameraAnimBlend(FVector::ZeroVector, FRotator::ZeroRotator, 0.0f, 0.0f);
	}
#if WITH_EDITOR
	if (bShowEditorCameraActor)
	{
		GetGameWorld()->DestroyEditorCameraActor(GameObject->GetObjectID().Value);
		return;
	}
#endif
}

bool FT4ActionCameraWorkNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}

bool FT4ActionCameraWorkNode::PlayInternal(float InOffsetTimeSec)
{
	AT4PlayerCameraManager* CameraManager = GetPlayerCameraManager();
	if (nullptr == CameraManager)
	{
		return false;
	}
	PlayKeyDatas.Sort([](const FT4PlayCameraWorkSectionKeyData& A, const FT4PlayCameraWorkSectionKeyData& B)
	{
		return A.KeyData.ChannelKey < B.KeyData.ChannelKey;
	});
#if WITH_EDITOR
	if (!bShowEditorCameraActor && ActionParameterPtr.IsValid())
	{
		bShowEditorCameraActor = ActionParameterPtr->EditorParams.bDebugPlay; // #58 : 디버깅용 모델로 대체!
	}
#endif
	float EndPlayTimeSec = GetDesiredPlayTime();
	if (0.0f >= EndPlayTimeSec)
	{
		return false;
	}

	int32 NextKeyIndex = INDEX_NONE;
	float BlendInOffsetTimeSec = 0.0f; // #58 : 시작 시점이 필요에 의해 늦어질 경우 (ex. CameraWork)
	float BlendOutOffsetTimeSec = 0.0f; // #58 : 끝 시점이 필요에 의해 늦어질 경우 (ex. CameraWork)
	{
		// #58 : CameraWork 는 블랜딩 처리를 위해 첫번째와 마지막 키를 추가하고, BlendIn/Out 시간을 설정해준다. (SectionKey 이기 때문)
		//       LookAtPont 가 NAME_None 이면 현재 Player 카메라 위치를 사용한다.
		{
			FT4PlayCameraWorkSectionKeyData StartKeyData = PlayKeyDatas.InsertDefaulted_GetRef(0);
			StartKeyData.KeyData = PlayKeyDatas[1].KeyData;
			StartKeyData.KeyData.ChannelKey = INDEX_NONE;
			StartKeyData.KeyData.LookAtPoint = NAME_None; // Current Camera
			if (StartKeyData.KeyData.StartTimeSec < EasingCurveBlender.GetBlendInTimeSec())
			{
				EasingCurveBlender.SetBlendInTimeSec(StartKeyData.KeyData.StartTimeSec);
			}
			else
			{
				// BlendWeight 계산시 BlendInOffsetTimeSec 시간 후 계산하도록 조정
				BlendInOffsetTimeSec = FMath::Max(0.0f, StartKeyData.KeyData.StartTimeSec - EasingCurveBlender.GetBlendInTimeSec());
				StartKeyData.KeyData.StartTimeSec = EasingCurveBlender.GetBlendInTimeSec();
			}
		}
		// #58 : 참고로 마지막 SectionKey 의 EasingCurve 는 사용되지 않는다.
		{
			FT4PlayCameraWorkSectionKeyData& EndKeyData = PlayKeyDatas.AddDefaulted_GetRef();
			EndKeyData.KeyData = PlayKeyDatas[PlayKeyDatas.Num() - 2].KeyData;
			EndKeyData.KeyData.ChannelKey = INDEX_NONE;
			EndKeyData.KeyData.LookAtPoint = NAME_None; // Current Camera
			EndKeyData.KeyData.StartTimeSec += EasingCurveBlender.GetBlendOutTimeSec(); // 종료를 블랜딩 타입을 고려해서 시간 설정!
			if (EndPlayTimeSec < EndKeyData.KeyData.StartTimeSec)
			{
				float OverTimeSec = EndKeyData.KeyData.StartTimeSec - EndPlayTimeSec;
				EndKeyData.KeyData.StartTimeSec -= OverTimeSec;
				float BlendOutTimeSec = FMath::Max(0.0f, EasingCurveBlender.GetBlendOutTimeSec() - OverTimeSec);
				EasingCurveBlender.SetBlendOutTimeSec(BlendOutTimeSec);
			}
			else
			{
				// BlendWeight 계산시 BlendOutOffsetTimeSec 을 제외하고 계산하도록 조정
				BlendOutOffsetTimeSec = FMath::Max(0.0f, EndPlayTimeSec - EndKeyData.KeyData.StartTimeSec);
			}
		}
	}
	EasingCurveBlender.SetOffsetTimeSec(BlendInOffsetTimeSec, BlendOutOffsetTimeSec);

	int32 NumPlayKeyDatas = PlayKeyDatas.Num();
	for (int i = NumPlayKeyDatas - 1; i >= 0; --i)
	{
		FT4PlayCameraWorkSectionKeyData& PlayKeyData = PlayKeyDatas[i];
		PlayKeyData.StartTimeSec = PlayKeyData.KeyData.StartTimeSec - DelayTimeSec;
		PlayKeyData.EndTimeSec = EndPlayTimeSec - DelayTimeSec;
		PlayKeyData.RangeTimeSec = PlayKeyData.EndTimeSec - PlayKeyData.StartTimeSec;
		PlayKeyData.NextKeyIndex = NextKeyIndex;
		EndPlayTimeSec = PlayKeyData.StartTimeSec;
		NextKeyIndex = i;
	}
	return true;
}
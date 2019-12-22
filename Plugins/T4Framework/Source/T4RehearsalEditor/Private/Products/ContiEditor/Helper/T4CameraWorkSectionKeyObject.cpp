// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4CameraWorkSectionKeyObject.h"

#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Classes/Camera/T4EditorCameraActor.h" // #58

#include "IDetailsView.h"
#include "ScopedTransaction.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "UT4CameraWorkSectionKeyObject"

/**
  * #68
 */
UT4CameraWorkSectionKeyObject::UT4CameraWorkSectionKeyObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ChannelKey(INDEX_NONE)
	, StartTimeSec(0.0f)
	, EasingCurve(ET4BuiltInEasing::Linear)
	, LookAtPoint(NAME_None)
	, bInverse(false)
	, ViewDirection(FVector::BackwardVector)
	, Distance(100.0f)
	, FOVDegree(0.0f)
	, ViewModelRef(nullptr)
	, ActionHeaderKey(INDEX_NONE)
	, FocusSectionKeyIndexSelected(INDEX_NONE)
	, SavedEasingCurve(ET4BuiltInEasing::Linear) // #102 : Droplist 선택시 PropertyChanged event 가 와서 변경을 못하는 문제가 있어 추가
	, CachedLookAtPoint(NAME_None)
	, bCachedInverse(false)
	, CachedCameraLocation(FVector::ZeroVector)
	, CachedCameraRotation(FRotator::ZeroRotator)
{
}

void UT4CameraWorkSectionKeyObject::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent
)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UT4CameraWorkSectionKeyObject::Reset()
{
	ChannelKey = INDEX_NONE;
	StartTimeSec = 0.0f;
	EasingCurve = ET4BuiltInEasing::Linear;
	LookAtPoint = NAME_None;
	bInverse = false;
	ViewDirection = FVector::BackwardVector;
	Distance = 100.0f;
	FOVDegree = 0.0f;

	SavedEasingCurve = ET4BuiltInEasing::Linear; // #102
	CachedLookAtPoint = NAME_None;
	bCachedInverse = false;
	CachedCameraLocation = FVector::ZeroVector;
	CachedCameraRotation = FRotator::ZeroRotator;
}

void UT4CameraWorkSectionKeyObject::SetDetailView(TSharedRef<IDetailsView> InDetailView)
{
	DetailViewPtr = InDetailView;
}

void UT4CameraWorkSectionKeyObject::RefreshDetailView()
{
	if (DetailViewPtr.IsValid())
	{
		DetailViewPtr->ForceRefresh();
	}
}

bool UT4CameraWorkSectionKeyObject::Select(UT4ContiAsset* InContiAsset, int32 InChannelKey)
{
	check(nullptr != InContiAsset);
	FT4ContiActionStruct* ActionStruct = InContiAsset->CompositeData.GetActionStruct(ActionHeaderKey);
	if (nullptr == ActionStruct)
	{
		return false;
	}
	const FT4CameraWorkAction* CameraWorkStruct = static_cast<const FT4CameraWorkAction*>(ActionStruct);
	check(nullptr != CameraWorkStruct);
	const FT4CameraWorkSectionData& SectionData = CameraWorkStruct->SectionData;
	bool bValid = false;
	uint32 NumIndex = 0;
	for (const FT4CameraWorkSectionKeyData& KeyData : SectionData.KeyDatas)
	{
		if (0 == InChannelKey || InChannelKey == KeyData.ChannelKey)
		{
			ChannelKey = KeyData.ChannelKey;
			StartTimeSec = KeyData.StartTimeSec;
			EasingCurve = KeyData.EasingCurve;
			LookAtPoint = KeyData.LookAtPoint; // ActionPoint
			bInverse = KeyData.bInverse;
			ViewDirection = KeyData.ViewDirection; // Local
			Distance = KeyData.Distance;
			FOVDegree = KeyData.FOVDegree;
			FocusSectionKeyIndexSelected = NumIndex;

			// #102 : Droplist 선택시 PropertyChanged event 가 와서 변경을 못하는 문제가 있어 추가
			SavedEasingCurve = KeyData.EasingCurve;
			return true;
		}
		NumIndex++;
	}
	return false;
}

bool UT4CameraWorkSectionKeyObject::SaveTo(UT4ContiAsset* InContiAsset)
{
	check(nullptr != InContiAsset);
	FT4ContiActionStruct* ActionStruct = InContiAsset->CompositeData.GetActionStruct(ActionHeaderKey);
	if (nullptr == ActionStruct)
	{
		return false;
	}
	FT4CameraWorkAction* CameraWorkStruct = static_cast<FT4CameraWorkAction*>(ActionStruct);
	check(nullptr != CameraWorkStruct);
	FT4CameraWorkSectionData& SectionData = CameraWorkStruct->SectionData;
	bool bValid = false;
	for (FT4CameraWorkSectionKeyData& KeyData : SectionData.KeyDatas)
	{
		if (ChannelKey == KeyData.ChannelKey)
		{
			const FScopedTransaction Transaction(LOCTEXT("UT4CameraWorkSectionKeyObjectSaveTo_Transaction", "Save to CameraWork SectionKey"));
			InContiAsset->Modify();

			//KeyData.ChannelKey = EditorCameraSectionKey->ChannelKey; // #58 : 이 두값은 UI 로
			//KeyData.DelayTimeSec = EditorCameraSectionKey->DelayTimeSec;
			KeyData.EasingCurve = EasingCurve;
			KeyData.LookAtPoint = LookAtPoint; // ActionPoint
			KeyData.bInverse = bInverse;
			KeyData.ViewDirection = ViewDirection; // Local
			KeyData.Distance = Distance;
			KeyData.FOVDegree = FOVDegree;
			return true;
		}
	}
	return false;
}

AT4EditorCameraActor* UT4CameraWorkSectionKeyObject::GetFocusCameraActor(
	IT4GameWorld* InGameWorld,
	bool bInCreate
) // #58
{
	if (INDEX_NONE == FocusSectionKeyIndexSelected)
	{
		return nullptr;
	}
	return GetCameraActor(InGameWorld, GetCameraActorKey(FocusSectionKeyIndexSelected), bInCreate);
}

void UT4CameraWorkSectionKeyObject::SetFocusCameraActor(IT4GameObject* InPlayerObject)
{
	if (nullptr == InPlayerObject)
	{
		return;
	}
	if (INDEX_NONE == FocusSectionKeyIndexSelected)
	{
		return;
	}
	AT4EditorCameraActor* EditorCameraActor = GetCameraActor(
		InPlayerObject->GetGameWorld(),
		GetCameraActorKey(FocusSectionKeyIndexSelected),
		true
	);
	if (nullptr == EditorCameraActor)
	{
		return;
	}
	SetCameraActorTransform(
		EditorCameraActor,
		InPlayerObject,
		LookAtPoint,
		bInverse,
		ViewDirection,
		Distance,
		true
	);
	RefreshDetailView();
}

void UT4CameraWorkSectionKeyObject::CheckAndSpawnCameraActors(
	UT4ContiAsset* InContiAsset, 
	IT4GameObject* InPlayerObject
)
{
	check(nullptr != InContiAsset);
	if (nullptr == InPlayerObject)
	{
	return;
	}
	FT4ContiActionStruct* ActionStruct = InContiAsset->CompositeData.GetActionStruct(ActionHeaderKey);
	if (nullptr == ActionStruct)
	{
		return;
	}
	IT4GameWorld* GameWorld = InPlayerObject->GetGameWorld();
	check(nullptr != GameWorld);
	const FT4CameraWorkAction* CameraWorkStruct = static_cast<const FT4CameraWorkAction*>(ActionStruct);
	check(nullptr != CameraWorkStruct);
	const FT4CameraWorkSectionData& SectionData = CameraWorkStruct->SectionData;
	uint32 NumIndex = 0;
	for (const FT4CameraWorkSectionKeyData& KeyData : SectionData.KeyDatas)
	{
		uint32 CurrCameraActorKey = GetCameraActorKey(NumIndex);
		AT4EditorCameraActor* CameraActor = GetCameraActor(
			GameWorld,
			CurrCameraActorKey,
			true
		); // #58
		if (nullptr != CameraActor)
		{
			SetCameraActorTransform(
				CameraActor,
				InPlayerObject,
				KeyData.LookAtPoint,
				KeyData.bInverse,
				KeyData.ViewDirection,
				KeyData.Distance,
				false
			);
			SpawnCameraActorKeys.Add(CurrCameraActorKey);
		}
		NumIndex++;
	}
	// 삭제된 카메라 정리
	uint32 NumSpawnCameraActors = SpawnCameraActorKeys.Num();
	for (uint32 i = SectionData.KeyDatas.Num(); i < NumSpawnCameraActors; ++i)
	{
		uint32 RemoveCameraActorKey = GetCameraActorKey(i);
		GameWorld->DestroyEditorCameraActor(RemoveCameraActorKey);
		SpawnCameraActorKeys.Remove(RemoveCameraActorKey);
	}
}

AT4EditorCameraActor* UT4CameraWorkSectionKeyObject::GetCameraActor(
	IT4GameWorld* InGameWorld,
	uint32 InCameraActorKey,
	bool bInCreate
) // #58
{
	if (nullptr == InGameWorld)
	{
		return nullptr;
	}
	return InGameWorld->FindOrCreateEditorCameraActor(InCameraActorKey, bInCreate, false);
}

void UT4CameraWorkSectionKeyObject::ClearCameraActors(IT4GameWorld* InGameWorld)
{
	if (nullptr == InGameWorld)
	{
		return;
	}
	for (uint32 CameraKey : SpawnCameraActorKeys)
	{
		InGameWorld->DestroyEditorCameraActor(CameraKey);
	}
	SpawnCameraActorKeys.Empty();
	FocusSectionKeyIndexSelected = INDEX_NONE;

	SavedEasingCurve = ET4BuiltInEasing::Linear; // #102
	CachedLookAtPoint = NAME_None;
	bCachedInverse = false;
	CachedCameraLocation = FVector::ZeroVector;
	CachedCameraRotation = FRotator::ZeroRotator;
}

void UT4CameraWorkSectionKeyObject::SetCameraActorTransform(
	AT4EditorCameraActor* InCameraActor,
	IT4GameObject* InPlayerObject,
	FName InLootAtPont,
	bool bInInverse,
	const FVector& InViewDirection,
	float InDistance,
	bool bInFocus
)
{
	if (nullptr == InPlayerObject || nullptr == InCameraActor)
	{
		return;
	}
	FVector LookAtLocation = FVector::ZeroVector;
	if (!InPlayerObject->GetSocketLocation(InLootAtPont, LookAtLocation))
	{
		// TODO : Fallback
	}
	FVector WorldViewDirection = InPlayerObject->GetRotation().RotateVector(InViewDirection);
	FVector CameraLocation = LookAtLocation + (-WorldViewDirection * InDistance); // Distance 카메라 방향 반대로 적용
	FRotator CameraRotation;
	if (bInInverse)
	{
		CameraRotation = (-WorldViewDirection).ToOrientationRotator();
	}
	else
	{
		CameraRotation = WorldViewDirection.ToOrientationRotator();
	}
	InCameraActor->SetActorLocationAndRotation(CameraLocation, CameraRotation);
	if (bInFocus)
	{
		InCameraActor->SetActorScale3D(FVector(0.5f));
	}
	else
	{
		InCameraActor->SetActorScale3D(FVector(0.4f));
	}
}

bool UT4CameraWorkSectionKeyObject::UpdateCameraActor(IT4GameObject* InPlayerObject)
{
	if (nullptr == InPlayerObject)
	{
		return false;
	}
	if (INDEX_NONE == FocusSectionKeyIndexSelected)
	{
		return false;
	}
	AT4EditorCameraActor* EditorCameraActor = GetCameraActor(
		InPlayerObject->GetGameWorld(), 
		GetCameraActorKey(FocusSectionKeyIndexSelected), 
		true
	);
	if (nullptr == EditorCameraActor)
	{
		return false;
	}
	const FVector CameraLocation = EditorCameraActor->GetActorLocation();
	const FRotator CameraRotation = EditorCameraActor->GetActorRotation();
	if (CachedLookAtPoint == LookAtPoint &&
		bCachedInverse == bInverse &&
		FVector(CameraLocation - CachedCameraLocation).IsNearlyZero() &&
		FRotator(CameraRotation - CachedCameraRotation).IsNearlyZero())
	{
		return false; // 비교가 좀 많은데, 작은 차이로 에셋 수정이 발생하지 않는 것이 더 중요함으로 꼼꼼히 체크해준다.
	}
	FVector LookAtLocation = FVector::ZeroVector;
	if (!InPlayerObject->GetSocketLocation(LookAtPoint, LookAtLocation))
	{
		// TODO : Fallback
	}
	FVector WorldViewDirection = LookAtLocation - CameraLocation;
	float NewViewDistance = WorldViewDirection.Size();
	check(0 <= NewViewDistance);
	WorldViewDirection /= NewViewDistance;
	ViewDirection = InPlayerObject->GetRotation().UnrotateVector(WorldViewDirection); // 저장은 Local, 카메라는 World
	Distance = NewViewDistance;
	FRotator FinalCameraRotation;
	if (bInverse)
	{
		FinalCameraRotation = (-WorldViewDirection).ToOrientationRotator();
	}
	else
	{
		FinalCameraRotation = WorldViewDirection.ToOrientationRotator();
	}
	{
		CachedLookAtPoint = LookAtPoint;
		bCachedInverse = bInverse;
		CachedCameraLocation = CameraLocation;
		CachedCameraRotation = FinalCameraRotation;
	}
	EditorCameraActor->SetActorRotation(FinalCameraRotation);
	return true;
}

void UT4CameraWorkSectionKeyObject::HandleOnSectionKeySelected(int32 InSelectedChannelKey)
{
	if (nullptr == ViewModelRef)
	{
		return;
	}
	ViewModelRef->FindOrCreateCameraSectionKeyObject(ActionHeaderKey, InSelectedChannelKey, true);
	RefreshDetailView();
}

void UT4CameraWorkSectionKeyObject::HandleOnDetailPropertiesChanged(const FPropertyChangedEvent& InEvent)
{
	if (nullptr == ViewModelRef)
	{
		return;
	}
	static bool GTest = 0;
	if (InEvent.GetPropertyName() == FName(TEXT("EasingCurve")))
	{
		// #102 : Droplist 선택시 PropertyChanged event 가 와서 변경을 못하는 문제가 있어 추가
		if (EasingCurve == SavedEasingCurve)
		{
			return;
		}
	}
	UT4ContiAsset* ContiAsset = ViewModelRef->GetContiAsset();
	check(nullptr != ContiAsset);
	bool bResult = SaveTo(ContiAsset);
	if (bResult)
	{
		SetFocusCameraActor(ViewModelRef->GetPlayerObject());
	}
}

#undef LOCTEXT_NAMESPACE
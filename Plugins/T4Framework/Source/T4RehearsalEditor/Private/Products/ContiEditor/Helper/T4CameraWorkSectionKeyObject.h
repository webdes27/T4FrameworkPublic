// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionTypes.h"
#include "T4CameraWorkSectionKeyObject.generated.h"

/**
  * #58
 */
class UT4ContiAsset;
class IT4GameObject;
class IT4GameWorld;
class IDetailsView;
class AT4EditorCameraActor;
class FT4ContiViewModel;
struct FT4CameraWorkSectionKeyData;
UCLASS()
class UT4CameraWorkSectionKeyObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// UObject
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	void Reset();

	void SetDetailView(TSharedRef<IDetailsView> InDetailView);

	void RefreshDetailView(); // #58

	bool Select(UT4ContiAsset* InContiAsset, int32 InChannelKey);
	bool SaveTo(UT4ContiAsset* InContiAsset);

	AT4EditorCameraActor* GetFocusCameraActor(IT4GameWorld* InGameWorld, bool bInCreate); // #58
	void SetFocusCameraActor(IT4GameObject* InPlayerObject);

	void CheckAndSpawnCameraActors(UT4ContiAsset* InContiAsset, IT4GameObject* InPlayerObject);

	void ClearCameraActors(IT4GameWorld* InGameWorld);
	bool UpdateCameraActor(IT4GameObject* InPlayerObject);

public:
	int32 HandleOnGetValueIndexSelcted() const { return ChannelKey; }
	void HandleOnSectionKeySelected(int32 InSelectedChannelKey);

	void HandleOnDetailPropertiesChanged(const FPropertyChangedEvent& InEvent); // #58

private:
	uint32 GetCameraActorKey(uint32 InIndex) const { return (ActionHeaderKey * 50) + InIndex; } // #58 : 최대 50개까지 Key...
	AT4EditorCameraActor* GetCameraActor(IT4GameWorld* InGameWorld, uint32 InCameraActorKey, bool bInCreate); // #58

	void SetCameraActorTransform(
		AT4EditorCameraActor* InCameraActor,
		IT4GameObject* InPlayerObject,
		FName InLootAtPont,
		bool bInInverse,
		const FVector& InViewDirection,
		float InDistance,
		bool bInFocus
	);

public:
	// #58 : Property 수정시 FT4CameraWorkSectionKeyData 에도 추가해줄 것!
	//       SaveCameraSectionKeyObject, UpdateCameraSectionKeyObject
	//       FT4CameraWorkSectionKeyDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InBuilder)
	UPROPERTY(VisibleAnywhere, Category = KeyData)
	int32 ChannelKey;

	UPROPERTY(EditAnywhere, Category = KeyData)
	float StartTimeSec;

	UPROPERTY(EditAnywhere, Category = KeyData)
	ET4BuiltInEasing EasingCurve;

	UPROPERTY(EditAnywhere, Category = KeyData)
	FName LookAtPoint; // ActionPoint

	UPROPERTY(EditAnywhere, Category = KeyData)
	bool bInverse; // LookAtPoint Inverse

	UPROPERTY(VisibleAnywhere, Category = KeyData)
	FVector ViewDirection; // Local

	UPROPERTY(EditAnywhere, Category = KeyData)
	float Distance;

	UPROPERTY(EditAnywhere, Category = KeyData)
	float FOVDegree;

public:
	FT4ContiViewModel* ViewModelRef;

	uint32 ActionHeaderKey;

	uint32 FocusSectionKeyIndexSelected;
	TSet<uint32> SpawnCameraActorKeys;

	ET4BuiltInEasing SavedEasingCurve; // #102 : Droplist 선택시 PropertyChanged event 가 와서 변경을 못하는 문제가 있어 추가
	FName CachedLookAtPoint;
	bool bCachedInverse;
	FVector CachedCameraLocation;
	FRotator CachedCameraRotation;

	TSharedPtr<IDetailsView> DetailViewPtr;
};

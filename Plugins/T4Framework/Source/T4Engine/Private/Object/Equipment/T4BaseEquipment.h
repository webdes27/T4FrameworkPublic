// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4Engine.h"
#include "Public/T4EngineAnimNotify.h" // #111
#include "Public/Asset/T4AssetLoader.h"

#include "T4Asset/Public/Entity/T4EntityKey.h"

/**
  * #68, #107
 */
enum ET4EquipLoadState
{
	ELS_Ready,
	ELS_Loading,
	ELS_Loaded,
	ELS_TryAttach, // #108 
	ELS_Completed, // #108 
	ELS_NotSet,
	ELS_Failed,
};

class UMeshComponent;
class AT4GameObject;
class UMaterialInstanceDynamic; // #108
class FT4BaseEquipment
{
public:
	FT4BaseEquipment(AT4GameObject* InOwnerObject, const FT4ActionKey& InActionKey);
	virtual ~FT4BaseEquipment();

	static FT4BaseEquipment* NewInstance(
		AT4GameObject* InOwnerObject,
		const FT4ActionKey& InActionKey,
		bool bInMainWeapon, // #111
		const FT4EntityKey& InEntityKey,
		const FName InOverrideEquipPoint,
		bool bInChangeStance, // #110
		bool bInUseAnimNotify // #111 : Stance 변경에 있는 AnimNotify_Equipment Mount 에 따라 Hide => Show 처리가 됨
	);

	void OnReset();

	void OnAdvance(const FT4UpdateTime& InUpdateTime);

	void OnAttach(
		bool bInMainWeapon, // #111
		const FT4EntityKey& InEntityKey,
		FName InOverrideEquipPoint,
		bool bChangeStance,
		bool bUseAnimNotify // #111 : Stance 변경에 있는 AnimNotify_Equipment Mount 에 따라 Hide => Show 처리가 됨
	);
	void OnDetach(FName InRestoreStanceName, bool bUseAnimNotify); // #110, #111 : Stance 변경에 있는 AnimNotify_Equipment Unmount 에 따라 Show => Hide 처리가 됨

	void OnAnimNotify(const FT4AnimNotifyEquipment& InAnimNotify); // #111

	void OnStartLoading();

	bool IsAttachable() const { return !bPendingAttach; } // #111
	bool IsDetachable() const { return bDetachable; } // #111
	bool IsDestroyable() const { return !bPendingDetach; } // #111 : Detach Pending 이 있다면 끝나고 삭제!

	virtual bool IsWeapon() const { return false; }
	virtual bool IsMainWeapon() const { return bMainWeapon; } // #111

	virtual bool PlayAnimation(const FT4AnimParameters& InAnimParameters) { return false; } // #107

	const FT4ActionKey& GetActionKey() const { return ActionKey; } // #111
	const FName GetEquipPoint() const { return EquipPoint; }
	const FSoftObjectPath GetObjectPath() const;

#if (WITH_EDITOR || WITH_SERVER_CODE)
	bool HasOverlapEvent() const { return bOverlapEventEnabled; }
	FName GetOverlapEventName() const { return OverlapEventName; }

	void OnBeginOverlapEvents(const FName& InOverlapEventName); // #49
	void OnEndOverlapEvents(); // #49
#endif

protected:
	virtual void Reset() {}
	virtual void Advance(const FT4UpdateTime& InUpdateTime) {}
	virtual void AttackPrepare(const FT4EntityKey& InEntityKey, FName InOverrideEquipPoint) {}
	virtual void DetachPrepare() {}
	virtual void AnimNotify(const FT4AnimNotifyEquipment& InAnimNotify) {} // #111
	virtual void StartLoading() {}

	bool IsParentLoaded() const;

	void Attach(); // #111
	void Detach(); // #111

	void SePrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent);
	void ResePrimitiveComponent();

	bool AttachToParent(FAttachmentTransformRules InAttachTransformRule);
	void DetachFromParent(FDetachmentTransformRules InDetachTransformRule);

	void ClearOverrideMaterialLoader();
	void SetOverrideMaterials(UMeshComponent* InMeshComponent); // #80

	void AddDynamicMaterialInstances(UMeshComponent* InMeshComponent); // #78, #108
	void ResetDynamicMaterialInstances(); // #78, #108
	void SetDynamicMaterialInstanceParameter(FName InParameterName, float InValue); // #78, #108

	void UpdateMaterialOpacityParameter(); // #108

	void ChangeStanceByWeapon(FName InStanceName); // #110

	template <class T>
	T* NewComponentTemplate(UObject* InOuter, bool bInAutoActivate)
	{
		T* NewComponent = NewObject<T>(InOuter);
		NewComponent->bAutoActivate = bInAutoActivate;
		NewComponent->SetRelativeLocation(FVector::ZeroVector);
		return NewComponent;
	}

private:
	void AdvanceOpacity(const FT4UpdateTime& InUpdateTime); // #78, #108

	USceneComponent* TryGetAttachComponent(USceneComponent* InParentComponent); // #108

protected:
	bool bAttached;
	bool bDetachable; // #111 : OnDetach
	bool bPendingAttach; // #111
	bool bPendingDetach; // #111

	bool bMainWeapon; // #111 : MainWeapon 만 Stance 를 변경하도록 처리
	FT4ActionKey ActionKey; // #111
	FName EquipPoint;
	FRotator RelativeRotation; // #108
	float RelativeScale; // #108
	FName StanceName; // #111
	bool bOverlapEvent; // #106

	FT4EntityKey EntityKey;
	FSoftObjectPath ObjectPath;

	TWeakObjectPtr<AT4GameObject> OwnerObjectPtr;

	ET4EquipLoadState OverrideMaterialLoadState;
	TArray<FT4MaterialLoader> OverrideMaterialLoaders; // #80
	TArray<FSoftObjectPath> OverrideMaterialPaths; // #80

	float CurrentOpacityValue; // #108
	TArray<UMaterialInstanceDynamic*> MaterialDynamicInstances; // #108

private:
	TWeakObjectPtr<UPrimitiveComponent> PrimitiveComponentPtr;

#if (WITH_EDITOR || WITH_SERVER_CODE)
	bool bOverlapEventEnabled;
	FName OverlapEventName; // #49
#endif
};

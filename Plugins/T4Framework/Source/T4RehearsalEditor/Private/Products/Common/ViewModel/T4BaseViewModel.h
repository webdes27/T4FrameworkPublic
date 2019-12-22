// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4RehearsalViewModel.h"

#include "T4Asset/Public/Entity/T4EntityKey.h"
#include "T4Asset/Public/Entity/T4EntityTypes.h"
#include "T4Asset/Public/Action/T4ActionTypes.h"

#include "T4Engine/Public/T4EngineTypes.h"
#include "T4Engine/Public/T4EngineStructs.h" // #87
#include "T4Engine/Public/Action/T4ActionKey.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

/**
  * #76
 */
class FViewport;
class FCanvas;
class UWorld;
struct FT4ActionParameters;
class UAnimSequence;
class UT4ContiAsset;
class UT4EntityAsset;
class IT4GameObject;
class IT4GameWorld; // #93
class IT4PlayerController;
class IT4GameFrame;
class UT4EnvironmentDetailObject; // #94
class UT4EditorActionPlaybackController;
struct FT4HUDDrawInfo;
struct FPostProcessSettings; // #100
struct FT4EditorTestAutomation; // #103
struct FT4EditorPointOfInterest; // #103
class FT4BaseViewModel : public IT4RehearsalViewModel
{
public:
	DECLARE_MULTICAST_DELEGATE(FT4OnViewModelChanged); // #77, #85
	DECLARE_MULTICAST_DELEGATE(FT4OnViewModelDetailPropertyChanged); // #85

public:
	FT4BaseViewModel();
	virtual ~FT4BaseViewModel();

	// IT4RehearsalViewModel
	virtual ET4LayerType GetLayerType() const override { return LayerType; } // #104 : World Editor 는 WorldMapViewModel 에서 별도 처리

	virtual ET4ViewModelEditMode GetEditMode() const override { return ET4ViewModelEditMode::None; }
	virtual bool IsEditWidgetMode() const override { return false; } // #94

	virtual const FString GetAssetPath() override { return FString(); } // #79

	virtual UT4EnvironmentDetailObject* GetEnvironmentDetailObject() override { return nullptr; } // #90, #94

	void OnReset() override; // #79
	void OnStartPlay(FT4RehearsalViewportClient* InViewportClient) override;

	void OnDrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo* InOutDrawInfo) override; // #59, #83

	IT4GameFrame* CreateGameFrame() override; // #87
	IT4GameFrame* GetGameFrame() const override { return GameFrameOwner; } // #79
	IT4GameWorld* GetGameWorld() const override; // #93
	IT4GameObject* GetPlayerObject() const; // #58

	virtual AActor* GetEditWidgetModeTarget() const override { return nullptr; } // #94

	virtual void ChangeWorldEnvironment(FName InTimeTagName) override {} // #94

	void SetViewportShowOptionCapsule(bool bShow) override; // #76
	bool IsShownViewportShowOptionCapsule() const override; //#76

	void SetGameWorldTimeStop(bool bPause) override; // #94
	bool IsGameWorldTimeStopped() const override; // #94

	void SetGameWorldTimelapseScale(float inScale) override; // #93
	float GetGameWorldTimelapseScale() const override; // #93

	void SetGameWorldTimeHour(float InHour) override; // #93
	float GetGameWorldTimeHour() const override; // #93

	virtual void NotifyActionPlaybackRec() override {} // #104
	virtual void NotifyActionPlaybackPlay() override {} // #104

public:
	void OnCleanup(); // #85

	FT4OnViewModelChanged& GetOnViewModelChanged() { return OnViewModelChanged; } // #77, #85
	FT4OnViewModelDetailPropertyChanged& GetOnViewModelDetailPropertyChanged() { return OnViewModelDetailPropertyChanged; }

	bool HasActionPlaybackController() const { return EditorActionPlaybackControllerPtr.IsValid(); } // #104
	UT4EditorActionPlaybackController* GetActionPlaybackController(); // #60, #68, #104

	// Common
	bool GetValidSpawnLocation(
		const FVector& InOriginLocation,
		const FVector2D& InRange,
		int32 InTryCount,
		FVector& OutSpawnLocation
	); // #76

	void SelectPointOfInterest(int32 InIndex); // #100, #103
	void UpdatePointOfInterest(int32 InIndex); // #100, #103
	void TravelPointOfInterest(int32 InIndex); // #100, #103
	int32 AddPointOfInterest(); // #100, #103
	void RemovePointOfInterest(int32 InIndex); // #100, #103

public:

	// Server => Client
	virtual void ServerDespawnAll(bool bClearPlayerObject); // #68
	virtual bool ServerSpawnObject(const FName& InGameDataID); // #60

	virtual bool ServerEquipWeapon(const FName& InWeaponGameDataID, bool bInUnEquip); // #60
	bool ServerEquipWeapon(ET4LayerType InLayerType, const FName& InWeaponGameDataID, bool bInUnEquip); // #60

	// Client
	virtual bool ClientIsPlayingAction(const FT4ActionKey& InActionKey); // #54

	virtual void ClientSetPauseObject(bool bInPause); // #54
	void ClientSetPauseObject(ET4LayerType InLayerType, bool bInPause); // #54

	virtual void ClientStopAction(const FT4ActionKey& InActionKey);
	void ClientStopAction(ET4LayerType InLayerType, const FT4ActionKey& InActionKey);

	virtual bool ClientSpawnObject(UT4EntityAsset* InEntityAsset, const FName InStanceName); // #36, #30, #73
	IT4GameObject* ClientSpawnObjectEx(
		const FT4EntityKey& InEntityKey,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FName InStanceName,
		bool bPlayer
	); // #83

	virtual void ClientDespawnObject(const FT4ObjectID InObjectID); // #67

	virtual bool ClientPlayAnimSequence(UAnimSequence* InAnimSequence); // #39

	virtual void ClientPlayConti(UT4ContiAsset* InContiAsset, const FT4ActionParameters* InActionParameters); // #39, #56
	void ClientPlayConti(
		UT4ContiAsset* InContiAsset,
		const FT4ActionKey& InActionKey,
		const FT4ActionParameters* InActionParameters,
		bool bOverride
	); // #39, #56
	void ClientPlayConti(
		ET4LayerType InLayerType,
		UT4ContiAsset* InContiAsset,
		const FT4ActionKey& InActionKey,
		const FT4ActionParameters* InActionParameters,
		bool bOverride
	); // #39, #56

	virtual void ClientEquipWeapon(UT4EntityAsset* InWeaponEntity, FName InEquipPointName, bool bEquip); // #72
	virtual void ClientExchangeCostume(UT4EntityAsset* InCostumeEntity, FName InCompositePartName, bool bSet); // #72

	virtual void ClientChangeStance(FName InStanceName); // #73
	virtual void ClientGetPlayerStanceList(TSet<FName>& OutStanceNamelist); // #73

	virtual void ClientPlayReaction(
		FName InReactionName,
		ET4EntityReactionType InReactionType,
		const FVector& InShotDirection
	); // #76

	bool ClientTeleport(const FVector& InLocation); // #86
	bool ClientTeleport(const FVector2D& InLocation); // #90

	virtual void ClientPlayLayerTag(FName InLayerTagName, ET4LayerTagType InLayerTagType); // #81
	virtual void ClientStopLayerTag(FName InLayerTagName, ET4LayerTagType InLayerTagType); // #81

	bool ClientWorldTravel(const UT4EntityAsset* InEntityAsset); // #79
	bool ClientWorldTravel(const FT4EntityKey& InMapEntityKey); // #87

	virtual void ClientEditorAction(ET4EditorAction InEditorActionType); // #71

	UWorld* GetWorld() const;

protected:
	virtual void Cleanup() {} // #85
	virtual void Reset() {} // #79
	virtual void StartPlay() {} // #86
	virtual void RestartPlay() {} // #94 : 월드 이동후 호출

	virtual void DrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo* InOutDrawInfo) {} // #59, #83

	// #87 : ViewModel 시작시 특정 레벨을 열고 싶다면, MapEntityAssetPath 를 채울 것!
	virtual void SetupStartWorld(FT4WorldConstructionValues& InWorldConstructionValues) {}

	virtual void NotifyViewTargetChanged(IT4GameObject* InViewTarget) {} // #87, #99 : SubClass 별로 처리해야 할 기능이 있다면...구현

	virtual UObject* GetEditObject() const { return nullptr; } // #103
	virtual FT4EditorTestAutomation* GetTestAutomation() const { return nullptr; } // #103

	virtual void TravelPOI(FT4EditorPointOfInterest* InPOIData); // #100, #103
	virtual bool GetPOIInfo(FT4EditorPointOfInterest* OutPOIData); // #100, #103

	// #99 : SubClass 별로 처리해야 할 기능이 있다면...구현
	virtual void ViewTargetChanged(IT4GameObject* InViewTarget) {} // #79, #83, #99

	virtual FString GetActionPlaybackAssetName() const { return TEXT("ActionPlaybackAsset"); } // #68, #104
	virtual FString GetActionPlaybackFolderName() const { return TEXT("Default"); } // #68, #104

	FT4RehearsalViewportClient* GetViewportClient() const { return ViewportClientRef; }
	IT4PlayerController* GetPlayerController() const;
	IT4GameObject* GetPlayerObject(ET4LayerType InLayerType) const;

	bool IsPreviewSpawnable(ET4EntityType InSelectEntityType) const // #94
	{
		return (ET4EntityType::Map != InSelectEntityType && ET4EntityType::Zone != InSelectEntityType) ? true : false;
	}

	bool IsSpawnable(ET4EntityType InSelectEntityType) const // #94
	{
		return (ET4EntityType::Map != InSelectEntityType) ? true : false;
	}

	bool IsControllable(ET4EntityType InSelectEntityType) const // #94
	{
		return (ET4EntityType::Map != InSelectEntityType && ET4EntityType::Zone != InSelectEntityType) ? true : false;
	}

	bool TryValidSpawnObjectLocation(FVector& OutLocation); // #87

	void SavePlayerSettingsInfo(); // #87
	void RestorePlayerSettingsInfo(); // #87

private:
	void RegisterPlayerViewTargetChanged();

	void HandleOnDestroyViewportClient(); // #79
	void HandleOnViewTargetChanged(IT4GameObject* InViewTarget); // #79, #83

private: // #79 : 월드 교체로 Framework 가 변경될 수 있음으로 관리를 위하 private 로 변경함
	ET4LayerType LayerType;
	IT4GameFrame* GameFrameOwner;
	FT4RehearsalViewportClient* ViewportClientRef;

	FDelegateHandle ViewportClientResetHandle;

	FT4OnViewModelChanged OnViewModelChanged; // #77, #85
	FT4OnViewModelDetailPropertyChanged OnViewModelDetailPropertyChanged;

	TWeakObjectPtr<UT4EditorActionPlaybackController> EditorActionPlaybackControllerPtr; // #68, #104

protected:
	// #87
	bool bCachedPlayerSettingsSaved;
	float CachedCameraZoomDistance;
	FRotator CachedCameraControlRotation;
	FRotator CachedPlayerRotation;

	bool bTestAutomation; // #100, #103
	float TestAutomationGameTimeHour; // #100, #103
	FVector TestAutomationSpawnLocation; // #100, #103
	FRotator TestAutomationSpawnRotation; // #100, #103
};
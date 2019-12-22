// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionContiStructs.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "Input/Reply.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */

class ST4ContiObjectWidget;
class FT4ContiViewModel;
class ST4EntityDropListWidget;
class ST4StanceDropListWidget;
class ST4PointOfInterestListWidget; // #100
class ST4CameraWorkSectionKeyListWidget; // #58
class FT4ContiDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(
		TSharedRef<ST4ContiObjectWidget> InContiObjectWidget
	); // #58
	
	FT4ContiDetailCustomization(TSharedPtr<FT4ContiViewModel> InContiViewModel);

	void CustomizeDetails(IDetailLayoutBuilder& InBuilder);

	void RefreshWidgets(); // #58

protected:
	void CustomizeEditorSetDetails(IDetailLayoutBuilder& InBuilder);

	void CustomizeCommonActionDetails(
		IDetailLayoutBuilder& InBuilder,
		IDetailCategoryBuilder& InDetailCategoryBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4ContiActionStruct* InAction
	);

	// #T4_ADD_ACTION_TAG_CONTI
	void CustomizeBranchActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4BranchAction& InAction,
		uint32 InActionArrayIndex
	); // #54
	void CustomizeSpecialMoveActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4SpecialMoveAction& InAction,
		uint32 InActionArrayIndex
	); // #54
	void CustomizeAnimationActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4AnimationAction& InAction,
		uint32 InActionArrayIndex
	);
	void CustomizeParticleActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4ParticleAction& InAction,
		uint32 InActionArrayIndex
	);
	void CustomizeDecalActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4DecalAction& InAction,
		uint32 InActionArrayIndex
	); // #54
	void CustomizeProjectileActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4ProjectileAction& InAction,
		uint32 InActionArrayIndex
	); // #63
	void CustomizeReactionActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4ReactionAction& InAction,
		uint32 InActionArrayIndex
	); // #76
	void CustomizeLayerSetActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4LayerSetAction& InAction,
		uint32 InActionArrayIndex
	); // #81
	void CustomizeTimeScaleActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4TimeScaleAction& InAction,
		uint32 InActionArrayIndex
	); // #102
	void CustomizeCameraWorkActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4CameraWorkAction& InAction,
		uint32 InActionArrayIndex
	); // #54
	void CustomizeCameraShakeActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4CameraShakeAction& InAction,
		uint32 InActionArrayIndex
	); // #101
	void CustomizePostProcessActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4PostProcessAction& InAction,
		uint32 InActionArrayIndex
	); // #100
	void CustomizeEnvironmentActionDetails(
		IDetailLayoutBuilder& InBuilder,
		TSharedPtr<IPropertyHandle> InHandle,
		const FT4EnvironmentAction& InAction,
		uint32 InActionArrayIndex
	); // #99

private:
	void HandleOnSpawnPreviewEntity(); // #67
	void HandleOnTotalPlayTimeSecChanged(); // #100

	void HandleOnAutomationPointOfInterestSelected(int32 InSelectedIndex); // #100
	void HandleOnAutomationPointOfInterestGo(int32 InSelectedIndex); // #100
	FReply HandleOnAutomationPointOfInterestUpdate(); // #100
	FReply HandleOnAutomationPointOfInterestAdd(); // #100
	FReply HandleOnAutomationPointOfInterestRemove(); // #100

	void HandleOnRefreshActionTrack(int32 InActionHeaderKey); // #100
	FReply HandleOnRefreshActionTrackButton(int32 InActionHeaderKey); // #100

	void HandleOnMapEntitySelected(const FName InName); // #87
	void HandleOnStanceSelected(const FName InName); // #73

	FReply HandleOnActionPlay(int32 InActionHeaderKey); // #100
	FReply HandleOnActionPlayStop(int32 InActionHeaderKey); // #100
	FReply HandleOnActionReset(int32 InActionHeaderKey); // #100

	void AddCustomActionPointProperty(
		IDetailGroup& InActionDetailGroup,
		TSharedPtr<IPropertyHandle> InHandle
	);

	IDetailCategoryBuilder& GetCategoryBuilder(
		IDetailLayoutBuilder& InBuilder,
		const FT4ContiActionStruct* InAction,
		uint32 InActionArrayIndex
	);

	TSharedPtr<SWidget> GetActionPlayHeaderWidget(
		int32 InActionHeaderKey,
		bool bShowResetButton
	); // #100

private:
	TSharedPtr<FT4ContiViewModel> ViewModelPtr;
	IDetailLayoutBuilder* DetailLayoutPtr;

	TSharedPtr<ST4EntityDropListWidget> MapEntityDropListWidgetPtr; // #87
	TSharedPtr<ST4StanceDropListWidget> StanceDropListWidgetPtr; // #73
	TSharedPtr<ST4PointOfInterestListWidget> PointOfInterestListWidgetPtr; // #100

	TArray<TSharedPtr<ST4CameraWorkSectionKeyListWidget>> CameraWorkSectionKeyWidgets; // #58
};

// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionTypes.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "Input/Reply.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */
class USkeleton;
class UT4EntityAsset;
class UT4CharacterEntityAsset;
class FT4EntityViewModel;
struct FT4EntityOverrideMaterialData;
class ST4OverrideMaterialListWidget; // #80
class ST4CompositePartListWidget;
class ST4CompositePartDropListWidget;
class ST4ReactionListWidget; // #76
class ST4ReactionDropListWidget; // #76
class ST4ActionPointDropListWidget; // #76
class ST4AnimSectionDropListWidget; // #76
class ST4StanceListWidget; // #73
class ST4StanceDropListWidget; // #73
class ST4LayerTagDropListWidget;
class ST4EquipPointDropListWidget;
class ST4EntityLayerTagListWidget; // #74
class ST4PointOfInterestListWidget; // #100, #103
class FT4EnvironmentDetailCustomization; // #94
class FT4EntityDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(TSharedPtr<FT4EntityViewModel> InEntityViewModel);
	
	FT4EntityDetailCustomization(TSharedPtr<FT4EntityViewModel> InEntityViewModel);

	void CustomizeDetails(IDetailLayoutBuilder& InBuilder);

private:
	void HandleOnRefreshLayout();

	bool HandleFilterCostumeEntityAsset(const FAssetData& InAssetData, USkeleton* InSkeleton) const; // #71, #72
	bool HandleFilterSkeletalMeshAsset(const FAssetData& InAssetData, USkeleton* InSkeleton) const; // #71
	bool HandleFilterAnimSetAsset(const FAssetData& InAssetData, USkeleton* InSkeleton) const; // #73

	// Common Entity

	void CustomizeEntityLayerTagMaterialDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); // #74, #81
	void CustomizeEntityLayerTagWeaponDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); // #74
	void CustomizeEntityLayerTagContiDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); // #74


	// Map Entity
	void CustomizeMapEntityDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); // #79


	// Zone Entity
	void CustomizeZoneEntityDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); // #94


	// Item Entity

	void CustomizeItemEntityOverrideMaterialDetails(
		IDetailLayoutBuilder& InBuilder,
		IDetailCategoryBuilder& InCategoryBuilder,
		const FT4EntityOverrideMaterialData* InOverrideMaterialData
	); // #80
	void CustomizeItemEntityDropMeshOverrideMaterialDetails(
		IDetailLayoutBuilder& InBuilder,
		IDetailCategoryBuilder& InCategoryBuilder,
		const FT4EntityOverrideMaterialData* InOverrideMaterialData
	); // #80
	void CustomizeItemCommonEntityDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); // #80
	void CustomizeCostumeEntityDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); // #72
	void CustomizeWeaponEntityDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); // #80


	// Character Entity

	// #71
	void CustomizeCharacterEntityDetails(IDetailLayoutBuilder& InBuilder, UT4EntityAsset* InEntityAsset); 
	void CustomizeCharacterFullbodyMeshDetails(IDetailLayoutBuilder& InBuilder, UT4CharacterEntityAsset* InCharacterEntityAsset);
	void CustomizeCharacterCompositeMeshDetails(IDetailLayoutBuilder& InBuilder, UT4CharacterEntityAsset* InCharacterEntityAsset); 
	// ~#71

	void CustomizeCharacterStanceSetDetails(IDetailLayoutBuilder& InBuilder, UT4CharacterEntityAsset* InCharacterEntityAsset); // #73
	void CustomizeCharacterReactionDetails(IDetailLayoutBuilder& InBuilder, UT4CharacterEntityAsset* InCharacterEntityAsset); // #76


	// Common Entity

	void HandleOnAutomationPointOfInterestSelected(int32 InSelectedIndex); // #100, #103
	void HandleOnAutomationPointOfInterestGo(int32 InSelectedIndex); // #100, #103
	FReply HandleOnAutomationPointOfInterestUpdate(); // #100, #103
	FReply HandleOnAutomationPointOfInterestAdd(); // #100, #103
	FReply HandleOnAutomationPointOfInterestRemove(); // #100, #103

	void HandleOnEntityLayerTagSelected(ET4LayerTagType InLayerTagType, int32 InSelectIndex); // #74, // #81
	void HandleOnEntityLayerTagDoubleClicked(ET4LayerTagType InLayerTagType, int32 InSelectIndex); // #74, // #81

	void HandleOnEntityLayerTagUpdateWeapon(); // #104
	FReply HandleOnEntityLayerTagAddWeapon(); // #74
	FReply HandleOnEntityLayerTagRemoveWeapon(); // #74
	FReply HandleOnEntityLayerTagPlayWeaponByName(); // #81
	FReply HandleOnEntityLayerTagStopWeaponByName(); // #81

	void HandleOnEntityLayerTagUpdateConti(); // #104
	FReply HandleOnEntityLayerTagAddConti(); // #74
	FReply HandleOnEntityLayerTagRemoveConti(); // #74
	FReply HandleOnEntityLayerTagPlayContiByName(); // #81
	FReply HandleOnEntityLayerTagStopContiByName(); // #81

	FReply HandleOnEntityLayerTagGetMateralSlots(); // #81
	FReply HandleOnEntityLayerTagClearMateralSlots(); // #81
	void HandleOnEntityLayerTagMaterialSlotSelected(const FName InName); // #81
	FReply HandleOnEntityLayerTagUpdateMaterialBySlot(); // #81

	FReply HandleOnEntityLayerTagAddMaterial(); // #81
	FReply HandleOnEntityLayerTagRemoveMaterial(); // #81
	FReply HandleOnEntityLayerTagPlayMaterialByName(); // #81
	FReply HandleOnEntityLayerTagRestoreMaterialByName(); // #81


	// Map Entity
	void HandleOnMapChange(); // #79


	// Zone Entity
	void HandleOnZoneEnvironmentChange(); // #94
	FReply HandleOnZoneEnvironmentAssetSave(); // #94
	ECheckBoxState HandleOnGetTimeStopEnabledCheckState() const; // #94
	void HandleOnTimeStopEnabledCheckStateChanged(ECheckBoxState InCheckState); // #94


	// Character Entity

	FReply HandleOnCharacterFullbodyGetMateralSlots(); // #80
	FReply HandleOnCharacterFullbodyClearMateralSlots(); // #80
	void HandleOnCharacterFullbodyOverrideMaterialSelected(const FName InName); // #80
	FReply HandleOnCharacterFullbodyUpdateOverrideMaterial(); // #80

	void HandleOnCharacterCompositePartSelected(const FName InName); // #71
	void HandleOnCharacterCompositePartAssetChanged(const FAssetData& InAssetData); // #95
	FReply HandleOnCharacterAddCompositePart(); // #71
	FReply HandleOnCharacterRemoveSelectedCompositePart(); // #71
	void HandleOnCharacterFullbodyMesh(); // #71, #74

	void HandleOnCharacterStanceSelected(const FName InName); // #73
	void HandleOnCharacterStanceChanged(); // #95, #104
	FReply HandleOnCharacterAddSelectedStance(); // #73
	FReply HandleOnCharacterRemoveSelectedStance(); // #73

	void HandleOnCharacterReactionSelected(const FName InName); // #76
	void HandleOnCharacterReactionDoubleClicked(const FName InName); // #76
	void HandleOnCharacterUpdateSelectedReaction(); // #95
	FReply HandleOnCharacterAddReaction(); // #76
	FReply HandleOnCharacterRemoveSelectedReaction(); // #76
	FReply HandleOnCharacterPlayReaction(); // #76
	FReply HandleOnCharacterResotreReaction(); // #76
	

	// Item Common Entity

	FReply HandleOnDropMeshGetMateralSlots(); // #80
	FReply HandleOnDropMeshClearMateralSlots(); // #80
	void HandleOnDropMeshOverrideMaterialSelected(const FName InName); // #80
	FReply HandleOnDropMeshUpdateOverrideMaterial(); // #80

	// Weapon Entity

	FReply HandleOnWeaponGetMateralSlots(); // #80
	FReply HandleOnWeaponClearMateralSlots(); // #80
	void HandleOnWeaponOverrideMaterialSelected(const FName InName); // #80
	FReply HandleOnWeaponUpdateOverrideMaterial(); // #80

	// Costume Entity

	FReply HandleOnCostumeGetMateralSlots(); // #80
	FReply HandleOnCostumeClearMateralSlots(); // #80
	void HandleOnCostumeOverrideMaterialSelected(const FName InName); // #80
	FReply HandleOnCostumeUpdateOverrideMaterial(); // #80

private:
	TSharedPtr<FT4EntityViewModel> ViewModelPtr;
	IDetailLayoutBuilder* DetailLayoutPtr;

	// Common Widget

	TSharedPtr<ST4PointOfInterestListWidget> PointOfInterestListWidgetPtr; // #100, #103

	TSharedPtr<ST4CompositePartDropListWidget> CompositePartDropListWidget; // #72, #73

	TSharedPtr<ST4OverrideMaterialListWidget> OverrideMaterialListWidget; // #80
	TSharedPtr<ST4OverrideMaterialListWidget> OverrideDropMeshMaterialListWidget; // #80
	TSharedPtr<ST4OverrideMaterialListWidget> EntityLayerOverrideMaterialListWidget; // #81

	TSharedPtr<ST4LayerTagDropListWidget> EntityLayerTagWeaponDropListWidget;
	TSharedPtr<ST4EquipPointDropListWidget> EntityLayerTagWeaponEquipPointDropListWidget;
	TSharedPtr<ST4LayerTagDropListWidget> EntityLayerTagContiDropListWidget;
	TSharedPtr<ST4LayerTagDropListWidget> EntityLayerTagMaterialDropListWidget; // #81

	TSharedPtr<ST4EntityLayerTagListWidget> EntityLayerTagMaterialListWidget; // #81
	TSharedPtr<ST4EntityLayerTagListWidget> EntityLayerTagWeaponListWidget; // #74
	TSharedPtr<ST4EntityLayerTagListWidget> EntityLayerTagContiListWidget; // #74


	// Character Widget

	TSharedPtr<ST4CompositePartListWidget> CharacterCompositePartListWidget; // #72
	
	TSharedPtr<ST4StanceListWidget> CharacterStanceListWidget; // #73
	TSharedPtr<ST4StanceDropListWidget> CharacterStanceDropListWidget; // #73
	TSharedPtr<ST4LayerTagDropListWidget> CharacterStanceActiveLayerTagDropListWidget; // #73, #74

	TSharedPtr<ST4ReactionListWidget> CharacterReactionListWidget; // #76
	TSharedPtr<ST4ReactionDropListWidget> CharacterReactionDropListWidget; // #76
	TSharedPtr<ST4ActionPointDropListWidget> CharacterReactionImpulseMainActionPointDropListWidget; // #76
	TSharedPtr<ST4ActionPointDropListWidget> CharacterReactionImpulseSubActionPointDropListWidget; // #76
	TSharedPtr<ST4AnimSectionDropListWidget> CharacterReactionStartAnimSectionDropListWidget; // #76
	TSharedPtr<ST4AnimSectionDropListWidget> CharacterReactionLoopAnimSectionDropListWidget; // #76


	// Zone Widget

	TSharedPtr<FT4EnvironmentDetailCustomization> EnvironmentDetailPtr; // #94


	// Common Handle

	TSharedPtr<IPropertyHandle> EditorTransientEntityDataHandle; // #81
	TSharedPtr<IPropertyHandle> EditorTransientEntityItemDataHandle; // #80
	TSharedPtr<IPropertyHandle> EditorTransientEntitySelectDataHandle;

	TSharedPtr<IPropertyHandle> TransientFullbodyOverrideMaterialAssetHandlePtr; // #80
	TSharedPtr<IPropertyHandle> TransientItemOverrideMaterialAssetHandlePtr; // #80
	TSharedPtr<IPropertyHandle> TransientDropMeshOverrideMaterialAssetHandlePtr; // #80

	TSharedPtr<IPropertyHandle> TransientCompositePartNameHandlePtr;
	TSharedPtr<IPropertyHandle> TransientCompositePartAssetHandlePtr;

	TSharedPtr<IPropertyHandle> TransientStanceNameHandlePtr; // #73
	TSharedPtr<IPropertyHandle> TransientStanceAssetHandlePtr; // #73
	TSharedPtr<IPropertyHandle> TransientStanceActiveLayerTagHandlePtr; // #73, #74

	TSharedPtr<IPropertyHandle> TransientLayerTagWeaponAssetHandlePtr; // #74
	TSharedPtr<IPropertyHandle> TransientLayerTagContiAssetHandlePtr; // #74
};

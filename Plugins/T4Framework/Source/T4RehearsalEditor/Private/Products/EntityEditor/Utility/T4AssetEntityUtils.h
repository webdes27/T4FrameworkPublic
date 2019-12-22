// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Asset/Public/Action/T4ActionTypes.h"
#include "T4Asset/Public/T4AssetUtils.h"

/**
  * #39
 */
#if WITH_EDITOR

class UMaterialInterface;
struct FT4EntityCharacterStanceData;
struct FT4EntityCharacterReactionData; // #76
class UT4ContiAsset;
class UT4EntityAsset;
class UT4ItemEntityAsset;
class UT4WeaponEntityAsset;
class UT4CostumeEntityAsset;
class UT4CharacterEntityAsset;

namespace T4AssetUtil
{
	bool EntitySaveThumbnailCameraInfo(
		UT4EntityAsset* InEntityAsset,
		const FRotator& ThumbnailRotation,
		const FVector& ThumbnailLocation
	);
	
	// #74, #81
	void EntityLayerTagSelectionByIndex(
		UT4EntityAsset* InOutEntityAsset,
		ET4LayerTagType InLayerTagType,
		const int32 InSelectIndex
	);
	bool EntityLayerTagUpdateWeaponData(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InEquipPointName,
		const TSoftObjectPtr<UT4WeaponEntityAsset>& InWeaponEntityAsset,
		FString& OutErrorMessage
	); // #74, #104 : add 와 update 분리
	bool EntityLayerTagAddWeaponData(
		UT4EntityAsset* InOutEntityAsset,
		const FName& InLayerTagName,
		const FName& InEquipPointName,
		const TSoftObjectPtr<UT4WeaponEntityAsset>& InWeaponEntityAsset,
		FString& OutErrorMessage
	);
	bool EntityLayerTagRemoveWeaponDataByIndex(
		UT4EntityAsset* InOutEntityAsset,
		int32 InRemoveArrayIndex,
		FString& OutErrorMessage
	);

	bool EntityLayerTagUpdateContiData(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const TSoftObjectPtr<UT4ContiAsset>& InContiAsset,
		FString& OutErrorMessage
	); // #74, #104 : add 와 update 분리
	bool EntityLayerTagAddContiData(
		UT4EntityAsset* InOutEntityAsset,
		const FName& InLayerTagName,
		const TSoftObjectPtr<UT4ContiAsset>& InContiAsset,
		FString& OutErrorMessage
	); // #74
	bool EntityLayerTagRemoveContiDataByIndex(
		UT4EntityAsset* InOutEntityAsset,
		int32 InRemoveArrayIndex,
		FString& OutErrorMessage
	);
	// ~#74, #81

	// #81
	bool EntityLayerTagAddOrUpdateMaterialData(
		UT4EntityAsset* InOutEntityAsset,
		const FName& InLayerTagName,
		FString& OutErrorMessage
	);
	bool EntityLayerTagRemoveMaterialDataByIndex(
		UT4EntityAsset* InOutEntityAsset,
		int32 InRemoveArrayIndex,
		FString& OutErrorMessage
	);

	bool EntityLayerTagGetMeterialSlots(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InSlotName,
		FString& OutErrorMessage
	);
	bool EntityLayerTagClearMeterialSlots(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InSlotName,
		FString& OutErrorMessage
	);
	void EntityLayerTagSelectMaterialBySlotName(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InSlotName
	);
	bool EntityLayerTagUpdatMaterialBySlotName(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	);
	// ~#81

	// #80
	bool EntityCharacterGetFullbodyMeterialSlots(
		UT4CharacterEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	);
	bool EntityCharacterClearFullbodyMeterialSlots(
		UT4CharacterEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	);
	void EntityCharacterSelectFullbodyOverrideMaterialBySlotName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InSlotName
	);
	bool EntityCharacterUpdateFullbodyOverrideMaterialBySlotName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	);

	bool EntityItemGetOverrideMeterialSlots(
		UT4ItemEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	);
	bool EntityItemClearOverrideMeterialSlots(
		UT4ItemEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	);
	void EntityItemSelectOverrideMaterialBySlotName(
		UT4ItemEntityAsset* InOutEntityAsset,
		const FName& InSlotName
	);
	bool EntityItemUpdateOverrideMaterialBySlotName(
		UT4ItemEntityAsset* InOutEntityAsset,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	);

	bool EntityWeaponGetOverrideMeterialSlots(
		UT4WeaponEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	);
	bool EntityWeaponClearOverrideMeterialSlots(
		UT4WeaponEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	);
	void EntityWeaponSelectOverrideMaterialBySlotName(
		UT4WeaponEntityAsset* InOutEntityAsset,
		const FName& InSlotName
	);
	bool EntityWeaponUpdateOverrideMaterialBySlotName(
		UT4WeaponEntityAsset* InOutEntityAsset,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	);

	bool EntityCostumeGetOverrideMeterialSlots(
		UT4CostumeEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	);
	bool EntityCostumeClearOverrideMeterialSlots(
		UT4CostumeEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	);
	void EntityCostumeSelectOverrideMaterialBySlotName(
		UT4CostumeEntityAsset* InOutEntityAsset,
		const FName& InSlotName
	);
	bool EntityCostumeUpdateOverrideMaterialBySlotName(
		UT4CostumeEntityAsset* InOutEntityAsset,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	);
	// ~#80

	// #71
	void EntityCharacterSelectCompositePartByPartName(
		UT4CharacterEntityAsset* InOutEntityAsset, 
		const FName& InPartName
	);
	bool EntityCharacterUpdateCompositePartByPartName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InPartName,
		const TSoftObjectPtr<UT4CostumeEntityAsset>& InCostumeEntityAsset,
		FString& OutErrorMessage
	); // #95
	bool EntityCharacterAddCompositePartByPartName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InPartName,
		const TSoftObjectPtr<UT4CostumeEntityAsset>& InCostumeEntityAsset,
		FString& OutErrorMessage
	); 
	bool EntityCharacterRemoveCompositePartByPartName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InPartName,
		FString& OutErrorMessage
	); 
	// ~#71

	// #73
	void EntityCharacterSelectStanceDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InStanceName
	);
	bool EntityCharacterUpdateStanceData(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InStanceName,
		const FT4EntityCharacterStanceData* InStanceData,
		FString& OutErrorMessage
	); // #95
	bool EntityCharacterAddStanceData(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InStanceName,
		const FT4EntityCharacterStanceData* InStanceData,
		FString& OutErrorMessage
	);
	bool EntityCharacterRemoveStanceDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InStanceName,
		FString& OutErrorMessage
	);
	// ~#73

	// #76
	void EntityCharacterSelectReactionDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InReactionName
	);
	bool EntityCharacterUpdateReactionDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InReactionName,
		const FT4EntityCharacterReactionData* InReactionData,
		FString& OutErrorMessage
	);
	bool EntityCharacterAddReactionData(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InReactionName,
		const FT4EntityCharacterReactionData* InReactionData,
		FString& OutErrorMessage
	);
	bool EntityCharacterRemoveReactionDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InReactionName,
		FString& OutErrorMessage
	);
	// ~#76

}

#endif

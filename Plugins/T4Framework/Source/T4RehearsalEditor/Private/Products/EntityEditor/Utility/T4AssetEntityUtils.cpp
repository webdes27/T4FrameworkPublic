// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetEntityUtils.h"

#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h"
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"

#include "Engine/StaticMesh.h" // #80
#include "Engine/SkeletalMesh.h" // #80

#include "T4RehearsalEditorInternal.h"

/**
  * #71
 */
#if WITH_EDITOR
namespace T4AssetUtil
{

	bool EntitySaveThumbnailCameraInfo(
		UT4EntityAsset* InEntityAsset,
		const FRotator& ThumbnailRotation,
		const FVector& ThumbnailLocation
	)
	{
		if (nullptr == InEntityAsset)
		{
			return false;
		}
		InEntityAsset->MarkPackageDirty();
		InEntityAsset->ThumbnailCameraInfo.Location = ThumbnailLocation;
		InEntityAsset->ThumbnailCameraInfo.Rotation = ThumbnailRotation;
		return true;
	}

	bool GetOverrideMeterialSlotsInStaticMesh(
		UStaticMesh* InStaticMesh,
		FT4EntityOverrideMaterialData& OutOverrideMaterialData
	) // #80
	{
		OutOverrideMaterialData.MaterialMap.Empty(); // #80 : TODO : 같은 Slot 은 삭제하지 않도록 처리 필요
		OutOverrideMaterialData.MaterialSortedSlotNames.Empty();
		if (nullptr == InStaticMesh)
		{
			return true;
		}
		for (FStaticMaterial& StaticMateral : InStaticMesh->StaticMaterials)
		{
			if (!OutOverrideMaterialData.MaterialMap.Contains(StaticMateral.MaterialSlotName))
			{
				OutOverrideMaterialData.MaterialMap.Add(StaticMateral.MaterialSlotName, nullptr);
			}
			OutOverrideMaterialData.MaterialSortedSlotNames.Add(StaticMateral.MaterialSlotName);
		}
		return true;
	}

	bool GetOverrideMeterialSlotsInSkeletalMesh(
		USkeletalMesh* InSkeletalMesh,
		FT4EntityOverrideMaterialData& OutOverrideMaterialData
	) // #80
	{
		OutOverrideMaterialData.MaterialMap.Empty(); // #80 : TODO : 같은 Slot 은 삭제하지 않도록 처리 필요
		OutOverrideMaterialData.MaterialSortedSlotNames.Empty();
		if (nullptr == InSkeletalMesh)
		{
			return true;
		}
		for (FSkeletalMaterial& SkeletalMateral : InSkeletalMesh->Materials)
		{
			if (!OutOverrideMaterialData.MaterialMap.Contains(SkeletalMateral.MaterialSlotName))
			{
				OutOverrideMaterialData.MaterialMap.Add(SkeletalMateral.MaterialSlotName, nullptr);
			}
			OutOverrideMaterialData.MaterialSortedSlotNames.Add(SkeletalMateral.MaterialSlotName);
		}
		return true;
	}

	void EntityLayerTagSelectionByIndex(
		UT4EntityAsset* InOutEntityAsset,
		ET4LayerTagType InLayerTagType,
		const int32 InSelectIndex
	) // #74
	{
		check(nullptr != InOutEntityAsset);

		FT4ScopedTransientTransaction TransientTransaction(InOutEntityAsset); // #88

		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		switch (InLayerTagType)
		{
			case ET4LayerTagType::Weapon: // #74
				{
					if (InSelectIndex >= LayerTagData.WeaponTags.Num())
					{
						return;
					}
					const FT4EntityLayerTagWeaponData& WeaponData = LayerTagData.WeaponTags[InSelectIndex];
					FT4EntityEditorTransientData& TransientData = InOutEntityAsset->EditorTransientData;
					if (TransientData.TransientLayerTagNameOfWeapon != WeaponData.LayerTag)
					{
						TransientData.TransientLayerTagNameOfWeapon = WeaponData.LayerTag;
					}
					if (TransientData.TransientLayerTagWeaponEquipPoint != WeaponData.EquipPoint)
					{
						TransientData.TransientLayerTagWeaponEquipPoint = WeaponData.EquipPoint;
					}
					if (TransientData.TransientLayerTagWeaponAsset != WeaponData.WeaponEntityAsset)
					{
						TransientData.TransientLayerTagWeaponAsset = WeaponData.WeaponEntityAsset;
					}
				}
				break;

			case ET4LayerTagType::Conti: // #74
				{
					if (InSelectIndex >= LayerTagData.ContiTags.Num())
					{
						return;
					}
					const FT4EntityLayerTagContiData& ContiData = LayerTagData.ContiTags[InSelectIndex];
					FT4EntityEditorTransientData& TransientData = InOutEntityAsset->EditorTransientData;
					TransientData.TransientLayerTagNameOfConti = ContiData.LayerTag;
					TransientData.TransientLayerTagContiAsset = ContiData.ContiAsset;
					if (TransientData.TransientLayerTagNameOfConti != ContiData.LayerTag)
					{
						TransientData.TransientLayerTagNameOfConti = ContiData.LayerTag;
					}
					if (TransientData.TransientLayerTagContiAsset != ContiData.ContiAsset)
					{
						TransientData.TransientLayerTagContiAsset = ContiData.ContiAsset;
					}
				}
				break;

			case ET4LayerTagType::Material: // #81
				{
					if (InSelectIndex >= LayerTagData.MaterialTags.Num())
					{
						return;
					}
					const FT4EntityLayerTagMaterialData& EntityTagOverrideMaterialData = LayerTagData.MaterialTags[InSelectIndex];
					FT4EntityEditorTransientData& TransientData = InOutEntityAsset->EditorTransientData;
					if (TransientData.TransientLayerTagNameOfMaterial != EntityTagOverrideMaterialData.LayerTag)
					{
						TransientData.TransientLayerTagNameOfMaterial = EntityTagOverrideMaterialData.LayerTag;
					}
					const FT4EntityOverrideMaterialData& OverrideMaterialData = EntityTagOverrideMaterialData.OverrideMaterialData;
					TransientData.TransientLayerTagMaterialData.MaterialMap.Empty();
					TransientData.TransientLayerTagMaterialData.MaterialSortedSlotNames.Empty();
					TransientData.TransientLayerTagMaterialData = OverrideMaterialData;
					if (0 >= OverrideMaterialData.MaterialSortedSlotNames.Num())
					{
						TransientData.TransientLayerTagMaterialSlotName = NAME_None;
						TransientData.TransientLayerTagMaterialAsset.Reset();
					}
					else
					{
						FName SlotName = OverrideMaterialData.MaterialSortedSlotNames[0];
						TSoftObjectPtr<UMaterialInterface> MaterialInstance = OverrideMaterialData.MaterialMap[SlotName];
						if (TransientData.TransientLayerTagMaterialSlotName != SlotName)
						{
							TransientData.TransientLayerTagMaterialSlotName = SlotName;
						}
						if (TransientData.TransientLayerTagMaterialAsset != MaterialInstance)
						{
							TransientData.TransientLayerTagMaterialAsset = MaterialInstance;
						}
					}
				}
				break;

			default:
				{
					UE_LOG(
						LogT4RehearsalEditor,
						Error,
						TEXT("EntityLayerTagSelectionByIndex : Unknown LayerTag type '%u'"),
						uint8(InLayerTagType)
					);
				}
				break;
		}
	}

	bool EntityLayerTagUpdateWeaponData(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InEquipPointName,
		const TSoftObjectPtr<UT4WeaponEntityAsset>& InWeaponEntityAsset,
		FString& OutErrorMessage
	) // #74, #104 : add 와 update 분리
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (-1 == InSelectIndex || InSelectIndex >= LayerTagData.WeaponTags.Num())
		{
			OutErrorMessage = TEXT("Not Selection Index");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityLayerTagWeaponData& WeaponData = LayerTagData.WeaponTags[InSelectIndex];
		WeaponData.LayerTag = InLayerTagName;
		WeaponData.EquipPoint = InEquipPointName;
		WeaponData.WeaponEntityAsset = InWeaponEntityAsset;
		return true;
	}

	bool EntityLayerTagAddWeaponData(
		UT4EntityAsset* InOutEntityAsset,
		const FName& InLayerTagName,
		const FName& InEquipPointName,
		const TSoftObjectPtr<UT4WeaponEntityAsset>& InWeaponEntityAsset,
		FString& OutErrorMessage
	) // #74
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		FT4EntityLayerTagWeaponData& NewWeaponData = LayerTagData.WeaponTags.AddDefaulted_GetRef();
		NewWeaponData.LayerTag = InLayerTagName;
		NewWeaponData.EquipPoint = InEquipPointName;
		NewWeaponData.WeaponEntityAsset = InWeaponEntityAsset;
		return true;
	}

	bool EntityLayerTagRemoveWeaponDataByIndex(
		UT4EntityAsset* InOutEntityAsset,
		int32 InRemoveArrayIndex,
		FString& OutErrorMessage
	) // #74
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (0 <= InRemoveArrayIndex && InRemoveArrayIndex < LayerTagData.WeaponTags.Num())
		{
			LayerTagData.WeaponTags.RemoveAt(InRemoveArrayIndex);
		}
		return true;
	}

	bool EntityLayerTagUpdateContiData(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const TSoftObjectPtr<UT4ContiAsset>& InContiAsset,
		FString& OutErrorMessage
	) // #74, #104 : add 와 update 분리
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (-1 == InSelectIndex || InSelectIndex >= LayerTagData.ContiTags.Num())
		{
			OutErrorMessage = TEXT("Not Selection Index");
			return false;
		}
		FT4EntityLayerTagContiData& ContiData = LayerTagData.ContiTags[InSelectIndex];
		InOutEntityAsset->MarkPackageDirty();
		ContiData.LayerTag = InLayerTagName;
		ContiData.ContiAsset = InContiAsset;
		return true;
	}

	bool EntityLayerTagAddContiData(
		UT4EntityAsset* InOutEntityAsset,
		const FName& InLayerTagName,
		const TSoftObjectPtr<UT4ContiAsset>& InContiAsset,
		FString& OutErrorMessage
	) // #74
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		FT4EntityLayerTagContiData& NewContiData = LayerTagData.ContiTags.AddDefaulted_GetRef();
		NewContiData.LayerTag = InLayerTagName;
		NewContiData.ContiAsset = InContiAsset;
		return true;
	}

	bool EntityLayerTagRemoveContiDataByIndex(
		UT4EntityAsset* InOutEntityAsset,
		int32 InRemoveArrayIndex,
		FString& OutErrorMessage
	) // #74
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (0 <= InRemoveArrayIndex && InRemoveArrayIndex < LayerTagData.ContiTags.Num())
		{
			LayerTagData.ContiTags.RemoveAt(InRemoveArrayIndex);
		}
		return true;
	}

	bool EntityLayerTagAddOrUpdateMaterialData(
		UT4EntityAsset* InOutEntityAsset,
		const FName& InLayerTagName,
		FString& OutErrorMessage
	) // #81
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		FT4EntityLayerTagMaterialData& NewOverrideMaterialData = LayerTagData.MaterialTags.AddDefaulted_GetRef();
		NewOverrideMaterialData.LayerTag = InLayerTagName;
		FT4EntityEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientData;
		EditorTransientData.TransientLayerTagNameOfMaterial = InLayerTagName;
		EditorTransientData.TransientLayerTagMaterialSlotName = NAME_None;
		EditorTransientData.TransientLayerTagMaterialData.MaterialMap.Empty();
		EditorTransientData.TransientLayerTagMaterialData.MaterialSortedSlotNames.Empty();
		EditorTransientData.TransientLayerTagMaterialAsset.Reset();
		return true;
	}

	bool EntityLayerTagRemoveMaterialDataByIndex(
		UT4EntityAsset* InOutEntityAsset,
		int32 InRemoveArrayIndex,
		FString& OutErrorMessage
	) // #74
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (0 <= InRemoveArrayIndex && InRemoveArrayIndex < LayerTagData.MaterialTags.Num())
		{
			LayerTagData.MaterialTags.RemoveAt(InRemoveArrayIndex);
		}
		FT4EntityEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientData;
		EditorTransientData.TransientLayerTagNameOfMaterial = NAME_None;
		EditorTransientData.TransientLayerTagMaterialData.MaterialMap.Empty();
		EditorTransientData.TransientLayerTagMaterialData.MaterialSortedSlotNames.Empty();
		EditorTransientData.TransientLayerTagMaterialSlotName = NAME_None;
		EditorTransientData.TransientLayerTagMaterialAsset.Reset();
		return true;
	}

	bool EntityLayerTagGetMeterialSlots(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InSlotName,
		FString& OutErrorMessage
	) // #81
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (-1 == InSelectIndex || InSelectIndex >= LayerTagData.MaterialTags.Num())
		{
			return false;
		}
		FT4EntityLayerTagMaterialData& EntityTagOverrideMaterialData = LayerTagData.MaterialTags[InSelectIndex];
		if (EntityTagOverrideMaterialData.LayerTag != InLayerTagName)
		{
			OutErrorMessage = TEXT("Miss match LayerTag");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityOverrideMaterialData& OverrideMaterialData = EntityTagOverrideMaterialData.OverrideMaterialData;
		OverrideMaterialData.MaterialMap.Empty();
		OverrideMaterialData.MaterialSortedSlotNames.Empty();
		USkeletalMesh* SkeltalMesh = InOutEntityAsset->GetPrimarySkeletalMeshAsset();
		if (nullptr != SkeltalMesh)
		{
			bool bResult = GetOverrideMeterialSlotsInSkeletalMesh(
				SkeltalMesh,
				OverrideMaterialData
			);
			if (bResult)
			{
				FT4EntityEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientData;
				EditorTransientData.TransientLayerTagMaterialData.MaterialMap.Empty();
				EditorTransientData.TransientLayerTagMaterialData.MaterialSortedSlotNames.Empty();
				EditorTransientData.TransientLayerTagMaterialData = OverrideMaterialData;
				return true;
			}
		}
		UStaticMesh* StaticMesh = InOutEntityAsset->GetPrimaryStaticMeshAsset();
		if (nullptr != StaticMesh)
		{
			bool bResult = GetOverrideMeterialSlotsInStaticMesh(
				StaticMesh,
				OverrideMaterialData
			);
			if (bResult)
			{
				FT4EntityEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientData;
				EditorTransientData.TransientLayerTagMaterialData.MaterialMap.Empty();
				EditorTransientData.TransientLayerTagMaterialData.MaterialSortedSlotNames.Empty();
				EditorTransientData.TransientLayerTagMaterialData = OverrideMaterialData;
				return true;
			}
		}
		return false;
	}

	bool EntityLayerTagClearMeterialSlots(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InSlotName,
		FString& OutErrorMessage
	) // #81
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (-1 == InSelectIndex || InSelectIndex >= LayerTagData.MaterialTags.Num())
		{
			return false;
		}
		FT4EntityLayerTagMaterialData& EntityTagOverrideMaterialData = LayerTagData.MaterialTags[InSelectIndex];
		if (EntityTagOverrideMaterialData.LayerTag != InLayerTagName)
		{
			OutErrorMessage = TEXT("Miss match LayerTag");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityOverrideMaterialData& OverrideMaterialData = EntityTagOverrideMaterialData.OverrideMaterialData;
		OverrideMaterialData.MaterialMap.Empty();
		OverrideMaterialData.MaterialSortedSlotNames.Empty();
		FT4EntityEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientData;
		EditorTransientData.TransientLayerTagMaterialData.MaterialMap.Empty();
		EditorTransientData.TransientLayerTagMaterialData.MaterialSortedSlotNames.Empty();
		return true;
	}

	void EntityLayerTagSelectMaterialBySlotName(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InSlotName
	)
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (-1 == InSelectIndex || InSelectIndex >= LayerTagData.MaterialTags.Num())
		{
			return;
		}
		FT4EntityLayerTagMaterialData& EntityTagOverrideMaterialData = LayerTagData.MaterialTags[InSelectIndex];
		if (EntityTagOverrideMaterialData.LayerTag != InLayerTagName)
		{
			return;
		}
		const FT4EntityOverrideMaterialData& OverrideMaterialData = EntityTagOverrideMaterialData.OverrideMaterialData;
		if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
		{
			return;
		}
		FT4EntityEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientData;
		EditorTransientData.TransientLayerTagMaterialSlotName = InSlotName;
		EditorTransientData.TransientLayerTagMaterialAsset = OverrideMaterialData.MaterialMap[InSlotName];
	}

	bool EntityLayerTagUpdatMaterialBySlotName(
		UT4EntityAsset* InOutEntityAsset,
		const int32 InSelectIndex,
		const FName& InLayerTagName,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	)
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityLayerTagData& LayerTagData = InOutEntityAsset->LayerTagData;
		if (-1 == InSelectIndex || InSelectIndex >= LayerTagData.MaterialTags.Num())
		{
			return false;
		}
		FT4EntityLayerTagMaterialData& EntityTagOverrideMaterialData = LayerTagData.MaterialTags[InSelectIndex];
		if (EntityTagOverrideMaterialData.LayerTag != InLayerTagName)
		{
			return false;
		}
		FT4EntityOverrideMaterialData& OverrideMaterialData = EntityTagOverrideMaterialData.OverrideMaterialData;
		if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
		{
			return false;
		}
		OverrideMaterialData.MaterialMap[InSlotName] = InMaterialAsset;
		FT4EntityEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientData;
		if (EditorTransientData.TransientLayerTagMaterialData.MaterialMap.Contains(InSlotName))
		{
			EditorTransientData.TransientLayerTagMaterialData.MaterialMap[InSlotName] = InMaterialAsset;
		}
		return true;
	}

	bool EntityCharacterGetFullbodyMeterialSlots(
		UT4CharacterEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		FT4EntityCharacterFullBodyMeshData& FullBodyMeshData = InOutEntityAsset->FullBodyMeshData;
		FT4EntityOverrideMaterialData& OverrideMaterialData = FullBodyMeshData.OverrideMaterialData;
		bool bResult = GetOverrideMeterialSlotsInSkeletalMesh(
			FullBodyMeshData.SkeletalMeshAsset.LoadSynchronous(),
			OverrideMaterialData
		);
		return bResult;
	}

	bool EntityCharacterClearFullbodyMeterialSlots(
		UT4CharacterEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		FT4EntityCharacterFullBodyMeshData& FullBodyMeshData = InOutEntityAsset->FullBodyMeshData;
		FT4EntityOverrideMaterialData& OverrideMaterialData = FullBodyMeshData.OverrideMaterialData;
		bool bResult = GetOverrideMeterialSlotsInSkeletalMesh(
			nullptr,
			OverrideMaterialData
		);
		return bResult;
	}

	void EntityCharacterSelectFullbodyOverrideMaterialBySlotName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InSlotName
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		
		FT4ScopedTransientTransaction TransientTransaction(InOutEntityAsset); // #88

		FT4EntityCharacterFullBodyMeshData& FullBodyMeshData = InOutEntityAsset->FullBodyMeshData;
		FT4EntityOverrideMaterialData& OverrideMaterialData = FullBodyMeshData.OverrideMaterialData;
		if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
		{
			return;
		}
		TSoftObjectPtr<UMaterialInterface> MaterialAsset = OverrideMaterialData.MaterialMap[InSlotName];
		FT4EntityCharacterEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientCharacterData;
		if (EditorTransientData.TransientFullbodyOverrideMaterialSlotName != InSlotName)
		{
			EditorTransientData.TransientFullbodyOverrideMaterialSlotName = InSlotName;
		}
		if (EditorTransientData.TransientFullbodyOverrideMaterialAsset != MaterialAsset)
		{
			EditorTransientData.TransientFullbodyOverrideMaterialAsset = MaterialAsset;
		}
	}

	bool EntityCharacterUpdateFullbodyOverrideMaterialBySlotName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);

		FT4EntityCharacterFullBodyMeshData& FullBodyMeshData = InOutEntityAsset->FullBodyMeshData;
		FT4EntityOverrideMaterialData& OverrideMaterialData = FullBodyMeshData.OverrideMaterialData;
		if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
		{
			OutErrorMessage = TEXT("SlotName Not found!");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		OverrideMaterialData.MaterialMap[InSlotName] = InMaterialAsset;
		return true;
	}

	bool EntityItemGetOverrideMeterialSlots(
		UT4ItemEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		bool bResult = true;
		FT4EntityItemDropMeshData& MeshData = InOutEntityAsset->DropMeshData;
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			bResult = GetOverrideMeterialSlotsInStaticMesh(
				MeshData.StaticMeshAsset.LoadSynchronous(),
				MeshData.StaticMeshOverrideMaterialData
			);
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			bResult = GetOverrideMeterialSlotsInSkeletalMesh(
				MeshData.SkeletalMeshAsset.LoadSynchronous(),
				MeshData.SkeletalMeshOverrideMaterialData
			);
		}
		return bResult;
	}

	bool EntityItemClearOverrideMeterialSlots(
		UT4ItemEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		bool bResult = true;
		FT4EntityItemDropMeshData& MeshData = InOutEntityAsset->DropMeshData;
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			bResult = GetOverrideMeterialSlotsInStaticMesh(
				nullptr,
				MeshData.StaticMeshOverrideMaterialData
			);
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			bResult = GetOverrideMeterialSlotsInSkeletalMesh(
				nullptr,
				MeshData.SkeletalMeshOverrideMaterialData
			);
		}

		return bResult;
	}

	void EntityItemSelectOverrideMaterialBySlotName(
		UT4ItemEntityAsset* InOutEntityAsset,
		const FName& InSlotName
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		
		FT4ScopedTransientTransaction TransientTransaction(InOutEntityAsset); // #88

		FT4EntityItemDropMeshData& MeshData = InOutEntityAsset->DropMeshData;
		TSoftObjectPtr<UMaterialInterface> MaterialAssetSelected;
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.StaticMeshOverrideMaterialData;
			if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
			{
				return;
			}
			MaterialAssetSelected = OverrideMaterialData.MaterialMap[InSlotName];
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.SkeletalMeshOverrideMaterialData;
			if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
			{
				return;
			}
			MaterialAssetSelected = OverrideMaterialData.MaterialMap[InSlotName];
		}
		else
		{
			return;
		}
		FT4EntityItemEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientItemData;
		if (EditorTransientData.TransientDropMeshOverrideMaterialSlotName != InSlotName)
		{
			EditorTransientData.TransientDropMeshOverrideMaterialSlotName = InSlotName;
		}
		if (EditorTransientData.TransientDropMeshOverrideMaterialAsset != MaterialAssetSelected)
		{
			EditorTransientData.TransientDropMeshOverrideMaterialAsset = MaterialAssetSelected;
		}
	}

	bool EntityItemUpdateOverrideMaterialBySlotName(
		UT4ItemEntityAsset* InOutEntityAsset,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);

		FT4EntityItemDropMeshData& MeshData = InOutEntityAsset->DropMeshData;
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.StaticMeshOverrideMaterialData;
			if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
			{
				OutErrorMessage = TEXT("SlotName Not found!");
				return false;
			}
			InOutEntityAsset->MarkPackageDirty();
			OverrideMaterialData.MaterialMap[InSlotName] = InMaterialAsset;
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.SkeletalMeshOverrideMaterialData;
			if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
			{
				OutErrorMessage = TEXT("SlotName Not found!");
				return false;
			}
			InOutEntityAsset->MarkPackageDirty();
			OverrideMaterialData.MaterialMap[InSlotName] = InMaterialAsset;
		}
		else
		{
			OutErrorMessage = TEXT("Not supported!");
			return false;
		}
		return true;
	}


	bool EntityWeaponGetOverrideMeterialSlots(
		UT4WeaponEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		bool bResult = true;
		FT4EntityItemWeaponMeshData& MeshData = InOutEntityAsset->MeshData;
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			bResult = GetOverrideMeterialSlotsInStaticMesh(
				MeshData.StaticMeshAsset.LoadSynchronous(),
				MeshData.StaticMeshOverrideMaterialData
			);
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			bResult = GetOverrideMeterialSlotsInSkeletalMesh(
				MeshData.SkeletalMeshAsset.LoadSynchronous(),
				MeshData.SkeletalMeshOverrideMaterialData
			);
		}
		return bResult;
	}

	bool EntityWeaponClearOverrideMeterialSlots(
		UT4WeaponEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		bool bResult = true;
		FT4EntityItemWeaponMeshData& MeshData = InOutEntityAsset->MeshData;
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			bResult = GetOverrideMeterialSlotsInStaticMesh(
				nullptr,
				MeshData.StaticMeshOverrideMaterialData
			);
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			bResult = GetOverrideMeterialSlotsInSkeletalMesh(
				nullptr,
				MeshData.SkeletalMeshOverrideMaterialData
			);
		}

		return bResult;
	}

	void EntityWeaponSelectOverrideMaterialBySlotName(
		UT4WeaponEntityAsset* InOutEntityAsset,
		const FName& InSlotName
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		
		FT4ScopedTransientTransaction TransientTransaction(InOutEntityAsset); // #88

		FT4EntityItemWeaponMeshData& MeshData = InOutEntityAsset->MeshData;
		TSoftObjectPtr<UMaterialInterface> MaterialAssetSelected;
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.StaticMeshOverrideMaterialData;
			if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
			{
				return;
			}
			MaterialAssetSelected = OverrideMaterialData.MaterialMap[InSlotName];
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.SkeletalMeshOverrideMaterialData;
			if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
			{
				return;
			}
			MaterialAssetSelected = OverrideMaterialData.MaterialMap[InSlotName];
		}
		else
		{
			return;
		}
		FT4EntityWeaponEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientWeaponData;
		if (EditorTransientData.TransientItemOverrideMaterialSlotName != InSlotName)
		{
			EditorTransientData.TransientItemOverrideMaterialSlotName = InSlotName;
		}
		if (EditorTransientData.TransientItemOverrideMaterialAsset != MaterialAssetSelected)
		{
			EditorTransientData.TransientItemOverrideMaterialAsset = MaterialAssetSelected;
		}
	}

	bool EntityWeaponUpdateOverrideMaterialBySlotName(
		UT4WeaponEntityAsset* InOutEntityAsset,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);

		FT4EntityItemWeaponMeshData& MeshData = InOutEntityAsset->MeshData;
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.StaticMeshOverrideMaterialData;
			if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
			{
				OutErrorMessage = TEXT("SlotName Not found!");
				return false;
			}
			InOutEntityAsset->MarkPackageDirty();
			OverrideMaterialData.MaterialMap[InSlotName] = InMaterialAsset;
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.SkeletalMeshOverrideMaterialData;
			if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
			{
				OutErrorMessage = TEXT("SlotName Not found!");
				return false;
			}
			InOutEntityAsset->MarkPackageDirty();
			OverrideMaterialData.MaterialMap[InSlotName] = InMaterialAsset;
		}
		else
		{
			OutErrorMessage = TEXT("Not supported!");
			return false;
		}
		return true;
	}

	bool EntityCostumeGetOverrideMeterialSlots(
		UT4CostumeEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		FT4EntityItemCostumeMeshData& MeshData = InOutEntityAsset->MeshData;
		FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.OverrideMaterialData;
		bool bResult = GetOverrideMeterialSlotsInSkeletalMesh(
			MeshData.SkeletalMeshAsset.LoadSynchronous(),
			OverrideMaterialData
		);
		return bResult;
	}

	bool EntityCostumeClearOverrideMeterialSlots(
		UT4CostumeEntityAsset* InOutEntityAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityItemCostumeMeshData& MeshData = InOutEntityAsset->MeshData;
		FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.OverrideMaterialData;
		bool bResult = GetOverrideMeterialSlotsInSkeletalMesh(
			nullptr,
			OverrideMaterialData
		);
		return bResult;
	}

	void EntityCostumeSelectOverrideMaterialBySlotName(
		UT4CostumeEntityAsset* InOutEntityAsset,
		const FName& InSlotName
	) // #80
	{
		check(nullptr != InOutEntityAsset);
		
		FT4ScopedTransientTransaction TransientTransaction(InOutEntityAsset); // #88

		FT4EntityItemCostumeMeshData& MeshData = InOutEntityAsset->MeshData;
		FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.OverrideMaterialData;
		if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
		{
			return;
		}
		TSoftObjectPtr<UMaterialInterface> MaterialAsset = OverrideMaterialData.MaterialMap[InSlotName];
		FT4EntityCostumeEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientCostumeData;
		if (EditorTransientData.TransientItemOverrideMaterialSlotName != InSlotName)
		{
			EditorTransientData.TransientItemOverrideMaterialSlotName = InSlotName;
		}
		if (EditorTransientData.TransientItemOverrideMaterialAsset != MaterialAsset)
		{
			EditorTransientData.TransientItemOverrideMaterialAsset = MaterialAsset;
		}
	}

	bool EntityCostumeUpdateOverrideMaterialBySlotName(
		UT4CostumeEntityAsset* InOutEntityAsset,
		const FName& InSlotName,
		const TSoftObjectPtr<UMaterialInterface>& InMaterialAsset,
		FString& OutErrorMessage
	) // #80
	{
		check(nullptr != InOutEntityAsset);

		FT4EntityItemCostumeMeshData& MeshData = InOutEntityAsset->MeshData;
		FT4EntityOverrideMaterialData& OverrideMaterialData = MeshData.OverrideMaterialData;
		if (!OverrideMaterialData.MaterialMap.Contains(InSlotName))
		{
			OutErrorMessage = TEXT("SlotName Not found!");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		OverrideMaterialData.MaterialMap[InSlotName] = InMaterialAsset;
		return true;
	}

	void EntityCharacterSelectCompositePartByPartName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InPartName
	) // #71
	{
		check(nullptr != InOutEntityAsset);
		
		FT4ScopedTransientTransaction TransientTransaction(InOutEntityAsset); // #88

		FT4EntityCharacterCompositeMeshData& CompositeMeshData = InOutEntityAsset->CopmpositeMeshData;
		if (CompositeMeshData.DefaultPartsData.Contains(InPartName))
		{
			const FT4EntityCharacterCompositePartMeshData& MeshData = CompositeMeshData.DefaultPartsData[InPartName];
			FT4EntityCharacterEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientCharacterData;
			if (EditorTransientData.TransientCompositePartName != InPartName)
			{
				EditorTransientData.TransientCompositePartName = InPartName;
			}
			if (EditorTransientData.TransientCompositePartAsset != MeshData.CostumeEntityAsset)
			{
				EditorTransientData.TransientCompositePartAsset = MeshData.CostumeEntityAsset;
			}
		}
	}

	bool EntityCharacterUpdateCompositePartByPartName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InPartName,
		const TSoftObjectPtr<UT4CostumeEntityAsset>& InCostumeEntityAsset,
		FString& OutErrorMessage
	) // #71
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityCharacterCompositeMeshData& CopmpositeMeshData = InOutEntityAsset->CopmpositeMeshData;
		if (!CopmpositeMeshData.DefaultPartsData.Contains(InPartName))
		{
			OutErrorMessage = TEXT("Part not found");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityCharacterCompositePartMeshData& PartMeshData = CopmpositeMeshData.DefaultPartsData[InPartName];
		PartMeshData.CostumeEntityAsset = InCostumeEntityAsset;
		return true;
	}

	bool EntityCharacterAddCompositePartByPartName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InPartName,
		const TSoftObjectPtr<UT4CostumeEntityAsset>& InCostumeEntityAsset,
		FString& OutErrorMessage
	) // #71
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityCharacterCompositeMeshData& CopmpositeMeshData = InOutEntityAsset->CopmpositeMeshData;
		if (CopmpositeMeshData.DefaultPartsData.Contains(InPartName))
		{
			OutErrorMessage = TEXT("Part Name is Already Exists");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityCharacterCompositePartMeshData NewData;
		NewData.CostumeEntityAsset = InCostumeEntityAsset;
		CopmpositeMeshData.DefaultPartsData.Add(InPartName, NewData);
		return true;
	}

	bool EntityCharacterRemoveCompositePartByPartName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InPartName,
		FString& OutErrorMessage
	) // #71
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityCharacterCompositeMeshData& CopmpositeMeshData = InOutEntityAsset->CopmpositeMeshData;
		if (!CopmpositeMeshData.DefaultPartsData.Contains(InPartName))
		{
			OutErrorMessage = TEXT("Not found Part!");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		CopmpositeMeshData.DefaultPartsData.Remove(InPartName);
		return true;
	}

	void EntityCharacterSelectStanceDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InStanceName
	) // #73
	{
		check(nullptr != InOutEntityAsset);
		FT4ScopedTransientTransaction TransientTransaction(InOutEntityAsset); // #88

		FT4EntityCharacterStanceSetData& StanceSetData = InOutEntityAsset->StanceSetData;
		if (!StanceSetData.StanceMap.Contains(InStanceName))
		{
			return;
		}
		const FT4EntityCharacterStanceData& StanceData = StanceSetData.StanceMap[InStanceName];
		FT4EntityCharacterEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientCharacterData;
		if (EditorTransientData.TransientStanceName != InStanceName)
		{
			EditorTransientData.TransientStanceName = InStanceName;
		}
		if (EditorTransientData.TransientStanceAsset != StanceData.AnimSetAsset)
		{
			EditorTransientData.TransientStanceAsset = StanceData.AnimSetAsset;
		}
		if (EditorTransientData.TransientStanceActiveLayerTag != StanceData.ActiveLayerTag) // ~#73, #74
		{
			EditorTransientData.TransientStanceActiveLayerTag = StanceData.ActiveLayerTag;
		}
	}

	bool EntityCharacterUpdateStanceData(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InStanceName,
		const FT4EntityCharacterStanceData* InStanceData,
		FString& OutErrorMessage
	) // #95
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		FT4EntityCharacterStanceSetData& StanceSetData = InOutEntityAsset->StanceSetData;
		if (!StanceSetData.StanceMap.Contains(InStanceName))
		{
			OutErrorMessage = TEXT("Stance data not found");
			return false;
		}
		FT4EntityCharacterStanceData& StanceData = StanceSetData.StanceMap[InStanceName];
		StanceData = *InStanceData;
		return true;
	}

	bool EntityCharacterAddStanceData(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InStanceName,
		const FT4EntityCharacterStanceData* InStanceData,
		FString& OutErrorMessage
	) // #71
	{
		check(nullptr != InOutEntityAsset);
		InOutEntityAsset->MarkPackageDirty();

		FT4EntityCharacterStanceData* SelectedData = nullptr;
		FT4EntityCharacterStanceSetData& StanceSetData = InOutEntityAsset->StanceSetData;
		if (StanceSetData.StanceMap.Contains(InStanceName))
		{
			OutErrorMessage = TEXT("Stance Name is Already exist");
			return false;
		}

		FT4EntityCharacterStanceData NewData;
		SelectedData = &StanceSetData.StanceMap.Add(
			InStanceName,
			NewData
		);

		*SelectedData = *InStanceData;
		return true;
	}

	bool EntityCharacterRemoveStanceDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InStanceName,
		FString& OutErrorMessage
	) // #71
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityCharacterStanceSetData& StanceSetData = InOutEntityAsset->StanceSetData;
		if (!StanceSetData.StanceMap.Contains(InStanceName))
		{
			OutErrorMessage = TEXT("Not found Stance!");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		StanceSetData.StanceMap.Remove(InStanceName);
		return true;
	}

	void EntityCharacterSelectReactionDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InReactionName
	) // #76
	{
		check(nullptr != InOutEntityAsset);

		FT4ScopedTransientTransaction TransientTransaction(InOutEntityAsset); // #88

		FT4EntityCharacterReactionSetData& ReactionSetData = InOutEntityAsset->ReactionSetData;
		if (!ReactionSetData.ReactionMap.Contains(InReactionName))
		{
			return;
		}
		const FT4EntityCharacterReactionData& ReactionData = ReactionSetData.ReactionMap[InReactionName];
		FT4EntityCharacterEditorTransientData& EditorTransientData = InOutEntityAsset->EditorTransientCharacterData;
		if (EditorTransientData.TransientReactionName != InReactionName)
		{
			EditorTransientData.TransientReactionName = InReactionName;
		}
		if (EditorTransientData.TransientReactionType != ReactionData.ReactionType)
		{
			EditorTransientData.TransientReactionType = ReactionData.ReactionType;
		}
		if (EditorTransientData.TransientReactionMaxPlayTimeSec != ReactionData.MaxPlayTimeSec)
		{
			EditorTransientData.TransientReactionMaxPlayTimeSec = ReactionData.MaxPlayTimeSec;
		}
		if (EditorTransientData.bTransientReactionPhysicsStartUsed != ReactionData.bUsePhysicsStart)
		{
			EditorTransientData.bTransientReactionPhysicsStartUsed = ReactionData.bUsePhysicsStart;
		}
		{
			const FT4EntityCharacterReactionPhysicsStartData& SourcePhysicsData = ReactionData.PhysicsStartData;
			FT4EntityCharacterReactionPhysicsStartData& PhysicsData = EditorTransientData.TransientReactionPhysicsStartData;
			if (PhysicsData.DelayTimeSec != SourcePhysicsData.DelayTimeSec)
			{
				PhysicsData.DelayTimeSec = SourcePhysicsData.DelayTimeSec;
			}
			if (PhysicsData.ImpulseMainActionPoint != SourcePhysicsData.ImpulseMainActionPoint)
			{
				PhysicsData.ImpulseMainActionPoint = SourcePhysicsData.ImpulseMainActionPoint;
			}
			if (PhysicsData.ImpulseSubActionPoint != SourcePhysicsData.ImpulseSubActionPoint)
			{
				PhysicsData.ImpulseSubActionPoint = SourcePhysicsData.ImpulseSubActionPoint;
			}
			if (PhysicsData.ImpulsePower != SourcePhysicsData.ImpulsePower)
			{
				PhysicsData.ImpulsePower = SourcePhysicsData.ImpulsePower;
			}
			if (PhysicsData.CenterOfMass != SourcePhysicsData.CenterOfMass)
			{
				PhysicsData.CenterOfMass = SourcePhysicsData.CenterOfMass;
			}
			if (PhysicsData.MassOverrideInKg != SourcePhysicsData.MassOverrideInKg)
			{
				PhysicsData.MassOverrideInKg = SourcePhysicsData.MassOverrideInKg;
			}
			if (PhysicsData.bSimulateBodiesBelow != SourcePhysicsData.bSimulateBodiesBelow)
			{
				PhysicsData.bSimulateBodiesBelow = SourcePhysicsData.bSimulateBodiesBelow;
			}
			if (PhysicsData.BlendData.TargetWeight != SourcePhysicsData.BlendData.TargetWeight)
			{
				PhysicsData.BlendData.TargetWeight = SourcePhysicsData.BlendData.TargetWeight;
			}
			if (PhysicsData.BlendData.BlendInTimeSec != SourcePhysicsData.BlendData.BlendInTimeSec)
			{
				PhysicsData.BlendData.BlendInTimeSec = SourcePhysicsData.BlendData.BlendInTimeSec;
			}
			if (PhysicsData.BlendData.BlendOutTimeSec != SourcePhysicsData.BlendData.BlendOutTimeSec)
			{
				PhysicsData.BlendData.BlendOutTimeSec = SourcePhysicsData.BlendData.BlendOutTimeSec;
			}
		}
		if (EditorTransientData.bTransientReactionPhysicsStopUsed != ReactionData.bUsePhysicsStop)
		{
			EditorTransientData.bTransientReactionPhysicsStopUsed = ReactionData.bUsePhysicsStop;
		}
		{
			const FT4EntityCharacterReactionPhysicsStopData& SourcePhysicsData = ReactionData.PhysicsStopData;
			FT4EntityCharacterReactionPhysicsStopData& PhysicsData = EditorTransientData.TransientReactionPhysicsStopData;
			if (PhysicsData.DelayTimeSec != SourcePhysicsData.DelayTimeSec)
			{
				PhysicsData.DelayTimeSec = SourcePhysicsData.DelayTimeSec;
			}
		}
		if (EditorTransientData.bTransientReactionAnimationUsed != ReactionData.bUseAnimation)
		{
			EditorTransientData.bTransientReactionAnimationUsed = ReactionData.bUseAnimation;
		}
		{
			const FT4EntityCharacterReactionAnimationData& SourceAnimationData = ReactionData.AnimationData;
			FT4EntityCharacterReactionAnimationData& AnimationData = EditorTransientData.TransientReactionAnimationData;
			if (AnimationData.DelayTimeSec != SourceAnimationData.DelayTimeSec)
			{
				AnimationData.DelayTimeSec = SourceAnimationData.DelayTimeSec;
			}
			if (AnimationData.StartAnimSectionName != SourceAnimationData.StartAnimSectionName)
			{
				AnimationData.StartAnimSectionName = SourceAnimationData.StartAnimSectionName;
			}
			if (AnimationData.LoopAnimSectionName != SourceAnimationData.LoopAnimSectionName)
			{
				AnimationData.LoopAnimSectionName = SourceAnimationData.LoopAnimSectionName;
			}
			if (AnimationData.BlendInTimeSec != SourceAnimationData.BlendInTimeSec)
			{
				AnimationData.BlendInTimeSec = SourceAnimationData.BlendInTimeSec;
			}
			if (AnimationData.BlendOutTimeSec != SourceAnimationData.BlendOutTimeSec)
			{
				AnimationData.BlendOutTimeSec = SourceAnimationData.BlendOutTimeSec;
			}
		}
	}

	bool EntityCharacterUpdateReactionDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InReactionName,
		const FT4EntityCharacterReactionData* InReactionData,
		FString& OutErrorMessage
	) // #95
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityCharacterReactionSetData& ReactionSetData = InOutEntityAsset->ReactionSetData;
		if (!ReactionSetData.ReactionMap.Contains(InReactionName))
		{
			OutErrorMessage = TEXT("Reaction not found");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityCharacterReactionData&	SelectedData = ReactionSetData.ReactionMap[InReactionName];
		SelectedData = *InReactionData;
		return true;
	}

	bool EntityCharacterAddReactionData(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InReactionName,
		const FT4EntityCharacterReactionData* InReactionData,
		FString& OutErrorMessage
	) // #76
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityCharacterReactionSetData& ReactionSetData = InOutEntityAsset->ReactionSetData;
		if (ReactionSetData.ReactionMap.Contains(InReactionName))
		{
			OutErrorMessage = TEXT("Reaction is Already exists");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		FT4EntityCharacterReactionData NewData;
		ReactionSetData.ReactionMap.Add(InReactionName, NewData);
		return true;
	}

	bool EntityCharacterRemoveReactionDataByName(
		UT4CharacterEntityAsset* InOutEntityAsset,
		const FName& InReactionName,
		FString& OutErrorMessage
	) // #76
	{
		check(nullptr != InOutEntityAsset);
		FT4EntityCharacterReactionSetData& ReactionSetData = InOutEntityAsset->ReactionSetData;
		if (!ReactionSetData.ReactionMap.Contains(InReactionName))
		{
			OutErrorMessage = TEXT("Not found Reaction!");
			return false;
		}
		InOutEntityAsset->MarkPackageDirty();
		ReactionSetData.ReactionMap.Remove(InReactionName);
		return true;
	}
}
#endif
// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetAnimSetUtils.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"
#include "T4Asset/Public/T4AssetDefinitions.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"

#include "Misc/PackageName.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #39
 */
#if WITH_EDITOR
namespace T4AssetUtil
{
	bool AnimSetSelectAnimSequenceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InSelectedName
	)
	{
		check(nullptr != InAnimSetAsset);

		FT4ScopedTransientTransaction TransientTransaction(InAnimSetAsset); // #88

		bool bChanged = false;
		if (T4AnimSetAnimMontageSkillName == InAnimMontageName)
		{
			FT4AnimSequenceInfo* FoundInfo = InAnimSetAsset->SkillAnimSequenceArray.FindByKey(InSelectedName);
			if (nullptr != FoundInfo)
			{
				FT4AnimSetEditorTransientData& EditorTransientData = InAnimSetAsset->EditorTransientData;
				if (EditorTransientData.TransientSelectSkillSectionName != InSelectedName)
				{
					EditorTransientData.TransientSelectSkillSectionName = InSelectedName;
					bChanged = true;
				}
				if (EditorTransientData.TransientSkillAnimSequenceAsset != FoundInfo->AnimSequnceAsset)
				{
					EditorTransientData.TransientSkillAnimSequenceAsset = FoundInfo->AnimSequnceAsset;
					bChanged = true;
				}
			}
		}
		else if (T4AnimSetAnimMontageAdditiveName == InAnimMontageName)
		{
			FT4AnimSequenceInfo* FoundInfo = InAnimSetAsset->AdditiveAnimSequenceArray.FindByKey(InSelectedName);
			if (nullptr != FoundInfo)
			{
				FT4AnimSetEditorTransientData& EditorTransientData = InAnimSetAsset->EditorTransientData;
				if (EditorTransientData.TransientSelectAdditiveSectionName != InSelectedName)
				{
					EditorTransientData.TransientSelectAdditiveSectionName = InSelectedName;
					bChanged = true;
				}
				if (EditorTransientData.TransientAdditiveAnimSequenceAsset != FoundInfo->AnimSequnceAsset)
				{
					EditorTransientData.TransientAdditiveAnimSequenceAsset = FoundInfo->AnimSequnceAsset;
					bChanged = true;
				}
			}
		}
		else if (T4AnimSetAnimMontageDefaultName == InAnimMontageName)
		{
			FT4AnimSequenceInfo* FoundInfo = InAnimSetAsset->DefaultAnimSequenceArray.FindByKey(InSelectedName);
			if (nullptr != FoundInfo)
			{
				FT4AnimSetEditorTransientData& EditorTransientData = InAnimSetAsset->EditorTransientData;
				if (EditorTransientData.TransientSelectDefaultSectionName != InSelectedName)
				{
					EditorTransientData.TransientSelectDefaultSectionName = InSelectedName;
					bChanged = true;
				}
				if (EditorTransientData.TransientDefaultAnimSequenceAsset != FoundInfo->AnimSequnceAsset)
				{
					EditorTransientData.TransientDefaultAnimSequenceAsset = FoundInfo->AnimSequnceAsset;
					bChanged = true;
				}
			}
		}

		return bChanged;
	}

	bool AnimSetSelectBlendSpaceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InSelectedName
	)
	{
		check(nullptr != InAnimSetAsset);

		FT4ScopedTransientTransaction TransientTransaction(InAnimSetAsset); // #88

		bool bChanged = false;
		FT4BlendSpaceInfo* FoundInfo = InAnimSetAsset->BlendSpaceArray.FindByKey(InSelectedName);
		if (nullptr != FoundInfo)
		{
			FT4AnimSetEditorTransientData& EditorTransientData = InAnimSetAsset->EditorTransientData;
			if (EditorTransientData.TransientSelectBlendSpaceName != InSelectedName)
			{
				EditorTransientData.TransientSelectBlendSpaceName = InSelectedName;
				bChanged = true;
			}
			if (EditorTransientData.TransientBlendSpaceAsset != FoundInfo->BlendSpaceAsset)
			{
				EditorTransientData.TransientBlendSpaceAsset = FoundInfo->BlendSpaceAsset;
				bChanged = true;
			}
		}
		return bChanged;
	}

	static bool GetAnimSequenceIndexAndArray(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InSelectedName,
		int32& OutIndexSelected,
		TArray<FT4AnimSequenceInfo>** OutArraySelected
	)
	{
		if (T4AnimSetAnimMontageSkillName == InAnimMontageName)
		{
			for (int32 idx = 0; idx < InAnimSetAsset->SkillAnimSequenceArray.Num(); ++idx)
			{
				const FT4AnimSequenceInfo& SequenceInfo = InAnimSetAsset->SkillAnimSequenceArray[idx];
				if (SequenceInfo.Name == InSelectedName)
				{
					OutIndexSelected = idx;
					*OutArraySelected = &InAnimSetAsset->SkillAnimSequenceArray;
					return true;
				}
			}
		}
		else if (T4AnimSetAnimMontageAdditiveName == InAnimMontageName)
		{
			for (int32 idx = 0; idx < InAnimSetAsset->AdditiveAnimSequenceArray.Num(); ++idx)
			{
				const FT4AnimSequenceInfo& SequenceInfo = InAnimSetAsset->AdditiveAnimSequenceArray[idx];
				if (SequenceInfo.Name == InSelectedName)
				{
					OutIndexSelected = idx;
					*OutArraySelected = &InAnimSetAsset->AdditiveAnimSequenceArray;
					return true;
				}
			}
		}
		else if (T4AnimSetAnimMontageDefaultName == InAnimMontageName)
		{
			for (int32 idx = 0; idx < InAnimSetAsset->DefaultAnimSequenceArray.Num(); ++idx)
			{
				const FT4AnimSequenceInfo& SequenceInfo = InAnimSetAsset->DefaultAnimSequenceArray[idx];
				if (SequenceInfo.Name == InSelectedName)
				{
					OutIndexSelected = idx;
					*OutArraySelected = &InAnimSetAsset->DefaultAnimSequenceArray;
					return true;
				}
			}
		}
		return false;
	}

	void AnimSetMoveUpAnimSequenceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InSelectedName
	)
	{
		check(nullptr != InAnimSetAsset);
		int32 IndexSelected = -1;
		TArray<FT4AnimSequenceInfo>* ArraySelected = nullptr;
		if (!GetAnimSequenceIndexAndArray(
			InAnimSetAsset,
			InAnimMontageName,
			InSelectedName,
			IndexSelected,
			&ArraySelected
		))
		{
			return;
		}
		check(nullptr != ArraySelected);
		const int32 NumAnimSequences = ArraySelected->Num();
		if (0 >= IndexSelected || IndexSelected >= NumAnimSequences)
		{
			return;
		}
		InAnimSetAsset->MarkPackageDirty();
		TArray<FT4AnimSequenceInfo>& ArraySelectedRef = *ArraySelected;
		FT4AnimSequenceInfo SwapSequenceInfo = ArraySelectedRef[IndexSelected - 1];
		ArraySelectedRef[IndexSelected - 1] = ArraySelectedRef[IndexSelected];
		ArraySelectedRef[IndexSelected] = SwapSequenceInfo;
	}

	void AnimSetMoveUpBlendSpaceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InSelectedName
	)
	{
		check(nullptr != InAnimSetAsset);
		const int32 NumAnimSequences = InAnimSetAsset->BlendSpaceArray.Num();
		for (int32 idx = 0; idx < NumAnimSequences; ++idx)
		{
			const FT4BlendSpaceInfo& BlendSpaceInfo = InAnimSetAsset->BlendSpaceArray[idx];
			if (BlendSpaceInfo.Name == InSelectedName)
			{
				if (0 >= idx)
				{
					return;
				}
				InAnimSetAsset->MarkPackageDirty();
				FT4BlendSpaceInfo SwapBlendSpaceInfo = InAnimSetAsset->BlendSpaceArray[idx - 1];
				InAnimSetAsset->BlendSpaceArray[idx - 1] = BlendSpaceInfo;
				InAnimSetAsset->BlendSpaceArray[idx] = SwapBlendSpaceInfo;
				return;
			}
		}
	}

	void AnimSetMoveDownAnimSequenceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InSelectedName
	)
	{
		check(nullptr != InAnimSetAsset);
		int32 IndexSelected = -1;
		TArray<FT4AnimSequenceInfo>* ArraySelected = nullptr;
		if (!GetAnimSequenceIndexAndArray(
			InAnimSetAsset,
			InAnimMontageName,
			InSelectedName,
			IndexSelected,
			&ArraySelected
		))
		{
			return;
		}
		check(nullptr != ArraySelected);
		const int32 NumAnimSequences = ArraySelected->Num();
		if (IndexSelected + 1 >= NumAnimSequences || IndexSelected >= NumAnimSequences)
		{
			return;
		}
		InAnimSetAsset->MarkPackageDirty();
		TArray<FT4AnimSequenceInfo>& ArraySelectedRef = *ArraySelected;
		FT4AnimSequenceInfo SwapSequenceInfo = ArraySelectedRef[IndexSelected + 1];
		ArraySelectedRef[IndexSelected + 1] = ArraySelectedRef[IndexSelected];
		ArraySelectedRef[IndexSelected] = SwapSequenceInfo;
	}

	void AnimSetMoveDownBlendSpaceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InSelectedName
	)
	{
		check(nullptr != InAnimSetAsset);
		const int32 NumAnimSequences = InAnimSetAsset->BlendSpaceArray.Num();
		for (int32 idx = 0; idx < NumAnimSequences; ++idx)
		{
			const FT4BlendSpaceInfo& BlendSpaceInfo = InAnimSetAsset->BlendSpaceArray[idx];
			if (BlendSpaceInfo.Name == InSelectedName)
			{
				if (idx + 1 >= NumAnimSequences)
				{
					return;
				}
				InAnimSetAsset->MarkPackageDirty();
				FT4BlendSpaceInfo SwapBlendSpaceInfo = InAnimSetAsset->BlendSpaceArray[idx + 1];
				InAnimSetAsset->BlendSpaceArray[idx + 1] = BlendSpaceInfo;
				InAnimSetAsset->BlendSpaceArray[idx] = SwapBlendSpaceInfo;
				return;
			}
		}
	}

	bool AnimSetAddOrUpdateAnimSeqeunceInfo(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InAnimSequenceName,
		const TSoftObjectPtr<UAnimSequence>& InAnimSequence,
		FString& OutErrorMessage
	)
	{
		check(nullptr != InAnimSetAsset);
		InAnimSetAsset->MarkPackageDirty();

		FT4AnimSequenceInfo* SelectedAnimSequenceInfo = nullptr;
		if (T4AnimSetAnimMontageSkillName == InAnimMontageName)
		{
			SelectedAnimSequenceInfo = InAnimSetAsset->SkillAnimSequenceArray.FindByKey(InAnimSequenceName);
			if (nullptr == SelectedAnimSequenceInfo)
			{
				FT4AnimSequenceInfo& NewAnimSequenceInfo = InAnimSetAsset->SkillAnimSequenceArray.AddDefaulted_GetRef();
				SelectedAnimSequenceInfo = &NewAnimSequenceInfo;
			}
		}
		else if (T4AnimSetAnimMontageAdditiveName == InAnimMontageName)
		{
			SelectedAnimSequenceInfo = InAnimSetAsset->AdditiveAnimSequenceArray.FindByKey(InAnimSequenceName);
			if (nullptr == SelectedAnimSequenceInfo)
			{
				FT4AnimSequenceInfo& NewAnimSequenceInfo = InAnimSetAsset->AdditiveAnimSequenceArray.AddDefaulted_GetRef();
				SelectedAnimSequenceInfo = &NewAnimSequenceInfo;
			}
		}
		else if (T4AnimSetAnimMontageDefaultName == InAnimMontageName)
		{
			// #38
			SelectedAnimSequenceInfo = InAnimSetAsset->DefaultAnimSequenceArray.FindByKey(InAnimSequenceName);
			if (nullptr == SelectedAnimSequenceInfo)
			{
				FT4AnimSequenceInfo& NewAnimSequenceInfo = InAnimSetAsset->DefaultAnimSequenceArray.AddDefaulted_GetRef();
				SelectedAnimSequenceInfo = &NewAnimSequenceInfo;
			}
		}
		else
		{
			return false;
		}

		SelectedAnimSequenceInfo->Name = InAnimSequenceName;
		SelectedAnimSequenceInfo->AnimSequnceAsset = InAnimSequence;
		UAnimSequence* AnimSequence = InAnimSequence.LoadSynchronous();
		check(nullptr != AnimSequence);
		SelectedAnimSequenceInfo->DurationSec = AnimSequence->GetMaxCurrentTime();
		return true;
	}

	bool AnimSetRemoveAnimSeqeunceInfo(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InAnimSequenceName,
		FString& OutErrorMessage
	)
	{
		check(nullptr != InAnimSetAsset);

		if (T4AnimSetAnimMontageSkillName == InAnimMontageName)
		{
			FT4AnimSequenceInfo* FoundInfo = InAnimSetAsset->SkillAnimSequenceArray.FindByKey(InAnimSequenceName);
			if (nullptr == FoundInfo)
			{
				OutErrorMessage = TEXT("Not found Skill AnimSeuqneceInfo!");
				return false;
			}
			int32 FoundIndex = InAnimSetAsset->SkillAnimSequenceArray.IndexOfByKey(InAnimSequenceName);
			if (INDEX_NONE == FoundIndex)
			{
				OutErrorMessage = TEXT("Not found Skill AnimSeuqneceInfo!");
				return false;
			}
			InAnimSetAsset->MarkPackageDirty();
			InAnimSetAsset->SkillAnimSequenceArray.RemoveAt(FoundIndex);
		}
		else if (T4AnimSetAnimMontageAdditiveName == InAnimMontageName)
		{
			FT4AnimSequenceInfo* FoundInfo = InAnimSetAsset->AdditiveAnimSequenceArray.FindByKey(InAnimSequenceName);
			if (nullptr == FoundInfo)
			{
				OutErrorMessage = TEXT("Not found Additive AnimSeuqneceInfo!");
				return false;
			}
			int32 FoundIndex = InAnimSetAsset->AdditiveAnimSequenceArray.IndexOfByKey(InAnimSequenceName);
			if (INDEX_NONE == FoundIndex)
			{
				OutErrorMessage = TEXT("Not found Additive AnimSeuqneceInfo!");
				return false;
			}
			InAnimSetAsset->MarkPackageDirty();
			InAnimSetAsset->AdditiveAnimSequenceArray.RemoveAt(FoundIndex);
		}
		else if (T4AnimSetAnimMontageDefaultName == InAnimMontageName)
		{
			// #38
			FT4AnimSequenceInfo* FoundInfo = InAnimSetAsset->DefaultAnimSequenceArray.FindByKey(InAnimSequenceName);
			if (nullptr == FoundInfo)
			{
				OutErrorMessage = TEXT("Not found Default AnimSeuqneceInfo!");
				return false;
			}
			int32 FoundIndex = InAnimSetAsset->DefaultAnimSequenceArray.IndexOfByKey(InAnimSequenceName);
			if (INDEX_NONE == FoundIndex)
			{
				OutErrorMessage = TEXT("Not found Default AnimSeuqneceInfo!");
				return false;
			}
			InAnimSetAsset->MarkPackageDirty();
			InAnimSetAsset->DefaultAnimSequenceArray.RemoveAt(FoundIndex);
		}
		else
		{
			return false;
		}

		return true;
	}

	bool AnimSetAddOrUpdateBlendSpaceInfo(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InBlendSpaceName,
		const TSoftObjectPtr<UBlendSpaceBase>& InBlendSpace,
		FString& OutErrorMessage
	)
	{
		check(nullptr != InAnimSetAsset);
		InAnimSetAsset->MarkPackageDirty();
		FT4BlendSpaceInfo* SelectedInfo = InAnimSetAsset->BlendSpaceArray.FindByKey(InBlendSpaceName);
		if (nullptr == SelectedInfo)
		{
			FT4BlendSpaceInfo& NewBlendSpaceInfo = InAnimSetAsset->BlendSpaceArray.AddDefaulted_GetRef();
			SelectedInfo = &NewBlendSpaceInfo;
		}
		SelectedInfo->Name = InBlendSpaceName;
		SelectedInfo->BlendSpaceAsset = InBlendSpace;
		return true;
	}

	bool AnimSetRemoveBlendSpaceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InBlendSpaceName,
		FString& OutErrorMessage
	)
	{
		check(nullptr != InAnimSetAsset);
		FT4BlendSpaceInfo* SelectedInfo = InAnimSetAsset->BlendSpaceArray.FindByKey(InBlendSpaceName);
		if (nullptr == SelectedInfo)
		{
			OutErrorMessage = TEXT("Not found BlendSpaceInfo!");
			return false;
		}
		InAnimSetAsset->MarkPackageDirty();
		InAnimSetAsset->BlendSpaceArray.Remove(*SelectedInfo);
		return true;
	}

	bool AnimSetUpdateBlendSpaceInfo(
		UT4AnimSetAsset* InAnimSetAsset
	)
	{
		check(nullptr != InAnimSetAsset);
		if (0 < InAnimSetAsset->BlendSpaceArray.Num())
		{
			InAnimSetAsset->MarkPackageDirty();

			float StartTimeSec = 0.0f;
			for (TArray<FT4BlendSpaceInfo>::TIterator It(InAnimSetAsset->BlendSpaceArray); It; ++It)
			{
				FT4BlendSpaceInfo& BlendSpaceInfo = *It;
				// empty
			}
		}
		return true;
	}

	inline const FString GetAssetNameFromObjectPath(
		const FString& InObjectPathString
	)
	{
		FString AssetName = FPackageName::GetLongPackageAssetName(InObjectPathString);
		int32 StartIdx = 0;
		if (AssetName.FindChar(TEXT('.'), StartIdx))
		{
			int32 LenString = AssetName.Len();
			AssetName.RemoveAt(StartIdx - 1, LenString - StartIdx);
		}
		return AssetName;
	}

	static UAnimMontage* NewAnimMontageInAnimSet(
		const FString& InAssetName, // #69
		const FString& InPackagePath // #69
	)
	{
		UAnimMontage* NewAnimMontage = Cast<UAnimMontage>(T4AssetUtil::NewAsset(
			UAnimMontage::StaticClass(),
			InAssetName,
			InPackagePath
		));
		if (nullptr == NewAnimMontage)
		{
			return nullptr;
		}
		return NewAnimMontage;
	}

	static bool UpdateAnimMontageInAnimSet(
		const UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		UAnimMontage* OutAnimMontage
	)
	{
		check(nullptr != OutAnimMontage);

		OutAnimMontage->MarkPackageDirty();

		{
			// #69 : 초기화!!
			OutAnimMontage->SetSkeleton(InAnimSetAsset->SkeletonAsset.LoadSynchronous());
			check(1 == OutAnimMontage->SlotAnimTracks.Num());
			FSlotAnimationTrack& SlotAnimationTrack = OutAnimMontage->SlotAnimTracks[0];
			SlotAnimationTrack.AnimTrack.AnimSegments.Empty();
			OutAnimMontage->CompositeSections.Empty();
			OutAnimMontage->SequenceLength = 0.0f;
		}

		FSlotAnimationTrack& SlotAnimationTrack = OutAnimMontage->SlotAnimTracks[0];
		const TArray<FT4AnimSequenceInfo>* AnimSequenceInfoArray = nullptr;
		if (T4AnimSetAnimMontageAdditiveName == InAnimMontageName)
		{
			static const FName T4AdditiveGroupSlotName = TEXT("AdditiveSlot"); // #39
			SlotAnimationTrack.SlotName = T4AdditiveGroupSlotName;
			AnimSequenceInfoArray = &InAnimSetAsset->AdditiveAnimSequenceArray;
		}
		else if (T4AnimSetAnimMontageSkillName == InAnimMontageName)
		{
			static const FName T4SkillGroupSlotName = TEXT("SkillSlot"); // #39
			SlotAnimationTrack.SlotName = T4SkillGroupSlotName;
			AnimSequenceInfoArray = &InAnimSetAsset->SkillAnimSequenceArray;
		}
		else if (T4AnimSetAnimMontageDefaultName == InAnimMontageName)
		{
			AnimSequenceInfoArray = &InAnimSetAsset->DefaultAnimSequenceArray; // #39
		}
		else
		{
			return false;
		}

		if (0 < AnimSequenceInfoArray->Num())
		{
			FAnimTrack& DefaultAnimTrack = SlotAnimationTrack.AnimTrack;

			float StartTimeSec = 0.0f;
			for (TArray<FT4AnimSequenceInfo>::TConstIterator It(*AnimSequenceInfoArray); It; ++It)
			{
				const FT4AnimSequenceInfo& AnimSequenceInfo = *It;
				UAnimSequence* AnimSequence = AnimSequenceInfo.AnimSequnceAsset.LoadSynchronous();
				if (nullptr == AnimSequence)
				{
					continue;
				}
				{
					FAnimSegment NewSegment;
					NewSegment.AnimReference = AnimSequence;
					NewSegment.AnimStartTime = 0.f;
					NewSegment.AnimEndTime = AnimSequence->GetMaxCurrentTime();
					NewSegment.AnimPlayRate = 1.0f;
					NewSegment.LoopingCount = 1;
					NewSegment.StartPos = StartTimeSec;
					DefaultAnimTrack.AnimSegments.Add(NewSegment);
				}
				{
					FCompositeSection NewSection;
					NewSection.SectionName = AnimSequenceInfo.Name;
					NewSection.SetTime(StartTimeSec);
					OutAnimMontage->CompositeSections.Add(NewSection);
				}
				StartTimeSec += AnimSequenceInfo.DurationSec;
			}
			OutAnimMontage->SequenceLength = StartTimeSec;
		}

		return true;
	}

	static bool UpdateAnimMontageInAnimSet(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName InAnimMontageName,
		TArray<FT4AnimSequenceInfo>& InOutAnimSequenceArray,
		bool bInAutoGen,
		TSoftObjectPtr<UAnimMontage>& OutAnimMontage
	) // #69
	{
		for (TArray<FT4AnimSequenceInfo>::TIterator It(InOutAnimSequenceArray); It; ++It)
		{
			FT4AnimSequenceInfo& AnimSequenceInfo = *It;
			UAnimSequence* AnimSequence = AnimSequenceInfo.AnimSequnceAsset.LoadSynchronous();
			if (nullptr == AnimSequence)
			{
				continue;
			}
			AnimSequenceInfo.DurationSec = AnimSequence->GetMaxCurrentTime();
		}
		if (!bInAutoGen)
		{
			return true;
		}
		UAnimMontage* AnimMontage = nullptr;
		if (!OutAnimMontage.IsNull())
		{
			AnimMontage = OutAnimMontage.LoadSynchronous();
		}
		if (nullptr == AnimMontage)
		{
			FString AnimMontageAssetName = FString::Printf(
				TEXT("AM_AutoGen_%s_%s"),
				*(InAnimMontageName.ToString()),
				*(GetAssetNameFromObjectPath(InAnimSetAsset->GetPathName()))
			);
			FString AnimMontagePackagePath = FPackageName::GetLongPackagePath(InAnimSetAsset->GetPathName());
			AnimMontagePackagePath += TEXT("/AnimMontage/");

			AnimMontage = NewAnimMontageInAnimSet(
				AnimMontageAssetName,
				AnimMontagePackagePath
			);
		}
		if (nullptr == AnimMontage)
		{
			return false;
		}
		bool bResult = UpdateAnimMontageInAnimSet(
			InAnimSetAsset,
			InAnimMontageName,
			AnimMontage
		);
		if (bResult)
		{
			OutAnimMontage = AnimMontage;
		}
		return true;
	}

	bool AnimSetUpdateAll(
		UT4AnimSetAsset* InAnimSetAsset,
		FString& OutErrorMessage
	)
	{
		check(nullptr != InAnimSetAsset);
		InAnimSetAsset->MarkPackageDirty();

		bool bResult = true;
		bResult = UpdateAnimMontageInAnimSet(
			InAnimSetAsset,
			T4AnimSetAnimMontageSkillName,
			InAnimSetAsset->SkillAnimSequenceArray,
			InAnimSetAsset->bSkillAnimMontageAutoGen,
			InAnimSetAsset->SkillAnimMontageAsset
		);
		if (!bResult)
		{
			OutErrorMessage = TEXT("Failed to Build Skill animation!");
			return false;
		}
		bResult = UpdateAnimMontageInAnimSet(
			InAnimSetAsset,
			T4AnimSetAnimMontageAdditiveName,
			InAnimSetAsset->AdditiveAnimSequenceArray,
			InAnimSetAsset->bAdditiveAnimMontageAutoGen,
			InAnimSetAsset->AdditiveAnimMontageAsset
		);
		if (!bResult)
		{
			OutErrorMessage = TEXT("Failed to Build Additive animation!");
			return false;
		}
		bResult = UpdateAnimMontageInAnimSet(
			InAnimSetAsset,
			T4AnimSetAnimMontageDefaultName,
			InAnimSetAsset->DefaultAnimSequenceArray,
			InAnimSetAsset->bDefaultAnimMontageAutoGen,
			InAnimSetAsset->DefaultAnimMontageAsset
		);
		if (!bResult)
		{
			OutErrorMessage = TEXT("Failed to Build Default animation!");
			return false;
		}
		bResult = T4AssetUtil::AnimSetUpdateBlendSpaceInfo(InAnimSetAsset);
		if (!bResult)
		{
			OutErrorMessage = TEXT("Failed to Build BlendSpace!");
			return false;
		}

		return bResult;
	}

	static bool SaveAnimMontageInAnimSet(
		TSoftObjectPtr<UAnimMontage>& InAnimMontage,
		FString& OutErrorMessage
	) // #69
	{
		if (InAnimMontage.IsNull())
		{
			return true;
		}
		UAnimMontage* AnimMontage = InAnimMontage.LoadSynchronous();
		if (nullptr == AnimMontage)
		{
			return false;
		}
		bool bResult = true;
		UPackage* AnimMontagePackage = AnimMontage->GetOutermost();
		check(nullptr != AnimMontagePackage);
		if (AnimMontagePackage->IsDirty())
		{
			bResult = T4AssetUtil::SaveAsset(AnimMontage, true);
		}
		return bResult;
	}

	bool AnimSetSaveAll(
		UT4AnimSetAsset* InAnimSetAsset,
		FString& OutErrorMessage
	)
	{
		if (nullptr == InAnimSetAsset)
		{
			OutErrorMessage = TEXT("No Set AnimSetAsset");
			return false;
		}
		bool bResult = true;
		{
			// #69
			if (InAnimSetAsset->bSkillAnimMontageAutoGen)
			{
				bResult = SaveAnimMontageInAnimSet(InAnimSetAsset->SkillAnimMontageAsset, OutErrorMessage);
				if (!bResult)
				{
					OutErrorMessage = TEXT("Failed to load Skill AnimMontage");
					return false;
				}
			}
			if (InAnimSetAsset->bAdditiveAnimMontageAutoGen)
			{
				bResult = SaveAnimMontageInAnimSet(InAnimSetAsset->AdditiveAnimMontageAsset, OutErrorMessage);
				if (!bResult)
				{
					OutErrorMessage = TEXT("Failed to load Additive AnimMontage");
					return false;
				}
			}
			if (InAnimSetAsset->bDefaultAnimMontageAutoGen)
			{
				bResult = SaveAnimMontageInAnimSet(InAnimSetAsset->DefaultAnimMontageAsset, OutErrorMessage);
				if (!bResult)
				{
					OutErrorMessage = TEXT("Failed to load Default AnimMontage");
					return false;
				}
			}
		}
		UPackage* AnimSetPackage = InAnimSetAsset->GetOutermost();
		check(nullptr != AnimSetPackage);
		bool bWasAnimSetPackageDirty = AnimSetPackage->IsDirty();
		if (bWasAnimSetPackageDirty)
		{
			bResult = T4AssetUtil::SaveAsset(InAnimSetAsset, true); // #39
		}
		return bResult;
	}

}
#endif
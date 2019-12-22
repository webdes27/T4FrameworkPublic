// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Asset/Public/T4AssetUtils.h"

/**
  * #39
 */
#if WITH_EDITOR

class UAnimSequence;
class UBlendSpaceBase;
class UT4AnimSetAsset;

namespace T4AssetUtil
{

	bool AnimSetSelectAnimSequenceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InSelectedName
	); // changed

	bool AnimSetSelectBlendSpaceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InSelectedName
	); // changed

	void AnimSetMoveUpAnimSequenceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InSelectedName
	);

	void AnimSetMoveUpBlendSpaceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InSelectedName
	);

	void AnimSetMoveDownAnimSequenceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InSelectedName
	);

	void AnimSetMoveDownBlendSpaceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InSelectedName
	);

	bool AnimSetAddOrUpdateAnimSeqeunceInfo(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InAnimSequenceName,
		const TSoftObjectPtr<UAnimSequence>& InAnimSequence,
		FString& OutErrorMessage
	);

	bool AnimSetRemoveAnimSeqeunceInfo(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InAnimMontageName,
		const FName& InAnimSequenceName,
		FString& OutErrorMessage
	);

	bool AnimSetAddOrUpdateBlendSpaceInfo(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InBlendSpaceName,
		const TSoftObjectPtr<UBlendSpaceBase>& InBlendSpace,
		FString& OutErrorMessage
	);

	bool AnimSetRemoveBlendSpaceInfoByName(
		UT4AnimSetAsset* InAnimSetAsset,
		const FName& InBlendSpaceName,
		FString& OutErrorMessage
	);

	bool AnimSetUpdateBlendSpaceInfo(
		UT4AnimSetAsset* InAnimSetAsset
	);

	bool AnimSetUpdateAll(
		UT4AnimSetAsset* InAnimSetAsset,
		FString& OutErrorMessage
	);

	bool AnimSetSaveAll(
		UT4AnimSetAsset* InAnimSetAsset,
		FString& OutErrorMessage
	); // #69

}

#endif

// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "T4Asset/Classes/Conti/T4ContiAsset.h"
#include "Public/Asset/T4AssetLoader.h"
#include "Public/T4Engine.h"

/**
  * #24
 */
class FT4ActionNodeControl;
class FT4ActionCompositeNodeBase : public FT4ActionNodeBase
{
public:
	explicit FT4ActionCompositeNodeBase(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionCompositeNodeBase();

protected:
	void ProcessCompositeData(
		const FT4ActionCompositeData& InCompositeData,
		const float InMaxPlayTimeSec, // #54
		const float InOffsetTimeSec // #56
	); // #54

private:
	FT4ActionNodeBase* NewChildActionNode(
		FT4ActionNodeBase* InParentNode,
		const FT4ActionContiCommand* InAction,
		const float InMaxPlayTimeSec, // #54
		const float InOffsetTimeSec, // #56
		FT4ActionKey& OutChildActionKey,
		TMap<uint32, FT4ActionNodeBase*>& OutTempActionMap
	);

	FT4ActionNodeBase* NewChildActionNode(
		FT4ActionNodeBase* InParentNode,
		const ET4ActionType InActionType, // #56
		const FT4ActionContiCommand* InAction,
		const float InMaxPlayTimeSec, // #54
		const float InOffsetTimeSec, // #56
		FT4ActionKey& OutChildActionKey,
		TMap<uint32, FT4ActionNodeBase*>& OutTempActionMap
	); // #56

#if WITH_EDITOR
	const FT4ActionContiCommand* GetActionStruct(
		const FT4ActionHeaderInfo& InHierarchyInfo,
		const FT4ActionCompositeData& InCompositeData
	) const;
#endif
};

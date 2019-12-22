// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/Action/ActionNode/T4ActionNode.h"
#include "T4Asset/Classes/Conti/T4ContiAsset.h"
#include "Public/Asset/T4AssetLoader.h"
#include "Public/T4Engine.h"

/**
  * #24
 */
class FT4ActionControl;
class FT4ActionCompositeNode : public FT4ActionNode
{
public:
	explicit FT4ActionCompositeNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionCompositeNode();

protected:
	void ProcessCompositeData(
		const FT4ActionCompositeData& InCompositeData,
		const float InMaxPlayTimeSec, // #54
		const float InOffsetTimeSec // #56
	); // #54

private:
	FT4ActionNode* NewChildActionNode(
		FT4ActionNode* InParentNode,
		const FT4ContiActionStruct* InAction,
		const float InMaxPlayTimeSec, // #54
		const float InOffsetTimeSec, // #56
		FT4ActionKey& OutChildActionKey,
		TMap<uint32, FT4ActionNode*>& OutTempActionMap
	);

	FT4ActionNode* NewChildActionNode(
		FT4ActionNode* InParentNode,
		const ET4ActionType InActionType, // #56
		const FT4ContiActionStruct* InAction,
		const float InMaxPlayTimeSec, // #54
		const float InOffsetTimeSec, // #56
		FT4ActionKey& OutChildActionKey,
		TMap<uint32, FT4ActionNode*>& OutTempActionMap
	); // #56

#if WITH_EDITOR
	const FT4ContiActionStruct* GetActionStruct(
		const FT4ActionHeaderInfo& InHierarchyInfo,
		const FT4ActionCompositeData& InCompositeData
	) const;
#endif
};

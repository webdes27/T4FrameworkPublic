// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4BaseAnimStates.h"

#include "Object/T4GameObject.h"
#include "Object/Animation/T4BaseAnimControl.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  * #47
 */
bool FT4BaseAnimState::ChangeNextState(
	FT4BaseAnimControl* InAnimControl, 
	const FName& InNextStateName
)
{
	bool bResult = InAnimControl->ChangeNextAnimState(InNextStateName);
	return bResult;
}
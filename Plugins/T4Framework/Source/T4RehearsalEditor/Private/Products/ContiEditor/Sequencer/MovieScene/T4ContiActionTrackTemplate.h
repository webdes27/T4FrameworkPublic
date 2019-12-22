// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Evaluation/MovieSceneTrackImplementation.h"
#include "T4ContiActionTrackTemplate.generated.h"

/**
  * #56
 */
USTRUCT()
struct FT4ContiActionTrackTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()

public:
	FT4ContiActionTrackTemplate();

private:
	UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }

	void Evaluate(
		const FMovieSceneEvaluationOperand& Operand, 
		const FMovieSceneContext& Context, 
		const FPersistentEvaluationData& PersistentData, 
		FMovieSceneExecutionTokens& ExecutionTokens
	) const override; // #56
};

// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiActionTrackTemplate.h"

#include "MovieSceneExecutionToken.h"
#include "IMovieScenePlayer.h"
#include "Evaluation/MovieSceneEvaluationTemplateInstance.h"
#include "MovieSceneSequence.h"
#include "MovieScene.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #56
 */
struct FT4ActionExecutionToken
	: IMovieSceneExecutionToken
{
	FT4ActionExecutionToken()
	{
	}

	virtual void Execute(
		const FMovieSceneContext& Context, 
		const FMovieSceneEvaluationOperand& Operand, 
		FPersistentEvaluationData& PersistentData, 
		IMovieScenePlayer& Player
	) override
	{

	}
};

FT4ContiActionTrackTemplate::FT4ContiActionTrackTemplate()
{
}

void FT4ContiActionTrackTemplate::Evaluate(
	const FMovieSceneEvaluationOperand& Operand,
	const FMovieSceneContext& Context,
	const FPersistentEvaluationData& PersistentData,
	FMovieSceneExecutionTokens& ExecutionTokens
) const // #56
{
	ExecutionTokens.Add(FT4ActionExecutionToken());
}
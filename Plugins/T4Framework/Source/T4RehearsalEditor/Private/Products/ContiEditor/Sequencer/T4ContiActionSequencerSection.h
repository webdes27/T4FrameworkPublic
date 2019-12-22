// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "ISequencerSection.h"

/**
  *
 */
class UMovieSceneSection;
class UT4ContiActionSection;
class FT4ContiActionSequencerSection : public ISequencerSection
{
public:
	FT4ContiActionSequencerSection(UMovieSceneSection& InSection);

	//~ ISequencerSection interface
	virtual UMovieSceneSection* GetSectionObject() override;
	virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override;
	virtual FText GetSectionTitle() const override;

protected:
	UT4ContiActionSection* Section;
};

// #56
class FT4ContiSequencerKeySection : public FT4ContiActionSequencerSection
{
public:
	FT4ContiSequencerKeySection(UMovieSceneSection& InSection);

	//~ ISequencerSection interface
	virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override;
};


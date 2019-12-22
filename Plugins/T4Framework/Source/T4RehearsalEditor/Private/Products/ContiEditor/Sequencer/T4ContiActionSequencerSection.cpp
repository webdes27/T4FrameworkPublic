// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiActionSequencerSection.h"
#include "MovieScene/T4ContiActionSection.h"
#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"

#include "SequencerSectionPainter.h"

#include "T4RehearsalEditorInternal.h"

/**
  *
 */
#define LOCTEXT_NAMESPACE "FT4ContiActionSequencerSection"

FT4ContiActionSequencerSection::FT4ContiActionSequencerSection(UMovieSceneSection& InSection)
	: Section(Cast<UT4ContiActionSection>(&InSection))
{
}

UMovieSceneSection* FT4ContiActionSequencerSection::GetSectionObject()
{
	return Section;
}

int32 FT4ContiActionSequencerSection::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	check(nullptr != Section);
	return InPainter.PaintSectionBackground(Section->GetBackgroundColor());
}

FText FT4ContiActionSequencerSection::GetSectionTitle() const
{
	check(nullptr != Section);
	FT4ContiViewModel* ContiViewModel = Section->GetContiViewModel();
	check(nullptr != ContiViewModel);
	return ContiViewModel->GetActionSectionDisplayName(Section); // #54
}

FT4ContiSequencerKeySection::FT4ContiSequencerKeySection(UMovieSceneSection& InSection)
	: FT4ContiActionSequencerSection(InSection)
{
}

int32 FT4ContiSequencerKeySection::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	check(nullptr != Section);
	return InPainter.PaintSectionBackground(FLinearColor::Transparent);
}

#undef LOCTEXT_NAMESPACE
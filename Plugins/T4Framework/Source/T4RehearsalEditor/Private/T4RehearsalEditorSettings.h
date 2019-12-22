// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "T4RehearsalEditorSettings.generated.h"

/**
  *
 */
UCLASS(config = T4Framework, defaultconfig)
class UT4RehearsalEditorSettings : public UDeveloperSettings
{
public:
	GENERATED_UCLASS_BODY()
	
	// Begin UDeveloperSettings Interface
	FName GetCategoryName() const override;
	FText GetSectionText() const override;
	// END UDeveloperSettings Interface

	void PostInitProperties() override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FT4OnRehearsalEdSettingsChanged, const FString&, const UT4RehearsalEditorSettings*);

	static FT4OnRehearsalEdSettingsChanged& OnSettingsChanged();

public:
	// Plugins/T4Framework/Config/DefaultT4Framework.ini

	UPROPERTY(config, noclear, EditAnywhere, Category=PlayMode)
	bool bT4GameplayEnabled; // #56

	UPROPERTY(config, noclear, EditAnywhere, Category= PlayMode)
	FSoftClassPath DefaultGameInstanceClass; // #56

	UPROPERTY(config, noclear, EditAnywhere, Category= PlayMode)
	FSoftClassPath DefaultGameModeClass; // #56

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath BlendSpaceNameTable; // #39

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath DefaultSectionNameTable; // #39

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath AdditiveSectionNameTable; // #39

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath SkillSectionNameTable; // #39

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath ActionPointNameTable; // #57

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath EquipPointNameTable; // #72

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath CompositePartNameTable; // #71

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath LayerTagNameTable; // #74

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath StanceNameTable; // #73

	UPROPERTY(config, EditAnywhere, Category=NameTable)
	FSoftObjectPath ReactionNameTable; // #76

	UPROPERTY(config, EditAnywhere, Category=ContiEditor)
	bool bDefaultActionPlaybackRepeat; // #56

	UPROPERTY(config, EditAnywhere, Category=ContiEditor)
	bool bDefaultActionPlaybackPlayerPossessed; // #56

protected:
	static FT4OnRehearsalEdSettingsChanged SettingsChangedDelegate;
};

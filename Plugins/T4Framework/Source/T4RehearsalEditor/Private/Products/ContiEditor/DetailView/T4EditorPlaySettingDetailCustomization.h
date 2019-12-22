// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/ // #60
 */

class FT4ContiViewModel;
class UT4EditorGameplaySettingObject;
class FT4EditorPlaySettingDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(TSharedPtr<FT4ContiViewModel> InContiViewModel);
	
	FT4EditorPlaySettingDetailCustomization(TSharedPtr<FT4ContiViewModel> InContiViewModel);

	void CustomizeDetails(IDetailLayoutBuilder& InBuilder);

protected:
	void CustomizePlayerSettingsDetails(
		IDetailLayoutBuilder& InBuilder,
		UT4EditorGameplaySettingObject* InEditorPlaySettings
	); // #60

	void CustomizeSandbackSettingsDetails(
		IDetailLayoutBuilder& InBuilder,
		UT4EditorGameplaySettingObject* InEditorPlaySettings
	); // #60

	void CustomizeAttackerDataDetails(
		IDetailLayoutBuilder& InBuilder,
		UT4EditorGameplaySettingObject* InEditorPlaySettings
	); // #60

	void CustomizeDefenderDataDetails(
		IDetailLayoutBuilder& InBuilder,
		UT4EditorGameplaySettingObject* InEditorPlaySettings
	); // #60

	FReply HandleOnSaveEditorPlaySettings();

	FReply HandleOnButtonNPCShowPreview();
	FReply HandleOnButtonNPCDoSpawn();
	FReply HandleOnButtonNPCDoSpawn4();

	FReply HandleOnButtonWeaponShowPreview();
	FReply HandleOnButtonWeaponEquip();
	FReply HandleOnButtonWeaponUnequip();

	FReply HandleOnButtonCostumeShowPreview();
	FReply HandleOnButtonCostumeExchange();

	void HandleOnNPCContentNameIDSelected(const FName InName);

	void HandleOnWeaponContentNameIDSelected(const FName InName);
	void HandleOnCostumeContentNameIDSelected(const FName InName);
	void HandleOnSkillContentNameIDSelected(const FName InName);
	void HandleOnEffectContentNameIDSelected(const FName InName);

private:
	void HandleOnRefreshLayout();

private:
	TSharedPtr<FT4ContiViewModel> ViewModel;
	IDetailLayoutBuilder* DetailLayoutPtr;

	FString AttackerCategoryName;
	FString DefenderCategoryName;
};

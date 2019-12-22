// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4EntityBrowserWidget.h"

#include "T4RehearsalEditorModule.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Entity/T4EntityAssetMinimal.h"

#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "FrontendFilterBase.h"

#include "Animation/DebugSkelMeshComponent.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "ObjectEditorUtils.h"
#include "EditorStyleSet.h"
#include "FileHelpers.h"

#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "AssetRegistryModule.h"

#include "Toolkits/AssetEditorManager.h"
#include "Toolkits/GlobalEditorCommonCommands.h"
#include "Framework/Application/SlateApplication.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #36 : refer SAnimationSequenceBrowser
 */

#define LOCTEXT_NAMESPACE "T4EntityBrowserWidget"

const FString ST4EntityBrowserWidget::SettingsIniSection = TEXT("T4EntityBrowserWidget");

// #T4_ADD_ENTITY_TAG
class FFrontendFilter_T4MapEntityAssets : public FFrontendFilter
{
public:
	FFrontendFilter_T4MapEntityAssets(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

	// FFrontendFilter implementation
	virtual FString GetName() const override { return TEXT("ShowMapEntityAssets"); }
	virtual FText GetDisplayName() const override { return LOCTEXT("T4EntityBrowserWidgetMapEntityAssets", "Show MapEntity Assets"); }
	virtual FText GetToolTipText() const override { return LOCTEXT("T4EntityBrowserWidgetMapEntityAssets_Tooltip", "Show MapEntity Assets"); }

	// IFilter implementation
	virtual bool PassesFilter(FAssetFilterType InItem) const override
	{
		return !InItem.GetClass()->IsChildOf(UT4MapEntityAsset::StaticClass());
	}
};

class FFrontendFilter_T4CharacterEntityAssets : public FFrontendFilter
{
public:
	FFrontendFilter_T4CharacterEntityAssets(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

	// FFrontendFilter implementation
	virtual FString GetName() const override { return TEXT("ShowCharacterEntityAssets"); }
	virtual FText GetDisplayName() const override { return LOCTEXT("T4EntityBrowserWidgetCharacterEntityAssets", "Show ActorEntity Assets"); }
	virtual FText GetToolTipText() const override { return LOCTEXT("T4EntityBrowserWidgetCharacterEntityAssets_Tooltip", "Show ActorEntity Assets"); }

	// IFilter implementation
	virtual bool PassesFilter(FAssetFilterType InItem) const override
	{
		return !InItem.GetClass()->IsChildOf(UT4CharacterEntityAsset::StaticClass());
	}
};

class FFrontendFilter_T4PropEntityAssets : public FFrontendFilter
{
public:
	FFrontendFilter_T4PropEntityAssets(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

	// FFrontendFilter implementation
	virtual FString GetName() const override { return TEXT("ShowPropEntityAssets"); }
	virtual FText GetDisplayName() const override { return LOCTEXT("T4EntityBrowserWidgetPropEntityAssets", "Show PropEntity Assets"); }
	virtual FText GetToolTipText() const override { return LOCTEXT("T4EntityBrowserWidgetPropEntityAssets_Tooltip", "Show PropEntity Assets"); }

	// IFilter implementation
	virtual bool PassesFilter(FAssetFilterType InItem) const override
	{
		return !InItem.GetClass()->IsChildOf(UT4PropEntityAsset::StaticClass());
	}
};

class FFrontendFilter_T4CostumeEntityAssets : public FFrontendFilter
{
public:
	FFrontendFilter_T4CostumeEntityAssets(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

	// FFrontendFilter implementation
	virtual FString GetName() const override { return TEXT("ShowCostumeEntityAssets"); }
	virtual FText GetDisplayName() const override { return LOCTEXT("T4EntityBrowserWidgetCostumeEntityAssets", "Show CostumeEntity Assets"); }
	virtual FText GetToolTipText() const override { return LOCTEXT("T4EntityBrowserWidgetCostumeEntityAssets_Tooltip", "Show CostumeEntity Assets"); }

	// IFilter implementation
	virtual bool PassesFilter(FAssetFilterType InItem) const override
	{
		return !InItem.GetClass()->IsChildOf(UT4CostumeEntityAsset::StaticClass());
	}
};

class FFrontendFilter_T4WeaponEntityAssets : public FFrontendFilter
{
public:
	FFrontendFilter_T4WeaponEntityAssets(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

	// FFrontendFilter implementation
	virtual FString GetName() const override { return TEXT("ShowWeaponEntityAssets"); }
	virtual FText GetDisplayName() const override { return LOCTEXT("T4EntityBrowserWidgetWeaponEntityAssets", "Show WeaponEntity Assets"); }
	virtual FText GetToolTipText() const override { return LOCTEXT("T4EntityBrowserWidgetWeaponEntityAssets_Tooltip", "Show WeaponEntity Assets"); }

	// IFilter implementation
	virtual bool PassesFilter(FAssetFilterType InItem) const override
	{
		return !InItem.GetClass()->IsChildOf(UT4WeaponEntityAsset::StaticClass());
	}
};

class FFrontendFilter_T4ZoneEntityAssets : public FFrontendFilter // #94
{
public:
	FFrontendFilter_T4ZoneEntityAssets(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

	// FFrontendFilter implementation
	virtual FString GetName() const override { return TEXT("ShowZoneEntityAssets"); }
	virtual FText GetDisplayName() const override { return LOCTEXT("T4EntityBrowserWidgetZoneEntityAssets", "Show ZoneEntity Assets"); }
	virtual FText GetToolTipText() const override { return LOCTEXT("T4EntityBrowserWidgetZoneEntityAssets_Tooltip", "Show ZoneEntity Assets"); }

	// IFilter implementation
	virtual bool PassesFilter(FAssetFilterType InItem) const override
	{
		return !InItem.GetClass()->IsChildOf(UT4ZoneEntityAsset::StaticClass());
	}
};

ST4EntityBrowserWidget::~ST4EntityBrowserWidget()
{
}

void ST4EntityBrowserWidget::HandleOnSelectAsset(const FAssetData& AssetData)
{
	if (UObject* RawAsset = AssetData.GetAsset())
	{
		if (UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(RawAsset))
		{
			OnSelectAsset.ExecuteIfBound(EntityAsset);
		}
	}
}

void ST4EntityBrowserWidget::HandleOnDoubleClicked(const FAssetData& AssetData, bool bFromHistory)
{
	if (UObject* RawAsset = AssetData.GetAsset())
	{
		if (UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(RawAsset))
		{
			OnDoubleClicked.ExecuteIfBound(EntityAsset);
		}
	}
}

TSharedPtr<SWidget> ST4EntityBrowserWidget::HandleOnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	// #72
	UT4EntityAsset* EntityAssetSelected = nullptr;
	ET4EntityType EntityTypeSelected = ET4EntityType::None;
	for (const FAssetData& AssetData : SelectedAssets)
	{
		if (UObject* RawAsset = AssetData.GetAsset())
		{
			EntityAssetSelected = Cast<UT4EntityAsset>(RawAsset);
			EntityTypeSelected = EntityAssetSelected->GetEntityType();
		}
	}

	const bool bMultiSelected = (1 < SelectedAssets.Num()) ? true : false;

	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/ true, Commands);

	MenuBuilder.BeginSection("EntityAssetOptions", LOCTEXT("T4EntityBrowserWidgetContextMenu", "Options") );
	
	{
		if (!bMultiSelected)
		{
			if (ET4EntityType::Weapon == EntityTypeSelected || ET4EntityType::Costume == EntityTypeSelected) // #72
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("T4EntityBrowserWidgetDropSelectedAssets", "Drop Item"),
					LOCTEXT("T4EntityBrowserWidgetDropSelectedAssets_Tooltip", "Drop the selected item a entity"),
					FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
					FUIAction(
						FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnSpawnOrDropSelectedAssets, SelectedAssets),
						FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnCanSelectedAssets, SelectedAssets)
					)
				);

				if (ET4EntityType::Weapon == EntityTypeSelected && OnEquipItemSubMenuSelected.IsBound())
				{
					MenuBuilder.AddWrapperSubMenu(
						LOCTEXT("T4EntityBrowserWidgetEquipSelectedAssets", "Equip Weapon"),
						LOCTEXT("T4EntityBrowserWidgetEquipSelectedAssets_Tooltip", "Equip the selected item a entity"),
						FOnGetContent::CreateSP(this, &ST4EntityBrowserWidget::HandleOnEquipItemSubMenu, EntityAssetSelected),
						FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x")
					);
				}
				else if (ET4EntityType::Costume == EntityTypeSelected && OnExchangeItemSubMenuSelected.IsBound())
				{
					MenuBuilder.AddWrapperSubMenu(
						LOCTEXT("T4EntityBrowserWidgetExchangeSelectedAssets", "Exchange Costume"),
						LOCTEXT("T4EntityBrowserWidgetExchangeSelectedAssets_Tooltip", "Exchange the selected item a entity"),
						FOnGetContent::CreateSP(this, &ST4EntityBrowserWidget::HandleOnExchangeItemSubMenu, EntityAssetSelected),
						FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x")
					);
				}
			}
			else if (ET4EntityType::Map == EntityTypeSelected) // #81
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("T4EntityBrowserWidgetLoadMapSelectedAssets", "Load Map"),
					LOCTEXT("T4EntityBrowserWidgetLoadMapSelectedAssets_Tooltip", "Load the selected a entity"),
					FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
					FUIAction(
						FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnLoadMapSelectedAssets, SelectedAssets),
						FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnCanSelectedAssets, SelectedAssets)
					)
				);
			}
			else
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("T4EntityBrowserWidgetSpawnSelectedAssets", "Spawn"),
					LOCTEXT("T4EntityBrowserWidgetSpawnSelectedAssets_Tooltip", "Spawn the selected a entity"),
					FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
					FUIAction(
						FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnSpawnOrDropSelectedAssets, SelectedAssets),
						FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnCanSelectedAssets, SelectedAssets)
					)
				);
			}
		}
		else
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("T4EntityBrowserWidgetSpawnSelectedAssets", "Spawn or Drop Item"),
				LOCTEXT("T4EntityBrowserWidgetSpawnSelectedAssets_Tooltip", "Spawn or DropItem the selected entities"),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
				FUIAction(
					FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnSpawnOrDropSelectedAssets, SelectedAssets),
					FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnCanSelectedAssets, SelectedAssets)
				)
			);
		}

		FText OpenLabelText = LOCTEXT("T4EntityBrowserWidgetOpenSelectedAssets", "Open");
		FText OpenTooltipLabelText = LOCTEXT("T4EntityBrowserWidgetOpenSelectedAssets_Tooltip", "Open the selected entity");
		FText SaveLabelText = LOCTEXT("T4EntityBrowserWidgetSaveSelectedAssets", "Save");
		FText SaveTooltipLabelText = LOCTEXT("T4EntityBrowserWidgetSaveSelectedAssets_Tooltip", "Save the selected entity");

		if (bMultiSelected)
		{
			OpenLabelText = LOCTEXT("T4EntityBrowserWidgetOpenSelectedAssets", "Open Entities");
			OpenTooltipLabelText = LOCTEXT("T4EntityBrowserWidgetOpenSelectedAssets_Tooltip", "Open the selected entities");
			SaveLabelText = LOCTEXT("T4EntityBrowserWidgetSaveSelectedAssets", "Save Entities");
			SaveTooltipLabelText = LOCTEXT("T4EntityBrowserWidgetSaveSelectedAssets_Tooltip", "Save the selected entities");
		}

		MenuBuilder.AddMenuEntry(
			OpenLabelText,
			OpenTooltipLabelText,
			FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
			FUIAction(
				FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnOpenSelectedAssets, SelectedAssets),
				FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleCanOpenSelectedAssets, SelectedAssets)
			)
		);

		MenuBuilder.AddMenuEntry(
			SaveLabelText,
			SaveTooltipLabelText,
			FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
			FUIAction(
				FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnSaveSelectedAssets, SelectedAssets), // #73
				FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleCanSaveSelectedAssets, SelectedAssets) // #73
			)
		);

		if (!bMultiSelected)
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("T4EntityBrowserWidgetThumbnailSelectedAssets", "Update Thumbnail"),
				LOCTEXT("T4EntityBrowserWidgetThumbnailSelectedAssets_Tooltip", "Thumbnail the selected a entity"),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
				FUIAction(
					FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnThumbnailSelectedAssets, SelectedAssets),
					FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleCanThumbnailSelectedAssets, SelectedAssets)
				)
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("T4EntityBrowserWidgetSavePreviewCameraInfoSelectedAssets", "Save Preview Camera Info"),
				LOCTEXT("T4EntityBrowserWidgetSavePreviewCameraInfoSelectedAssets_Tooltip", "Get Preview CameraInfo the selected a entity"),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
				FUIAction(
					FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleOnSavePreviewCameraInfoSelectedAssets, SelectedAssets),
					FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::HandleCanSavePreviewCameraInfoSelectedAssets, SelectedAssets)
				)
			);
		}

		MenuBuilder.AddMenuEntry(FGlobalEditorCommonCommands::Get().FindInContentBrowser);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> ST4EntityBrowserWidget::HandleOnEquipItemSubMenu(UT4EntityAsset* InEntityAssetSelected) // #72
{
	FMenuBuilder MenuBuilder(true, nullptr);

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	const TArray<FT4ConstantDataRow>& EquipPointNames = EngineConstants->GetConstantDatas(ET4EngineConstantType::EquipPoint);
	for (const FT4ConstantDataRow& NameInfo : EquipPointNames)
	{
		FString DisplayString = FString::Printf(TEXT("[%s] %s"), *(NameInfo.Name.ToString()), *NameInfo.Description);
		MenuBuilder.AddWrapperSubMenu(
			FText::FromString(DisplayString),
			FText(),
			FOnGetContent::CreateSP(
				this, 
				&ST4EntityBrowserWidget::HandleOnEquipItemSubMenuExecute, 
				InEntityAssetSelected,
				NameInfo.Name
			),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x")
		);
	}

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> ST4EntityBrowserWidget::HandleOnEquipItemSubMenuExecute(
	UT4EntityAsset* InEntityAssetSelected,
	FName InActionPointName
) // #72
{
	FMenuBuilder MenuBuilder(true, nullptr);

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	static const FName EquipString = TEXT("Equip");
	static const FName UnEquipString = TEXT("UnEquip");

	const TArray<FT4ConstantDataRow>& EquipPointNames = EngineConstants->GetConstantDatas(ET4EngineConstantType::EquipPoint);
	for (const FT4ConstantDataRow& NameInfo : EquipPointNames)
	{
		MenuBuilder.AddMenuEntry(
			FText::FromName(EquipString),
			FText(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(
					this,
					&ST4EntityBrowserWidget::HandleOnEquipOrUnEquipItem,
					InEntityAssetSelected,
					InActionPointName,
					true
				)
			)
		);
		MenuBuilder.AddMenuEntry(
			FText::FromName(UnEquipString),
			FText(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(
					this,
					&ST4EntityBrowserWidget::HandleOnEquipOrUnEquipItem,
					InEntityAssetSelected,
					InActionPointName,
					false
				)
			)
		);
	}

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> ST4EntityBrowserWidget::HandleOnExchangeItemSubMenu(UT4EntityAsset* InEntityAssetSelected) // #72
{
	FMenuBuilder MenuBuilder(true, nullptr);

	UT4CostumeEntityAsset* CostumeEntityAsset = Cast<UT4CostumeEntityAsset>(InEntityAssetSelected);
	if (nullptr != CostumeEntityAsset)
	{
		FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
		check(nullptr != EngineConstants);

		const TArray<FT4ConstantDataRow>& CompositePartNames = EngineConstants->GetConstantDatas(ET4EngineConstantType::CompositePart);
		for (const FT4ConstantDataRow& NameInfo : CompositePartNames)
		{
			if (CostumeEntityAsset->MeshData.CompositePartName == NameInfo.Name)
			{
				FString DisplayString = FString::Printf(TEXT("[%s] %s"), *(NameInfo.Name.ToString()), *NameInfo.Description);
				MenuBuilder.AddWrapperSubMenu(
					FText::FromString(DisplayString),
					FText(),
					FOnGetContent::CreateSP(
						this,
						&ST4EntityBrowserWidget::HandleOnExchangeItemSubMenuExecute,
						InEntityAssetSelected,
						NameInfo.Name
					),
					FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x")
				);
			}
		}
	}

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> ST4EntityBrowserWidget::HandleOnExchangeItemSubMenuExecute(
	UT4EntityAsset* InEntityAssetSelected,
	FName InCompositePartName
) // #72
{
	FMenuBuilder MenuBuilder(true, nullptr);

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	static const FName ExchangeString = TEXT("Exchange");
	static const FName ClearDefaultString = TEXT("Clear default");

	const TArray<FT4ConstantDataRow>& EquipPointNames = EngineConstants->GetConstantDatas(ET4EngineConstantType::EquipPoint);
	for (const FT4ConstantDataRow& NameInfo : EquipPointNames)
	{
		MenuBuilder.AddMenuEntry(
			FText::FromName(ExchangeString),
			FText(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(
					this,
					&ST4EntityBrowserWidget::HandleOnExchangeItem,
					InEntityAssetSelected,
					InCompositePartName,
					true
				)
			)
		);
		MenuBuilder.AddMenuEntry(
			FText::FromName(ClearDefaultString),
			FText(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(
					this,
					&ST4EntityBrowserWidget::HandleOnExchangeItem,
					InEntityAssetSelected,
					InCompositePartName,
					false
				)
			)
		);
	}

	return MenuBuilder.MakeWidget();
}

void ST4EntityBrowserWidget::FindInContentBrowser()
{
	TArray<FAssetData> CurrentSelection = GetCurrentSelectionDelegate.Execute();
	if (CurrentSelection.Num() > 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(CurrentSelection);
	}
}

bool ST4EntityBrowserWidget::CanFindInContentBrowser() const
{
	TArray<FAssetData> CurrentSelection = GetCurrentSelectionDelegate.Execute();
	return CurrentSelection.Num() > 0;
}

void ST4EntityBrowserWidget::GetSelectedPackages(const TArray<FAssetData>& Assets, TArray<UPackage*>& OutPackages) const
{
	for (int32 AssetIdx = 0; AssetIdx < Assets.Num(); ++AssetIdx)
	{
		UPackage* Package = FindPackage(NULL, *Assets[AssetIdx].PackageName.ToString());

		if ( Package )
		{
			OutPackages.Add(Package);
		}
	}
}

bool ST4EntityBrowserWidget::HandleOnCanSelectedAssets(TArray<FAssetData> InObjects) const
{
	TArray<UPackage*> Packages;
	GetSelectedPackages(InObjects, Packages);
	// Don't offer save option if none of the packages are loaded
	return Packages.Num() > 0;
}

void ST4EntityBrowserWidget::HandleOnSpawnOrDropSelectedAssets(TArray<FAssetData> InObjects) const
{
	for (FAssetData& AssetData : InObjects)
	{
		if (UObject* RawAsset = AssetData.GetAsset())
		{
			if (UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(RawAsset))
			{
				OnSpawnAsset.ExecuteIfBound(EntityAsset);
			}
		}
	}
}

void ST4EntityBrowserWidget::HandleOnEquipOrUnEquipItem(
	UT4EntityAsset* InEntityAssetSelected,
	FName InActionPointName,
	bool bEquip
) const // #72
{
	if (OnEquipItemSubMenuSelected.IsBound())
	{
		OnEquipItemSubMenuSelected.ExecuteIfBound(InEntityAssetSelected, InActionPointName, bEquip);
	}
}

void ST4EntityBrowserWidget::HandleOnExchangeItem(
	UT4EntityAsset* InEntityAssetSelected, 
	FName InCompositePartName,
	bool bSet
) const // #72
{
	if (OnExchangeItemSubMenuSelected.IsBound())
	{
		OnExchangeItemSubMenuSelected.ExecuteIfBound(InEntityAssetSelected, InCompositePartName, bSet);
	}
}

void ST4EntityBrowserWidget::HandleOnLoadMapSelectedAssets(TArray<FAssetData> InObjects) const // #81
{
	for (FAssetData& AssetData : InObjects)
	{
		if (UObject* RawAsset = AssetData.GetAsset())
		{
			if (UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(RawAsset))
			{
				OnSpawnAsset.ExecuteIfBound(EntityAsset);
			}
		}
	}
}

void ST4EntityBrowserWidget::HandleOnOpenSelectedAssets(TArray<FAssetData> InObjects) const
{
	for (FAssetData& AssetData : InObjects)
	{
		if (UObject* RawAsset = AssetData.GetAsset())
		{
			if (UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(RawAsset))
			{
				OnOpenAsset.ExecuteIfBound(EntityAsset);
			}
		}
	}
}

bool ST4EntityBrowserWidget::HandleCanOpenSelectedAssets(TArray<FAssetData> InObjects) const
{
	TArray<UPackage*> Packages;
	GetSelectedPackages(InObjects, Packages);
	// Don't offer save option if none of the packages are loaded
	return Packages.Num() > 0;
}

void ST4EntityBrowserWidget::HandleOnSaveSelectedAssets(TArray<FAssetData> InObjects) const
{
	for (FAssetData& AssetData : InObjects)
	{
		if (UObject* RawAsset = AssetData.GetAsset())
		{
			if (UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(RawAsset))
			{
				OnSaveAsset.ExecuteIfBound(EntityAsset);
			}
		}
	}
}

bool ST4EntityBrowserWidget::HandleCanSaveSelectedAssets(TArray<FAssetData> InObjects) const
{
	TArray<UPackage*> Packages;
	GetSelectedPackages(InObjects, Packages);
	// Don't offer save option if none of the packages are loaded
	return Packages.Num() > 0;
}

void ST4EntityBrowserWidget::HandleOnThumbnailSelectedAssets(TArray<FAssetData> InObjects) const
{
	for (FAssetData& AssetData : InObjects)
	{
		if (UObject* RawAsset = AssetData.GetAsset())
		{
			if (UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(RawAsset))
			{
				OnUpdateThumbnail.ExecuteIfBound(EntityAsset);
				break; // 최초 한개만...
			}
		}
	}
}

bool ST4EntityBrowserWidget::HandleCanThumbnailSelectedAssets(TArray<FAssetData> InObjects) const
{
	TArray<UPackage*> Packages;
	GetSelectedPackages(InObjects, Packages);
	return (1 == Packages.Num()) ? true : false; // 오직 한개만!
}

void ST4EntityBrowserWidget::HandleOnSavePreviewCameraInfoSelectedAssets(TArray<FAssetData> InObjects) const
{
	if (0 >= InObjects.Num())
	{
		return;
	}
	// WARN : 선택시 ViewModel 의 EntityAsset 이 바뀌었기 때문에 Notify 만 전달한다.
	OnSavePreviewCameraInfo.ExecuteIfBound();
}

bool ST4EntityBrowserWidget::HandleCanSavePreviewCameraInfoSelectedAssets(TArray<FAssetData> InObjects) const
{
	TArray<UPackage*> Packages;
	GetSelectedPackages(InObjects, Packages);
	return (1 == Packages.Num()) ? true : false; // 오직 한개만!
}

bool ST4EntityBrowserWidget::CanShowColumnForAssetRegistryTag(FName AssetType, FName TagName) const
{
	return !AssetRegistryTagsToIgnore.Contains(TagName);
}

void ST4EntityBrowserWidget::Construct(const FArguments& InArgs)
{
	OnSelectAsset = InArgs._OnSelectAsset;
	OnSpawnAsset = InArgs._OnSpawnAsset;
	OnOpenAsset = InArgs._OnOpenAsset;
	OnSaveAsset = InArgs._OnSaveAsset;
	OnUpdateThumbnail = InArgs._OnUpdateThumbnail;
	OnSavePreviewCameraInfo = InArgs._OnSavePreviewCameraInfo;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	OnEquipItemSubMenuSelected = InArgs._OnEquipItemSubMenuSelected; // #72
	OnExchangeItemSubMenuSelected = InArgs._OnExchangeItemSubMenuSelected; // #72

	Commands = MakeShareable(new FUICommandList());
	Commands->MapAction(FGlobalEditorCommonCommands::Get().FindInContentBrowser, FUIAction(
		FExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::FindInContentBrowser),
		FCanExecuteAction::CreateSP(this, &ST4EntityBrowserWidget::CanFindInContentBrowser)
	));

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Configure filter for asset picker
	Filter.bRecursiveClasses = true;

	// #T4_ADD_ENTITY_TAG
	Filter.ClassNames.Add(UT4MapEntityAsset::StaticClass()->GetFName());
	Filter.ClassNames.Add(UT4CharacterEntityAsset::StaticClass()->GetFName());
	Filter.ClassNames.Add(UT4PropEntityAsset::StaticClass()->GetFName());
	Filter.ClassNames.Add(UT4CostumeEntityAsset::StaticClass()->GetFName()); // #37
	Filter.ClassNames.Add(UT4WeaponEntityAsset::StaticClass()->GetFName());
	Filter.ClassNames.Add(UT4ZoneEntityAsset::StaticClass()->GetFName()); // #94

	FAssetPickerConfig Config;
	Config.Filter = Filter;
	Config.SelectionMode = ESelectionMode::Multi;
	Config.InitialAssetViewType = EAssetViewType::Column;
	Config.bAddFilterUI = true;
	Config.bShowPathInColumnView = true;
	Config.bSortByPathInColumnView = true;

	// Configure response to click and double-click
	Config.OnAssetSelected = FOnAssetSelected::CreateSP(this, &ST4EntityBrowserWidget::HandleOnSelectAsset);
	Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &ST4EntityBrowserWidget::HandleOnDoubleClicked, false);
	Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &ST4EntityBrowserWidget::HandleOnGetAssetContextMenu);
	Config.OnAssetTagWantsToBeDisplayed = FOnShouldDisplayAssetTag::CreateSP(this, &ST4EntityBrowserWidget::CanShowColumnForAssetRegistryTag);
	Config.SyncToAssetsDelegates.Add(&SyncToAssetsDelegate);
	Config.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &ST4EntityBrowserWidget::HandleOnFilterAsset);
	Config.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);
	Config.SetFilterDelegates.Add(&SetFilterDelegate);
	Config.bFocusSearchBoxWhenOpened = false;
	Config.DefaultFilterMenuExpansion = GetT4AssetCategory();

	Config.SaveSettingsName = SettingsIniSection;

	// #T4_ADD_ENTITY_TAG
	{
		TSharedPtr<FFrontendFilterCategory> MapEntityCategory = MakeShareable(
			new FFrontendFilterCategory(
				LOCTEXT("T4EntityBrowserWidgetExtraMapEntityFilters", "Map Entity Filters"), 
				LOCTEXT("T4EntityBrowserWidgetExtraMapEntityFilters_Tooltip", "Filter assets by all filters in this category.")
			)
		);
		Config.ExtraFrontendFilters.Add(MakeShareable(new FFrontendFilter_T4MapEntityAssets(MapEntityCategory)));

		TSharedPtr<FFrontendFilterCategory> ActorEntityCategory = MakeShareable(
			new FFrontendFilterCategory(
				LOCTEXT("T4EntityBrowserWidgetExtraActorEntityFilters", "Actor Entity Filters"),
				LOCTEXT("T4EntityBrowserWidgetExtraActorEntityFilters_Tooltip", "Filter assets by all filters in this category.")
			)
		);
		Config.ExtraFrontendFilters.Add(MakeShareable(new FFrontendFilter_T4CharacterEntityAssets(ActorEntityCategory)));

		TSharedPtr<FFrontendFilterCategory> PropEntityCategory = MakeShareable(
			new FFrontendFilterCategory(
				LOCTEXT("T4EntityBrowserWidgetExtraPropEntityFilters", "Prop Entity Filters"),
				LOCTEXT("T4EntityBrowserWidgetExtraPropEntityFilters_Tooltip", "Filter assets by all filters in this category.")
			)
		);
		Config.ExtraFrontendFilters.Add(MakeShareable(new FFrontendFilter_T4PropEntityAssets(PropEntityCategory)));

		TSharedPtr<FFrontendFilterCategory> CostumeEntityCategory = MakeShareable(
			new FFrontendFilterCategory(
				LOCTEXT("T4EntityBrowserWidgetExtraCostumeEntityFilters", "Costume Entity Filters"),
				LOCTEXT("T4EntityBrowserWidgetExtraCostumeEntityFilters_Tooltip", "Filter assets by all filters in this category.")
			)
		);
		Config.ExtraFrontendFilters.Add(MakeShareable(new FFrontendFilter_T4CostumeEntityAssets(CostumeEntityCategory)));

		TSharedPtr<FFrontendFilterCategory> WeaponEntityCategory = MakeShareable(
			new FFrontendFilterCategory(
				LOCTEXT("T4EntityBrowserWidgetExtraWeaponEntityFilters", "Weapon Entity Filters"),
				LOCTEXT("T4EntityBrowserWidgetExtraWeaponEntityFilters_Tooltip", "Filter assets by all filters in this category.")
			)
		);
		Config.ExtraFrontendFilters.Add(MakeShareable(new FFrontendFilter_T4WeaponEntityAssets(WeaponEntityCategory)));

		TSharedPtr<FFrontendFilterCategory> ZoneEntityCategory = MakeShareable(
			new FFrontendFilterCategory(
				LOCTEXT("T4EntityBrowserWidgetExtraZoneEntityFilters", "Zone Entity Filters"),
				LOCTEXT("T4EntityBrowserWidgetExtraZoneEntityFilters_Tooltip", "Filter assets by all filters in this category.")
			)
		);
		Config.ExtraFrontendFilters.Add(MakeShareable(new FFrontendFilter_T4ZoneEntityAssets(ZoneEntityCategory))); // #94
	}
	
	Config.OnIsAssetValidForCustomToolTip = FOnIsAssetValidForCustomToolTip::CreateLambda(
		[](const FAssetData& AssetData) 
		{
			return AssetData.IsAssetLoaded(); 
		}
	);

	// hide all asset registry columns by default (we only really want the name and path)
	TArray<UObject::FAssetRegistryTag> AssetRegistryTags;
	{
		// #T4_ADD_ENTITY_TAG
		UT4MapEntityAsset::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
		UT4CharacterEntityAsset::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
		UT4PropEntityAsset::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
		UT4CostumeEntityAsset::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
		UT4WeaponEntityAsset::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
		UT4ZoneEntityAsset::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
		for(UObject::FAssetRegistryTag& AssetRegistryTag : AssetRegistryTags)
		{
			Config.HiddenColumnNames.Add(AssetRegistryTag.Name.ToString());
		}
	}

	// Also hide the type column by default (but allow users to enable it, so don't use bShowTypeInColumnView)
	Config.HiddenColumnNames.Add(TEXT("Class"));

	this->ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				ContentBrowserModule.Get().CreateAssetPicker(Config)
			]
		]
	];

#if 0
	// Create the ignore set for asset registry tags
	// Making Skeleton to be private, and now GET_MEMBER_NAME_CHECKED doesn't work
	AssetRegistryTagsToIgnore.Add(TEXT("Skeleton"));
	AssetRegistryTagsToIgnore.Add(GET_MEMBER_NAME_CHECKED(UAnimSequenceBase, SequenceLength));
	AssetRegistryTagsToIgnore.Add(GET_MEMBER_NAME_CHECKED(UAnimSequenceBase, RateScale));
#endif
}

FReply ST4EntityBrowserWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (Commands->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void ST4EntityBrowserWidget::SelectAsset(UT4EntityAsset* InEntityAsset)
{
	FAssetData AssetData(InEntityAsset);

	if (AssetData.IsValid())
	{
		TArray<FAssetData> CurrentSelection = GetCurrentSelectionDelegate.Execute();

		if (!CurrentSelection.Contains(AssetData))
		{
			TArray<FAssetData> AssetsToSelect;
			AssetsToSelect.Add(AssetData);

			SyncToAssetsDelegate.Execute(AssetsToSelect);
		}
	}
}

bool ST4EntityBrowserWidget::HandleOnFilterAsset(const FAssetData& InAssetData) const
{
	// #T4_ADD_ENTITY_TAG
	if (InAssetData.GetClass()->IsChildOf(UT4EntityAsset::StaticClass()))
	{
		/*
		USkeleton* DesiredSkeleton = PersonaToolkitPtr.Pin()->GetSkeleton();
		if (DesiredSkeleton)
		{
			FString SkeletonString = FAssetData(DesiredSkeleton).GetExportTextName();

			return (InAssetData.TagsAndValues.FindRef(TEXT("Skeleton")) != SkeletonString);
		}
		*/
	}

	return false;
}

#undef LOCTEXT_NAMESPACE

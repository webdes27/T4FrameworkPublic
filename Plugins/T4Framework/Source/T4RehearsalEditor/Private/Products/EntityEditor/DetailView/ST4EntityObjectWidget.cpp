// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4EntityObjectWidget.h"
#include "T4EntityDetailCustomization.h"

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"

#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"
#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h"
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"
#include "T4Asset/Classes/Entity/T4PropEntityAsset.h"
#include "T4Asset/Classes/Entity/T4MapEntityAsset.h"
#include "T4Asset/Classes/Entity/T4ZoneEntityAsset.h" // #94

#include "Modules/ModuleManager.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "IDetailsView.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EntityObjectsDetails"

/**
  *
 */

void ST4EntityObjectWidget::Construct(
	const FArguments& InArgs, 
	TSharedPtr<FT4EntityViewModel> InEntityViewModel
)
{
	bUpdatingChangingPropertiesFromDetailView = false; // #54 : 이벤트에 의한 루핑 방지

	EntityViewModel = InEntityViewModel;

	// #77 : 뷰모델의 변경사항을 노티 받는다.
	EntityViewModel->GetOnViewModelChanged().AddRaw(
		this,
		&ST4EntityObjectWidget::HandleOnViewModelChanged
	);

	// #77 : Property 변경을 ViewModel 에 노티한다.
	GetOnDetailsPropertiesChanged().AddRaw(
		EntityViewModel.Get(),
		&FT4EntityViewModel::HandleOnDetailsPropertiesChanged
	);

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	// #77 : Property 변경을 노티 받는다.
	DetailsView->OnFinishedChangingProperties().AddRaw(
		this,
		&ST4EntityObjectWidget::HandleOnDetailsPropertiesChanged
	);

	UT4EntityAsset* EntityAsset = EntityViewModel->GetEntityAsset();
	if (nullptr != EntityAsset)
	{
		UStruct* SelectClass = UT4EntityAsset::StaticClass();

		// #T4_ADD_ENTITY_TAG
		const ET4EntityType EntityType = EntityAsset->GetEntityType();
		switch (EntityType)
		{
			case ET4EntityType::Map:
				SelectClass = UT4MapEntityAsset::StaticClass();
				break;

			case ET4EntityType::Character:
				SelectClass = UT4CharacterEntityAsset::StaticClass();
				break;

			case ET4EntityType::Prop:
				SelectClass = UT4PropEntityAsset::StaticClass();
				break;

			case ET4EntityType::Costume:
				SelectClass = UT4CostumeEntityAsset::StaticClass();
				break;

			case ET4EntityType::Weapon:
				SelectClass = UT4WeaponEntityAsset::StaticClass();
				break;

			case ET4EntityType::Zone: // #94
				SelectClass = UT4ZoneEntityAsset::StaticClass();
				break;

			default:
				{
					UE_LOG(
						LogT4RehearsalEditor,
						Error,
						TEXT("ST4EntityObjectWidget::Construct : Unknown EntityType '%u'"),
						uint8(EntityType)
					);
				}
				break;
		}
		DetailsView->RegisterInstancedCustomPropertyLayout(
			SelectClass,
			FOnGetDetailCustomizationInstance::CreateStatic(
				&FT4EntityDetailCustomization::MakeInstance, 
				EntityViewModel
			)
		);
		DetailsView->SetObject((UObject*)EntityAsset);
	}

	ChildSlot
	[
		DetailsView.ToSharedRef()
	];
}

void ST4EntityObjectWidget::HandleOnDetailsPropertiesChanged(
	const FPropertyChangedEvent& InEvent
) // #77
{
	if (bUpdatingChangingPropertiesFromDetailView)
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingChangingPropertiesFromDetailView, true);
	GetOnDetailsPropertiesChanged().Broadcast(InEvent.GetPropertyName());
}

void ST4EntityObjectWidget::HandleOnViewModelChanged() // #77
{
	if (bUpdatingChangingPropertiesFromDetailView)
	{
		return;
	}
	UT4EntityAsset* EntityAsset = EntityViewModel->GetEntityAsset();
	if (nullptr != EntityAsset)
	{
		DetailsView->SetObject((UObject*)EntityAsset, true);
	}
	else
	{
		DetailsView->SetObject(nullptr);
	}
}

#undef LOCTEXT_NAMESPACE // "T4EntityObjectsDetails"

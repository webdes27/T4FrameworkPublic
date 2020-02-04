// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/Conti/T4ContiAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #24
 */
const FGuid FT4ContiCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0xFEA12D33);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4ContiCustomVersion(
	FT4ContiCustomVersion::GUID,
	FT4ContiCustomVersion::LatestVersion,
	TEXT("T4ContiVer")
);

void FT4ActionCompositeData::Reset()
{
	HeaderInfoMap.Empty();
#if WITH_EDITOR
	FolderInfoMap.Empty(); // #56
#endif

	// #T4_ADD_ACTION_TAG_CONTI
	BranchActions.Empty(); // #54
	SpecialMoveActions.Empty(); // #54
	AnimationActions.Empty(); // #17
	MeshActions.Empty(); // #108
	ParticleActions.Empty(); // #20
	DecalActions.Empty(); // #54
	ReactionActions.Empty(); // #76
	PlayTagActions.Empty(); // #81
	TimeScaleActions.Empty(); // #102
	CameraWorkActions.Empty(); // #54
	CameraShakeActions.Empty(); // #101
	PostProcessActions.Empty(); // #100
	EnvironmentActions.Empty(); // #99
}

#if WITH_EDITOR
uint32 FT4ActionCompositeData::GetNewHeaderKey() const
{
	// TODO : temp Unique idx
	uint32 NewHeaderKey = HeaderInfoMap.Num();
	for (uint32 NewIdx = NewHeaderKey; NewIdx < (uint32)-1; ++NewIdx)
	{
		if (!HeaderInfoMap.Contains(NewIdx))
		{
			NewHeaderKey = NewIdx;
			break;
		}
	}
	return NewHeaderKey;
}

FT4ActionContiCommand* FT4ActionCompositeData::NewAndAddAction(
	ET4ActionType InNewActionType
)
{
	// #24
	// #T4_ADD_ACTION_TAG_CONTI
	FT4ActionContiCommand* ReturnAction = nullptr;
	uint32 AllocedHeaderKey = 0;
	int32 ChildActionArrayIndex = INDEX_NONE;
	switch (InNewActionType)
	{
		case ET4ActionType::Branch: // #54
			{
				FT4BranchAction& NewAction = BranchActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = BranchActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::SpecialMove: // #54
			{
				FT4SpecialMoveAction& NewAction = SpecialMoveActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = SpecialMoveActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::Animation: // #17
			{
				FT4AnimationAction& NewAction = AnimationActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = AnimationActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::Mesh: // #108
			{
				FT4MeshAction& NewAction = MeshActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = MeshActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::Particle: // #20
			{
				FT4ParticleAction& NewAction = ParticleActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = ParticleActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::Decal: // #54
			{
				FT4DecalAction& NewAction = DecalActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = DecalActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::Projectile: // #63
			{
				FT4ProjectileAction& NewAction = ProjectileActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = ProjectileActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::Reaction: // #76
			{
				FT4ReactionAction& NewAction = ReactionActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = ReactionActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::PlayTag: // #81
			{
				FT4PlayTagAction& NewAction = PlayTagActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = PlayTagActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::TimeScale: // #102
			{
				FT4TimeScaleAction& NewAction = TimeScaleActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = TimeScaleActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::CameraWork: // #54
			{
				FT4CameraWorkAction& NewAction = CameraWorkActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = CameraWorkActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::CameraShake: // #101
			{
				FT4CameraShakeAction& NewAction = CameraShakeActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = CameraShakeActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::PostProcess: // #100
			{
				FT4PostProcessAction& NewAction = PostProcessActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = PostProcessActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		case ET4ActionType::Environment: // #99
			{
				FT4EnvironmentAction& NewAction = EnvironmentActions.AddDefaulted_GetRef();
				AllocedHeaderKey = GetNewHeaderKey();
				ChildActionArrayIndex = EnvironmentActions.Num() - 1;
				ReturnAction = static_cast<FT4ActionContiCommand*>(&NewAction);
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown action type '%u'"),
					uint8(InNewActionType)
				);
			}
			break;
	}
	if (nullptr == ReturnAction)
	{
		return nullptr;
	}

	FT4ActionHeaderInfo NewHierarchyInfo;
	NewHierarchyInfo.ActionType = InNewActionType;
	NewHierarchyInfo.ActionArrayIndex = ChildActionArrayIndex;
	HeaderInfoMap.Add(AllocedHeaderKey, NewHierarchyInfo);
	ReturnAction->HeaderKey = AllocedHeaderKey;

	FString ActionDisplayName = FString::Printf(
		TEXT("New%s_%u"),
		*(ReturnAction->ToString()),
		ChildActionArrayIndex
	);
	ReturnAction->DisplayName = *ActionDisplayName;

	return ReturnAction;
}

template <class T>
void FT4ActionCompositeData::CopyAction(
	const FT4ActionContiCommand* InSourceAction,
	T* InOutTargetAction
) // #65
{
	const T* SourceAction = static_cast<const T*>(InSourceAction);
	check(nullptr != SourceAction);

	int32 SaveHeaderKey = InOutTargetAction->HeaderKey;

#if WITH_EDITOR
	FName SaveDisplayName = InOutTargetAction->DisplayName;
	FColor SaveDebugColorTint = InOutTargetAction->DebugColorTint;
#endif

	*InOutTargetAction = *SourceAction;

	InOutTargetAction->HeaderKey = SaveHeaderKey;

#if WITH_EDITOR
	InOutTargetAction->DisplayName = SaveDisplayName;
	InOutTargetAction->DebugColorTint = SaveDebugColorTint;
#endif
}

FT4ActionContiCommand* FT4ActionCompositeData::CloneAndAddAction(
	uint32 InSourceHeaderKey
) // #65
{
	FT4ActionContiCommand* SourceActionBase = GetActionStruct(InSourceHeaderKey);
	if (nullptr == SourceActionBase)
	{
		return nullptr;
	}
	// #T4_ADD_ACTION_TAG_CONTI
	FT4ActionContiCommand* ReturnActionBase = nullptr;
	switch (SourceActionBase->ActionType)
	{
		case ET4ActionType::Branch: // #54
			{
				FT4BranchAction* TargetAction = AddChild<FT4BranchAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4BranchAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::SpecialMove: // #54
			{
				FT4SpecialMoveAction* TargetAction = AddChild<FT4SpecialMoveAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4SpecialMoveAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::Animation: // #17
			{
				FT4AnimationAction* TargetAction = AddChild<FT4AnimationAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4AnimationAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::Mesh: // #108
			{
				FT4MeshAction* TargetAction = AddChild<FT4MeshAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4MeshAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::Particle: // #20
			{
				FT4ParticleAction* TargetAction = AddChild<FT4ParticleAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4ParticleAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::Decal: // #54
			{
				FT4DecalAction* TargetAction = AddChild<FT4DecalAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4DecalAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::Projectile: // #63
			{
				FT4ProjectileAction* TargetAction = AddChild<FT4ProjectileAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4ProjectileAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::Reaction: // #76
			{
				FT4ReactionAction* TargetAction = AddChild<FT4ReactionAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4ReactionAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::PlayTag: // #81
			{
				FT4PlayTagAction* TargetAction = AddChild<FT4PlayTagAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4PlayTagAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::TimeScale: // #102
			{
				FT4TimeScaleAction* TargetAction = AddChild<FT4TimeScaleAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4TimeScaleAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::CameraWork: // #54
			{
				FT4CameraWorkAction* TargetAction = AddChild<FT4CameraWorkAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4CameraWorkAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::CameraShake: // #101
			{
				FT4CameraShakeAction* TargetAction = AddChild<FT4CameraShakeAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4CameraShakeAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::PostProcess: // #100
			{
				FT4PostProcessAction* TargetAction = AddChild<FT4PostProcessAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4PostProcessAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		case ET4ActionType::Environment: // #99
			{
				FT4EnvironmentAction* TargetAction = AddChild<FT4EnvironmentAction>();
				check(nullptr != TargetAction);
				CopyAction<FT4EnvironmentAction>(SourceActionBase, TargetAction);
				ReturnActionBase = static_cast<FT4ActionContiCommand*>(TargetAction);
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown action type '%u'"),
					uint8(SourceActionBase->ActionType)
				);
			}
			break;
	}
	return ReturnActionBase;
}

#endif

FT4ActionContiCommand* FT4ActionCompositeData::GetActionStruct(
	uint32 InActionHeaderKey
) // #56
{
	if (!HeaderInfoMap.Contains(InActionHeaderKey))
	{
		return nullptr;
	}
	// #T4_ADD_ACTION_TAG_CONTI
	const FT4ActionHeaderInfo& HierarchyInfo = HeaderInfoMap[InActionHeaderKey];
	switch (HierarchyInfo.ActionType)
	{
		case ET4ActionType::Branch: // #54
			{
				check(HierarchyInfo.ActionArrayIndex < BranchActions.Num());
				FT4BranchAction& CurrentAction = BranchActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::SpecialMove: // #54
			{
				check(HierarchyInfo.ActionArrayIndex < SpecialMoveActions.Num());
				FT4SpecialMoveAction& CurrentAction = SpecialMoveActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::Animation: // #17
			{
				check(HierarchyInfo.ActionArrayIndex < AnimationActions.Num());
				FT4AnimationAction& CurrentAction = AnimationActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::Mesh: // #108
			{
				check(HierarchyInfo.ActionArrayIndex < MeshActions.Num());
				FT4MeshAction& CurrentAction = MeshActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::Particle: // #20
			{
				check(HierarchyInfo.ActionArrayIndex < ParticleActions.Num());
				FT4ParticleAction& CurrentAction = ParticleActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::Decal: // #54
			{
				check(HierarchyInfo.ActionArrayIndex < DecalActions.Num());
				FT4DecalAction& CurrentAction = DecalActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::Projectile: // #63
			{
				check(HierarchyInfo.ActionArrayIndex < ProjectileActions.Num());
				FT4ProjectileAction& CurrentAction = ProjectileActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::Reaction: // #76
			{
				check(HierarchyInfo.ActionArrayIndex < ReactionActions.Num());
				FT4ReactionAction& CurrentAction = ReactionActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::PlayTag: // #81
			{
				check(HierarchyInfo.ActionArrayIndex < PlayTagActions.Num());
				FT4PlayTagAction& CurrentAction = PlayTagActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::TimeScale: // #102
			{
				check(HierarchyInfo.ActionArrayIndex < TimeScaleActions.Num());
				FT4TimeScaleAction& CurrentAction = TimeScaleActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::CameraWork: // #54
			{
				check(HierarchyInfo.ActionArrayIndex < CameraWorkActions.Num());
				FT4CameraWorkAction& CurrentAction = CameraWorkActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::CameraShake: // #100
			{
				check(HierarchyInfo.ActionArrayIndex < CameraShakeActions.Num());
				FT4CameraShakeAction& CurrentAction = CameraShakeActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::PostProcess: // #100
			{
				check(HierarchyInfo.ActionArrayIndex < PostProcessActions.Num());
				FT4PostProcessAction& CurrentAction = PostProcessActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		case ET4ActionType::Environment: // #99
			{
				check(HierarchyInfo.ActionArrayIndex < EnvironmentActions.Num());
				FT4EnvironmentAction& CurrentAction = EnvironmentActions[HierarchyInfo.ActionArrayIndex];
				return static_cast<FT4ActionContiCommand*>(&CurrentAction);
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown action type '%u'"),
					uint8(HierarchyInfo.ActionType)
				);
			}
			break;
	};
	return nullptr;
}

UT4ContiAsset::UT4ContiAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TotalPlayTimeSec(T4Const_ContiMaxPlayTimeSec) // #56
#if WITH_EDITORONLY_DATA
	, ThumbnailImage(nullptr)
#endif
{
}

void UT4ContiAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4ContiCustomVersion::GUID); // only changes version if not loading
}

void UT4ContiAsset::PostLoad()
{
	Super::PostLoad();

	const int32 ContiVar = GetLinkerCustomVersion(FT4ContiCustomVersion::GUID);

#if 0
	if (ContiVar < FT4ContiCustomVersion::CommonPropertyNameChanged)
	{
		TotalPlayTimeSec = MaxPlayTimeSec_DEPRECATED; // #56
		if (0.0f >= TotalPlayTimeSec)
		{
			TotalPlayTimeSec = 10.0f;
		}
		for (TMap<uint32, FT4ActionHeaderInfo>::TConstIterator It(CompositeData.HeaderInfoMap); It; ++It)
		{
			FT4ActionContiCommand* ContiActionStruct = CompositeData.GetActionStruct(It.Key());
			check(nullptr != ContiActionStruct);
			ContiActionStruct->StartTimeSec = ContiActionStruct->DelayTimeSec_DEPRECATED;
			ContiActionStruct->LifecycleType = ContiActionStruct->LifecyclePolicy_DEPRECATED;
			if (ET4ActionType::CameraWork == ContiActionStruct->ActionType)
			{
				FT4CameraWorkAction* CameraWorkAction = static_cast<FT4CameraWorkAction*>(ContiActionStruct);
				check(nullptr != CameraWorkAction);
				for (FT4CameraWorkSectionKeyData& KeyData : CameraWorkAction->SectionData.KeyDatas)
				{
					KeyData.StartTimeSec = KeyData.DelayTimeSec_DEPRECATED;
				}
			}
		}
	}
#endif
}

#if WITH_EDITOR
void UT4ContiAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (nullptr == PropertyChangedEvent.Property) // #77
	{
		return;
	}
	if (PropertyChangedEvent.Property->HasAnyPropertyFlags(CPF_Transient))
	{
		return; // #71 : Transient Property 는 Changed 이벤트를 보내지 않도록 조치
	}
	OnPropertiesChanged().Broadcast();
}
#endif

void UT4ContiAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
#if WITH_EDITOR
	if (!PreviewEntityAsset.IsNull()) // #71
	{
		OutTags.Add(
			FAssetRegistryTag(
				TEXT("PreviewEntityAsset"),
				PreviewEntityAsset.ToString(),
				FAssetRegistryTag::TT_Hidden
			)
		);
	}
#endif
}

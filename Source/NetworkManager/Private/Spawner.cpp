// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawner.h"


#include "InventoryComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

ASpawner::ASpawner()
{
	bReplicates = true;
	
	SphereMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("SM_Sphere");
	if (IsValid(SphereMeshComponent))
	{
		SphereMeshComponent->SetMobility(EComponentMobility::Static);
		SetRootComponent(SphereMeshComponent);
		SphereMeshComponent->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
		SphereMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereMeshComponent->SetGenerateOverlapEvents(true);
		SphereMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		SphereMeshComponent->SetVisibility(true);
		SphereMeshComponent->bHiddenInGame = false;

		SphereMeshComponent->CanCharacterStepUpOn = ECB_No;
		RootComponent = SphereMeshComponent;
	}
	
	ItemsRingBuffer = TRingBuffer<UInventoryItemBaseAsset*>();
	OfferedItemName = "Empty";
	
	ItemTextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>("ItemTextRender");
	if (IsValid(ItemTextRenderComponent))
	{
		ItemTextRenderComponent->SetupAttachment(RootComponent);
		ItemTextRenderComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));
		ItemTextRenderComponent->SetText(FText::FromName(OfferedItemName));
		ItemTextRenderComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
		ItemTextRenderComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	}
}

void ASpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.Condition = COND_SkipOwner;
	Params.bIsPushBased = true;
	Params.RepNotifyCondition = REPNOTIFY_OnChanged;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OfferedItemName, Params);
	
}

// Called when the game starts or when spawned
void ASpawner::BeginPlay()
{
	Super::BeginPlay();

	OfferedItem = nullptr;
	
	/** Setup manager references */
	AssetManager = UAssetManager::GetIfValid();
	checkf(IsValid(AssetManager), TEXT("Inventory Component could not find a valid Asset Manager"));

	if (!bInitialAssetsLoaded)
	{
		const FPrimaryAssetType PrimaryAssetTypeToLoad = FPrimaryAssetType("InventoryBaseAsset");
		TArray<FPrimaryAssetId> AssetIdList;
		if (AssetManager->GetPrimaryAssetIdList(PrimaryAssetTypeToLoad, AssetIdList))
		{ LoadItemAssets(AssetIdList); }
		else
		{ UE_LOG(LogTemp, Warning, TEXT("Spawner did get a list of assets from asset manager")); }

		bInitialAssetsLoaded = true;
	}
}

void ASpawner::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsRunningClientOnly() || !IsValid(OtherActor))
	{ return; }
	
	if (UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(OtherActor->GetComponentByClass(UInventoryComponent::StaticClass())))
	{
		InventoryComponent->AddItem(FInventoryItem(OfferedItem->GetPrimaryAssetId(), OfferedItem->Quantity));
		/** Offer up a new Item
		 * Can change this to take quantity into account instead of always advancing to the next item in the collection */
		SetOfferedItem();
	}
}

void ASpawner::LoadItemAssets(const TArray<FPrimaryAssetId>& PrimaryIdCollection)
{
	if (PrimaryIdCollection.IsEmpty())
	{ return; }
	
	/** Asset Categories to load, use empty array to get all of them */
	const TArray<FName> CategoryBundles;
	/** Async Load Delegate */
	const FStreamableDelegate AssetAsyncLoadDelegate = FStreamableDelegate::CreateUObject(
		this,
		&ThisClass::OnItemAssetLoadFromAssetId,
		PrimaryIdCollection);

	/** Load assets with async load delegate */
	AssetManager->LoadPrimaryAssets(PrimaryIdCollection, CategoryBundles, AssetAsyncLoadDelegate); // TODO Investigate if we can use a 0 parameter delegate
}

void ASpawner::OnItemAssetLoadFromAssetId(const TArray<FPrimaryAssetId> PrimaryIdCollection)
{
	/** for clients we just want to have the asset manager load the items
	 * the server will take care of handling the inventory for the spawner */
	if(IsRunningClientOnly())
	{ return; }
	
	for (FPrimaryAssetId PrimaryId : PrimaryIdCollection)
	{
		if (UInventoryItemBaseAsset* InventoryAsset = Cast<UInventoryItemBaseAsset>(AssetManager->GetPrimaryAssetObject(PrimaryId)))
		{ ItemsRingBuffer.Add(InventoryAsset); }
	}
	
	if (!ItemsRingBuffer.IsEmpty())
	{
		SetOfferedItem(ItemsRingBuffer.First());
		ItemsRingBuffer.ShiftIndexToBack(0);
	}
}

void ASpawner::SetOfferedItem()
{
	OfferedItem = ItemsRingBuffer.First();
	checkf(IsValid(OfferedItem), TEXT("Spawner had a nul item when setting offered item"));
	ItemsRingBuffer.ShiftIndexToBack(0);

	/** Set the name to be displayed by the spawner on the clients */
	OfferedItemName = OfferedItem->InventoryItemName;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OfferedItemName, this);
}

void ASpawner::SetOfferedItem(UInventoryItemBaseAsset* InInventoryItem)
{
	if (OfferedItem == nullptr || OfferedItem->GetPrimaryAssetId() != InInventoryItem->GetPrimaryAssetId())
	{ OfferedItem = InInventoryItem; }
	else
	{
		/** Find something else */
		OfferedItem = ItemsRingBuffer.First();
		checkf(IsValid(OfferedItem), TEXT("Spawner had a nul item when setting offered item"));
		ItemsRingBuffer.ShiftIndexToBack(0);
	}

	/** Set the name to be displayed by the spawner on the clients */
	OfferedItemName = OfferedItem->InventoryItemName;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OfferedItemName, this);
}

void ASpawner::OnRep_OfferedItemName()
{
	if (IsRunningDedicatedServer() || !IsValid(ItemTextRenderComponent))
	{ return; }
	
	/** Change spawner text display so player can see which item is offered */
	const FText OfferedItemNameToDisplay = FText::FromName(OfferedItemName);
	ItemTextRenderComponent->SetText(OfferedItemNameToDisplay);
}


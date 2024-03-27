// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawner.h"


#include "InventoryItemBaseAsset.h"
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
	
	ItemsRingBuffer = TRingBuffer<FInventoryItem>();
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

	OfferedItem = FInventoryItem();
	
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
		InventoryComponent->AddItem(OfferedItem);
		/** Offer up a new Item
		 * Can change this to take quantity into account instead of always advancing to the next item in the collection */
		SetOfferedItem();
	}
}

void ASpawner::LoadItemAssets(const TArray<FPrimaryAssetId>& PIDs)
{
	if (PIDs.IsEmpty())
	{ return; }
	
	/** Asset Categories to load, use empty array to get all of them */
	TArray<FName> CategoryBundles;

	/** Async Load Delegate */
	const FStreamableDelegate AssetAsyncLoadDelegate = FStreamableDelegate::CreateUObject(
		this,
		&ThisClass::OnItemAssetLoadFromAssetId,
		PIDs);

	/** Load assets with async load delegate */
	AssetManager->LoadPrimaryAssets(PIDs, CategoryBundles, AssetAsyncLoadDelegate);
}

void ASpawner::OnItemAssetLoadFromAssetId(const TArray<FPrimaryAssetId> PIDs)
{
	/** for clients we just want to have the asset manager load the items
	 * the server will take care of handling the inventory for the spawner */
	if(IsRunningClientOnly())
	{ return; }
	
	for (FPrimaryAssetId PID : PIDs)
	{
		if (UInventoryItemBaseAsset* InventoryAsset = Cast<UInventoryItemBaseAsset>(AssetManager->GetPrimaryAssetObject(PID)))
		{ ItemsRingBuffer.Add(FInventoryItem(InventoryAsset, PID, InventoryAsset->Quantity)); }
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
	checkf(IsValid(OfferedItem.Item), TEXT("Spawner had a nul item when setting offered item"));
	ItemsRingBuffer.ShiftIndexToBack(0);

	/** Set the name to be displayed by the spawner on the clients */
	OfferedItemName = OfferedItem.Item->InventoryItemName;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OfferedItemName, this);
	UE_LOG(LogTemp, Warning, TEXT("Server Spawner item name: %s"), *OfferedItemName.ToString());
}

// TODO multicast and create replication for items
void ASpawner::SetOfferedItem(const FInventoryItem& InventoryItem)
{
	if (OfferedItem.Item == nullptr || OfferedItem.PID != InventoryItem.PID)
	{ OfferedItem = InventoryItem; }
	else
	{
		/** Find something else */
		OfferedItem = ItemsRingBuffer.First();
		checkf(IsValid(OfferedItem.Item), TEXT("Spawner had a nul item when setting offered item"));
		ItemsRingBuffer.ShiftIndexToBack(0);
	}

	/** Set the name to be displayed by the spawner on the clients */
	OfferedItemName = OfferedItem.Item->InventoryItemName;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, OfferedItemName, this);
	UE_LOG(LogTemp, Warning, TEXT("Server Spawner item name: %s"), *OfferedItemName.ToString());
}

void ASpawner::OnRep_OfferedItemName()
{
	if (IsRunningDedicatedServer() || !IsValid(ItemTextRenderComponent))
	{ return; }
	
	/** Change spawner text display so player can see which item is offered */
	const FText OfferedItemNameToDisplay = FText::FromName(OfferedItemName);
	UE_LOG(LogTemp, Warning, TEXT("Client Spawner item name: %s"), *OfferedItemNameToDisplay.ToString());
	ItemTextRenderComponent->SetText(OfferedItemNameToDisplay);
}


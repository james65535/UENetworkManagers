// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "Components/TextRenderComponent.h"
#include "Containers/RingBuffer.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

UCLASS()
class NETWORKMANAGER_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawner();

private:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Sphere", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SphereMeshComponent;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* ItemTextRenderComponent;

protected:
	//** Class Overrides */
	virtual void BeginPlay() override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere)
	UAssetManager* AssetManager;

	void LoadItemAssets(const TArray<FPrimaryAssetId>& PIDs);
	bool bInitialAssetsLoaded = false;

	UFUNCTION()
	void OnItemAssetLoadFromAssetId(const TArray<FPrimaryAssetId> PIDs);
	TRingBuffer<FInventoryItem> ItemsRingBuffer;

	/**
	 * @brief Gets the next item from container and sets the current offered item
	 */
	void SetOfferedItem();
	/**
	 * @brief Sets the Offered item to the specified InventoryItem
	 */
	void SetOfferedItem(const FInventoryItem& InventoryItem);
	UPROPERTY()
	FInventoryItem OfferedItem;


	UPROPERTY(ReplicatedUsing = "OnRep_OfferedItemName")
	FName OfferedItemName;
	UFUNCTION()
	void OnRep_OfferedItemName();



	
	
	
};

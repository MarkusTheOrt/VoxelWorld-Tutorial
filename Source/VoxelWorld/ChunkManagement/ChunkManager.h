// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkActor.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "../SaveGames/SaveManager.h"
#include "../SaveGames/LoadManager.h"
#include "GameFramework/Actor.h"
#include "ChunkManager.generated.h"

UCLASS()
class VOXELWORLD_API AChunkManager : public AActor
{
	GENERATED_BODY()
	
/// ------ VARIABLES ------ \\\
/// ------ Size ------ \\\


protected:
	// The size in unreal units (~cm) for every voxel.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Size", Meta = (UIMin = 1, UIMax = 512, ClampMin = 1, ClampMax = 512))
		int voxelSize = 100;

	// The width of the chunk in voxels.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Size", Meta = (UIMin = 1, UIMax = 512, ClampMin = 1, ClampMax = 512))
		int chunkWidth = 16;

	// The height of the chunk in voxels.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Size", Meta = (UIMin = 1, UIMax = 512, ClampMin = 1, ClampMax = 512))
		int chunkHight = 128;

/// ------ Voxel ------ \\\

protected:
	// A list of all voxel assets.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Voxel")
		TArray<UVoxelAsset*> AssetList;

/// ------ Default ------ \\\

	// The seed to create the chunks.
	UPROPERTY(editanywhere, BlueprintReadOnly, category = "settings|Default")
		int randomseed = 0;

	// Temporary class type to spawn the chunks
	UPROPERTY(editanywhere, BlueprintReadOnly, category = "settings|Default")
		TSubclassOf<AChunkActor> chunkClass;

	// A map of all chunks combined with their position as keys.
	UPROPERTY()
		TMap<FVector2D, AChunkActor*> chunks;

	FTimerHandle LoadThreadTimerHandle;

	FTimerHandle SaveThreadTimerHandle;

	FWorldInformation worldInfo;

	TArray<FChunkInformation> chunksList;


/// ------ FUNCTIONS ------ \\\
/// ------ Initialization ------ \\\

	// Sets default values for this actor's properties
	AChunkManager();

protected:

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

public:
#if WITH_EDITOR
	void ReshuffleAssetList();
#endif

	UFUNCTION(BlueprintNativeEvent, Category = "Generation")
		void GenerateNewWorld();
	virtual void GenerateNewWorld_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Generation")
		bool GenerateWorldFromSave(FString name);

	void CheckWorldLoadDone();

	void WorldLoadedCallback(FWorldInformation world, TArray<FChunkInformation> chunkList);

	UFUNCTION(BlueprintCallable, Category = "Update")
		void SetVoxel(FVector position, int value);

	UFUNCTION(BlueprintCallable, Category = "Update")
	void SpawnChunk(const FVector2D& position);

	void SpawnChunk(const FVector2D& position, const FChunkInformation& information);

	UFUNCTION(BlueprintCallable, Category = "Update")
		void SaveWorld();

	void CheckWorldSaveDone();

	void SaveWorldCallback(bool succeeded);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/// ------ Debug ------ \\\

protected:
	// Show a debug warning on the screen and log it.
	// Also adds additional information about the chunk in the log.
	// @param information - All lines of the warning message. The first one will be displayed with a prefix.
	// @return - VOID
	UFUNCTION(BlueprintCallable, Category = "Debug", Meta = (Keywords = "Debug, Print, Log, Warning, Screen, OutputLog"))
		void PrintDebugWarning(TArray<FString> information);

	TQueue<FVector2D> ChunksQueue;

	virtual void Tick(float DeltaSeconds) override;
};

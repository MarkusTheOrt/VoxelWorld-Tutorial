// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "../Libraries/SimplexNoiseLibrary.h"
#include "../Assets/VoxelAsset.h"
#include "GameFramework/Actor.h"
#include "ChunkActor.generated.h"

/* This struct stores all necessary information to create a procedural mesh. */
struct FVoxelMeshInformation {
	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	int elementID = 0;
};

UCLASS()
class VOXELWORLD_API AChunkActor : public AActor
{
	GENERATED_BODY()

/// ------ VARIABLES ------ \\\
/// ------ Voxel ------ \\\

protected:
	// All possible voxel assets as a list. Can't be accessed inside BluePrints.
	UPROPERTY()
		TArray<UVoxelAsset*> assetList;

	int regionSize = 16;

	// The total amount of possible voxels inside the chunk. This precalculation increases performance.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Voxel")
		int chunkTotalElements;

	// The procedural mesh component, which represents the entire chunk. Can't be accessed inside BluePrints.
	UPROPERTY()
		UProceduralMeshComponent* proceduralComponent;

public:
	UPROPERTY()
		FVector2D assignedRegion = FVector2D(0, 0);

	// The stored asset IDs of voxels inside the chunk. 
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Voxel")
		TArray<int> voxelAssetIDs;

	TArray<int> GetVoxelAssetIDs() {
		return voxelAssetIDs;
	};

	// Should this chunk be saved with the next save command. 
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Voxel")
		bool markedForSaving = false;

	// The stored asset IDs of voxels inside the chunk. 
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Voxel")
		TSet<int> voxelAssetChanged;

/// ------ Size ------ \\\

public:
	// The size in unreal units (~cm) for every single voxel.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Size")
		int voxelSize = 100;
	
	// The width of the chunk in voxels.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Size")
		int chunkWidth = 16; 

	// The height of the chunk in voxels.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Size")
		int chunkHeight = 128;

protected:
	// The size of a single voxel divided by two. This precalculation increases performance.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Size")
		int voxelSizeHalved = 8;

	// The squared value of the chunk width. This precalculation increases performance.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Size")
		int chunkWidthSquared = 256;
	
/// ------ Position ------ \\\
	
public:
	// The X index of the chunk.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Position")
		int chunkIndexX = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Position")
		int chunkIndexXOld;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Position")
		int chunkIndexYOld;

	// The Y index of the chunk.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Position")
		int chunkIndexY = 0;

protected:
	// The relative offset around the chunk center.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Position")
		int chunkOffset = -800;

/// ------ Default ------ \\\

protected:
	// The seed to create the chunk.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Default")
		int randomSeed = 0;

	// The name of the chunk.
	UPROPERTY(BlueprintReadOnly, Category = "Settings|Default")
		FName chunkName = "defaultName";

	// The default warning information added to the log.
	UPROPERTY()
		TArray<FString> defaultInformation = {"None"};



/// ------ FUNCTIONS ------ \\\
/// ------ Initialization ------ \\\

public:
	// Sets default values for this actor's properties.
	// @return - VOID
	AChunkActor();

	// Initialize the needed variables and set up the procedural mesh.
	// @param _assetList - A list of all possible voxel assets.
	// @param _voxelSize - The size of a single voxel in unreal units.
	// @param _chunkWidth - The width of the chunk in voxels.
	// @param _chunkHeight - The height of the chunk in voxels.
	// @param _position - The X and Y index of the chunk.
	// @return - VOID
	UFUNCTION(BlueprintCallable, Category = "Initialization", Meta = ( Keywords = "Init, Create, Initialize, Chunk, Set, Variables" ))
		void Initialize(TArray<UVoxelAsset*> _assetList, int _voxelSize, int _chunkWidth, int _chunkHeight, FVector2D _position, int _regionSize);

/// ------ Generation ------ \\\

public:
	// Generate the chunk for the first time or after destruction.
	// @return - Did the generation succeed?
	UFUNCTION(BlueprintCallable, Category = "Generation", Meta = ( Keywords = "generate, generation, Chunk, new, creation" ))
		bool GenerateChunk();

	// Generate the chunk for the first time or after destruction.
	// @return - Did the generation succeed?
	bool GenerateChunk(const TMap<int,int>& chunkMap);

protected:
	// Calculate the corresponding noise to the x and y position.
	// @param x - The relativ X position for the calculation.
	// @param y - The relativ Y position for the calculation.
	// @return - The noise value (height variation) of the current position.
	UFUNCTION(BlueprintNativeEvent, Category = "Generation", Meta = ( BlueprintProtected ))
		int CalculateNoiseValue(const int& x, const int& y);	
	virtual int CalculateNoiseValue_Implementation(const int& x, const int& y);

	// The distribution of the voxel over the hight.
	// @param z - The relative z position of the current voxel.
	// @param noise - The noise value (height variation) of the current voxel.
	// @return - The voxel asset ID for the current height.
	UFUNCTION(BlueprintNativeEvent, Category = "Generation", Meta = ( BlueprintProtected ))
		int VoxelAssetDistribution(const int& z, const int& noise);
	virtual int VoxelAssetDistribution_Implementation(const int& z, const int& noise);

/// ------ Chunk update ------ \\\

protected:
	// Check, if the given voxel asset ID is valid.
	// @param assetID - The asset ID that should be checked.
	// @return - Is the asset ID valid?
	bool IsValidVoxelID(int assetID);

public:
	// Replace a voxel (cube) inside the chunk.
	// @param position - A vague position indide the voxel boundaries.
	// @param voxelID - The ID of the new voxel type.
	// @return - Did the replacement succeed?
	UFUNCTION(BlueprintCallable, Category = "Update", Meta = ( Keywords = "Replace, Set, Voxel, Cube, Chunk, Update" ))
		bool ReplaceVoxel(FVector position, int value);

	// Update the procedural mesh by recalculating every verticy.
	// @return - Did the update succeed?
	UFUNCTION(BlueprintCallable, Category = "Update", Meta = ( Keywords = "Renew, New, Voxel, Cube, Chunk, Update, Mesh, Actor, Object" ))
		bool UpdateMesh();

/// ------ Debug ------ \\\

protected:
	// Show a debug warning on the screen and log it.
	// Also adds additional information about the chunk in the log.
	// @param information - All lines of the warning message. The first one will be displayed with a prefix.
	// @return - VOID
	UFUNCTION(BlueprintCallable, Category = "Debug", Meta = ( Keywords = "Debug, Print, Log, Warning, Screen, OutputLog" ))
		void PrintDebugWarning(TArray<FString> information);
};

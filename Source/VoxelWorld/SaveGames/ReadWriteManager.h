#pragma once

#include "Containers/Set.h"
#include "CoreMinimal.h"

// Forward-Declarations
class FBufferArchive;
class FMemoryReader;

// The struct that contains every necessary about a chunk for the generation.
struct FChunkInformation {

	// The position of the chunk relativ to its region.
	FVector position = FVector(0, 0, 0);

	// The map of the chunk. Contains the voxel index and the corresponding voxel type
	TMap<int, int> containedVoxel;

	// The number of voxel in this chunk.
	int numOfVoxel = 0;

	// The flag, if the chunk is compressed.
	bool bCompressed = false;

	// The voxel type that has been removed in this chunk to reduce size.
	int removedVoxel = 0;

	// The flag, if this struct contains valid information.
	bool bValidInformation = false;
};

// The struct that contains every necessary about a region for the generation.
struct FRegionInformation {

	// The position of the region relativ to the world.
	FVector2D position = FVector2D(0, 0);

	// All chunks contained in this region.
	TArray<FChunkInformation> containedChunks;

	// The number of chunks in this region.
	int numOfChunks = 0;

	// The flag, if this struct contains valid information.
	bool bValidInformation = false;
};

// The struct that contains every necessary about the world for the generation.
struct FWorldInformation {

	// The name of the world save.
	FString name = "DefaultWorld";

	// A set of all regions in the world.
	TSet<FVector2D> containedRegions;

	// The number of regions in the world.
	int numOfRegions = 0;

	// The width of each region.
	int regionWidth = 32;

	// The width of each chunk.
	int chunkWidth = 16;

	// The height of each chunk.
	int chunkHeight = 128;

	// The flag, if this struct contains valid information.
	bool bValidInformation = false;
};

// The function manger to read and write saves from and to an archive.
class ReadWriteManager {

public:
	// Stores the given FRegionInformation into the archive.
	static FBufferArchive ConvertRegionToBinary(FRegionInformation region);

	// Reads the FRegionInformation from the given archive.
	static FRegionInformation ConvertBinaryToRegion(FMemoryReader archive);

	// Stores the given FWorldInformation into the archive.
	static FBufferArchive ConvertWorldToBinary(FWorldInformation world);

	// Reads the FWorldInformation from the given archive.
	static FWorldInformation ConvertBinaryToWorld(FMemoryReader archive);

	// Receive the full path to the world save location.
	static FString GetWorldPath(FString name);

	// Receive the full path to the region save location.
	static FString GetRegionPath(FString name, FVector2D position);
};
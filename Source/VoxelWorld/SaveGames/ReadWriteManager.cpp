#include "ReadWriteManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/Paths.h"

/// ~~~~~~ FUNCTIONS ~~~~~~ \\\

FBufferArchive ReadWriteManager::ConvertRegionToBinary(FRegionInformation region) {

	// Create the empty archive
	FBufferArchive archive;

	// Store the valid information flag.
	uint8 validInformation = region.bValidInformation;
	archive << validInformation;
	if (!region.bValidInformation) return FBufferArchive();

	// Store the region position.
	int regionX = FMath::RoundToInt(region.position.X);
	int regionY = FMath::RoundToInt(region.position.Y);
	archive << regionX;
	archive << regionY;

	// Store the number of chunks in this region.
	uint8 numOfChunksLower = region.numOfChunks;
	uint8 numOfChunksUpper = region.numOfChunks >> 8;
	archive << numOfChunksLower;
	archive << numOfChunksUpper;

	// Store each chunk in the region.
	for (FChunkInformation chunk : region.containedChunks) {

		// Store the valid information flag.
		uint8 validChunkInformation = chunk.bValidInformation;
		archive << validChunkInformation;
		if (!chunk.bValidInformation) continue; 

		// Store the chunk position.
		uint8 positionX = FMath::RoundToInt(chunk.position.X);
		uint8 positionY = FMath::RoundToInt(chunk.position.Y);
		uint8 positionZ = FMath::RoundToInt(chunk.position.Z);
		archive << positionX;
		archive << positionY;
		archive << positionZ;

		// Store the the compressed flag.
		uint8 compressed = chunk.bCompressed;
		archive << compressed;

		// Store the removed voxel value.
		if (chunk.bCompressed) {
			uint8 removedVoxelValueLower = chunk.removedVoxel;
			uint8 removedVoxelValueUpper = chunk.removedVoxel >> 8;
			uint8 removedVoxelSubValue = chunk.removedVoxel >> 16;
			archive << removedVoxelValueLower;
			archive << removedVoxelValueUpper;
			archive << removedVoxelSubValue;
		}

		// Store the number of voxel in the chunk.
		uint8 numOfVoxelLower = chunk.numOfVoxel;
		uint8 numOfVoxelUpper = chunk.numOfVoxel >> 8;
		archive << numOfVoxelLower;
		archive << numOfVoxelUpper;

		// Store each voxel in the chunk.
		for (const TPair<int,int>& voxel : chunk.containedVoxel)	{

			// Store the index of the voxel.
			if (chunk.bCompressed) {
				uint8 voxelIndexLower = voxel.Key;
				uint8 voxelIndexUpper = voxel.Key >> 8;
				archive << voxelIndexLower;
				archive << voxelIndexUpper;
				if(voxel.Key > 4096)
					UE_LOG(LogTemp, Warning, TEXT("WARNING %s"), *(FString::FromInt(voxel.Key)));
			}

			// Store the value of the voxel.
			uint8 voxelValueLower = voxel.Value;
			uint8 voxelValueUpper = voxel.Value >> 8;
			uint8 voxelSubValue = voxel.Value >> 16;
			archive << voxelValueLower;
			archive << voxelValueUpper;
			archive << voxelSubValue;
		}
	}

	// Return the archive.
	return archive;
}

FRegionInformation ReadWriteManager::ConvertBinaryToRegion(FMemoryReader archive) {

	// Create an empty region information structure.
	FRegionInformation region;

	// Read the valid information flag.
	uint8 validInformation;
	archive << validInformation;
	region.bValidInformation = (bool)validInformation;
	if (!region.bValidInformation) return FRegionInformation();

	// Read the region position.
	int regionX;
	int regionY;
	archive << regionX;
	archive << regionY;
	region.position.X = regionX;
	region.position.Y = regionY;

	// Read the number of chunks in this region.
	uint8 numOfChunksLower;
	uint8 numOfChunksUpper;
	archive << numOfChunksLower;
	archive << numOfChunksUpper;
	region.numOfChunks = numOfChunksLower + (numOfChunksUpper << 8);

	// Read each chunk in the region.
	for (int i = 0; i < region.numOfChunks; i++) {

		// Create an empty chunk.
		FChunkInformation chunk;

		// Read the valid information flag.
		uint8 validChunkInformation;
		archive << validChunkInformation;
		chunk.bValidInformation = (bool)validChunkInformation;
		if (!chunk.bValidInformation) continue;

		// Read the chunk position.
		uint8 positionX;
		uint8 positionY;
		uint8 positionZ;
		archive << positionX;
		archive << positionY;
		archive << positionZ;
		chunk.position = FVector(positionX, positionY, positionZ);

		// Read the the compressed flag.
		uint8 compressed;
		archive << compressed;
		chunk.bCompressed = (bool)compressed;

		// Read the removed voxel value.
		if (chunk.bCompressed) {
			uint8 removedVoxelValueLower;
			uint8 removedVoxelValueUpper;
			uint8 removedVoxelSubValue;
			archive << removedVoxelValueLower;
			archive << removedVoxelValueUpper;
			archive << removedVoxelSubValue;
			chunk.removedVoxel = removedVoxelValueLower + (removedVoxelValueUpper << 8) + (removedVoxelSubValue << 16);
		}

		// Read the number of voxel in the chunk.
		uint8 numOfVoxelLower;
		uint8 numOfVoxelUpper;
		archive << numOfVoxelLower;
		archive << numOfVoxelUpper;
		chunk.numOfVoxel = numOfVoxelLower + (numOfVoxelUpper << 8);

		// Read each voxel in the chunk.
		for (int x = 0; x < chunk.numOfVoxel; x++) {

			// Create an empty Pait
			TPair<int, int> voxel;

			// Read the index of the voxel.
			if (chunk.bCompressed) {
				uint8 voxelIndexLower;
				uint8 voxelIndexUpper;
				archive << voxelIndexLower;
				archive << voxelIndexUpper;
				voxel.Key = voxelIndexLower + (voxelIndexUpper << 8);
			}
			else {
				voxel.Key = x;
			}

			// Read the value of the voxel.
			uint8 voxelValueLower;
			uint8 voxelValueUpper;
			uint8 voxelSubValue;
			archive << voxelValueLower;
			archive << voxelValueUpper;
			archive << voxelSubValue;
			voxel.Value = voxelValueLower + (voxelValueUpper << 8) + (voxelSubValue << 16);

			// Add the voxel to the chunk
			chunk.containedVoxel.Add(voxel);
		}

		// Add the newly created chunk to the set.
		region.containedChunks.Add(chunk);
	}

	// Return the region structure.
	return region;
}

FBufferArchive ReadWriteManager::ConvertWorldToBinary(FWorldInformation world) {

	// Create the empty archive
	FBufferArchive archive;

	// Store the valid information flag.
	uint8 validInformation = world.bValidInformation;
	archive << validInformation;
	if (!world.bValidInformation) return FBufferArchive();

	// Store the number of regions in the world.
	archive << world.numOfRegions;

	// Store all region positions.
	for (FVector2D region : world.containedRegions) {
		int regionX = region.X;
		int regionY = region.Y;
		archive << regionX;
		archive << regionY;
	}

	// Store the name.
	archive << world.name;

	// Store the width of the regions.
	uint8 regionWidth = world.regionWidth;
	archive << regionWidth;

	// Store the width of the chunks.
	uint8 chunkWidth = world.chunkWidth;
	archive << chunkWidth;

	// Store the height of the chunks.
	uint8 chunkHeight = world.chunkHeight;
	archive << chunkHeight;

	// Return the archive.
	return archive;
}

FWorldInformation ReadWriteManager::ConvertBinaryToWorld(FMemoryReader archive) {
	
	// Create an empty region information structure.
	FWorldInformation world;
	world.bValidInformation = true;

	// Read the valid information flag.
	uint8 validInformation;
	archive << validInformation;
	world.bValidInformation = (bool)validInformation;
	if (!world.bValidInformation) return FWorldInformation();

	// Read the number of regions in the world.
	archive << world.numOfRegions;

	// Read all region positions.
	for (int i = 0; i < world.numOfRegions; i++) {
		int regionX;
		int regionY;
		archive << regionX;
		archive << regionY;
		world.containedRegions.Add(FVector2D(regionX, regionY));
	}

	// Read the name.
	archive << world.name;

	// Read the width of the regions.
	uint8 regionWidth;
	archive << regionWidth;
	world.regionWidth = regionWidth;

	// Read the width of the chunks.
	uint8 chunkWidth;
	archive << chunkWidth;
	world.chunkWidth = chunkWidth;

	// Read the height of the chunks.
	uint8 chunkHeight;
	archive << chunkHeight;
	world.chunkHeight = chunkHeight;

	// Return the world structure.
	return world;
}

FString ReadWriteManager::GetWorldPath(FString name) {
	return FPaths::ProjectSavedDir() + "SaveGames\\" + name + "\\Region\\World.sav";
}

FString ReadWriteManager::GetRegionPath(FString name, FVector2D position) {
	return FPaths::ProjectSavedDir() + "SaveGames\\" + name + "\\Region\\Reg_(" + FString::FromInt(position.X) + ")-(" + FString::FromInt(position.Y) + ").sav";
}

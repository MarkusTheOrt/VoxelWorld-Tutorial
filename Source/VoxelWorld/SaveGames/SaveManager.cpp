#include "SaveManager.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Serialization/BufferArchive.h"

#include "Misc/FileHelper.h"

#include "../ChunkManagement/ChunkManager.h"
#include "../ChunkManagement/ChunkActor.h"

#include "Runtime/Core/Public/HAL/RunnableThread.h"


FSaveManager* FSaveManager::runnable = NULL;


FSaveManager::FSaveManager(FWorldInformation world, TSet<AChunkActor*> chunkList, AChunkManager * manager)
	: world(world)
	, bCompleted(false)
	, chunkList(chunkList)
	, manager(manager)
{
	thread = FRunnableThread::Create(this, TEXT("FSaveManager"), 0, TPri_SlightlyBelowNormal);
}

FSaveManager::~FSaveManager()
{
	delete thread;
	thread = NULL;
}

bool FSaveManager::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("~ Saving World"));
	return true;
}

uint32 FSaveManager::Run()
{
	FPlatformProcess::Sleep(0.03);
	while (stopTaskCounter.GetValue() == 0 && !IsFinished())
	{
		bCompleted = SaveWorld();
	}

	return 0;
}

void FSaveManager::Stop()
{
	stopTaskCounter.Increment();
}

void FSaveManager::EnsureCompletion()
{
	Stop();
	thread->WaitForCompletion();
}

void FSaveManager::Shutdown()
{

	UE_LOG(LogTemp, Warning, TEXT("~ World Saved."));
	if (runnable){
		runnable->EnsureCompletion();
		delete runnable;
		runnable = NULL;
	}

}

bool FSaveManager::IsThreadFinished()
{
	if (runnable) return runnable->IsFinished();
	return true;
}

FSaveManager * FSaveManager::JoyInit(FWorldInformation _world, TSet<AChunkActor*> _chunks, AChunkManager * _manager)
{
	if (!runnable && FPlatformProcess::SupportsMultithreading()) {
		runnable = new FSaveManager(_world, _chunks, _manager);
	}

	return runnable;
}



bool FSaveManager::SaveWorld() {

	// Check if the given information is valid.
	if (!world.bValidInformation) 
		return false;

	// Gather the region information.
	TMap<FVector2D, FRegionInformation> regionMap;
	for (AChunkActor * chunk : chunkList) {

		// Skip unnecessary chunks.
		if (!chunk->markedForSaving) continue;

		// Create the region save path.
		FVector2D regionPosition = chunk->assignedRegion;

		// Add a new region, if none exsists already.
		if (!regionMap.Contains(regionPosition)) {
			FRegionInformation region;
			region.bValidInformation = true;
			region.position = regionPosition;
			regionMap.Add(regionPosition, region);
		}

		// Add all sub chunks of this chunk to the corresponding region.
		for (FChunkInformation subChunk : GatherChunkInformation(chunk)) {
			regionMap[regionPosition].containedChunks.Add(subChunk);
		}

		// Update the number of chunks in the region.
		regionMap[regionPosition].numOfChunks = regionMap[regionPosition].containedChunks.Num();
	}

	// Save regions to 
	for (const TPair<FVector2D, FRegionInformation>& region : regionMap) {

		// Get the region position.
		FVector2D position = region.Value.position;

		// Save the region to a file.
		if (!WriteRegionToSave(ReadWriteManager::GetRegionPath(world.name, position), region.Value))
			return false;

		world.containedRegions.Add(position);
		world.numOfRegions = world.containedRegions.Num();
	}

	// Create the world save file.
	if (!WriteWorldToSave(ReadWriteManager::GetWorldPath(world.name)))
		return false;

	return true;
}

bool FSaveManager::WriteRegionToSave(const FString& filePath, FRegionInformation region) {


	// Create the uncompressed Archive
	FBufferArchive bufferArchive = ReadWriteManager::ConvertRegionToBinary(region);

	// Check if the creation was successful.
	if (bufferArchive.Num() <= 0) 
		return false;

	// Compress the Archive
	TArray<uint8> compressedArchive;
	FArchiveSaveCompressedProxy  compressor = FArchiveSaveCompressedProxy(compressedArchive, "ZLib");

	// Check if the compressor is valid.
	if (compressor.GetError())
		return false;

	// Store the archive in the compressor.
	compressor << bufferArchive;
	compressor.Flush();

	// Try to save the data to the given file path
	if (FFileHelper::SaveArrayToFile(compressedArchive, *filePath)) {

		// Success
		// Free compressed data
		compressor.FlushCache();
		compressedArchive.Empty();

		// Free uncompressed data
		bufferArchive.FlushCache();
		bufferArchive.Empty();
		bufferArchive.Close();

		return true;
	}
	else {

		// Failur
		// Free compressed data
		compressor.FlushCache();
		compressedArchive.Empty();

		// Free uncompressed data
		bufferArchive.FlushCache();
		bufferArchive.Empty();
		bufferArchive.Close();

		return false;
	}
}

bool FSaveManager::WriteWorldToSave(const FString& filePath) {

	// Create the uncompressed Archive
	FBufferArchive bufferArchive = ReadWriteManager::ConvertWorldToBinary(world);

	// Check if the creation was successful.
	if (bufferArchive.Num() <= 0)
		return false;

	// Compress the Archive
	TArray<uint8> compressedArchive;
	FArchiveSaveCompressedProxy  compressor = FArchiveSaveCompressedProxy(compressedArchive, "ZLib");
	compressor << bufferArchive;
	compressor.Flush();

	// Try to save the data to the given file path
	if (FFileHelper::SaveArrayToFile(compressedArchive, *filePath)) {

		// Success
		// Free compressed data
		compressor.FlushCache();
		compressedArchive.Empty();

		// Free uncompressed data
		bufferArchive.FlushCache();
		bufferArchive.Empty();
		bufferArchive.Close();

		return true;
	}
	else {

		// Failur
		// Free compressed data
		compressor.FlushCache();
		compressedArchive.Empty();

		// Free uncompressed data
		bufferArchive.FlushCache();
		bufferArchive.Empty();
		bufferArchive.Close();

		return false;
	}
}

TArray<FChunkInformation> FSaveManager::GatherChunkInformation(AChunkActor * chunkActor) {

	// Check if the fiven chunk is valid.
	if (!chunkActor) return TArray<FChunkInformation>();

	// Initialize an empty chunk.
	TArray<FChunkInformation> chunk;

	// Split the chunk into multiple sub chunks.
	int numOfVerticalSplits = chunkActor->chunkHeight / chunkActor->chunkWidth;
	int numOfVoxel = chunkActor->voxelAssetIDs.Num() / numOfVerticalSplits;
	for (int s = 0; s < numOfVerticalSplits; s++) {

		// Initialize the information for this sub chunk.
		TArray<int> numOfVoxelValues;
		FChunkInformation subChunk;
		int lowerCap = (chunkActor->voxelAssetIDs.Num() * s) / numOfVerticalSplits;
		int upperCap = (chunkActor->voxelAssetIDs.Num() * (s + 1)) / numOfVerticalSplits;

		// Add the asset values of the chunk to the sub chunk.
		for (int i = lowerCap; i < upperCap; i++) {
			int voxelValue = chunkActor->voxelAssetIDs[i];
			subChunk.containedVoxel.Add(i - lowerCap, voxelValue);

			if (numOfVoxelValues.IsValidIndex(voxelValue))
				numOfVoxelValues[voxelValue] ++;
			else {
				numOfVoxelValues.SetNumZeroed(voxelValue + 1, false);
				numOfVoxelValues[voxelValue] ++;
			}
		}

		// Calculate the voxel value which is represented the most.
		int maxValue = FMath::Max<int>(numOfVoxelValues, &subChunk.removedVoxel);

		// Hard coded line at which a removed voxel is worse in size than just storing everything.
		// With 3 Bytes uncompressed vs 5 Bytes compressed this line is at 40% of a single voxel type.
		if (maxValue > (numOfVoxel * 0.4)) {
			subChunk.bCompressed = true;
			for (int i = 0; i < maxValue; i++) {
				if (subChunk.containedVoxel[i] == subChunk.removedVoxel) {
					subChunk.containedVoxel.Remove(i);
				}
			}
		}

		// Add the remaining information about this sub chunk.
		subChunk.numOfVoxel = subChunk.containedVoxel.Num();
		subChunk.position = FVector(chunkActor->chunkIndexX, chunkActor->chunkIndexY, s);
		subChunk.bValidInformation = true;

		// Add this sub chunk to the pool of other sub chunks.
		chunk.Add(subChunk);
	}

	// Return all sub chunks.
	return chunk;
}



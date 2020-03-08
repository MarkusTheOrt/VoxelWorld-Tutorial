#include "LoadManager.h"

#include "Serialization/ArchiveLoadCompressedProxy.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include "../ChunkManagement/ChunkManager.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"


FLoadManager* FLoadManager::runnable = NULL;

FLoadManager::FLoadManager(FString name, FWorldInformation& _world, TArray<FChunkInformation>& _chunks)
	: bCompleted(false), name(name)
{

	world = &_world;
	chunks = &_chunks;

	thread = FRunnableThread::Create(this, TEXT("FLoadManager"), 0, TPri_Normal);
}

FLoadManager::~FLoadManager()
{
	delete thread;
	thread = NULL;
}

bool FLoadManager::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("~ Loading World \"%s\""), *name);
	return true;
}

uint32 FLoadManager::Run()
{
	FPlatformProcess::Sleep(0.03);

	while (stopTaskCounter.GetValue() == 0 && !IsFinished()) {
		bCompleted = LoadWorld();
		UE_LOG(LogTemp, Warning, TEXT("Loaded Segments: %d"), chunks->Num());
	}

	return 0;
}

void FLoadManager::Stop()
{
	stopTaskCounter.Increment();
}

void FLoadManager::EnsureCompletion()
{
	Stop();
	thread->WaitForCompletion();
}

void FLoadManager::Shutdown()
{
	UE_LOG(LogTemp, Warning, TEXT("~ World Loaded."));
	if (runnable)
	{
		runnable->EnsureCompletion();
		delete runnable;
		runnable = NULL;
	}
}

bool FLoadManager::IsThreadFinished()
{
	if (runnable) return runnable->IsFinished();
	return true;
}

FLoadManager * FLoadManager::JoyInit(FString name, FWorldInformation & _world, TArray<FChunkInformation>& _chunks)
{
	if (!runnable && FPlatformProcess::SupportsMultithreading())
	{
		runnable = new FLoadManager(name, _world, _chunks);
	}

	return runnable;
}


bool FLoadManager::LoadWorld() {

	// Create the path to the world save.
	FString worldPath = ReadWriteManager::GetWorldPath(name);

	// Does the file exist.
	if (!FPaths::FileExists(worldPath)) 
		return false;

	// Try to read the world save file.
	if (!ReadWorldFromSave(worldPath))
		return false;

	UE_LOG(LogTemp, Warning, TEXT("Found World File \"%s\""), *worldPath);

	if (world)
	{
		
		UE_LOG(LogTemp, Warning, TEXT("World Ptr IsGuud, length: %d"), world->containedRegions.Num());
	}

	// Try to read all region save files.
	TArray<FRegionInformation> regionList;
	for (FVector2D regionPosition : world->containedRegions) {

		// Create the path to the region save.
		FString regionPath = ReadWriteManager::GetRegionPath(name, regionPosition);
		FRegionInformation region;


		// Does the file exist.
		if (!FPaths::FileExists(regionPath))
			return false;


		// Try to read the region save files.
		if (!ReadRegionFromSave(regionPath, region))
			return false;

		UE_LOG(LogTemp, Warning, TEXT("Loading Region: %s"), *regionPath);

		// Add the region information to the list.
		regionList.Add(region);
	}

	// Calculate the size of each sub chunk.
	int chunkWidth = world->chunkWidth;
	int chunkSize = chunkWidth * chunkWidth *chunkWidth;

	TMap<FVector2D, FChunkInformation> chunkMap;
	// Check every chunk in all regions.
	for (FRegionInformation region : regionList) {

		int verticalSplit = 0;
		FChunkInformation nextChunk = FChunkInformation();

		for (FChunkInformation chunk : region.containedChunks) {

			// Rearange the voxel indices and values of the chunk.
			for (int i = 0; i < chunkSize; i++) {

				// Replace missing values.
				if (chunk.bCompressed) {
					if (!chunk.containedVoxel.Contains(i))
						chunk.containedVoxel.Add(i, chunk.removedVoxel);
				}

				// Shift the indices according to the Z position.
				int index = i + chunk.position.Z * chunkSize;
				if (i != index) {
					chunk.containedVoxel.Add(index, chunk.containedVoxel[i]);
					chunk.containedVoxel.Remove(i);
				}
			}

			nextChunk.containedVoxel.Append(chunk.containedVoxel);

			if (verticalSplit == 7) {
				verticalSplit = 0;
				nextChunk.bValidInformation = true;
				nextChunk.position = FVector(chunk.position.X, chunk.position.Y, 0);
				chunks->Add(nextChunk);
				nextChunk = FChunkInformation();
			}
			else {
				verticalSplit++;
			}
		}
	}
	return true;

}

bool FLoadManager::ReadRegionFromSave(const FString & filePath, FRegionInformation& region) {

	// Create a compressed archive.
	TArray<uint8> compressedArchive;

	// Try to load the data to the compressed archive.
	if (!FFileHelper::LoadFileToArray(compressedArchive, *filePath)) 
		return false;
	
	// Try to decompress the data.
	//FArchiveLoadCompressedProxy decompressor = FArchiveLoadCompressedProxy(compressedArchive, ECompressionFlags::COMPRESS_ZLIB);
	FArchiveLoadCompressedProxy decompressor = FArchiveLoadCompressedProxy(compressedArchive, "ZLib");
	if (decompressor.GetError()) 
		return false;

	// Create the uncompressed Archive
	FBufferArchive bufferArchive;
	decompressor << bufferArchive;

	// Try to read the uncompressed data into information.
	FMemoryReader readerArchive = FMemoryReader(bufferArchive, true);
	readerArchive.Seek(0);
	region = ReadWriteManager::ConvertBinaryToRegion(readerArchive);

	// Free compressed data
	compressedArchive.Empty();
	decompressor.FlushCache();
	readerArchive.FlushCache();

	// Free uncompressed data
	bufferArchive.Empty();
	bufferArchive.Close();

	// Check if the received information is valid.
	if (!region.bValidInformation)
		return false;
	return true;
}

bool FLoadManager::ReadWorldFromSave(const FString & filePath) {

	
	// Create a compressed archive.
	TArray<uint8> compressedArchive;

	// Try to load the data to the compressed archive.
	if (!FFileHelper::LoadFileToArray(compressedArchive, *filePath))
		return false;

	// Try to decompress the data.
	FArchiveLoadCompressedProxy decompressor = FArchiveLoadCompressedProxy(compressedArchive, "ZLib");
	if (decompressor.GetError())
		return false;

	// Create the uncompressed Archive
	FBufferArchive bufferArchive;
	decompressor << bufferArchive;

	// Try to read the uncompressed data into information.
	FMemoryReader readerArchive = FMemoryReader(bufferArchive, true);
	readerArchive.Seek(0);
	FWorldInformation worldInf = ReadWriteManager::ConvertBinaryToWorld(readerArchive);
	

	world->bValidInformation = worldInf.bValidInformation;
	world->chunkHeight = worldInf.chunkHeight;
	world->chunkWidth = worldInf.chunkWidth;
	world->containedRegions = worldInf.containedRegions;
	world->name = worldInf.name;
	world->numOfRegions = worldInf.numOfRegions;
	world->regionWidth = worldInf.regionWidth;

	// Free compressed data
	compressedArchive.Empty();
	decompressor.FlushCache();
	readerArchive.FlushCache();

	// Free uncompressed data
	bufferArchive.Empty();
	bufferArchive.Close();

	// Check if the received information is valid.
	if (!world->bValidInformation)
		return false;
	return true;
}


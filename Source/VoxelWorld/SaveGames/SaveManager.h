#pragma once

#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "ReadWriteManager.h"
#include "CoreMinimal.h"

// Forward-Declarations
class AChunkManager;
class AChunkActor;

// This save manager will save the given information into files.
class FSaveManager : public FRunnable {

	static FSaveManager* runnable;

	FRunnableThread* thread;

	FThreadSafeCounter stopTaskCounter;

public:

	
	bool bCompleted;

	// The world information.
	FWorldInformation world;

	// The chunks to save.
	TSet<AChunkActor*> chunkList;

	// The chunk manager to call the return function.
	AChunkManager* manager;


public:

	// The default constructor. Also sets the internal variables.
	FSaveManager(FWorldInformation world, TSet<AChunkActor*> chunkList, AChunkManager* manager);
	virtual ~FSaveManager();

	bool IsFinished() const {
		return bCompleted;
	}

	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	void EnsureCompletion();

	static void Shutdown();

	static bool IsThreadFinished();

	static FSaveManager* JoyInit(FWorldInformation _world, TSet<AChunkActor*> _chunks, AChunkManager* _manager);
	
	// Save the given chunks to a save file.
	bool SaveWorld();

	// Write the given region to a save file.
	bool WriteRegionToSave(const FString& filePath, FRegionInformation region);

	// Write the given world information to a save file.
	bool WriteWorldToSave(const FString& filePath);

	// Breack a chunk actor into its information.
	TArray<FChunkInformation> GatherChunkInformation(AChunkActor* chunkActor);


};

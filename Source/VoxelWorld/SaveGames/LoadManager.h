#pragma once

#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "ReadWriteManager.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "CoreMinimal.h"

// Forward-Declarations
class AChunkManager;
class AChunkActor;

// This load manager will load all information from the corresponding files.
class FLoadManager : public FRunnable {
	
	static FLoadManager* runnable;

	FRunnableThread* thread;

	FThreadSafeCounter stopTaskCounter;

public:

	FWorldInformation* world;

	TArray<FChunkInformation>* chunks;

	bool bCompleted;

	// The given world name.
	FString name;

	// The chunk manager to call the return function.
	AChunkManager* manager;

public:

	// The default constructor. Also sets the internal variables.
	FLoadManager(FString name, FWorldInformation& _world, TArray<FChunkInformation>& _chunks);
	virtual ~FLoadManager();

	bool IsFinished() const {
		return bCompleted;
	}

	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	void EnsureCompletion();

	static void Shutdown();

	static bool IsThreadFinished();

	static FLoadManager* JoyInit(FString name, FWorldInformation& _world, TArray<FChunkInformation>& _chunks);

	// Load the world with the given name.
	bool LoadWorld();

	// Read a region from a save file.
	bool ReadRegionFromSave(const FString& filePath, FRegionInformation& region);

	// Read the world information from a save file.
	bool ReadWorldFromSave(const FString& filePath);




};

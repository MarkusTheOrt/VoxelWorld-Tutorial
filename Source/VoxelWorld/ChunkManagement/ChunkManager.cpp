// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Editor/EditorStyle/Public/EditorStyleSet.h"
#include "Runtime/Engine/Public/TimerManager.h"


// Sets default values
AChunkManager::AChunkManager() {
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

#define LOCTEXT_NAMESPACE "FChunkManager"


// Called when the game starts or when spawned
void AChunkManager::BeginPlay() {

	//GenerateWorldFromSave("DefaultWorld");
	GenerateNewWorld();

	Super::BeginPlay();
}

#if WITH_EDITORONLY_DATA

void AChunkManager::ReshuffleAssetList()
{
	TArray<UVoxelAsset*> workingArray;
	workingArray.SetNumUninitialized(65536);
	int highestID = -1;

	for (int i = 0; i < AssetList.Num(); i++)
	{
		if (AssetList[i]) {
			int assetID = AssetList[i]->assetID;

			if (highestID < assetID)
				highestID = assetID;



			workingArray[assetID] = AssetList[i];
		}

	}
	AssetList.Empty();
	AssetList.SetNum(highestID + 1);
	for (int j = 0; j < workingArray.Num(); j++)
	{

		if (j < AssetList.Num())
			AssetList[j] = workingArray[j];
	}
	workingArray.Empty();

	FNotificationInfo Info(LOCTEXT("ListRefreshed", "VoxelType List Recalculated!"));
	//Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = 1.5f;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	NotificationItem->ExpireAndFadeout();

}

#endif

void AChunkManager::GenerateNewWorld_Implementation(){
	if (!IsValid(chunkClass)) return;
	for (int x = -5; x <= 5; x++) {
		for (int y = -5; y <= 5; y++) {
			SpawnChunk(FVector2D(x, y));
		}
	}
}



bool AChunkManager::GenerateWorldFromSave(FString name) {
	if (!IsValid(chunkClass)) return false;

	//(new FAutoDeleteAsyncTask<FLoadManager>(name, this))->StartBackgroundTask();

	
	name = "DefaultWorld";
	FLoadManager::JoyInit(name, worldInfo, chunksList);
	GetWorldTimerManager().SetTimer(LoadThreadTimerHandle, this, &AChunkManager::CheckWorldLoadDone, 1, true);
	return true;


}

void AChunkManager::CheckWorldLoadDone()
{
	if (FLoadManager::IsThreadFinished()) {
		WorldLoadedCallback(worldInfo, chunksList);
		GetWorldTimerManager().ClearTimer(LoadThreadTimerHandle);
	}
}

void AChunkManager::WorldLoadedCallback(FWorldInformation world, TArray<FChunkInformation> chunkList) {


	UE_LOG(LogTemp, Warning, TEXT("WARNING Invalid value:%s"), *(FString::FromInt(chunkList.Num())));
	for (FChunkInformation chunk : chunkList) {
		SpawnChunk(FVector2D(chunk.position), chunk);
	}
	FLoadManager::Shutdown();
	return;
}

void AChunkManager::SetVoxel(FVector position, int value){
	FVector chunkPosition = position / (voxelSize * chunkWidth);
	AChunkActor* selectedChunk = chunks.FindRef(FVector2D(FMath::RoundToInt(chunkPosition.X), FMath::RoundToInt(chunkPosition.Y)));

	if (!selectedChunk) return;

	selectedChunk->ReplaceVoxel(position, value);
}

void AChunkManager::SpawnChunk(const FVector2D& position)
{
	AChunkActor* chunk = GetWorld()->SpawnActorDeferred<AChunkActor>(
		chunkClass,
		FTransform(FVector(position.X * voxelSize * chunkWidth, position.Y * voxelSize * chunkWidth, 0)),
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
	chunk->Initialize(AssetList, voxelSize, chunkWidth, chunkHight, FVector2D(position.X, position.Y), 16);
	chunks.Add(FVector2D(position.X, position.Y), chunk);
	chunk->FinishSpawning(FTransform(FVector(position.X * voxelSize * chunkWidth, position.Y * voxelSize * chunkWidth, 0)), true, nullptr);
	chunk->GenerateChunk();
}

void AChunkManager::SpawnChunk(const FVector2D& position, const FChunkInformation& information)
{

	int chunkIndexX = FMath::RoundToInt(position.X);
	int chunkIndexY = FMath::RoundToInt(position.Y);


	int indexX = chunkIndexX;
	if (indexX < 0)
		indexX = indexX - 16 + 1;

	int indexY = chunkIndexY;
	if (indexY < 0)
		indexY = indexY - 16 + 1;

	FVector2D assignedRegion = FVector2D(indexX / 16, indexY / 16);

	int sign = FMath::Sign(assignedRegion.X);
	if (sign == 0) sign = 1;
	chunkIndexX = (sign * chunkIndexX) % 16;

	sign = FMath::Sign(assignedRegion.Y);
	if (sign == 0) sign = 1;
	chunkIndexY = (sign * chunkIndexY) % 16;


	AChunkActor* chunk = GetWorld()->SpawnActorDeferred<AChunkActor>(
		chunkClass,
		FTransform(FVector((chunkIndexX + 16 * assignedRegion.X) * voxelSize * chunkWidth, (chunkIndexY + 16 * assignedRegion.Y) * voxelSize * chunkWidth, 0)),
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
	chunk->Initialize(AssetList, voxelSize, chunkWidth, chunkHight, FVector2D(position.X, position.Y), 16);
	chunks.Add(FVector2D(position.X, position.Y), chunk);
	chunk->FinishSpawning(FTransform(FVector((chunkIndexX + 16 * assignedRegion.X) * voxelSize * chunkWidth, (chunkIndexY + 16 * assignedRegion.Y) * voxelSize * chunkWidth, 0)), true, nullptr);
	chunk->GenerateChunk(information.containedVoxel);
}

void AChunkManager::SaveWorld() {

	FWorldInformation information = FWorldInformation();
	information.bValidInformation = true;
	information.chunkHeight = chunkHight;
	information.chunkWidth = chunkWidth;
	information.regionWidth = 16;
	TSet<AChunkActor*> chunkActor;

	for (const TPair<FVector2D, AChunkActor*>& pair : chunks) {
		chunkActor.Add(pair.Value);
	}

	FSaveManager::JoyInit(information, chunkActor, this);
	GetWorldTimerManager().SetTimer(SaveThreadTimerHandle, this, &AChunkManager::CheckWorldSaveDone, 1, true);

	
}

void AChunkManager::CheckWorldSaveDone()
{
	if (FSaveManager::IsThreadFinished()) {
		SaveWorldCallback(true);
		GetWorldTimerManager().ClearTimer(SaveThreadTimerHandle);
	}
}

void AChunkManager::SaveWorldCallback(bool succeeded) {

	for (const TPair<FVector2D, AChunkActor*>& pair : chunks) {
		pair.Value->markedForSaving = false;
		pair.Value->voxelAssetChanged = TSet<int>();
	}


	FSaveManager::Shutdown();

}

void AChunkManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FLoadManager::Shutdown();
	FSaveManager::Shutdown();
	Super::EndPlay(EndPlayReason);
}

/// ------ Debug ------ \\\

void AChunkManager::PrintDebugWarning(TArray<FString> information) {

	// Print and log the warning
	UE_LOG(LogTemp, Warning, TEXT("WARNING - %s"), *(information[0]));

	// Print and log any information
	for (int i = 1; i < information.Num(); i++) {
		UE_LOG(LogTemp, Warning, TEXT("|-> %s"), *(information[i]));
	}
}

void AChunkManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector2D Ref;
	
	

}


#undef LOCTEXT_NAMESPACE
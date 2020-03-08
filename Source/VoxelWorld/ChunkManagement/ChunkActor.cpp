// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkActor.h"

#pragma region Voxel Values
const int bTriangles[] = { 2,1,0,0,3,2 };
const FVector2D bUVs[] = { FVector2D(0,0), FVector2D(0,1), FVector2D(1,1), FVector2D(1,0) };
const FVector bNormals0[] = { FVector(0,0,1), FVector(0,0,1), FVector(0,0,1), FVector(0,0,1) };
const FVector bNormals1[] = { FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1), FVector(0,0,-1) };
const FVector bNormals2[] = { FVector(0,1,0), FVector(0,1,0), FVector(0,1,0), FVector(0,1,0) };
const FVector bNormals3[] = { FVector(0,-1,0), FVector(0,-1,0), FVector(0,-1,0), FVector(0,-1,0) };
const FVector bNormals4[] = { FVector(1,0,0), FVector(1,0,0), FVector(1,0,0), FVector(1,0,0) };
const FVector bNormals5[] = { FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0), FVector(-1,0,0) };
const FVector bMask[] = { FVector(0,0,1), FVector(0,0,-1), FVector(0,1,0), FVector(0,-1,0), FVector(1,0,0), FVector(-1,0,0) };
#pragma endregion



/// ------ FUNCTIONS ------ \\\
/// ------ Initialization ------ \\\

AChunkActor::AChunkActor() {

	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = false;
}

void AChunkActor::Initialize(TArray<UVoxelAsset*> _assetList, int _voxelSize, int _chunkWidth, int _chunkHeight, FVector2D _position, int _regionSize) {

	// Set the default variables
	assetList = _assetList;
	voxelSize = _voxelSize;
	chunkWidth = _chunkWidth;
	chunkHeight = _chunkHeight;
	regionSize = _regionSize;
	chunkIndexX = FMath::RoundToInt(_position.X);
	chunkIndexY = FMath::RoundToInt(_position.Y);
	chunkIndexXOld = chunkIndexX;
	chunkIndexYOld = chunkIndexY;

	// Set related variables
	// They exist mostly to save performance
	chunkTotalElements = chunkWidth * chunkWidth * chunkHeight;
	voxelSizeHalved = voxelSize / 2;
	chunkWidthSquared = chunkWidth * chunkWidth;
	chunkOffset = -chunkWidth / 2 * voxelSize;
	voxelAssetIDs.SetNumUninitialized(chunkTotalElements);


	int indexX = chunkIndexX;
	if (indexX < 0)
		indexX = indexX - regionSize + 1;

	int indexY = chunkIndexY;
	if (indexY < 0)
		indexY = indexY - regionSize + 1;

	assignedRegion = FVector2D(indexX / regionSize, indexY / regionSize);

	int sign = FMath::Sign(assignedRegion.X);
	if (sign == 0) sign = 1;
	chunkIndexX = (sign * chunkIndexX) % regionSize;

	sign = FMath::Sign(assignedRegion.Y);
	if (sign == 0) sign = 1;
	chunkIndexX = (sign * chunkIndexY) % regionSize;


	// Set the name of the chunk
	FString string = "Chunk_(" + FString::FromInt(chunkIndexX) + ")_(" + FString::FromInt(chunkIndexY) + ")_Reg" + FString::FromInt(chunkIndexX) + FString::FromInt(chunkIndexY);
	chunkName = FName(*string);

	// Set default chunk Information
	defaultInformation = {
		"Actor name: " + this->GetName(),
		"Position X: " + FString::FromInt(chunkIndexX),
		"Position Y: " + FString::FromInt(chunkIndexY),
		"Number of voxels: " + FString::FromInt(chunkTotalElements)
		};




}

/// ------ Generation ------ \\\

/* Generate the chunk with it's noise and voxels. */
bool AChunkActor::GenerateChunk(const TMap<int,int>& chunkMap) {

	// Setup the chunk internally
	proceduralComponent = NewObject<UProceduralMeshComponent>(this, chunkName);
	FTransform transform = RootComponent->GetComponentTransform();
	proceduralComponent->RegisterComponent();
	proceduralComponent->SetMobility(EComponentMobility::Static);
	RootComponent = proceduralComponent;
	RootComponent->SetWorldTransform(transform);

	for (const TPair<int, int>& voxel : chunkMap) {
		if (!voxelAssetIDs.IsValidIndex(voxel.Key)) {
			voxelAssetIDs.SetNumZeroed(voxel.Key + 1, false);
		}
		voxelAssetIDs[voxel.Key] = voxel.Value;
	}

	// Try to update the procedural mesh.
	if (!UpdateMesh()) {

		// Aboard the generation, if the mesh couldn't been updated.
		PrintDebugWarning({
			"Generation of chunk failed.",
			"Reason: Mesh couldn't been updated!"
			});
		return false;
	}
	else
		return true;
}

/* Generate the chunk with it's noise and voxels. */
bool AChunkActor::GenerateChunk() {

	// Setup the chunk internally
	proceduralComponent = NewObject<UProceduralMeshComponent>(this, chunkName);
	FTransform transform = RootComponent->GetComponentTransform();
	proceduralComponent->RegisterComponent();
	RootComponent = proceduralComponent;
	RootComponent->SetWorldTransform(transform);
	markedForSaving = true;

	// Initialize the noise Array.
	TArray<int> noise;
	noise.SetNum(chunkWidthSquared);

	// Set the noise value of each position.
	for (int x = 0; x < chunkWidth; x++) {
	for (int y = 0; y < chunkWidth; y++) {
		noise[x + y * chunkWidth] = CalculateNoiseValue(x, y);
	}
	}

	// Calculate the ID of very voxel inside the chunk.
	for (int x = 0; x < chunkWidth; x++) {
	for (int y = 0; y < chunkWidth; y++) {
	for (int z = 0; z < chunkHeight; z++) {

		// Calculate the index and corresponding asset ID.
		int index = x + y * chunkWidth + z * chunkWidthSquared;
		int voxelAssetID = VoxelAssetDistribution(z, noise[x + y * chunkWidth]);
		voxelAssetChanged.Add(index);

		// Set the calculated asset ID.
		if (IsValidVoxelID(voxelAssetID)) {
			voxelAssetIDs[index] = voxelAssetID;
		}
		else {
			voxelAssetIDs[index] = 0;
			PrintDebugWarning({
				"Couldn't set the voxel ID.",
				"Reason: Voxel ID is not vaild!",
				"Replaced the ID with 0.",
				"Given voxel ID: " + FString::FromInt(voxelAssetID),
				"Voxel Index: " + FString::FromInt(index),
				"X Position: " + FString::FromInt(x),
				"Y Position: " + FString::FromInt(y),
				"Z Position: " + FString::FromInt(z)
				});
		}
	}
	}
	}

	// Try to update the procedural mesh.
	if (!UpdateMesh()) {

		// Aboard the generation, if the mesh couldn't been updated.
		PrintDebugWarning({
			"Generation of chunk failed.",
			"Reason: Mesh couldn't been updated!"
			});
		return false;
	}
	else
		return true;
}

int AChunkActor::CalculateNoiseValue_Implementation(const int& x, const int& y) {

	// Just return 0 for implementation
	return 0;
}

int AChunkActor::VoxelAssetDistribution_Implementation(const int& z, const int& noise) {

	// Create one layer of grass, two of dirt and the rest of stone
	if (z == 30 + noise)
		return 2;
	else if (28 + noise <= z && z < 30 + noise)
		return 1;
	else if (z < 28 + noise)
		return 3;
	else
		return 0;
}

/// ------ Chunk update ------ \\\

bool AChunkActor::IsValidVoxelID(int assetID) {

	// ID means empty. This is always valid.
	if (assetID == 0) return true;

	// For every other ID check, if a reference in the asset list exists.
	if (assetList.IsValidIndex(assetID))
		if (assetList[assetID])
			return true;
	return false;
}

bool AChunkActor::ReplaceVoxel(FVector position, int voxelID) {

	// Check if the given ID is valid.
	if (!IsValidVoxelID(voxelID)) {
		PrintDebugWarning({ 
			"Aborted replacement of voxel.",
			"Reason: Voxel ID is not vaild!",
			"Given voxel ID: " + FString::FromInt(voxelID) 
			});
		return false;
	}

	// Calculate the index of the voxel that will be changed.
	FVector adjustedPosition = position + FVector(chunkOffset - voxelSizeHalved, chunkOffset + voxelSizeHalved, voxelSize) - this->GetActorLocation() ;
	int x = FMath::RoundToInt(adjustedPosition.X / voxelSize);
	int y = FMath::RoundToInt(adjustedPosition.Y / voxelSize);
	int z = FMath::RoundToInt(adjustedPosition.Z / voxelSize);
	int index = x + y * chunkWidth + z * chunkWidthSquared;

	// Replace the voxel ID with the new one.
	int voxelIDold = voxelAssetIDs[index];
	voxelAssetIDs[index] = voxelID;

	// Try to update the procedural mesh.
	if (!UpdateMesh()) {

		// Set the old ID, if the updated failed.
		voxelAssetIDs[index] = voxelIDold;
		PrintDebugWarning({ 
			"Aborted replacement of voxel.",
			"Reason: Mesh couldn't been updated!",
			"Voxel Index: " + FString::FromInt(index),
			"New voxel ID: " + FString::FromInt(voxelID),
			"Previous voxel ID: " + FString::FromInt(voxelIDold) 
			});
		return false;
	}

	markedForSaving = true;
	voxelAssetChanged.Add(index);
	return true;
}

bool AChunkActor::UpdateMesh() {

	// Check, if there is at least one valid voxel asset.
	if (assetList.Num() < 1) {
		PrintDebugWarning({
			"Aboarded mesh update.",
			"Reason: No available voxel assets!"
			});
		return false;
	}

	// Initialize the needed amount of voxel assets.
	TArray<FVoxelMeshInformation> voxelMeshInformation;
	voxelMeshInformation.SetNum(assetList.Num());
	int numOfElements = 0;

	// Check every Voxel position to creat the mesh
	for (int x = 0; x < chunkWidth; x++) {
	for (int y = 0; y < chunkWidth; y++) {
	for (int z = 0; z < chunkHeight; z++) {
		int index = x + y * chunkWidth + z * chunkWidthSquared;
		int voxelAssetID = voxelAssetIDs[index];

		// Check if the asset ID is valid.
		if (voxelAssetID == 0) continue;
		if (!IsValidVoxelID(voxelAssetID)) {
			PrintDebugWarning({
				"Couldn't process voxel ID.",
				"Reason: Voxel ID is not vaild!",
				"Skipped this voxel.",
				"Given voxel ID: " + FString::FromInt(voxelAssetID),
				"Voxel Index: " + FString::FromInt(index),
				"X Position: " + FString::FromInt(x),
				"Y Position: " + FString::FromInt(y),
				"Z Position: " + FString::FromInt(z)
				});
			continue;
		}

		#pragma region Calculate values for the procedual mesh

		// Setup the reference to the voxel mesh information.
		TArray<FVector>& Vertices = voxelMeshInformation[voxelAssetID].Vertices;
		TArray<int>& Triangles = voxelMeshInformation[voxelAssetID].Triangles;
		TArray<FVector>& Normals = voxelMeshInformation[voxelAssetID].Normals;
		TArray<FVector2D>& UVs = voxelMeshInformation[voxelAssetID].UVs;
		TArray<FColor>& VertexColors = voxelMeshInformation[voxelAssetID].VertexColors;
		TArray<FProcMeshTangent>& Tangents = voxelMeshInformation[voxelAssetID].Tangents;

		int numOfTriangle = 0;
		for (int i = 0; i < 6; i++) {
			int newIndex = index + bMask[i].X + bMask[i].Y * chunkWidth + bMask[i].Z * chunkWidthSquared;

			// Check, if verticies needs to be calculated
			bool flag = false;
			if ((x + bMask[i].X < chunkWidth) && (x + bMask[i].X >= 0) && (y + bMask[i].Y < chunkWidth) && (y + bMask[i].Y >= 0)) {
				if (voxelAssetIDs.IsValidIndex(newIndex))
					if (voxelAssetIDs[newIndex] < 1) 
						flag = true;
			}
			else 
				flag = true;

			if (!flag) continue;

			Triangles.Add(bTriangles[0] + numOfTriangle + voxelMeshInformation[voxelAssetID].elementID);
			Triangles.Add(bTriangles[1] + numOfTriangle + voxelMeshInformation[voxelAssetID].elementID);
			Triangles.Add(bTriangles[2] + numOfTriangle + voxelMeshInformation[voxelAssetID].elementID);
			Triangles.Add(bTriangles[3] + numOfTriangle + voxelMeshInformation[voxelAssetID].elementID);
			Triangles.Add(bTriangles[4] + numOfTriangle + voxelMeshInformation[voxelAssetID].elementID);
			Triangles.Add(bTriangles[5] + numOfTriangle + voxelMeshInformation[voxelAssetID].elementID);
			numOfTriangle += 4;

			// Set the verticies for the faces.
			switch (i) {
			case 0: {
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + y * voxelSize,		voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + (y + 1) * voxelSize,	voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + (y + 1) * voxelSize,	voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + y * voxelSize,		voxelSizeHalved + z * voxelSize));

				Normals.Append(bNormals0, UE_ARRAY_COUNT(bNormals0));
				break;
			}
			case 1: {
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + y * voxelSize,		-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + (y + 1) * voxelSize,	-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + (y + 1) * voxelSize,	-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + y * voxelSize,		-voxelSizeHalved + z * voxelSize));

				Normals.Append(bNormals1, UE_ARRAY_COUNT(bNormals1));
				break;
			}
			case 2: {
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + (y + 1) * voxelSize,	voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + (y + 1) * voxelSize,	-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + (y + 1) * voxelSize,	-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + (y + 1) * voxelSize,	voxelSizeHalved + z * voxelSize));

				Normals.Append(bNormals2, UE_ARRAY_COUNT(bNormals2));
				break;
			}
			case 3: {
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + y * voxelSize,		voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + y * voxelSize,		-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + y * voxelSize,		-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + y * voxelSize,		voxelSizeHalved + z * voxelSize));

				Normals.Append(bNormals3, UE_ARRAY_COUNT(bNormals3));
				break;
			}
			case 4: {
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + y * voxelSize,		voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + y * voxelSize,		-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + (y + 1) * voxelSize,	-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + (x + 1) * voxelSize,		chunkOffset + (y + 1) * voxelSize,	voxelSizeHalved + z * voxelSize));

				Normals.Append(bNormals4, UE_ARRAY_COUNT(bNormals4));
				break;
			}
			case 5: {
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + (y + 1) * voxelSize,	voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + (y + 1) * voxelSize,	-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + y * voxelSize,		-voxelSizeHalved + z * voxelSize));
				Vertices.Add(FVector(chunkOffset + x * voxelSize,			chunkOffset + y * voxelSize,		voxelSizeHalved + z * voxelSize));

				Normals.Append(bNormals5, UE_ARRAY_COUNT(bNormals5));
				break;
			}
			}

			// Add the UVs and color informations to the faces.
			UVs.Append(bUVs, UE_ARRAY_COUNT(bUVs));
			FColor color = FColor(255, 255, 255, i);
			VertexColors.Add(color);
			VertexColors.Add(color);
			VertexColors.Add(color);
			VertexColors.Add(color);
		}
		numOfElements += numOfTriangle;
		voxelMeshInformation[voxelAssetID].elementID += numOfTriangle;
		#pragma endregion
	}
	}
	}

	proceduralComponent->ClearAllMeshSections();

	for (int i = 1; i < voxelMeshInformation.Num(); i++) {
		if(voxelMeshInformation[i].Vertices.Num() > 0)
			proceduralComponent->CreateMeshSection(
				i, 
				voxelMeshInformation[i].Vertices, 
				voxelMeshInformation[i].Triangles, 
				voxelMeshInformation[i].Normals, 
				voxelMeshInformation[i].UVs, 
				voxelMeshInformation[i].VertexColors, 
				voxelMeshInformation[i].Tangents, 
				true
			);
	}

	for (int m = 1; m < assetList.Num(); m++){
		if (assetList.IsValidIndex(m)){
			if (assetList[m]) {
				if (assetList[m]->material)
					proceduralComponent->SetMaterial(m, assetList[m]->material);
				else 
					PrintDebugWarning({
						"Couldn't attach material.",
						"Reason: No valid material for this voxel asset ID!",
						"Voxel ID: " + FString::FromInt(m)
						});
				
			}
		}
	}
	return true;
}

/// ------ Debug ------ \\\

void AChunkActor::PrintDebugWarning(TArray<FString> information) {
	return;
	// Print and log the warning
	UE_LOG(LogTemp, Warning, TEXT("WARNING - %s"), *(information[0]));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "WARNING - " + information[0]);

	// Print and log any information
	for (int i = 1; i < information.Num(); i++) {
		UE_LOG(LogTemp, Warning, TEXT("|-> %s"), *(information[i]));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "|-> " + information[i]);
	}

	// Log default chunk information
	UE_LOG(LogTemp, Log, TEXT("Chunk Information :"));
	for (int i = 0; i < defaultInformation.Num(); i++) {
		UE_LOG(LogTemp, Log, TEXT("|-> %s"), *(defaultInformation[i]));
	}
}
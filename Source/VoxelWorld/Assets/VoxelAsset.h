#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VoxelAsset.generated.h"

UENUM(BlueprintType)
enum class EToolType : uint8
{
	VE_None 	UMETA(DisplayName = "None"),
	VE_Axe 		UMETA(DisplayName = "Axe"),
	VE_Pickaxe	UMETA(DisplayName = "Pickaxe"),
	VE_Shovel	UMETA(DisplayName = "Shovel"),
	VE_Hoe		UMETA(DisplayName = "Hoe"),
	VE_Weapon	UMETA(DisplayName = "Weapon")
};

UCLASS(BlueprintType)
class VOXELWORLD_API UVoxelAsset : public UDataAsset
{
	GENERATED_BODY()
	
/// ------ Voxel ------ \\\

public:
	// The identification number of the asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", Meta = (UIMin = 0, UIMax = 65536, ClampMin = 0, ClampMax = 65536))
		int assetID = 0;

	// The sub identification number of the asset to further specify the asset. Not necessary for the creation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", Meta = (UIMin = 0, UIMax = 65536, ClampMin = 0, ClampMax = 65536))
		int assetSubID = 0;

	// The name of the asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
		FName assetName = "defaultName";
	
	// The material reference for the asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
		UMaterialInterface* material;

/// ------ Mining ------ \\\

public:
	// The prefered tool to mine this asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining")
		EToolType preferedTool = EToolType::VE_None;

	// The required tool level to effectivly mine this asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining", Meta = (UIMin = 0, UIMax = 16, ClampMin = 0, ClampMax = 16))
		int neededToolLevel = 0;

	// The default time it takes to mine this asset with the prefered tool at the requiered level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining", Meta = (UIMin = 0, UIMax = 10, ClampMin = 0, ClampMax = 10))
		float defaultMineTime = 1;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BiomeAsset.generated.h"

/**
 * 
 */
UCLASS()
class VOXELWORLD_API UBiomeAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat", SimpleDisplay, Meta = (UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1, DisplayName = "Min"))
	float HeatMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat", SimpleDisplay, Meta = (UIMIN = 0, UIMax = 1, ClampMin = 0, ClampMax = 1, DisplayName = "Max"))
	float HeatMax;

#if WITH_EDITOR

	UPROPERTY(VisibleDefaultsOnly, Category = "Heat", Meta = (DisplayName = "Range"))
	FString HeatRange;

#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rainfall", SimpleDisplay, Meta = (UIMIN = 0, UIMax = 1, ClampMin = 0, ClampMax = 1, DisplayName = "Min"))
	float RainFallMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rainfall", SimpleDisplay, Meta = (UIMIN = 0, UIMax = 1, ClampMin = 0, ClampMax = 1, DisplayName = "Max"))
	float RainFallMax;

	UPROPERTY(VisibleDefaultsOnly, Category = "Rainfall", Meta = (DisplayName = "Range"))
	FString RainfallRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeightDistribution", SimpleDisplay, Meta = (UIMIN = 0, UIMax = 128, ClampMin = 0, ClampMax = 128, DisplayName = "Min"))
	uint8 HeightMapMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeightDistribution", SimpleDisplay, Meta = (UIMIN = 0, UIMax = 128, ClampMin = 0, ClampMax = 128, DisplayName = "Max"))
	uint8 HeightMapMax;

	UPROPERTY(VisibleDefaultsOnly, Category = "HeightDistribution", Meta = (DisplayName = "Range"))
	FString HeightRange;


#if WITH_EDITOR
	
	void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& e) override;

#endif

};

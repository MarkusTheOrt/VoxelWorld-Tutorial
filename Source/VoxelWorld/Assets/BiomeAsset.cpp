// Fill out your copyright notice in the Description page of Project Settings.

#include "BiomeAsset.h"


#if WITH_EDITOR
void UBiomeAsset::PostEditChangeChainProperty(FPropertyChangedChainEvent & e)
{
	Super::PostEditChangeProperty(e);

	if (e.GetPropertyName() == "HeatMax")
	{
		if (HeatMax < HeatMin)
			HeatMin = HeatMax;
	}
	else if (e.GetPropertyName() == "HeatMin")
	{
		if (HeatMin > HeatMax)
			HeatMax = HeatMin;
	}

	HeatRange = FString::Printf(TEXT("%s - %s / %s%%"), *FString::SanitizeFloat(HeatMin, 2).Left(4), *FString::SanitizeFloat(HeatMax, 2).Left(4), *FString::SanitizeFloat(1 - (HeatMin / HeatMax), 2).Left(4));

	if (e.GetPropertyName() == "RainFallMax")
	{
		if (RainFallMax < RainFallMin)
			RainFallMin = RainFallMax;
	}
	else if (e.GetPropertyName() == "RainFallMin")
	{
		if (RainFallMin > RainFallMax)
			RainFallMax = RainFallMin;
	}

	RainfallRange = FString::Printf(TEXT("%s - %s / %s%%"), *FString::SanitizeFloat(RainFallMin, 2).Left(4), *FString::SanitizeFloat(RainFallMax, 2).Left(4), *FString::SanitizeFloat(1 - (RainFallMin / RainFallMax), 2).Left(4));

	if (e.GetPropertyName() == "HeightMapMax")
	{
		if (HeightMapMax < HeightMapMin)
			HeightMapMin = HeightMapMax;
	}
	else if (e.GetPropertyName() == "HeightMapMin")
	{
		if (HeightMapMin > HeightMapMax)
			HeightMapMax = HeightMapMin;
	}

	HeightRange = FString::Printf(TEXT("%s - %s / %s%%"), *FString::FromInt(HeightMapMin), *FString::FromInt(HeightMapMax), *FString::SanitizeFloat(1 - ((float)HeightMapMin / (float)HeightMapMax), 2).Left(4));
	

	
}
#endif
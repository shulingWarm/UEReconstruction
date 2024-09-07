// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ReconstructionFile.generated.h"

/**
 * 
 */
UCLASS()
class RECONSTRUCTION_API UReconstructionFile : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	//获取用于重建的图片路径
	UFUNCTION(BlueprintCallable)
	static FString getImageFolder();

};

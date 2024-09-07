// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GaussianTextureManager.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RECONSTRUCTION_API UGaussianTextureManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	struct TextureContainer
	{
		//gaussian里面的点个数
		uint32 gaussianNum;
		UTexture2D* posTexture; //用于表示位置的纹理
		UTexture2D* colorTexture; //用于表示颜色的纹理
		UTexture2D* sizeTexture; //用于表示size大小的texture
		UTexture2D* rotTexture; //用于表示旋转信息的纹理
	};

	TextureContainer textures;

	// Sets default values for this component's properties
	UGaussianTextureManager();

	//获取位置纹理
	UFUNCTION(BlueprintCallable)
	UTexture2D* getPosTexture();

	UFUNCTION(BlueprintCallable)
	UTexture2D* getColorTexture();

	UFUNCTION(BlueprintCallable)
	UTexture2D* getSizeTexture();

	UFUNCTION(BlueprintCallable)
	UTexture2D* getRotTexture();

	//获得采样步长
	UFUNCTION(BlueprintCallable)
	FVector2D getSampleStep();

	//获取采样维度
	UFUNCTION(BlueprintCallable)
	int getSampleSize(int idDim);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

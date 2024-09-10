// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GaussianLoader.generated.h"

UCLASS()
class RECONSTRUCTION_API AGaussianLoader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGaussianLoader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 用于测试载入3D gaussian的接口
	UFUNCTION(BlueprintCallable)
	UTexture2D* load3DGaussian();

	//位置纹理
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* posTexture;

	//颜色纹理
	//这个颜色纹理可能仅仅是一个临时概念，这个到后面可能还是会用球谐函数来代替
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* colorTexture;

	//view的视角范围对应的纹理
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* viewTextureRange;

	//size信息的纹理
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* sizeTexture;

	//关于旋转信息的纹理
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* rotTexture;

	//有效点的个数
	UPROPERTY(BlueprintReadWrite)
	int gaussianPointNum;

	//预加载3D 高斯
	UFUNCTION(BlueprintCallable)
	void preload3DGaussian();

	//获取纹理采样时的采样步长
	UFUNCTION(BlueprintCallable)
	FVector2D getSampleStep();

	//获取粒子系统原始的采样维度
	UFUNCTION(BlueprintCallable)
	int getSampleSize(int idDim);
};

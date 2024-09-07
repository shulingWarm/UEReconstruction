// Fill out your copyright notice in the Description page of Project Settings.


#include "GaussianTextureManager.h"

// Sets default values for this component's properties
UGaussianTextureManager::UGaussianTextureManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

UTexture2D* UGaussianTextureManager::getPosTexture()
{
	return this->textures.posTexture;
}

UTexture2D* UGaussianTextureManager::getColorTexture()
{
	return this->textures.colorTexture;
}

UTexture2D* UGaussianTextureManager::getSizeTexture()
{
	return this->textures.sizeTexture;
}

UTexture2D* UGaussianTextureManager::getRotTexture()
{
	return this->textures.rotTexture;
}

FVector2D UGaussianTextureManager::getSampleStep()
{
	//如果还没有初始化texture,那就随便返回
	if (this->textures.colorTexture == nullptr)
	{
		return FVector2D(1, 1);
	}
	return FVector2D(
		1.f / (this->textures.colorTexture->GetSizeX() + 0.5f),
		1.f / (this->textures.colorTexture->GetSizeY())
	);
}

int UGaussianTextureManager::getSampleSize(int idDim)
{
	if (this->textures.colorTexture == nullptr)
	{
		return 0;
	}
	if (idDim == 0)
	{
		return this->textures.colorTexture->GetSizeX();
	}
	return this->textures.gaussianNum / this->textures.colorTexture->GetSizeX() + 1;
}


// Called when the game starts
void UGaussianTextureManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGaussianTextureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


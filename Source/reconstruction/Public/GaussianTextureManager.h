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
		//gaussian����ĵ����
		uint32 gaussianNum;
		UTexture2D* posTexture; //���ڱ�ʾλ�õ�����
		UTexture2D* colorTexture; //���ڱ�ʾ��ɫ������
		UTexture2D* sizeTexture; //���ڱ�ʾsize��С��texture
		UTexture2D* rotTexture; //���ڱ�ʾ��ת��Ϣ������
	};

	TextureContainer textures;

	// Sets default values for this component's properties
	UGaussianTextureManager();

	//��ȡλ������
	UFUNCTION(BlueprintCallable)
	UTexture2D* getPosTexture();

	UFUNCTION(BlueprintCallable)
	UTexture2D* getColorTexture();

	UFUNCTION(BlueprintCallable)
	UTexture2D* getSizeTexture();

	UFUNCTION(BlueprintCallable)
	UTexture2D* getRotTexture();

	//��ò�������
	UFUNCTION(BlueprintCallable)
	FVector2D getSampleStep();

	//��ȡ����ά��
	UFUNCTION(BlueprintCallable)
	int getSampleSize(int idDim);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

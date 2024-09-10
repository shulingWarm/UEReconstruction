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

	// ���ڲ�������3D gaussian�Ľӿ�
	UFUNCTION(BlueprintCallable)
	UTexture2D* load3DGaussian();

	//λ������
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* posTexture;

	//��ɫ����
	//�����ɫ������ܽ�����һ����ʱ��������������ܻ��ǻ�����г����������
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* colorTexture;

	//view���ӽǷ�Χ��Ӧ������
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* viewTextureRange;

	//size��Ϣ������
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* sizeTexture;

	//������ת��Ϣ������
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* rotTexture;

	//��Ч��ĸ���
	UPROPERTY(BlueprintReadWrite)
	int gaussianPointNum;

	//Ԥ����3D ��˹
	UFUNCTION(BlueprintCallable)
	void preload3DGaussian();

	//��ȡ�������ʱ�Ĳ�������
	UFUNCTION(BlueprintCallable)
	FVector2D getSampleStep();

	//��ȡ����ϵͳԭʼ�Ĳ���ά��
	UFUNCTION(BlueprintCallable)
	int getSampleSize(int idDim);
};

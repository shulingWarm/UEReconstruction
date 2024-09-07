// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/GaussianTextureManager.h"
#include "ReconstructionClient.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RECONSTRUCTION_API UReconstructionClient : public UActorComponent
{
	GENERATED_BODY()

public:	
	//����Texture������
	using TextureContainer = UGaussianTextureManager::TextureContainer;

	//����׼��������module
	typedef void (*ReconstructionFunc)(const char*,const char*,void*);

	//�ؽ��õĽ��
	struct _SplatScene
	{
		//�����
		uint32_t pointNum;
		//�ؽ��õ�3D��
		float* pointList;
		//��ɫ��Ϣ
		float* color;
		//scale��Ϣ
		float* scale;
		//��ת��Ϣ
		float* rotation;
		//͸����
		float* opacity;
	};

	//���ڶ������ʱ��
	class ReconstructionRunner : public FRunnable
	{
	public:
		//ʵ�ʱ����õĺ���ָ��
		ReconstructionFunc funcPtr;
		//�����ؽ���ͼƬ·���͹���·��
		std::string imagePath;
		std::string workspacePath;
		//�ؽ���ɺ�Ļص�
		UReconstructionClient* clientCallback;

		//���캯������Ҫ�������ڹ���ĺ���
		ReconstructionRunner(ReconstructionFunc funcPtr,
			std::string imagePath,
			std::string workspacePath,
			UReconstructionClient* clientCallback
		);

		virtual uint32 Run() override;
	};

	//���ڴ洢�ؽ����������
	TextureContainer* reconstructionResult = nullptr;
	_SplatScene* splatScenePtr = nullptr;

	//�����������ݵ�ģ��
	//����ʵҲ��һ��ָ�룬ֻ��������ģ��ָ��
	HMODULE hModule;
	//����ָ��
	ReconstructionFunc reconstructionFunc;

	//��Ӵ�ȡ�����ؽ����
	void addReconstructionResult(_SplatScene* tempResult);

	UFUNCTION(BlueprintCallable)
	void loadDll();

	//����reconstruction
	UFUNCTION(BlueprintCallable)
	void callReconstruction(FString string);

	// Sets default values for this component's properties
	UReconstructionClient();

	//�ж��Ƿ��д���ȡ���ؽ����
	UFUNCTION(BlueprintCallable)
	bool isResultAvailable();

	//��ʼ��gausian��texture manager
	UFUNCTION(BlueprintCallable)
	void initGaussianTextureManager(UGaussianTextureManager* manager);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

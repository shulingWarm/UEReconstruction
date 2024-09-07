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
	//保存Texture的容器
	using TextureContainer = UGaussianTextureManager::TextureContainer;

	//用于准备函数的module
	typedef void (*ReconstructionFunc)(const char*,const char*,void*);

	//重建好的结果
	struct _SplatScene
	{
		//点个数
		uint32_t pointNum;
		//重建好的3D点
		float* pointList;
		//颜色信息
		float* color;
		//scale信息
		float* scale;
		//旋转信息
		float* rotation;
		//透明度
		float* opacity;
	};

	//类内定义的临时类
	class ReconstructionRunner : public FRunnable
	{
	public:
		//实际被调用的函数指针
		ReconstructionFunc funcPtr;
		//用于重建的图片路径和工作路径
		std::string imagePath;
		std::string workspacePath;
		//重建完成后的回调
		UReconstructionClient* clientCallback;

		//构造函数，需要传入用于构造的函数
		ReconstructionRunner(ReconstructionFunc funcPtr,
			std::string imagePath,
			std::string workspacePath,
			UReconstructionClient* clientCallback
		);

		virtual uint32 Run() override;
	};

	//用于存储重建结果的内容
	TextureContainer* reconstructionResult = nullptr;
	_SplatScene* splatScenePtr = nullptr;

	//包含函数内容的模块
	//这其实也是一个指针，只不过这是模块指针
	HMODULE hModule;
	//函数指针
	ReconstructionFunc reconstructionFunc;

	//添加待取出的重建结果
	void addReconstructionResult(_SplatScene* tempResult);

	UFUNCTION(BlueprintCallable)
	void loadDll();

	//调用reconstruction
	UFUNCTION(BlueprintCallable)
	void callReconstruction(FString string);

	// Sets default values for this component's properties
	UReconstructionClient();

	//判断是否有待读取的重建结果
	UFUNCTION(BlueprintCallable)
	bool isResultAvailable();

	//初始化gausian的texture manager
	UFUNCTION(BlueprintCallable)
	void initGaussianTextureManager(UGaussianTextureManager* manager);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

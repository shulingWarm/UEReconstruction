// Fill out your copyright notice in the Description page of Project Settings.


#include "ReconstructionClient.h"
#include"TextureFunction.h"
#include<sstream>

using SplatScene = UReconstructionClient::_SplatScene;

//初始化texture
static UTexture2D* createTexture(uint32 textureWidth, uint32 textureHeight)
{
	auto texture = UTexture2D::CreateTransient(
		textureWidth, textureHeight, PF_FloatRGBA
	);
	//采样方式改成最近采样
	texture->Filter = TextureFilter::TF_Nearest;
	texture->CompressionSettings = TextureCompressionSettings::TC_HDR;
	texture->SRGB = false;
	texture->NeverStream = true;
	auto platformData = texture->GetPlatformData();
	platformData->SetNumSlices(1);
	return texture;
}

//获取纹理的颜色数据
template<class DataType>
static DataType* getTextureData(UTexture2D& texture)
{
	//获取纹理的平台数据
	auto& platformData = *texture.GetPlatformData();
	//获取platform里面的mip数据
	auto& mip = platformData.Mips.Last();
	//锁定主体数据
	return reinterpret_cast<DataType*>(mip.BulkData.Lock(LOCK_READ_WRITE));
}

//把SplatScene里面的数据转换到数据指针里面
template<class T>
static void loadSplatSceneToTextureData(SplatScene& srcSplatScene,
	T* posData, T* colorData, T* sizeData, T* rotData,uint32 textureSize
)
{
	//pos位置的缩放，为了减小量化误差
	const float POS_SCALE = 1.f / 32;
	//纹理中每个点对应的数据块大小
	const uint32 TEXTURE_BLOCK_SIZE = 4;
	for (uint32 idPoint = 0; idPoint < srcSplatScene.pointNum; ++idPoint)
	{
		//准备写入数据的指针头
		auto posHead = posData + TEXTURE_BLOCK_SIZE * idPoint;
		auto colorHead = colorData + TEXTURE_BLOCK_SIZE * idPoint;
		auto sizeHead = sizeData + TEXTURE_BLOCK_SIZE * idPoint;
		auto rotHead = rotData + TEXTURE_BLOCK_SIZE * idPoint;
		//读取原始的输入结果的数据头
		auto srcPosHead = srcSplatScene.pointList + idPoint * 3;
		auto srcColorHead = srcSplatScene.color + idPoint * 3;
		auto srcSizeHead = srcSplatScene.scale + idPoint * 3;
		auto srcRotHead = srcSplatScene.rotation + idPoint * 4;
		//记录size信息
		posHead[0] = T(srcPosHead[0] * POS_SCALE);
		posHead[1] = T(-srcPosHead[2] * POS_SCALE);
		posHead[2] = T(-srcPosHead[1] * POS_SCALE);
		posHead[3] = T(0);
		//遍历记录每个颜色
		for (uint32 idDim = 0; idDim < 3; ++idDim)
		{
			//记录颜色数据
			colorHead[idDim] = T(srcColorHead[idDim] * 0.2820948f + 0.5f);
			//记录size数据
			sizeHead[idDim] = T(std::exp(srcSizeHead[idDim]) * POS_SCALE);
		}
		sizeHead[3] = T(0);
		//记录颜色里面的透明度
		colorHead[3] = T(1.f / (1 + std::exp(-srcSplatScene.opacity[idPoint])));
		//把旋转信息正则化
		auto srcRotation = FVector4f(srcRotHead[0], srcRotHead[1], srcRotHead[2], srcRotHead[3]);
		//正则化之后的向量
		auto vecLength = srcRotation.Size();
		if (vecLength > 0)
			srcRotation = srcRotation / vecLength;
		//记录旋转信息
		for (uint32 idDim = 0; idDim < 4; ++idDim)
		{
			rotHead[idDim] = T(srcRotation[idDim]);
		}
	}
	//打印2200开始位置的10个数
	std::stringstream sstream;
	for (int i = 0; i < 10; ++i)
	{
		sstream << (float)sizeData[2200 + i] << " ";
	}
	UE_LOG(LogTemp, Warning, TEXT("PosData: %s\n"), *FString(sstream.str().c_str()));
	const uint32 pointNum = srcSplatScene.pointNum;
	for (uint32 idPoint = pointNum; idPoint < textureSize; ++idPoint)
	{
		auto posHead = posData + TEXTURE_BLOCK_SIZE * idPoint;
		auto colorHead = colorData + TEXTURE_BLOCK_SIZE * idPoint;
		auto sizeHead = sizeData + TEXTURE_BLOCK_SIZE * idPoint;
		auto rotHead = rotData + TEXTURE_BLOCK_SIZE * idPoint;
		for (int i = 0; i < TEXTURE_BLOCK_SIZE; ++i)
		{
			posHead[i] = T(0);
			colorHead[i] = T(0);
			sizeHead[i] = T(0);
			rotHead[i] = T(0);
		}
	}
}

//把SplatScene转换成Texture
static UReconstructionClient::TextureContainer convertSplatSceneToGaussianTextureManager(SplatScene& splatScene)
{
	UReconstructionClient::TextureContainer tempContainer;
	//获取texture的大小
	uint32 textureWidth = 2048;
	uint32 textureHeight = FMath::RoundUpToPowerOfTwo((float)splatScene.pointNum / 2048);
	//初始化每个texture
	tempContainer.posTexture = createTexture(textureWidth, textureHeight);
	tempContainer.colorTexture = createTexture(textureWidth, textureHeight);
	tempContainer.sizeTexture = createTexture(textureWidth, textureHeight);
	tempContainer.rotTexture = createTexture(textureWidth, textureHeight);
	UE_LOG(LogTemp, Warning, TEXT("Texture size %d %d\n"), textureWidth, textureHeight);
	//记录gaussian的个数
	tempContainer.gaussianNum = splatScene.pointNum;
	//从纹理中解析出相关的数据
	auto posData = getTextureData<FFloat16>(*tempContainer.posTexture);
	auto colorData = getTextureData<FFloat16>(*tempContainer.colorTexture);
	auto sizeData = getTextureData<FFloat16>(*tempContainer.sizeTexture);
	auto rotData = getTextureData<FFloat16>(*tempContainer.rotTexture);
	loadSplatSceneToTextureData<FFloat16>(splatScene, posData, colorData, sizeData, rotData,
		textureWidth * textureHeight);
	TextureFunction::unlockTexture(*tempContainer.posTexture);
	TextureFunction::unlockTexture(*tempContainer.colorTexture);
	TextureFunction::unlockTexture(*tempContainer.sizeTexture);
	TextureFunction::unlockTexture(*tempContainer.rotTexture);
	//对所有texture的最终结算
	TextureFunction::textureFinalUpdate(*tempContainer.posTexture);
	TextureFunction::textureFinalUpdate(*tempContainer.colorTexture);
	TextureFunction::textureFinalUpdate(*tempContainer.sizeTexture);
	TextureFunction::textureFinalUpdate(*tempContainer.rotTexture);
	return tempContainer;
}

void UReconstructionClient::addReconstructionResult(_SplatScene* tempResult)
{
	this->splatScenePtr = tempResult;
}

//测试载入dll
void UReconstructionClient::loadDll()
{
	DWORD prev = SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	//设置dll的搜索路径
	SetDefaultDllDirectories(prev | LOAD_LIBRARY_SEARCH_APPLICATION_DIR |
		LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
	AddDllDirectory(TEXT("E:/privateSpace/workSpace/504OpenSplat/OpenSplat-main/OpenSplat-main/build/Release"));
	AddDllDirectory(TEXT("E:/lib/vcpkg/installed/x64-windows/bin"));
	AddDllDirectory(TEXT("E:/lib/libtorch/libtorch/lib"));
	AddDllDirectory(TEXT("E:/lib/cuda-win/bin"));
	hModule = LoadLibrary(TEXT("E:/privateSpace/workSpace/504OpenSplat/OpenSplat-main/OpenSplat-main/build/Release/IntegralReconstruction.dll"));
	if (hModule == nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("Cannot load module"));
		return;
	}
	reconstructionFunc = (ReconstructionFunc)GetProcAddress(hModule,"reconstruct");
	if (reconstructionFunc == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find function"));
		return;
	}
}

void UReconstructionClient::callReconstruction(FString string)
{
	auto tempRunner = new ReconstructionRunner(this->reconstructionFunc,
		std::string(TCHAR_TO_UTF8(*string)), "E:/temp/reconstruction", this);
	//构造线程来开始执行
	FRunnableThread* tempThread = FRunnableThread::Create(tempRunner, TEXT("Reconstruction task"));
}

// Sets default values for this component's properties
UReconstructionClient::UReconstructionClient()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

//判断是否有待读取的重建结果
bool UReconstructionClient::isResultAvailable()
{
	return splatScenePtr != nullptr;
}

void UReconstructionClient::initGaussianTextureManager(UGaussianTextureManager* manager)
{
	//把manager里面的内容赋值进去
	if (this->splatScenePtr != nullptr)
	{
		manager->textures = convertSplatSceneToGaussianTextureManager(*this->splatScenePtr);
		//释放texture里面的内容
		this->splatScenePtr = nullptr;
	}
}


// Called when the game starts
void UReconstructionClient::BeginPlay()
{
	Super::BeginPlay();
	this->loadDll();
	// ...
	
}


// Called every frame
void UReconstructionClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UReconstructionClient::ReconstructionRunner::ReconstructionRunner(ReconstructionFunc funcPtr, std::string imagePath,
	std::string workspacePath, UReconstructionClient* clientCallback
)
{
	this->funcPtr = funcPtr;
	this->imagePath = imagePath;
	this->workspacePath = workspacePath;
	//记录重建结果的回调
	this->clientCallback = clientCallback;
	UE_LOG(LogTemp, Warning, TEXT("Constructing ReconstructionRunner!!!"));
}

uint32 UReconstructionClient::ReconstructionRunner::Run()
{
	UE_LOG(LogTemp,Warning,TEXT("Begin run reconstruction"));
	//准备用于接收重建结果的结构
	SplatScene* tempResult = new SplatScene();
	//直接调用用于重建的函数
	this->funcPtr(imagePath.c_str(), workspacePath.c_str(),
		reinterpret_cast<void*>(tempResult));
	//在本机里面记录临时的splat scene
	clientCallback->addReconstructionResult(tempResult);
	return 0;
}

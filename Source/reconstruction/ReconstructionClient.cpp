// Fill out your copyright notice in the Description page of Project Settings.


#include "ReconstructionClient.h"
#include"TextureFunction.h"
#include<sstream>

using SplatScene = UReconstructionClient::_SplatScene;

//��ʼ��texture
static UTexture2D* createTexture(uint32 textureWidth, uint32 textureHeight)
{
	auto texture = UTexture2D::CreateTransient(
		textureWidth, textureHeight, PF_FloatRGBA
	);
	//������ʽ�ĳ��������
	texture->Filter = TextureFilter::TF_Nearest;
	texture->CompressionSettings = TextureCompressionSettings::TC_HDR;
	texture->SRGB = false;
	texture->NeverStream = true;
	auto platformData = texture->GetPlatformData();
	platformData->SetNumSlices(1);
	return texture;
}

//��ȡ�������ɫ����
template<class DataType>
static DataType* getTextureData(UTexture2D& texture)
{
	//��ȡ�����ƽ̨����
	auto& platformData = *texture.GetPlatformData();
	//��ȡplatform�����mip����
	auto& mip = platformData.Mips.Last();
	//������������
	return reinterpret_cast<DataType*>(mip.BulkData.Lock(LOCK_READ_WRITE));
}

//��SplatScene���������ת��������ָ������
template<class T>
static void loadSplatSceneToTextureData(SplatScene& srcSplatScene,
	T* posData, T* colorData, T* sizeData, T* rotData,uint32 textureSize
)
{
	//posλ�õ����ţ�Ϊ�˼�С�������
	const float POS_SCALE = 1.f / 32;
	//������ÿ�����Ӧ�����ݿ��С
	const uint32 TEXTURE_BLOCK_SIZE = 4;
	for (uint32 idPoint = 0; idPoint < srcSplatScene.pointNum; ++idPoint)
	{
		//׼��д�����ݵ�ָ��ͷ
		auto posHead = posData + TEXTURE_BLOCK_SIZE * idPoint;
		auto colorHead = colorData + TEXTURE_BLOCK_SIZE * idPoint;
		auto sizeHead = sizeData + TEXTURE_BLOCK_SIZE * idPoint;
		auto rotHead = rotData + TEXTURE_BLOCK_SIZE * idPoint;
		//��ȡԭʼ��������������ͷ
		auto srcPosHead = srcSplatScene.pointList + idPoint * 3;
		auto srcColorHead = srcSplatScene.color + idPoint * 3;
		auto srcSizeHead = srcSplatScene.scale + idPoint * 3;
		auto srcRotHead = srcSplatScene.rotation + idPoint * 4;
		//��¼size��Ϣ
		posHead[0] = T(srcPosHead[0] * POS_SCALE);
		posHead[1] = T(-srcPosHead[2] * POS_SCALE);
		posHead[2] = T(-srcPosHead[1] * POS_SCALE);
		posHead[3] = T(0);
		//������¼ÿ����ɫ
		for (uint32 idDim = 0; idDim < 3; ++idDim)
		{
			//��¼��ɫ����
			colorHead[idDim] = T(srcColorHead[idDim] * 0.2820948f + 0.5f);
			//��¼size����
			sizeHead[idDim] = T(std::exp(srcSizeHead[idDim]) * POS_SCALE);
		}
		sizeHead[3] = T(0);
		//��¼��ɫ�����͸����
		colorHead[3] = T(1.f / (1 + std::exp(-srcSplatScene.opacity[idPoint])));
		//����ת��Ϣ����
		auto srcRotation = FVector4f(srcRotHead[0], srcRotHead[1], srcRotHead[2], srcRotHead[3]);
		//����֮�������
		auto vecLength = srcRotation.Size();
		if (vecLength > 0)
			srcRotation = srcRotation / vecLength;
		//��¼��ת��Ϣ
		for (uint32 idDim = 0; idDim < 4; ++idDim)
		{
			rotHead[idDim] = T(srcRotation[idDim]);
		}
	}
	//��ӡ2200��ʼλ�õ�10����
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

//��SplatSceneת����Texture
static UReconstructionClient::TextureContainer convertSplatSceneToGaussianTextureManager(SplatScene& splatScene)
{
	UReconstructionClient::TextureContainer tempContainer;
	//��ȡtexture�Ĵ�С
	uint32 textureWidth = 2048;
	uint32 textureHeight = FMath::RoundUpToPowerOfTwo((float)splatScene.pointNum / 2048);
	//��ʼ��ÿ��texture
	tempContainer.posTexture = createTexture(textureWidth, textureHeight);
	tempContainer.colorTexture = createTexture(textureWidth, textureHeight);
	tempContainer.sizeTexture = createTexture(textureWidth, textureHeight);
	tempContainer.rotTexture = createTexture(textureWidth, textureHeight);
	UE_LOG(LogTemp, Warning, TEXT("Texture size %d %d\n"), textureWidth, textureHeight);
	//��¼gaussian�ĸ���
	tempContainer.gaussianNum = splatScene.pointNum;
	//�������н�������ص�����
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
	//������texture�����ս���
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

//��������dll
void UReconstructionClient::loadDll()
{
	DWORD prev = SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	//����dll������·��
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
	//�����߳�����ʼִ��
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

//�ж��Ƿ��д���ȡ���ؽ����
bool UReconstructionClient::isResultAvailable()
{
	return splatScenePtr != nullptr;
}

void UReconstructionClient::initGaussianTextureManager(UGaussianTextureManager* manager)
{
	//��manager��������ݸ�ֵ��ȥ
	if (this->splatScenePtr != nullptr)
	{
		manager->textures = convertSplatSceneToGaussianTextureManager(*this->splatScenePtr);
		//�ͷ�texture���������
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
	//��¼�ؽ�����Ļص�
	this->clientCallback = clientCallback;
	UE_LOG(LogTemp, Warning, TEXT("Constructing ReconstructionRunner!!!"));
}

uint32 UReconstructionClient::ReconstructionRunner::Run()
{
	UE_LOG(LogTemp,Warning,TEXT("Begin run reconstruction"));
	//׼�����ڽ����ؽ�����Ľṹ
	SplatScene* tempResult = new SplatScene();
	//ֱ�ӵ��������ؽ��ĺ���
	this->funcPtr(imagePath.c_str(), workspacePath.c_str(),
		reinterpret_cast<void*>(tempResult));
	//�ڱ��������¼��ʱ��splat scene
	clientCallback->addReconstructionResult(tempResult);
	return 0;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "GaussianLoader.h"
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<iostream>

//ply��ʽ�µ�����
class PlyProperty
{
public:
	std::string type;
	std::string name;
};

//ply���ļ���ʽ��Ϣ
class PlyHeader
{
public:
	std::string format;
	//�ڵ�ĸ���
	int verticeNum = 0;
	//��Ƭ�ĸ���
	int faceNum = 0;
	//���е������б������ڵ�ĺ���Ƭ��
	std::vector<PlyProperty> propertyList;

	//��ӡply header����Ϣ
	void print()
	{
		auto tempString = FString(format.c_str());
		UE_LOG(LogTemp, Warning, TEXT("format: %s\n"), *tempString);
		UE_LOG(LogTemp, Warning, TEXT("vertice num: %d\n"), verticeNum);
		UE_LOG(LogTemp, Warning, TEXT("face num: %d\n"), faceNum);
	}
};

//һ���ڵ������
class Vertex
{
public:
	//λ��
	float position[3];
	//������
	float normal[3];
	//��г�����Ĳ���
	float shs[48];
	//͸����
	float opacity;
	//���������scale
	float scale[3];
	//����Ԫ�����ɵ���ת
	float rotation[4];
};

//������Ƶ�����ͷ
static void loadPlyHeader(std::ifstream& fileHandle,
	PlyHeader& header
)
{
	//�м��ȡ����ÿһ�еĽ��
	std::string line;
	while (std::getline(fileHandle, line))
	{
		//�½�������
		std::istringstream tempStream(line);
		//��ȡһ������
		std::string token;
		tempStream >> token;
		//�ж��ǲ��Ǹ�ʽ��Ϣ
		if (token == "format")
		{
			tempStream >> header.format;
		}
		//�ж϶�ȡ�����ǲ���element��Ϣ
		else if (token == "element")
		{
			//�ٶ�ȡelement������
			tempStream >> token;
			//�ж��ǽڵ����������Ƭ�ĸ���
			if (token == "vertex")
			{
				tempStream >> header.verticeNum;
			}
			else if (token == "face")
			{
				tempStream >> header.faceNum;
			}
			else
			{
				throw std::runtime_error("Unknown element type");
			}
		}
		//���ж��Ƿ��ȡ����������Ϣ
		else if (token == "property")
		{
			//�½�һ����ʱ������
			PlyProperty tempProperty;
			//��¼����type������
			tempStream >> tempProperty.type >> tempProperty.name;
			//�����Էŵ��б�����
			header.propertyList.push_back(tempProperty);
		}
		//header���ֵĽ�����
		else if (token == "end_header")
		{
			break;
		}

	}
}

//����һ��ply�ļ�
//������ǲ����õĺ���
static void loadPointcloud(std::vector<Vertex>& vertexList)
{
	//�������ڵ��ļ�
	std::string filePath = "E:/temp/test.ply";
	//���ļ���������
	std::ifstream fileHandle(filePath, std::ios::binary);
	//�½�ply��ͷ��������
	PlyHeader header;
	loadPlyHeader(fileHandle, header);
	UE_LOG(LogTemp, Warning, TEXT("Read ply header ok"));
	//����֮���ӡһ��header��Ϣ
	header.print();
	//������ȡÿ����
	std::vector<Vertex> tempVertex(header.verticeNum);
	if (sizeof(Vertex) != 62 * sizeof(float))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid vertex size"));
		throw std::runtime_error("Invalid vertex size");
	}
	int idData = 0;
	for (auto& eachVertex : tempVertex)
	{
		//��ȡ�ڵ�����
		fileHandle.read(reinterpret_cast<char*>(&eachVertex), sizeof(Vertex));
		//��ӡ62������
		float* vertexData = (float*)&eachVertex;
		if (idData < 20)
		{
			std::stringstream tempStream;
			for (int i = 0; i < 62; ++i)
			{
				tempStream << vertexData[i] << " ";
			}
			UE_LOG(LogTemp, Warning, TEXT("%s\n"), *FString(tempStream.str().c_str()));
		}
		++idData;
	}
	//����ÿ���ڵ�
	vertexList.reserve(tempVertex.size());
	for (auto& eachVertex : tempVertex)
	{
		//�ж��Ƿ��ڰ뾶��
		float sum = 0;
		for (int i = 0; i < 3; ++i)
		{
			auto tempVal = eachVertex.position[i];
			sum += tempVal * tempVal;
		}
		if (sum < 81)
		{
			vertexList.push_back(eachVertex);
		}
	}
}

//�ѽڵ��б�ת��������
static void convertVertexListToTexture(
	std::vector<Vertex>& vertexList,
	UTexture2D& texture
)
{
	//��ȡ�����ƽ̨����
	auto& platformData = *texture.GetPlatformData();
	//��ӡplatform�����xy
	UE_LOG(LogTemp, Warning, TEXT("platform: %d %d\n"), platformData.SizeX, platformData.SizeY);
	//��ȡplatform�����mip����
	auto& mip = platformData.Mips.Last();
	//��ӡbulk������ڴ����ݣ�ȷ�������Ѿ����ٹ��ڴ��
	UE_LOG(LogTemp, Warning, TEXT("Bulk data size %d\n"), mip.BulkData.GetBulkDataSize());
	//������������
	auto mainData = reinterpret_cast<FFloat16*>(mip.BulkData.Lock(LOCK_READ_WRITE));
	int idData = 0;
	//����ÿ������
	for (auto& eachVertex : vertexList)
	{
		//��ǰλ�õ�ͷָ��
		auto vertexHead = mainData + idData * 4;
		//����xyz
		for (int i = 0; i < 3; ++i)
		{
			vertexHead[i] = FFloat16(eachVertex.position[i]);
		}
		vertexHead[3] = FFloat16(1);
		++idData;
	}
	//��������
	mip.BulkData.Unlock();
}

//����һ�����������texture
static void makeSampleTexture(UTexture2D& texture)
{
	//��ȡ�����ƽ̨����
	auto& platformData = *texture.GetPlatformData();
	//��ӡplatform�����xy
	UE_LOG(LogTemp, Warning, TEXT("platform: %d %d\n"), platformData.SizeX, platformData.SizeY);
	//��ȡplatform�����mip����
	auto& mip = platformData.Mips.Last();
	//��ӡbulk������ڴ����ݣ�ȷ�������Ѿ����ٹ��ڴ��
	UE_LOG(LogTemp, Warning, TEXT("Bulk data size %d\n"), mip.BulkData.GetBulkDataSize());
	//������������
	auto mainData = reinterpret_cast<FFloat16*>(mip.BulkData.Lock(LOCK_READ_WRITE));
	//����ÿ��λ�õ�����
	for (int idy = 0; idy < platformData.SizeY; ++idy)
	{
		//��ǰ�е�ͷָ��
		auto rowPtr = mainData + idy * 4 * platformData.SizeX;
		float zAngle = (float)idy / platformData.SizeY * PI / 2.f;
		//��ǰ������Ƕ�
		auto zCos = FMath::Cos(zAngle);
		auto zSin = FMath::Sin(zAngle);
		//������ǰ��
		for (int idx = 0; idx < platformData.SizeX; ++idx)
		{
			//���㵱ǰλ�õĽǶ�
			float angle = (float)idx / platformData.SizeX * 2 * PI;
			//��ǰλ�õ�����ͷ
			auto dataHead = rowPtr + idx * 4;
			dataHead[0] = FFloat16(FMath::Sin(angle) * zSin);
			dataHead[1] = FFloat16(FMath::Cos(angle) * zSin);
			dataHead[2] = FFloat16(zCos);//����0����z
			dataHead[3] = FFloat16(1);
		}
	}
	//��������
	mip.BulkData.Unlock();
}

// Sets default values
AGaussianLoader::AGaussianLoader()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	colorTexture = nullptr;
	posTexture = nullptr;

}

// Called when the game starts or when spawned
void AGaussianLoader::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AGaussianLoader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UTexture2D* AGaussianLoader::load3DGaussian()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading 3D gaussian"));
	//��ʼ��һ���ڵ��б���������ֵ
	std::vector<Vertex> vertexList;
	//���������˹����
	loadPointcloud(vertexList);
	//��ӡ�ܵĵ����
	UE_LOG(LogTemp, Warning, TEXT("Value point: %d\n"), vertexList.size());
	// ����һ������ ��Ҫע�⣬���float RGBA�ǰ뾫��
	auto tempTexture = UTexture2D::CreateTransient(
		1400, 1000, PF_FloatRGBA, "DynamicTexture"
	);
	//�ѽڵ��б�ת��������
	convertVertexListToTexture(vertexList, *tempTexture);
	// makeSampleTexture(*tempTexture);
	//��֪����ʲô�õģ���֮��һ�°�
	tempTexture->UpdateResource();
	tempTexture->AddToRoot();
	return tempTexture;
}

//���������ݵĻ�������
static void baseConfigTexture(UTexture2D& texture)
{
	//������ʽ�ĳ��������
	texture.Filter = TextureFilter::TF_Nearest;
	texture.CompressionSettings = TextureCompressionSettings::TC_HDR;
	texture.SRGB = false;
	texture.NeverStream = true;
	auto platformData = texture.GetPlatformData();
	platformData->SetNumSlices(1);
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

//�������ڶ�ȡ���ݵ�texture
static void unlockTexture(UTexture2D& texture)
{
	texture.GetPlatformData()->Mips.Last().BulkData.Unlock();
}

//һ����ʱ����ɫ�����������ڸ���id��ȡ����ɫ
class ColorManager
{
public:
	int pointNum;

	ColorManager(int pointNum)
	{
		this->pointNum = pointNum;
	}

	//��ȡ�������ɫ�ֶ�
	float getOneColorSeg(int idPoint, //�ֶ�����ĵڼ�����ɫ 
		int segNum //�ܹ��ķֶ���
	)
	{
		return  (float)idPoint / segNum;
	}

	void getColor(float* color, int idPoint)
	{
		//�ж��Ƿ�Ϊ��1��
		if (idPoint < pointNum / 3)
		{
			//�Ѻ�������ɫ��Ϊ0
			color[0] = 0;
			color[1] = 0;
			color[2] = 1;
		}
		else if (idPoint < pointNum * 2 / 3)
		{
			color[2] = 0;
			color[1] = 1;
			color[0] = 0;
		}
		else
		{
			color[2] = 0.f;
			color[1] = 0;
			color[0] = 1;
		}
	}

};

//��ӡһ����˹�������
//ֱ����������ľ��ǳ�����62�ĵ�
static void printGaussianUnitData(float* unitData)
{
	std::stringstream sstream;
	for (int i = 0; i < 62; ++i)
	{
		sstream << unitData[i] << " ";
	}
	UE_LOG(LogTemp, Warning, TEXT("%s\n"), *FString(sstream.str().c_str()));
}

//��ɫ���ݵ�ӳ���ϵ
static float colorAdjust(float srcColor)
{
	return srcColor * 0.2820948f + 0.5f;
}

//��͸���ȵĵ���
static float adjustOpacity(float opacity)
{
	return 1.f / (1 + std::exp(-opacity));
}

//��scale���ݵĵ���
static float sizeAdjust(float scale)
{
	return std::exp(scale);
}

//����ת��Ϣ�Ĵ���
static FVector4f rotAdjust(float* src)
{
	auto srcRotation = FVector4f(src[0], src[1], src[2], src[3]);
	//����֮�������
	auto vecLength = srcRotation.Size();
	if (vecLength > 0)
		return srcRotation / vecLength;
	return srcRotation;
}

//��ӡpoint buffer���������
// ��֮ǰ��Щ��ͬ���ǣ������ӡ���Ǵ����������
static void printDataInBuffer(float* pointBuffer)
{
	//��ӡx,y,z����
	std::stringstream sstream;
	for (int i = 0; i < 3; ++i)
		sstream << pointBuffer[i] << " ";
	sstream << "size: ";
	//��ӡsize����
	for (int i = 0; i < 3; ++i)
	{
		sstream << sizeAdjust(pointBuffer[55 + i]) << " ";
	}
	//��ӡ��������ת����
	sstream << "rot: ";
	for (int i = 0; i < 4; ++i)
	{
		sstream << pointBuffer[58 + i] << " ";
	}
	//��ӡ��ת����
	auto rotResult = rotAdjust(&pointBuffer[58]);
	sstream << "rot adjust: ";
	for (int i = 0; i < 4; ++i)
	{
		sstream << rotResult[i] << " ";
	}
	UE_LOG(LogTemp, Warning, TEXT("%s\n"), *FString(sstream.str().c_str()));
}

//����ɫ�������ݸ�ֵ
//Ū��������ʽ��Ϊ�˸�������ϵͳ������������
static void assignTextureData(
	std::ifstream& fileHandle, //���ڶ�ȡ���ı�������
	FFloat16* posData, //λ������
	FFloat16* colorData, //��ɫ����
	FFloat16* sizeData, //ÿ����˹���size������Ϣ
	FFloat16* rotData, //ÿ����˹�����ת������Ϣ
	int pointNum,
	int pixelNum //�����������
)
{
	//���ڶ�ȡ�����������ݴ�С��Ŀǰ��ʱ��������չ��
	const int BUFFER_SIZE = 62;
	//ÿ���������������Ӧ�����ݿ�
	const int TEX_BLOCK_SIZE = 4;
	//posλ�õ����ţ�Ϊ�˼�С�������
	const float POS_SCALE = 1.f / 32;
	//���ڶ�ȡ���ݵĻ�����
	float pointBuffer[BUFFER_SIZE];
	//������ȡÿ����
	for (int idPoint = 0; idPoint < pointNum; ++idPoint)
	{
		//��ǰ��pos��color������ͷ
		auto posHead = posData + idPoint * TEX_BLOCK_SIZE;
		auto colorHead = colorData + idPoint * TEX_BLOCK_SIZE;
		auto sizeHead = sizeData + idPoint * TEX_BLOCK_SIZE;
		auto rotHead = rotData + idPoint * TEX_BLOCK_SIZE;
		//��ʱ��ȡ����
		fileHandle.read((char*)pointBuffer, sizeof(float) * BUFFER_SIZE);
		//�ж��Ƿ���Ҫ��ӡ����
		//if (idPoint < 3)
		//{
		//	printDataInBuffer(pointBuffer);
		//}
		//��¼��������
		posHead[0] = FFloat16(pointBuffer[0] * POS_SCALE);
		posHead[1] = FFloat16(-pointBuffer[2] * POS_SCALE);
		posHead[2] = FFloat16(-pointBuffer[1] * POS_SCALE);
		for (int i = 0; i < 3; ++i) colorHead[i] = FFloat16(
			colorAdjust(pointBuffer[6 + i]));
		//��������ɫ�����һ��ͨ���ĳɹ̶�ֵ1
		posHead[3] = FFloat16(0);
		//��ɫ����ֵΪ͸����
		colorHead[3] = FFloat16(adjustOpacity(pointBuffer[54]));
		for (int i = 0; i < 3; ++i)
		{
			//������¼size��Ϣ
			sizeHead[i] = FFloat16(sizeAdjust(pointBuffer[55 + i]) * POS_SCALE);
		}
		//��ȡ���������ת����
		auto adjustedRot = rotAdjust(&pointBuffer[58]);
		//������ת��Ϣ
		for (int i = 0; i < 4; ++i)
		{
			rotHead[i] = FFloat16(adjustedRot[i]);
		}
	}
	//��ӡ2200��ʼλ�õ�10����
	std::stringstream sstream;
	for (int i = 0; i < 10; ++i)
	{
		sstream << (float)sizeData[2200 + i] << " ";
	}
	UE_LOG(LogTemp, Warning, TEXT("PosData: %s\n"), *FString(sstream.str().c_str()));
	//�ж������Ƿ���ʣ����ɫ
	for (int idPoint = pointNum; idPoint < pixelNum; ++idPoint)
	{
		auto posHead = posData + idPoint * TEX_BLOCK_SIZE;
		auto colorHead = colorData + idPoint * TEX_BLOCK_SIZE;
		for (int i = 0; i < 4; ++i) posHead[i] = FFloat16(0);
		for (int i = 0; i < 4; ++i) colorHead[i] = FFloat16(0);
	}
}

//��ȡ����Ŀ�Ⱥ͸߶�
static FInt32Vector2 getTextureSize(int gaussianNum)
{
	FInt32Vector2 ret;
	//xĬ����2048
	ret.X = 2048;
	//Y ȡ ����ȡ���Ķ�����������
	ret.Y = FMath::RoundUpToPowerOfTwo((float)gaussianNum / ret.X);
	return ret;
}

//��texture������֮�������ĸ���
static void textureFinalUpdate(UTexture2D& texture)
{
	texture.UpdateResource();
	texture.AddToRoot();
}

//��ǰ����ÿ��3D gaussian�ĽǶȷ�Χ
static UTexture2D* preloadGaussianViewRange(std::string filePath,
	uint32_t gaussianNum, //����Ԥ�ڵ�gaussian�ĸ���
	uint32_t xTextureSize, 
	uint32_t yTextureSize
)
{
	//���ļ�
	std::fstream fileHandle(filePath, std::ios::in | std::ios::binary);
	if (!fileHandle.is_open())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot open %s \n"), *FString(filePath.c_str()));
	}
	//��ȡ3D gaussian�ĸ���
	uint32 fileGaussian;
	fileHandle.read((char*)&fileGaussian, sizeof(uint32));
	if (fileGaussian != gaussianNum)
	{
		UE_LOG(LogTemp, Warning, TEXT("Gaussian num not equal\n"));
	}
	//��ʼ����Χ���ݶ�Ӧ��texture
	UTexture2D* texturePtr = UTexture2D::CreateTransient(
		xTextureSize, yTextureSize, PF_FloatRGBA
	);
	//��ȡtexture����ʵ�ʵ�����
	baseConfigTexture(*texturePtr);
	//��ȡ��ɫ�������������
	auto viewData = getTextureData<FFloat16>(*texturePtr);
	//��ʱ������ȡ��Χ�Ĳ���
	float rangeBuffer[4];
	for (uint32 idView = 0; idView < gaussianNum; ++idView)
	{
		//��ȡview���������
		fileHandle.read((char*)rangeBuffer, sizeof(float) * 4);
		//��������ʱת����fp16
		auto viewHead = viewData + idView * 4;
		//�Ѷ�ȡ����4�����ݷŵ���������
		for (int i = 0; i < 4; ++i)
			viewHead[i] = FFloat16(rangeBuffer[i]);
	}
	//�Ѻ�������Ч����д��0
	uint32 pixelNum = xTextureSize * yTextureSize;
	for (uint32 idView = gaussianNum; idView < pixelNum; ++idView)
	{
		auto viewHead = viewData + idView * 4;
		for (int i = 0; i < 4; ++i)
			viewHead[i] = FFloat16(0);
	}
	fileHandle.close();
	//��texture�����ݽ���
	unlockTexture(*texturePtr);
	return texturePtr;
}

//������Ԥ����3D ��˹
void AGaussianLoader::preload3DGaussian()
{
	//Ҫ��ȡ�ĵ���·��
	std::string plyFile = "E:/temp/splat.ply";
	std::ifstream fileHandle(plyFile, std::ios::binary);
	//��ȡply������ͷ
	PlyHeader header;
	loadPlyHeader(fileHandle, header);
	//��ӡheader���������
	header.print();
	//��¼��Ч��ĸ���
	this->gaussianPointNum = header.verticeNum;
	//��ȡ����Ĵ�С
	auto textureSize = getTextureSize(header.verticeNum);
	//��ʼ����ɫ�����λ������
	this->posTexture = UTexture2D::CreateTransient(
		textureSize.X, textureSize.Y, PF_FloatRGBA
	);
	//��ʼ����ɫ����
	this->colorTexture = UTexture2D::CreateTransient(
		textureSize.X, textureSize.Y, PF_FloatRGBA
	);
	//��ʼ��size����
	this->sizeTexture = UTexture2D::CreateTransient(
		textureSize.X, textureSize.Y, PF_FloatRGBA
	);
	//��ʼ����ת����
	this->rotTexture = UTexture2D::CreateTransient(
		textureSize.X, textureSize.Y, PF_FloatRGBA
	);
	//���������ݵĻ�������
	baseConfigTexture(*posTexture);
	baseConfigTexture(*colorTexture);
	baseConfigTexture(*rotTexture);
	baseConfigTexture(*sizeTexture);
	//�����������ɫ����
	auto posData = getTextureData<FFloat16>(*posTexture);
	auto colorData = getTextureData<FFloat16>(*colorTexture);
	auto sizeData = getTextureData<FFloat16>(*sizeTexture);
	auto rotData = getTextureData<FFloat16>(*rotTexture);
	assignTextureData(fileHandle, posData, 
		colorData, sizeData, rotData, header.verticeNum,
		posTexture->GetSizeX() * posTexture->GetSizeY()
	);
	//���ֵ���˽�������
	unlockTexture(*posTexture);
	unlockTexture(*colorTexture);
	unlockTexture(*sizeTexture);
	unlockTexture(*rotTexture);
	//ͬʱ���� view range
	this->viewTextureRange = preloadGaussianViewRange("E:/temp/viewRange.bin",
		gaussianPointNum, textureSize.X, textureSize.Y);
	textureFinalUpdate(*posTexture);
	textureFinalUpdate(*colorTexture);
	textureFinalUpdate(*sizeTexture);
	textureFinalUpdate(*rotTexture);
	//����view rangeҲҪ����������������ո���
	textureFinalUpdate(*viewTextureRange);
}

FVector2D AGaussianLoader::getSampleStep()
{
	//�����û��texture���Ǿ���㷵��
	if (colorTexture == nullptr)
	{
		return FVector2D(1, 1);
	}
	return FVector2D(
		1.f / (colorTexture->GetSizeX() + 0.5f),
		1.f / (colorTexture->GetSizeY())
	);
}

int AGaussianLoader::getSampleSize(int idDim)
{
	if(colorTexture == nullptr)
		return 0;
	if (idDim == 0)
		return colorTexture->GetSizeX();
	return gaussianPointNum / colorTexture->GetSizeX() + 1;
}


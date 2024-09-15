// Fill out your copyright notice in the Description page of Project Settings.


#include "GaussianLoader.h"
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<iostream>

//ply格式下的属性
class PlyProperty
{
public:
	std::string type;
	std::string name;
};

//ply的文件格式信息
class PlyHeader
{
public:
	std::string format;
	//节点的个数
	int verticeNum = 0;
	//面片的个数
	int faceNum = 0;
	//所有的属性列表，包括节点的和面片的
	std::vector<PlyProperty> propertyList;

	//打印ply header的信息
	void print()
	{
		auto tempString = FString(format.c_str());
		UE_LOG(LogTemp, Warning, TEXT("format: %s\n"), *tempString);
		UE_LOG(LogTemp, Warning, TEXT("vertice num: %d\n"), verticeNum);
		UE_LOG(LogTemp, Warning, TEXT("face num: %d\n"), faceNum);
	}
};

//一个节点的数据
class Vertex
{
public:
	//位置
	float position[3];
	//法向量
	float normal[3];
	//球谐函数的参数
	float shs[48];
	//透明度
	float opacity;
	//三个方向的scale
	float scale[3];
	//由四元数构成的旋转
	float rotation[4];
};

//载入点云的数据头
static void loadPlyHeader(std::ifstream& fileHandle,
	PlyHeader& header
)
{
	//中间读取到的每一行的结果
	std::string line;
	while (std::getline(fileHandle, line))
	{
		//新建输入流
		std::istringstream tempStream(line);
		//读取一个单词
		std::string token;
		tempStream >> token;
		//判断是不是格式信息
		if (token == "format")
		{
			tempStream >> header.format;
		}
		//判断读取到的是不是element信息
		else if (token == "element")
		{
			//再读取element的类型
			tempStream >> token;
			//判断是节点个数还是面片的个数
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
		//再判断是否读取到了属性信息
		else if (token == "property")
		{
			//新建一个临时的属性
			PlyProperty tempProperty;
			//记录它的type和名字
			tempStream >> tempProperty.type >> tempProperty.name;
			//把属性放到列表里面
			header.propertyList.push_back(tempProperty);
		}
		//header部分的结束符
		else if (token == "end_header")
		{
			break;
		}

	}
}

//载入一个ply文件
//这仅仅是测试用的函数
static void loadPointcloud(std::vector<Vertex>& vertexList)
{
	//点云所在的文件
	std::string filePath = "E:/temp/test.ply";
	//打开文件的输入流
	std::ifstream fileHandle(filePath, std::ios::binary);
	//新建ply的头部描述符
	PlyHeader header;
	loadPlyHeader(fileHandle, header);
	UE_LOG(LogTemp, Warning, TEXT("Read ply header ok"));
	//读完之后打印一下header信息
	header.print();
	//遍历读取每个点
	std::vector<Vertex> tempVertex(header.verticeNum);
	if (sizeof(Vertex) != 62 * sizeof(float))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid vertex size"));
		throw std::runtime_error("Invalid vertex size");
	}
	int idData = 0;
	for (auto& eachVertex : tempVertex)
	{
		//读取节点数据
		fileHandle.read(reinterpret_cast<char*>(&eachVertex), sizeof(Vertex));
		//打印62个数据
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
	//遍历每个节点
	vertexList.reserve(tempVertex.size());
	for (auto& eachVertex : tempVertex)
	{
		//判断是否在半径内
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

//把节点列表转换成纹理
static void convertVertexListToTexture(
	std::vector<Vertex>& vertexList,
	UTexture2D& texture
)
{
	//获取纹理的平台数据
	auto& platformData = *texture.GetPlatformData();
	//打印platform里面的xy
	UE_LOG(LogTemp, Warning, TEXT("platform: %d %d\n"), platformData.SizeX, platformData.SizeY);
	//获取platform里面的mip数据
	auto& mip = platformData.Mips.Last();
	//打印bulk里面的内存数据，确保它是已经开辟过内存的
	UE_LOG(LogTemp, Warning, TEXT("Bulk data size %d\n"), mip.BulkData.GetBulkDataSize());
	//锁定主体数据
	auto mainData = reinterpret_cast<FFloat16*>(mip.BulkData.Lock(LOCK_READ_WRITE));
	int idData = 0;
	//遍历每个数据
	for (auto& eachVertex : vertexList)
	{
		//当前位置的头指针
		auto vertexHead = mainData + idData * 4;
		//处理xyz
		for (int i = 0; i < 3; ++i)
		{
			vertexHead[i] = FFloat16(eachVertex.position[i]);
		}
		vertexHead[3] = FFloat16(1);
		++idData;
	}
	//解锁数据
	mip.BulkData.Unlock();
}

//制作一个纯粹的球形texture
static void makeSampleTexture(UTexture2D& texture)
{
	//获取纹理的平台数据
	auto& platformData = *texture.GetPlatformData();
	//打印platform里面的xy
	UE_LOG(LogTemp, Warning, TEXT("platform: %d %d\n"), platformData.SizeX, platformData.SizeY);
	//获取platform里面的mip数据
	auto& mip = platformData.Mips.Last();
	//打印bulk里面的内存数据，确保它是已经开辟过内存的
	UE_LOG(LogTemp, Warning, TEXT("Bulk data size %d\n"), mip.BulkData.GetBulkDataSize());
	//锁定主体数据
	auto mainData = reinterpret_cast<FFloat16*>(mip.BulkData.Lock(LOCK_READ_WRITE));
	//遍历每个位置的数据
	for (int idy = 0; idy < platformData.SizeY; ++idy)
	{
		//当前行的头指针
		auto rowPtr = mainData + idy * 4 * platformData.SizeX;
		float zAngle = (float)idy / platformData.SizeY * PI / 2.f;
		//当前的纵向角度
		auto zCos = FMath::Cos(zAngle);
		auto zSin = FMath::Sin(zAngle);
		//遍历当前行
		for (int idx = 0; idx < platformData.SizeX; ++idx)
		{
			//计算当前位置的角度
			float angle = (float)idx / platformData.SizeX * 2 * PI;
			//当前位置的数据头
			auto dataHead = rowPtr + idx * 4;
			dataHead[0] = FFloat16(FMath::Sin(angle) * zSin);
			dataHead[1] = FFloat16(FMath::Cos(angle) * zSin);
			dataHead[2] = FFloat16(zCos);//可能0才是z
			dataHead[3] = FFloat16(1);
		}
	}
	//解锁数据
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
	//初始化一个节点列表用作返回值
	std::vector<Vertex> vertexList;
	//调用载入高斯数据
	loadPointcloud(vertexList);
	//打印总的点个数
	UE_LOG(LogTemp, Warning, TEXT("Value point: %d\n"), vertexList.size());
	// 生成一个纹理 需要注意，这个float RGBA是半精度
	auto tempTexture = UTexture2D::CreateTransient(
		1400, 1000, PF_FloatRGBA, "DynamicTexture"
	);
	//把节点列表转换成纹理
	convertVertexListToTexture(vertexList, *tempTexture);
	// makeSampleTexture(*tempTexture);
	//不知是做什么用的，总之试一下吧
	tempTexture->UpdateResource();
	tempTexture->AddToRoot();
	return tempTexture;
}

//对纹理数据的基础配置
static void baseConfigTexture(UTexture2D& texture)
{
	//采样方式改成最近采样
	texture.Filter = TextureFilter::TF_Nearest;
	texture.CompressionSettings = TextureCompressionSettings::TC_HDR;
	texture.SRGB = false;
	texture.NeverStream = true;
	auto platformData = texture.GetPlatformData();
	platformData->SetNumSlices(1);
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

//解锁正在读取数据的texture
static void unlockTexture(UTexture2D& texture)
{
	texture.GetPlatformData()->Mips.Last().BulkData.Unlock();
}

//一个临时的颜色管理器，用于根据id获取渐变色
class ColorManager
{
public:
	int pointNum;

	ColorManager(int pointNum)
	{
		this->pointNum = pointNum;
	}

	//获取单层的颜色分段
	float getOneColorSeg(int idPoint, //分段里面的第几个颜色 
		int segNum //总共的分段数
	)
	{
		return  (float)idPoint / segNum;
	}

	void getColor(float* color, int idPoint)
	{
		//判断是否为第1段
		if (idPoint < pointNum / 3)
		{
			//把后两种颜色置为0
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

//打印一个高斯点的数据
//直接输入进来的就是长度是62的点
static void printGaussianUnitData(float* unitData)
{
	std::stringstream sstream;
	for (int i = 0; i < 62; ++i)
	{
		sstream << unitData[i] << " ";
	}
	UE_LOG(LogTemp, Warning, TEXT("%s\n"), *FString(sstream.str().c_str()));
}

//颜色数据的映射关系
static float colorAdjust(float srcColor)
{
	return srcColor * 0.2820948f + 0.5f;
}

//对透明度的调整
static float adjustOpacity(float opacity)
{
	return 1.f / (1 + std::exp(-opacity));
}

//对scale数据的调整
static float sizeAdjust(float scale)
{
	return std::exp(scale);
}

//对旋转信息的处理
static FVector4f rotAdjust(float* src)
{
	auto srcRotation = FVector4f(src[0], src[1], src[2], src[3]);
	//正则化之后的向量
	auto vecLength = srcRotation.Size();
	if (vecLength > 0)
		return srcRotation / vecLength;
	return srcRotation;
}

//打印point buffer里面的内容
// 与之前那些不同的是，这里打印的是处理过的数据
static void printDataInBuffer(float* pointBuffer)
{
	//打印x,y,z数据
	std::stringstream sstream;
	for (int i = 0; i < 3; ++i)
		sstream << pointBuffer[i] << " ";
	sstream << "size: ";
	//打印size数据
	for (int i = 0; i < 3; ++i)
	{
		sstream << sizeAdjust(pointBuffer[55 + i]) << " ";
	}
	//打印正常的旋转数据
	sstream << "rot: ";
	for (int i = 0; i < 4; ++i)
	{
		sstream << pointBuffer[58 + i] << " ";
	}
	//打印旋转数据
	auto rotResult = rotAdjust(&pointBuffer[58]);
	sstream << "rot adjust: ";
	for (int i = 0; i < 4; ++i)
	{
		sstream << rotResult[i] << " ";
	}
	UE_LOG(LogTemp, Warning, TEXT("%s\n"), *FString(sstream.str().c_str()));
}

//给颜色纹理数据赋值
//弄成这种形式是为了复用粒子系统里面的纹理采样
static void assignTextureData(
	std::ifstream& fileHandle, //正在读取的文本输入流
	FFloat16* posData, //位置数据
	FFloat16* colorData, //颜色数据
	FFloat16* sizeData, //每个高斯点的size数据信息
	FFloat16* rotData, //每个高斯点的旋转数据信息
	int pointNum,
	int pixelNum //纹理的像素数
)
{
	//用于读取缓冲区的数据大小，目前暂时不考虑扩展性
	const int BUFFER_SIZE = 62;
	//每个点在纹理里面对应的数据块
	const int TEX_BLOCK_SIZE = 4;
	//pos位置的缩放，为了减小量化误差
	const float POS_SCALE = 1.f / 32;
	//用于读取数据的缓冲区
	float pointBuffer[BUFFER_SIZE];
	//遍历读取每个点
	for (int idPoint = 0; idPoint < pointNum; ++idPoint)
	{
		//当前的pos和color的数据头
		auto posHead = posData + idPoint * TEX_BLOCK_SIZE;
		auto colorHead = colorData + idPoint * TEX_BLOCK_SIZE;
		auto sizeHead = sizeData + idPoint * TEX_BLOCK_SIZE;
		auto rotHead = rotData + idPoint * TEX_BLOCK_SIZE;
		//临时读取数据
		fileHandle.read((char*)pointBuffer, sizeof(float) * BUFFER_SIZE);
		//判断是否需要打印数据
		//if (idPoint < 3)
		//{
		//	printDataInBuffer(pointBuffer);
		//}
		//记录坐标数据
		posHead[0] = FFloat16(pointBuffer[0] * POS_SCALE);
		posHead[1] = FFloat16(-pointBuffer[2] * POS_SCALE);
		posHead[2] = FFloat16(-pointBuffer[1] * POS_SCALE);
		for (int i = 0; i < 3; ++i) colorHead[i] = FFloat16(
			colorAdjust(pointBuffer[6 + i]));
		//把两个颜色的最后一个通道改成固定值1
		posHead[3] = FFloat16(0);
		//颜色被赋值为透明度
		colorHead[3] = FFloat16(adjustOpacity(pointBuffer[54]));
		for (int i = 0; i < 3; ++i)
		{
			//遍历记录size信息
			sizeHead[i] = FFloat16(sizeAdjust(pointBuffer[55 + i]) * POS_SCALE);
		}
		//获取处理过的旋转数据
		auto adjustedRot = rotAdjust(&pointBuffer[58]);
		//保存旋转信息
		for (int i = 0; i < 4; ++i)
		{
			rotHead[i] = FFloat16(adjustedRot[i]);
		}
	}
	//打印2200开始位置的10个数
	std::stringstream sstream;
	for (int i = 0; i < 10; ++i)
	{
		sstream << (float)sizeData[2200 + i] << " ";
	}
	UE_LOG(LogTemp, Warning, TEXT("PosData: %s\n"), *FString(sstream.str().c_str()));
	//判断纹理是否有剩余颜色
	for (int idPoint = pointNum; idPoint < pixelNum; ++idPoint)
	{
		auto posHead = posData + idPoint * TEX_BLOCK_SIZE;
		auto colorHead = colorData + idPoint * TEX_BLOCK_SIZE;
		for (int i = 0; i < 4; ++i) posHead[i] = FFloat16(0);
		for (int i = 0; i < 4; ++i) colorHead[i] = FFloat16(0);
	}
}

//获取纹理的宽度和高度
static FInt32Vector2 getTextureSize(int gaussianNum)
{
	FInt32Vector2 ret;
	//x默认是2048
	ret.X = 2048;
	//Y 取 向上取整的二的整数次幂
	ret.Y = FMath::RoundUpToPowerOfTwo((float)gaussianNum / ret.X);
	return ret;
}

//给texture处理完之后，做最后的更新
static void textureFinalUpdate(UTexture2D& texture)
{
	texture.UpdateResource();
	texture.AddToRoot();
}

//提前载入每个3D gaussian的角度范围
static UTexture2D* preloadGaussianViewRange(std::string filePath,
	uint32_t gaussianNum, //这是预期的gaussian的个数
	uint32_t xTextureSize, 
	uint32_t yTextureSize
)
{
	//打开文件
	std::fstream fileHandle(filePath, std::ios::in | std::ios::binary);
	if (!fileHandle.is_open())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot open %s \n"), *FString(filePath.c_str()));
	}
	//读取3D gaussian的个数
	uint32 fileGaussian;
	fileHandle.read((char*)&fileGaussian, sizeof(uint32));
	if (fileGaussian != gaussianNum)
	{
		UE_LOG(LogTemp, Warning, TEXT("Gaussian num not equal\n"));
	}
	//初始化范围数据对应的texture
	UTexture2D* texturePtr = UTexture2D::CreateTransient(
		xTextureSize, yTextureSize, PF_FloatRGBA
	);
	//获取texture里面实际的数据
	baseConfigTexture(*texturePtr);
	//获取颜色纹理里面的数据
	auto viewData = getTextureData<FFloat16>(*texturePtr);
	//临时用来读取范围的操作
	float rangeBuffer[4];
	for (uint32 idView = 0; idView < gaussianNum; ++idView)
	{
		//读取view里面的数据
		fileHandle.read((char*)rangeBuffer, sizeof(float) * 4);
		//把数据临时转换成fp16
		auto viewHead = viewData + idView * 4;
		//把读取到的4个数据放到纹理里面
		for (int i = 0; i < 4; ++i)
			viewHead[i] = FFloat16(rangeBuffer[i]);
	}
	//把后续的无效纹理写成0
	uint32 pixelNum = xTextureSize * yTextureSize;
	for (uint32 idView = gaussianNum; idView < pixelNum; ++idView)
	{
		auto viewHead = viewData + idView * 4;
		for (int i = 0; i < 4; ++i)
			viewHead[i] = FFloat16(0);
	}
	fileHandle.close();
	//给texture的数据解锁
	unlockTexture(*texturePtr);
	return texturePtr;
}

//在这里预加载3D 高斯
void AGaussianLoader::preload3DGaussian()
{
	//要读取的点云路径
	std::string plyFile = "E:/temp/splat.ply";
	std::ifstream fileHandle(plyFile, std::ios::binary);
	//读取ply的数据头
	PlyHeader header;
	loadPlyHeader(fileHandle, header);
	//打印header里面的内容
	header.print();
	//记录有效点的个数
	this->gaussianPointNum = header.verticeNum;
	//获取纹理的大小
	auto textureSize = getTextureSize(header.verticeNum);
	//初始化颜色纹理和位置纹理
	this->posTexture = UTexture2D::CreateTransient(
		textureSize.X, textureSize.Y, PF_FloatRGBA
	);
	//初始化颜色参数
	this->colorTexture = UTexture2D::CreateTransient(
		textureSize.X, textureSize.Y, PF_FloatRGBA
	);
	//初始化size纹理
	this->sizeTexture = UTexture2D::CreateTransient(
		textureSize.X, textureSize.Y, PF_FloatRGBA
	);
	//初始化旋转纹理
	this->rotTexture = UTexture2D::CreateTransient(
		textureSize.X, textureSize.Y, PF_FloatRGBA
	);
	//对纹理数据的基本配置
	baseConfigTexture(*posTexture);
	baseConfigTexture(*colorTexture);
	baseConfigTexture(*rotTexture);
	baseConfigTexture(*sizeTexture);
	//开启纹理的颜色数据
	auto posData = getTextureData<FFloat16>(*posTexture);
	auto colorData = getTextureData<FFloat16>(*colorTexture);
	auto sizeData = getTextureData<FFloat16>(*sizeTexture);
	auto rotData = getTextureData<FFloat16>(*rotTexture);
	assignTextureData(fileHandle, posData, 
		colorData, sizeData, rotData, header.verticeNum,
		posTexture->GetSizeX() * posTexture->GetSizeY()
	);
	//最后赋值完了解锁数据
	unlockTexture(*posTexture);
	unlockTexture(*colorTexture);
	unlockTexture(*sizeTexture);
	unlockTexture(*rotTexture);
	//同时载入 view range
	this->viewTextureRange = preloadGaussianViewRange("E:/temp/viewRange.bin",
		gaussianPointNum, textureSize.X, textureSize.Y);
	textureFinalUpdate(*posTexture);
	textureFinalUpdate(*colorTexture);
	textureFinalUpdate(*sizeTexture);
	textureFinalUpdate(*rotTexture);
	//对于view range也要在最后调用纹理的最终更新
	textureFinalUpdate(*viewTextureRange);
}

FVector2D AGaussianLoader::getSampleStep()
{
	//如果还没有texture，那就随便返回
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


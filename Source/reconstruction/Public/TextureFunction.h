// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class RECONSTRUCTION_API TextureFunction
{
public:
	TextureFunction();

	//��texture�����պ���
	static void textureFinalUpdate(UTexture2D& texture);
	//����texture
	static void unlockTexture(UTexture2D& texture);

	~TextureFunction();
};

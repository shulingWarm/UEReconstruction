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

	//对texture的最终后处理
	static void textureFinalUpdate(UTexture2D& texture);
	//解锁texture
	static void unlockTexture(UTexture2D& texture);

	~TextureFunction();
};

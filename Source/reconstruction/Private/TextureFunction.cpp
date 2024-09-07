// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureFunction.h"

TextureFunction::TextureFunction()
{
}

void TextureFunction::textureFinalUpdate(UTexture2D& texture)
{
	texture.UpdateResource();
	texture.AddToRoot();
}

void TextureFunction::unlockTexture(UTexture2D& texture)
{
	texture.GetPlatformData()->Mips.Last().BulkData.Unlock();
}

TextureFunction::~TextureFunction()
{
}

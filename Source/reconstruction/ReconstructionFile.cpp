// Fill out your copyright notice in the Description page of Project Settings.


#include "ReconstructionFile.h"

#include "Runtime\Core\Public\Misc\FileHelper.h"
#include "Runtime\Core\Public\Misc\Paths.h"
#include "Developer\DesktopPlatform\Public\IDesktopPlatform.h"
#include "Developer\DesktopPlatform\Public\DesktopPlatformModule.h"

FString UReconstructionFile::getImageFolder()
{
	TArray<FString> OpenFileNames;//��ȡ���ļ�����·��
	FString ExtensionStr = TEXT("*.*");//�ļ�����

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	FString retFolder;
	DesktopPlatform->OpenDirectoryDialog(
		nullptr, TEXT("ѡ��ͼƬ"), "", retFolder);
	UE_LOG(LogTemp,Warning,TEXT("%s\n"),*retFolder);
	return retFolder;
}

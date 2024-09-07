// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class ExternalLibrary : ModuleRules
{
    public ExternalLibrary(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
				"E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main"
            }
            );

        //直接强行添加相关的库文件
        PublicAdditionalLibraries.AddRange(
            new string[]
            {
                "E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main\\build\\Release\\*.lib",
                "E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main\\build\\ColmapServer\\Release\\ColmapServer.lib",
                "E:\\lib\\libtorch\\libtorch\\lib\\*.lib",
                "E:\\lib\\opencv490\\opencv\\build\\x64\\vc16\\lib\\*.lib",
                "E:\\lib\\vcpkg\\installed\\x64-windows\\lib\\*.lib",
                "E:\\lib\\cuda-win\\lib\\x64\\*.lib"
            }
        );
    }
}

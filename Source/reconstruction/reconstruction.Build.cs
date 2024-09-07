// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class reconstruction : ModuleRules
{
	public reconstruction(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", "CoreUObject", "Engine", "InputCore"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

    //    PublicIncludePaths.AddRange(
    //        new string[] {
				//// ... add public include paths required here ...
				//"E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main"
    //        }
    //        );

    //    //直接强行添加相关的库文件
    //    PublicAdditionalLibraries.AddRange(
    //        new string[]
    //        {
    //            "E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main\\build\\Release\\*.lib",
    //            "E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main\\build\\ColmapServer\\Release\\ColmapServer.lib",
    //            "E:\\lib\\libtorch\\libtorch\\lib\\*.lib",
    //            "E:\\lib\\opencv490\\opencv\\build\\x64\\vc16\\lib\\*.lib",
    //            "E:\\lib\\vcpkg\\installed\\x64-windows\\lib\\*.lib",
    //            "E:\\lib\\cuda-win\\lib\\x64\\*.lib"
    //        }
    //    );

        //PublicDelayLoadDLLs.AddRange(
        //new string[]
        //    {
        //        "E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main\\build\\Release",
        //        "E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main\\build\\Release\\c10.dll",
        //        "E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main\\build\\Release\\c10_cuda.dll",
        //        "E:\\lib\\libtorch\\libtorch\\lib\\*.dll",
        //        "E:\\privateSpace\\workSpace\\504OpenSplat\\OpenSplat-main\\OpenSplat-main\\build\\Release\\opencv_world490.dll"
        //    }
        //);

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}

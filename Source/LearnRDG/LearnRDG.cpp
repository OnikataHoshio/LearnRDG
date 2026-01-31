// Fill out your copyright notice in the Description page of Project Settings.

#include "LearnRDG.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h" 

class FLearnRDGGameModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		const FString ShaderDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/MyShaders"), ShaderDir);
	}

	virtual void ShutdownModule() override
	{

	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FLearnRDGGameModule, LearnRDG, "LearnRDG");

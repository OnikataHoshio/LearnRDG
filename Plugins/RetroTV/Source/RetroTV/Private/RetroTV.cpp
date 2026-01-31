// Copyright Epic Games, Inc. All Rights Reserved.

#include "RetroTV.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FRetroTVModule"

void FRetroTVModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString PluginShaderDir = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("/RetroTV/Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugins/RetroTVShaders"), PluginShaderDir);
}

void FRetroTVModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRetroTVModule, RetroTV)
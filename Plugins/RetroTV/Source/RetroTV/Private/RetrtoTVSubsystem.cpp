// Fill out your copyright notice in the Description page of Project Settings.


#include "RetroTVSubsystem.h"
#include "RetroTVSettings.h"
#include "RetroTVSceneViewExtension.h"

inline void URetroTVSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	CustomSceneViewExtension = FSceneViewExtensions::NewExtension<FRetroTVSceneViewExtension>();
	UE_LOG(LogTemp, Log, TEXT("SceneViewExtensionTemplate: Subsystem initialized & SceneViewExtension created"));

	const URetroTVSettings* Settings = GetDefault<URetroTVSettings>();
	UTexture2D* TVMaskTexturePtr = Settings ? Settings->TVMaskTexture.LoadSynchronous() : nullptr;
	UTexture2D* ScanLineTexturePtr = Settings ? Settings->ScanLineTexture.LoadSynchronous() : nullptr;

	if (!TVMaskTexturePtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("RetroTV: TVMaskTexture not set or failed to load. (Project Settings -> RetroTV)"));
		return;
	}

	if (!ScanLineTexturePtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("RetroTV: ScanLineTexture not set or failed to load. (Project Settings -> RetroTV)"));
		return;
	}

	CustomSceneViewExtension->SetTVMaskTexture(TVMaskTexturePtr);
	UE_LOG(LogTemp, Log, TEXT("RetroTV: TVMaskTexture set to %s"), *TVMaskTexturePtr->GetName());

	CustomSceneViewExtension->SetScanLineTexture(ScanLineTexturePtr);
	UE_LOG(LogTemp, Log, TEXT("RetroTV: ScanLineTexture set to %s"), *ScanLineTexturePtr->GetName());
}

inline void URetroTVSubsystem::Deinitialize()
{
	{
		CustomSceneViewExtension->IsActiveThisFrameFunctions.Empty();

		FSceneViewExtensionIsActiveFunctor IsActiveFunctor;

		IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
			{
				return TOptional<bool>(false);
			};

		CustomSceneViewExtension->IsActiveThisFrameFunctions.Add(IsActiveFunctor);
	}

	CustomSceneViewExtension.Reset();
	CustomSceneViewExtension = nullptr;
}

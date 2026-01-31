// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SceneViewExtension.h"
/**
 * 
 */
class RETROTV_API FRetroTVSceneViewExtension : public FSceneViewExtensionBase
{
public:
	FRetroTVSceneViewExtension(const FAutoRegister& AutoRegister);

public:
	void SetTVMaskTexture(const UTexture2D* InTexture);
	void SetScanLineTexture(const UTexture2D* InTexture);

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void  BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};

	virtual void SubscribeToPostProcessingPass(EPostProcessingPass PassId, const FSceneView& View, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled) override;

	FScreenPassTexture CustomPostProcessing(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& Inputs);


private:
	FTextureRHIRef TVMaskTextureRHI;

	FTextureRHIRef ScanLineTextureRHI;
};

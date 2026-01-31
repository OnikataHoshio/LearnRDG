// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
 
/**
 * 
 */
namespace LearnRDGShader
{
	void AddTestComputePass(FRDGBuilder& GraphBuilder, float Scale, float Translate, FRHIGPUBufferReadback* Readback) ;

	void AddTestGraphicsPass(FRDGBuilder& GraphBuilder, FRDGTextureRef RenderTarget, float BValue);

	void AddTestFullscreenPass(FRDGBuilder& GraphBuilder, FRDGTextureRef RenderTarget, float BValue);

}

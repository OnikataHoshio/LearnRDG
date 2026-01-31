// Fill out your copyright notice in the Description page of Project Settings.


#include "GraphicsRDGHelper.h"
#include "RenderGraphBuilder.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LearnRDGShader.h"
// Sets default values
AGraphicsRDGHelper::AGraphicsRDGHelper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGraphicsRDGHelper::BeginPlay()
{
	Super::BeginPlay();

	check(GEngine != nullptr);

	if (IsValid(GraphicsRDGRenderTarget))
		AddTestGraphicsPass(GraphicsRDGRenderTarget.Get(), BValue);

	if (IsValid(FullScreenRDGRenderTarget))
		AddTestFullscreenPass(FullScreenRDGRenderTarget.Get(), BValue);
}

// Called every frame
void AGraphicsRDGHelper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGraphicsRDGHelper::AddTestGraphicsPass(UTextureRenderTarget2D* RenderTarget2D, float BChannelValue) const
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("AddTestGraphicsPass"));

	FTextureResource* RenderResource = RenderTarget2D->GetResource();

	ENQUEUE_RENDER_COMMAND(AddTestGraphicsPass)(
		[RenderResource, BChannelValue](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			FTextureRHIRef TextureRHI = RenderResource->GetTextureRHI();

			FRDGTextureRef RDGTexture =
				GraphBuilder.RegisterExternalTexture(
					CreateRenderTarget(TextureRHI, TEXT("MyRT2D_External"))
				);

			LearnRDGShader::AddTestGraphicsPass(GraphBuilder, RDGTexture, BChannelValue);

			GraphBuilder.Execute();
		}
		);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("AddTestGraphicsPassFinish"));
}

void AGraphicsRDGHelper::AddTestFullscreenPass(UTextureRenderTarget2D* RenderTarget2D, float BChannelValue) const
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("AddTestFullScreenPass"));

	FTextureResource* RenderResource = RenderTarget2D->GetResource();

	ENQUEUE_RENDER_COMMAND(AddTestFullscreenPass)(
		[RenderResource, BChannelValue](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList);

			FTextureRHIRef TextureRHI = RenderResource->GetTextureRHI();

			FRDGTextureRef RDGTexture =
				GraphBuilder.RegisterExternalTexture(
					CreateRenderTarget(TextureRHI, TEXT("MyRT2D_External_FullScreen"))
				);

			LearnRDGShader::AddTestFullscreenPass(GraphBuilder, RDGTexture, BChannelValue);

			GraphBuilder.Execute();
		}
		);

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("AddTestFullScreenPassFinish"));
}


// Fill out your copyright notice in the Description page of Project Settings.

#include "LearnRDGShader.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphDefinitions.h"
#include "RenderGraphUtils.h"
#include "RHIGPUReadback.h"
#include "ScreenPass.h"
#include "PixelShaderUtils.h"

class LEARNRDG_API FTestCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FTestCS);
	SHADER_USE_PARAMETER_STRUCT(FTestCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(float, Scale)
		SHADER_PARAMETER(float, Translate)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float>, InputBuffer)
		SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float>, OutputBuffer)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

IMPLEMENT_GLOBAL_SHADER(FTestCS, "/MyShaders/ComputeShader.usf", "MainCS", SF_Compute);

void LearnRDGShader::AddTestComputePass(FRDGBuilder& GraphBuilder, float Scale, float Translate, FRHIGPUBufferReadback* Readback)
{
	constexpr uint32 BufferElementNum = 2;
	constexpr uint32 BufferSize = sizeof(float) * BufferElementNum;

	TShaderMapRef<FTestCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	TArray<float> InitialData;
	InitialData.Init(10.0f, BufferElementNum);

	FRDGBufferRef InRDGBuffer = CreateStructuredBuffer(
		GraphBuilder, TEXT("TestCS.InRDGBuffer"), sizeof(float), BufferElementNum,
		InitialData.GetData(), BufferSize
	);

	FRDGBufferSRVRef InSRV = GraphBuilder.CreateSRV(InRDGBuffer);

	FRDGBufferRef OutRDGBuffer = GraphBuilder.CreateBuffer(
		FRDGBufferDesc::CreateStructuredDesc(sizeof(float), BufferElementNum),
		TEXT("TestCS.OutRDGBuffer")
	);
	FRDGBufferUAVRef OutUAV = GraphBuilder.CreateUAV(OutRDGBuffer);

	FTestCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FTestCS::FParameters>();
	PassParameters->Scale = Scale;
	PassParameters->Translate = Translate;
	PassParameters->InputBuffer = InSRV;
	PassParameters->OutputBuffer = OutUAV;

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("MyAddCS"),
		PassParameters,
		ERDGPassFlags::Compute,
		[PassParameters, ComputeShader, OutRDGBuffer, Readback](FRHICommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *PassParameters, FIntVector(1, 1, 1));
		}
	);

	AddEnqueueCopyPass(GraphBuilder, Readback, OutRDGBuffer, BufferSize);
}

class LEARNRDG_API FTestVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FTestVS);
	SHADER_USE_PARAMETER_STRUCT(FTestVS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )

	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

IMPLEMENT_GLOBAL_SHADER(FTestVS, "/MyShaders/GraphicsShader.usf", "MainVS", SF_Vertex);

class LEARNRDG_API FTestPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FTestPS);
	SHADER_USE_PARAMETER_STRUCT(FTestPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER(float, BValue)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

IMPLEMENT_GLOBAL_SHADER(FTestPS, "/MyShaders/GraphicsShader.usf", "MainPS", SF_Pixel);

void LearnRDGShader::AddTestGraphicsPass(FRDGBuilder& GraphBuilder, FRDGTextureRef RenderTarget, float BValue)
{
	constexpr uint32 NumVertices = 4;
	constexpr uint32 NumIndices = 6;
	const FIntRect ViewRect(0, 0, RenderTarget->Desc.Extent.X, RenderTarget->Desc.Extent.Y);


	FGlobalShaderMap* ShaderMap =  GetGlobalShaderMap(GMaxRHIFeatureLevel);

	TShaderMapRef<FTestVS> VertexShader(ShaderMap);
	TShaderMapRef<FTestPS> PixelShader(ShaderMap);

	struct VS_INPUT
	{
		FVector2f Position;
		FVector2f TexCoord;
	};

	FRDGBufferRef VertexBuffer = GraphBuilder.CreateBuffer(
		FRDGBufferDesc::CreateUploadDesc(sizeof(VS_INPUT), NumVertices),
		TEXT("MyVertexBuffer"));

	FRDGUploadData<VS_INPUT> VSInputData(GraphBuilder, 4);
	VSInputData[0] = { FVector2f(-1.0f,  1.0f), FVector2f(0.0f, 0.0f) };
	VSInputData[1] = { FVector2f(1.0f,  1.0f), FVector2f(1.0f, 0.0f) };
	VSInputData[2] = { FVector2f(-1.0f, -1.0f), FVector2f(0.0f, 1.0f) };
	VSInputData[3] = { FVector2f(1.0f, -1.0f), FVector2f(1.0f, 1.0f) };

	GraphBuilder.QueueBufferUpload(VertexBuffer, VSInputData, ERDGInitialDataFlags::NoCopy);

	FRDGBufferRef IndexBuffer = GraphBuilder.CreateBuffer(
		FRDGBufferDesc::CreateUploadDesc(sizeof(uint32), NumIndices),
		TEXT("MyIndexBuffer"));

	FRDGUploadData<int32> Indices(GraphBuilder, 6);
	Indices[0] = 0;
	Indices[1] = 1;
	Indices[2] = 2;
	Indices[3] = 2;
	Indices[4] = 1;
	Indices[5] = 3;

	GraphBuilder.QueueBufferUpload(IndexBuffer, Indices, ERDGInitialDataFlags::NoCopy);

	FTestPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FTestPS::FParameters>();
	PassParameters->BValue = BValue;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTarget, ERenderTargetLoadAction::EClear);

	uint16 Stride = sizeof(VS_INPUT);
	FVertexDeclarationElementList Elements;
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(VS_INPUT, Position), VET_Float2, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(VS_INPUT, TexCoord), VET_Float2, 1, Stride));
	FVertexDeclarationRHIRef VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("MyTestGraphicsPass"),
		PassParameters,
		ERDGPassFlags::Raster,
		[VertexShader, PixelShader, PassParameters, VertexBuffer, IndexBuffer, ViewRect, VertexDeclarationRHI](FRHICommandList& RHICmdList)
		{
			RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDeclarationRHI;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

			SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PassParameters);

			RHICmdList.SetStreamSource(0, VertexBuffer->GetRHI(), 0);

			RHICmdList.DrawIndexedPrimitive(IndexBuffer->GetRHI(), 0, 0, 4, 0, 2, 1);
		}
	);
}

class LEARNRDG_API FTestFullscreenPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FTestFullscreenPS);
	SHADER_USE_PARAMETER_STRUCT(FTestFullscreenPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(float, BValue)
		SHADER_PARAMETER(FVector2f, TargetSize)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

IMPLEMENT_GLOBAL_SHADER(FTestFullscreenPS, "/MyShaders/FullscreenShader.usf", "FullscreenPS", SF_Pixel);

void LearnRDGShader::AddTestFullscreenPass(FRDGBuilder& GraphBuilder, FRDGTextureRef RenderTarget, float BValue)
{
	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

	TShaderMapRef<FTestFullscreenPS> PixelShader(ShaderMap);

	FTestFullscreenPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FTestFullscreenPS::FParameters>();
	PassParameters->BValue = BValue;
	PassParameters->TargetSize = RenderTarget->Desc.Extent;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTarget, ERenderTargetLoadAction::EClear);

	FPixelShaderUtils::AddFullscreenPass(
		GraphBuilder,
		ShaderMap,
		RDG_EVENT_NAME("AddFullscreenPass"),
		PixelShader,
		PassParameters,
		FIntRect(FIntPoint::ZeroValue, RenderTarget->Desc.Extent));
}



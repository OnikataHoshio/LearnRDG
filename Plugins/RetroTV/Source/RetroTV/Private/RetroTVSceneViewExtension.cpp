// Fill out your copyright notice in the Description page of Project Settings.


#include "RetroTVSceneViewExtension.h"
#include "RenderGraphUtils.h"
#include "ScreenPass.h"

#include "PostProcess/PostProcessMaterial.h"
#include "DataDrivenShaderPlatformInfo.h"

class RETROTV_API FLenDistortionCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FLenDistortionCS);
	SHADER_USE_PARAMETER_STRUCT(FLenDistortionCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, SceneColorViewport)
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER(float, RetroScaleFactor)
		SHADER_PARAMETER(float, LensDistortionK1)
		SHADER_PARAMETER(float, LensDistortionK2)
		SHADER_PARAMETER(float, VignetteStart)
		SHADER_PARAMETER(float, VignetteEnd)
		SHADER_PARAMETER(float, VignettePower)
		SHADER_PARAMETER(float, VignetteIntensity)
		SHADER_PARAMETER(float, ScanlineSpeed)
		SHADER_PARAMETER(float, ScanlineIntensity)
		SHADER_PARAMETER(float, ScanlineScale)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, OriginalSceneColor)
		SHADER_PARAMETER_TEXTURE(Texture2D, TVMaskTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, ScanLineTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("THREADS_X"), 8);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), 8);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), 1);
	}
};

IMPLEMENT_GLOBAL_SHADER(FLenDistortionCS, "/Plugins/RetroTVShaders/LensDistortion.usf", "MainCS", SF_Compute);

namespace
{
	TAutoConsoleVariable<int32> CVarShaderOn(
		TEXT("r.RetroTV"),
		0,
		TEXT("Enable RetroTV SceneViewExtension \n")
		TEXT(" 0: OFF;")
		TEXT(" 1: ON."),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarScaleFactor(
		TEXT("r.RetroTV.ScaleFactor"),
		1.0f,
		TEXT("Update ScaleFactor \n"),
		ECVF_RenderThreadSafe);
}


namespace
{
	TAutoConsoleVariable<float> CVarLenDistortionK1(
		TEXT("r.RetroTV.LenDistortionK1"),
		0.05f,
		TEXT("Update LenDistortionK1 \n"),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarLenDistortionK2(
		TEXT("r.RetroTV.LenDistortionK2"),
		0.02f,
		TEXT("Update LenDistortionK2 \n"),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarVignetteStart(
		TEXT("r.RetroTV.VignetteStart"),
		0.7f,
		TEXT("Update VignetteStart \n"),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarVignetteEnd(
		TEXT("r.RetroTV.VignetteEnd"),
		1.0f,
		TEXT("Update VignetteEnd \n"),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarVignettePower(
		TEXT("r.RetroTV.VignettePower"),
		2.0f,
		TEXT("Update VignettePower \n"),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarVignetteIntensity(
		TEXT("r.RetroTV.VignetteIntensity"),
		0.7f,
		TEXT("Update VignetteIntensity \n"),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarScanlineSpeed(
		TEXT("r.RetroTV.ScanlineSpeed"),
		0.2f,
		TEXT("Update ScanlineSpeed \n"),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarScanlineIntensity(
		TEXT("r.RetroTV.ScanlineIntensity"),
		0.7f,
		TEXT("Update ScanlineIntensity \n"),
		ECVF_RenderThreadSafe);
}

namespace
{
	TAutoConsoleVariable<float> CVarScanlineScale(
		TEXT("r.RetroTV.ScanlineScale"),
		5.0f,
		TEXT("Update ScanlineScale \n"),
		ECVF_RenderThreadSafe);
}


void FRetroTVSceneViewExtension::SetTVMaskTexture(const UTexture2D* InTexture)
{
	FTextureRHIRef NewRHI = nullptr;

	if (InTexture && InTexture->GetResource())
	{
		NewRHI = InTexture->GetResource()->TextureRHI;
	}

	ENQUEUE_RENDER_COMMAND(RetroTV_SetExtraTexture)
		(
			[this, NewRHI](FRHICommandListImmediate& RHICmdList)
			{
				TVMaskTextureRHI = NewRHI;
			});
}

void FRetroTVSceneViewExtension::SetScanLineTexture(const UTexture2D* InTexture)
{
	FTextureRHIRef NewRHI = nullptr;

	if (InTexture && InTexture->GetResource())
	{
		NewRHI = InTexture->GetResource()->TextureRHI;
	}

	ENQUEUE_RENDER_COMMAND(RetroTV_SetExtraTexture)
		(
			[this, NewRHI](FRHICommandListImmediate& RHICmdList)
			{
				ScanLineTextureRHI = NewRHI;
			});
}


FRetroTVSceneViewExtension::FRetroTVSceneViewExtension(const FAutoRegister& AutoRegister):FSceneViewExtensionBase(AutoRegister)
{
	UE_LOG(LogTemp, Log, TEXT("RetroTV : FRetroTVSceneViewExtension registered"));
}

void FRetroTVSceneViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId, const FSceneView& View,
	FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
	if (PassId == EPostProcessingPass::MotionBlur)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FRetroTVSceneViewExtension::CustomPostProcessing));
	}
}

FScreenPassTexture FRetroTVSceneViewExtension::CustomPostProcessing(FRDGBuilder& GraphBuilder, const FSceneView& SceneView,
	const FPostProcessMaterialInputs& Inputs)
{
	const FSceneViewFamily& ViewFamily = *SceneView.Family;

	const FScreenPassTexture& SceneColor = FScreenPassTexture::CopyFromSlice(GraphBuilder, Inputs.GetInput(EPostProcessMaterialInput::SceneColor));

	if (!SceneColor.IsValid() || CVarShaderOn.GetValueOnRenderThread() == 0)
	{
		return SceneColor;
	}

	const FScreenPassTextureViewport SceneColorViewport(SceneColor);

	RDG_EVENT_SCOPE(GraphBuilder, "RetroTV Postprocess Effect");
	{
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(ViewFamily.GetFeatureLevel());
		TShaderMapRef<FLenDistortionCS> ComputeShader(GlobalShaderMap);

		FRDGTextureDesc OutputDesc;
		{
			OutputDesc = SceneColor.Texture->Desc;
			// OutputDesc.Reset();
			OutputDesc.Flags |= TexCreate_UAV;
			OutputDesc.Flags &= ~(TexCreate_RenderTargetable | TexCreate_FastVRAM);

			FLinearColor ClearColor(0., 0., 0., 0.);
			OutputDesc.ClearValue = FClearValueBinding(ClearColor);
		}

		FRDGTextureRef OutputTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("LenDistortion Effect Output Texture"));

		FLenDistortionCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FLenDistortionCS::FParameters>();

		FIntPoint PassViewSize = SceneColor.ViewRect.Size();

		PassParameters->View = SceneView.ViewUniformBuffer;
		PassParameters->RetroScaleFactor = CVarScaleFactor.GetValueOnRenderThread();
		PassParameters->LensDistortionK1 = CVarLenDistortionK1.GetValueOnRenderThread();
		PassParameters->LensDistortionK2 = CVarLenDistortionK2.GetValueOnRenderThread();
		PassParameters->VignetteStart = CVarVignetteStart.GetValueOnRenderThread();
		PassParameters->VignetteEnd = CVarVignetteEnd.GetValueOnRenderThread();
		PassParameters->VignettePower = CVarVignettePower.GetValueOnRenderThread();
		PassParameters->VignetteIntensity = CVarVignetteIntensity.GetValueOnRenderThread();
		PassParameters->ScanlineScale = CVarScanlineScale.GetValueOnRenderThread();
		PassParameters->ScanlineSpeed = CVarScanlineSpeed.GetValueOnRenderThread();
		PassParameters->ScanlineIntensity = CVarScanlineIntensity.GetValueOnRenderThread();
		PassParameters->OriginalSceneColor = SceneColor.Texture;
		PassParameters->TVMaskTexture = TVMaskTextureRHI.IsValid() ? TVMaskTextureRHI : GWhiteTexture->TextureRHI;
		PassParameters->ScanLineTexture = ScanLineTextureRHI.IsValid()? ScanLineTextureRHI : GWhiteTexture->TextureRHI;
		PassParameters->SceneColorViewport = GetScreenPassTextureViewportParameters(SceneColorViewport);
		PassParameters->Output = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));

		FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(PassViewSize, FComputeShaderUtils::kGolden2DGroupSize);

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("LenDistorted Post Processing CS Shader %dx%d", PassViewSize.X, PassViewSize.Y),
			ComputeShader,
			PassParameters,
			GroupCount
		);

		return FScreenPassTexture(OutputTexture, SceneColor.ViewRect);
	}
}


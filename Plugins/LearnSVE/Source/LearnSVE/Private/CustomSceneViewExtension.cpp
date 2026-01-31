// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomSceneViewExtension.h"
#include "RenderGraphUtils.h"
#include "ScreenPass.h"
#include "PostProcess/PostProcessMaterial.h"
#include "DataDrivenShaderPlatformInfo.h"


class LEARNSVE_API FCustomCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FCustomCS);
	SHADER_USE_PARAMETER_STRUCT(FCustomCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, SceneColorViewport)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, OriginalSceneColor)
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

IMPLEMENT_GLOBAL_SHADER(FCustomCS, "/Plugins/LearnSVEShaders/PostProcessCS.usf", "MainCS", SF_Compute);

namespace
{
	TAutoConsoleVariable<int32> CVarShaderOn(
		TEXT("r.LearnSVE"),
		0,
		TEXT("Enable Custom SceneViewExtension \n")
		TEXT(" 0: OFF;")
		TEXT(" 1: ON."),
		ECVF_RenderThreadSafe);
}

FCustomSceneViewExtension::FCustomSceneViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(AutoRegister)
{
	UE_LOG(LogTemp, Log, TEXT("LearnSVE : Custom SceneViewExtension registered"));
}

void FCustomSceneViewExtension::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	
}

void FCustomSceneViewExtension::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	FSceneViewExtensionBase::SetupView(InViewFamily, InView);
}

void FCustomSceneViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
	FSceneViewExtensionBase::BeginRenderViewFamily(InViewFamily);
}

void FCustomSceneViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId, const FSceneView& View, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
	if (PassId == EPostProcessingPass::MotionBlur)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FCustomSceneViewExtension::CustomPostProcessing));
	}
}

FScreenPassTexture FCustomSceneViewExtension::CustomPostProcessing(FRDGBuilder& GraphBuilder, const FSceneView& SceneView,
	const FPostProcessMaterialInputs& Inputs)
{
	const FSceneViewFamily& ViewFamily = *SceneView.Family;

	const FScreenPassTexture& SceneColor = FScreenPassTexture::CopyFromSlice(GraphBuilder, Inputs.GetInput(EPostProcessMaterialInput::SceneColor));

	if (!SceneColor.IsValid() || CVarShaderOn.GetValueOnRenderThread() == 0)
	{
		return SceneColor;
	}

	const FScreenPassTextureViewport SceneColorViewport(SceneColor);

	RDG_EVENT_SCOPE(GraphBuilder, "Custom Postprocess Effect");
	{
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(ViewFamily.GetFeatureLevel());
		TShaderMapRef<FCustomCS> ComputeShader(GlobalShaderMap);

		FRDGTextureDesc OutputDesc;
		{
			OutputDesc = SceneColor.Texture->Desc;

			OutputDesc.Reset();
			OutputDesc.Flags |= TexCreate_UAV;
			OutputDesc.Flags &= ~(TexCreate_RenderTargetable | TexCreate_FastVRAM);

			FLinearColor ClearColor(0., 0., 0., 0.);
			OutputDesc.ClearValue = FClearValueBinding(ClearColor);
		}

		FRDGTextureRef OutputTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("Custom Effect Output Texture"));

		FCustomCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FCustomCS::FParameters>();

		FIntPoint PassViewSize = SceneColor.ViewRect.Size();

		PassParameters->OriginalSceneColor = SceneColor.Texture;
		PassParameters->SceneColorViewport = GetScreenPassTextureViewportParameters(SceneColorViewport);
		PassParameters->Output = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));

		FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(PassViewSize, FComputeShaderUtils::kGolden2DGroupSize);

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("Custom SceneViewExtension Post Processing CS Shader %dx%d", PassViewSize.X, PassViewSize.Y),
			ComputeShader,
			PassParameters,
			GroupCount
		);

		return FScreenPassTexture(OutputTexture, SceneColor.ViewRect);
	}

}

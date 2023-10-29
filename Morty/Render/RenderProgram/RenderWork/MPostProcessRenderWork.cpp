#include "MPostProcessRenderWork.h"
#include "MPostProcessRenderWork.h"
#include "MPostProcessRenderWork.h"
#include "MPostProcessRenderWork.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"
#include "Mesh/MMeshManager.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"


#define ACES_ENABLE false

MORTY_CLASS_IMPLEMENT(MPostProcessRenderWork, ISinglePassRenderWork)

void MPostProcessRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	InitializeMaterial();
}

void MPostProcessRenderWork::Release(MEngine* pEngine)
{
	ReleaseMaterial();

	Super::Release(pEngine);
}

void MPostProcessRenderWork::Render(MRenderInfo& info)
{
	MORTY_ASSERT(m_pInputAdapter);

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	auto pInputTexture = m_pInputAdapter->GetTexture();

	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	pCommand->AddRenderToTextureBarrier({ pInputTexture.get() });

	pCommand->BeginRenderPass(&m_renderPass);

	const Vector2i n2Size = m_renderPass.GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, n2Size.x, n2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, n2Size.x, n2Size.y));

	m_pMaterial->GetMaterialPropertyBlock()->SetTexture(MShaderPropertyName::POSTPROCESS_SCREEN_TEXTURE, pInputTexture);
	if (pCommand->SetUseMaterial(m_pMaterial))
	{
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	pCommand->EndRenderPass();
}

void MPostProcessRenderWork::SetInputTexture(const std::shared_ptr<ITextureInputAdapter>& pAdapter)
{
	m_pInputAdapter = pAdapter;
}

void MPostProcessRenderWork::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	m_pMaterial = pResourceSystem->CreateResource<MMaterial>("PostProcess Material");

	std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");

#if ACES_ENABLE
	std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_aces.mps");
#else
	std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mps");
#endif
	m_pMaterial->LoadShader(pVertexShader);
	m_pMaterial->LoadShader(pPixelShader);
	m_pMaterial->SetCullMode(MECullMode::ECullNone);

#if ACES_ENABLE
	if (auto pPropertyBlock = m_pMaterial->GetMaterialPropertyBlock())
	{
		pPropertyBlock->SetValue<float>("FilmSlope", 0.88f);
		pPropertyBlock->SetValue<float>("FilmToe", 0.55f);
		pPropertyBlock->SetValue<float>("FilmShoulder", 0.26f);
		pPropertyBlock->SetValue<float>("FilmBlackClip", 0.0f);
		pPropertyBlock->SetValue<float>("FilmWhiteClip", 0.04f);

		pPropertyBlock->SetValue<bool>("bIsTemperatureWhiteBalance", true);

		pPropertyBlock->SetValue<float>("WhiteTemp", 6500.0f);
		pPropertyBlock->SetValue<float>("WhiteTint", 0.0f);

		// Color Correction controls
		pPropertyBlock->SetValue<Vector4>("ColorSaturation", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorContrast", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorGamma", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorGain", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorOffset", Vector4(0.0f, 0.0f, 0.0f, 0.0f));

		pPropertyBlock->SetValue<Vector4>("ColorSaturationShadows", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorContrastShadows", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorGammaShadows", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorGainShadows", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorOffsetShadows", Vector4(0.0f, 0.0f, 0.0f, 0.0f));

		pPropertyBlock->SetValue<Vector4>("ColorSaturationMidtones", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorContrastMidtones", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorGammaMidtones", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorGainMidtones", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorOffsetMidtones", Vector4(0.f, 0.0f, 0.0f, 0.0f));

		pPropertyBlock->SetValue<Vector4>("ColorSaturationHighlights", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorContrastHighlights", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorGammaHighlights", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorGainHighlights", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("ColorOffsetHighlights", Vector4(0.0f, 0.0f, 0.0f, 0.0f));

		pPropertyBlock->SetValue<float>("ColorCorrectionShadowsMax", 0.09f);
		pPropertyBlock->SetValue<float>("ColorCorrectionHighlightsMin", 0.5f);
		pPropertyBlock->SetValue<float>("ColorCorrectionHighlightsMax", 1.0f);

		pPropertyBlock->SetValue<float>("BlueCorrection", 0.6f);
		pPropertyBlock->SetValue<float>("ExpandGamut", 1.0f);
		pPropertyBlock->SetValue<float>("ToneCurveAmount", 1.0);


		float MinValue = 0.0f;
		float MidValue = 0.5f;
		float MaxValue = 1.0f;

		// x is the input value, y the output value
		// RGB = a, b, c where y = a * x*x + b * x + c
		float c = MinValue;
		float b = 4 * MidValue - 3 * MinValue - MaxValue;
		float a = MaxValue - MinValue - b;

		pPropertyBlock->SetValue<Vector3>("MappingPolynomial", Vector3(a, b, c));

		pPropertyBlock->SetValue<Vector4>("ColorScale", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pPropertyBlock->SetValue<Vector4>("OverlayColor", Vector4(0.0f, 0.0f, 0.0f, 0.0f));

		Vector3 InvDisplayGammaValue;
		InvDisplayGammaValue.x = 1.0f / 2.2f;
		InvDisplayGammaValue.y = 2.2f / 2.2f;
		InvDisplayGammaValue.z = 1.0f / 2.2f;

		pPropertyBlock->SetValue<Vector3>("InverseGamma", InvDisplayGammaValue);

	}

#endif

	
}

void MPostProcessRenderWork::ReleaseMaterial()
{
	m_pMaterial = nullptr;
}

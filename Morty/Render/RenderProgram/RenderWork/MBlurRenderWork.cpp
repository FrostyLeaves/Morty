#include "MBlurRenderWork.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"

#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "RenderProgram/RenderGraph/MRenderGraphSetting.h"


MORTY_CLASS_IMPLEMENT(MBlurRenderWork, MBasicPostProcessRenderWork)

const MStringId BlurOffsetName = MStringId("Gaussian Blur Offset");
const MStringId BlurDirectionName = MStringId("Gaussian Blur Vertical");

std::shared_ptr<MMaterial> MBlurRenderWork::CreateMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	auto pBlurMaterial = pResourceSystem->FindResource<MMaterialTemplate>("blur");
	if (pBlurMaterial == nullptr)
	{
		pBlurMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("blur");
		std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("Shader/PostProcess/gaussian_blur.mvs");
		std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/PostProcess/gaussian_blur.mps");
		pBlurMaterial->LoadShader(pVertexShader);
		pBlurMaterial->LoadShader(pPixelShader);
		pBlurMaterial->SetCullMode(MECullMode::ECullNone);
	}

	return MMaterial::CreateMaterial(pBlurMaterial);
}

void MBlurRenderWork::RenderSetup(const MRenderInfo& info)
{
	if (GetRenderGraph()->GetRenderGraphSetting()->IsDirty(GetNodeName()))
	{
		auto setting = GetRenderGraph()->GetRenderGraphSetting()->GetValue<MVariantStruct>(GetNodeName());

		const float fOffset = setting.GetVariant<float>(BlurOffsetName);
		const bool bVertical = setting.GetVariant<bool>(BlurDirectionName);

		const Vector2 f2Offset = Vector2(bVertical ? 0 : (fOffset / info.f2ViewportSize.x), bVertical ? (fOffset / info.f2ViewportSize.y) : 0);

		m_pMaterial->SetValue(MShaderPropertyName::POSTPROCESS_BLUR_OFFSET, f2Offset);
	}
}

void MBlurRenderWork::RegisterSetting()
{
	MVariantStruct blurSetting;
	MVariantStructBuilder(blurSetting)
		.AppendVariant(BlurOffsetName, 2.0f)
		.AppendVariant(BlurDirectionName, m_bVertical)
		.Finish();

	GetRenderGraph()->GetRenderGraphSetting()->RegisterProperty(GetNodeName(), blurSetting);
}

std::vector<MRenderTaskInputDesc> MBlurRenderWork::InitInputDesc()
{
	MORTY_ASSERT(false);
	return {};
}

std::vector<MRenderTaskOutputDesc> MBlurRenderWork::InitOutputDesc()
{
	MORTY_ASSERT(false);
	return {};
}
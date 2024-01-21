#include "MToneMappingRenderWork.h"

#include "MForwardRenderWork.h"
#include "MTransparentRenderWork.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Basic/MViewport.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"


MORTY_CLASS_IMPLEMENT(MToneMappingRenderWork, MBasicPostProcessRenderWork)

const MStringId MToneMappingRenderWork::ToneMappingResult = MStringId("Tone Mapping Output");

std::shared_ptr<MMaterial> MToneMappingRenderWork::CreateMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	auto pEdgeMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("PostProcess Edge Detection");
	std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
	std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mps");
	pEdgeMaterial->LoadShader(pVertexShader);
	pEdgeMaterial->LoadShader(pPixelShader);
	pEdgeMaterial->SetCullMode(MECullMode::ECullNone);

	return MMaterial::CreateMaterial(pEdgeMaterial);
}

std::vector<MStringId> MToneMappingRenderWork::GetInputName()
{
	return {
		MTransparentRenderWork::BackBufferOutput
	};
}

std::vector<MRenderTaskOutputDesc> MToneMappingRenderWork::GetOutputName()
{
	return {
		{ ToneMappingResult, {true, MColor::Black_T }},
	};
}
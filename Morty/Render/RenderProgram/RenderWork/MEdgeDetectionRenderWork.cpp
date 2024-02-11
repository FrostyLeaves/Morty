#include "MEdgeDetectionRenderWork.h"

#include "MForwardRenderWork.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Basic/MViewport.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MEdgeDetectionRenderWork, MBasicPostProcessRenderWork)

const MStringId MEdgeDetectionRenderWork::EdgeDetectionResult = MStringId("Edge Detection Output");

std::shared_ptr<MMaterial> MEdgeDetectionRenderWork::CreateMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	auto pEdgeMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("PostProcess Edge Detection");
	std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
	std::shared_ptr<MResource> pEdgePixelShader = pResourceSystem->LoadResource("Shader/PostProcess/sobel_edge_detection.mps");
	pEdgeMaterial->LoadShader(pVertexShader);
	pEdgeMaterial->LoadShader(pEdgePixelShader);
	pEdgeMaterial->SetCullMode(MECullMode::ECullNone);

	return MMaterial::CreateMaterial(pEdgeMaterial);
}

std::vector<MRenderTaskInputDesc> MEdgeDetectionRenderWork::InitInputDesc()
{
	return {
		{ MForwardRenderWork::BackBufferOutput, METextureBarrierStage::EPixelShaderSample },
	};
}

std::vector<MRenderTaskOutputDesc> MEdgeDetectionRenderWork::InitOutputDesc()
{
	return {
		{ EdgeDetectionResult, {true, MColor::Black_T }},
	};
}
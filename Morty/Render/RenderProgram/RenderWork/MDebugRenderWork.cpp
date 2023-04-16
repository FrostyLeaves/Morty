#include "MDebugRenderWork.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "Resource/MMaterialResource.h"

MORTY_CLASS_IMPLEMENT(MDebugRenderWork, ISinglePassRenderWork)

void MDebugRenderWork::Render(MRenderInfo& info)
{
	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	MViewport* pViewport = info.pViewport;


	pCommand->BeginRenderPass(&m_renderPass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));





	pCommand->EndRenderPass();
}

void MDebugRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);
	InitializeMaterial();
}

void MDebugRenderWork::Release(MEngine* pEngine)
{
	ReleaseMaterial();
	Super::Release(pEngine);
}

void MDebugRenderWork::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> skyboxVS = pResourceSystem->LoadResource("Shader/skybox.mvs");
	std::shared_ptr<MResource> skyboxPS = pResourceSystem->LoadResource("Shader/skybox.mps");
	m_pSkyBoxMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pSkyBoxMaterial->SetRasterizerType(MERasterizerType::ECullNone);
	m_pSkyBoxMaterial->LoadVertexShader(skyboxVS);
	m_pSkyBoxMaterial->LoadPixelShader(skyboxPS);

}

void MDebugRenderWork::ReleaseMaterial()
{
	m_pSkyBoxMaterial = nullptr;
}

#include "MDeferredRenderProgram.h"

#include "MScene.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MFunction.h"
#include "MSkeleton.h"
#include "MTaskNode.h"
#include "MMaterial.h"
#include "MTaskNodeOutput.h"

#include "MRenderCommand.h"

#include "MSceneComponent.h"
#include "MSkyBoxComponent.h"
#include "MRenderableMeshComponent.h"

#include "MObjectSystem.h"
#include "MRenderSystem.h"
#include "MResourceSystem.h"

#include "MShadowMapRenderWork.h"
#include "MTransparentRenderWork.h"
#include "MEnvironmentMapRenderWork.h"

#include "MTextureResource.h"
#include "MMaterialResource.h"

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)

MDeferredRenderProgram::MDeferredRenderProgram()
	: MIRenderProgram()
	, m_pRenderGraph(nullptr)
	, m_renderInfo()
	, m_frameParamSet()
	, m_pShadowMapWork(nullptr)
	, m_pTransparentWork(nullptr)
	, m_nFrameIndex(0)
	, m_pFinalOutputTexture(nullptr)
	, m_gbufferRenderPass()
	, m_pPrimaryCommand(nullptr)
	, m_pLightningMaterial(nullptr)
{
	
}

MDeferredRenderProgram::~MDeferredRenderProgram()
{
}

void MDeferredRenderProgram::Render(MIRenderCommand* pPrimaryCommand)
{
	if (!GetViewport())
		return;

	m_pPrimaryCommand = pPrimaryCommand;
	m_pRenderGraph->Run();
}

void MDeferredRenderProgram::RenderReady(MTaskNode* pTaskNode)
{
	MEngine* pEngine = GetEngine();
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();
	MViewport* pViewport = GetViewport();

	m_renderInfo = MRenderInfo();
	m_renderInfo.nFrameIndex = m_nFrameIndex++;
	m_renderInfo.pViewport = pViewport;
	m_renderInfo.pPrimaryRenderCommand = m_pPrimaryCommand;


	//Update RenderInfo
	CollectRenderMesh(m_renderInfo);

	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->CollectShadowMesh(m_renderInfo);
	}


	//Update Shader Params.
	UpdateFrameParams(m_renderInfo);
	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->UpdateShadowParams(m_renderInfo);
	}

	//Resize FrameBuffer.

	Vector2 v2Size = pViewport->GetSize();

	if (m_gbufferRenderPass.GetFrameBufferSize() != v2Size)
	{
		MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

		pRenderSystem->ResizeFrameBuffer(m_gbufferRenderPass, v2Size);
		pRenderSystem->ResizeFrameBuffer(m_lightningRenderPass, v2Size);

		m_forwardRenderPass.Resize(pRenderDevice);

		if (m_pTransparentWork)
		{
			m_pTransparentWork->Resize(v2Size);
			m_pTransparentWork->SetRenderTarget(GetOutputTexture(), m_gbufferRenderPass.GetDepthTexture());
		}
	}
}

void MDeferredRenderProgram::RenderGBuffer(MTaskNode* pTaskNode)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = m_renderInfo.pPrimaryRenderCommand;

	MViewport* pViewport = m_renderInfo.pViewport;


	if (MTexture* pShadowMap = GetShadowmapTexture())
	{
		pCommand->SetRenderToTextureBarrier({ pShadowMap });
	}

	pCommand->BeginRenderPass(&m_gbufferRenderPass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	DrawStaticMesh(m_renderInfo, pCommand, m_renderInfo.m_tDeferredMaterialGroupMesh);

	pCommand->EndRenderPass();
}

void MDeferredRenderProgram::RenderLightning(MTaskNode* pTaskNode)
{
	MIRenderCommand* pCommand = m_renderInfo.pPrimaryRenderCommand;

	pCommand->SetRenderToTextureBarrier(m_gbufferRenderPass.GetBackTextures());

	pCommand->BeginRenderPass(&m_lightningRenderPass);

	Vector2 v2Size = m_lightningRenderPass.GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));


	if (pCommand->SetUseMaterial(m_pLightningMaterial))
	{
		pCommand->SetShaderParamSet(&m_frameParamSet);
		pCommand->DrawMesh(&m_ScreenDrawMesh);
	}

	pCommand->EndRenderPass();
}

void MDeferredRenderProgram::RenderShadow(MTaskNode* pTaskNode)
{
	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->RenderShadow(m_renderInfo);

		MTexture* pShadowMap = m_pShadowMapWork->GetShadowMap();

		m_frameParamSet.SetShadowMapTexture(pShadowMap);
	}
}

void MDeferredRenderProgram::RenderForward(MTaskNode* pTaskNode)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = m_renderInfo.pPrimaryRenderCommand;

	MViewport* pViewport = m_renderInfo.pViewport;


	pCommand->BeginRenderPass(&m_forwardRenderPass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	DrawStaticMesh(m_renderInfo, pCommand, m_renderInfo.m_tMaterialGroupMesh);


	//Draw skybox
	if (MEntity* pSkyBox = m_renderInfo.pSkyBoxEntity)
	{
		if (MSkyBoxComponent* pSkyBoxComponent = pSkyBox->GetComponent<MSkyBoxComponent>())
		{
			m_pSkyBoxMaterial->SetTexutreParam("SkyTexCube", pSkyBoxComponent->GetSkyBoxResource());

			pCommand->SetUseMaterial(m_pSkyBoxMaterial);
			pCommand->SetShaderParamSet(&m_frameParamSet);
			pCommand->DrawMesh(&m_SkyBoxDrawMesh);
		}
	}


	pCommand->EndRenderPass();
}

void MDeferredRenderProgram::RenderTransparent(MTaskNode* pTaskNode)
{
	if (m_pTransparentWork)
	{
		m_pTransparentWork->RenderDepthPeel(m_renderInfo);
		m_pTransparentWork->Render(m_renderInfo);
	}
}

void MDeferredRenderProgram::RenderDebug(MTaskNode* pTaskNode)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = m_renderInfo.pPrimaryRenderCommand;

	MViewport* pViewport = m_renderInfo.pViewport;


	pCommand->BeginRenderPass(&m_forwardRenderPass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	pCommand->EndRenderPass();
}

MTexture* MDeferredRenderProgram::GetOutputTexture()
{
	return m_pFinalOutputTexture;
}

std::vector<MTexture*> MDeferredRenderProgram::GetOutputTextures()
{
	std::vector<MTexture*> vResult;

	if (m_pFinalOutputTexture)
		vResult.push_back(m_pFinalOutputTexture);

	if (m_pShadowMapWork)
	{
		if (MTexture* pTexture = m_pShadowMapWork->GetShadowMap())
		{
			vResult.push_back(pTexture);
		}
	}

	for (MBackTexture& tex: m_gbufferRenderPass.m_vBackTextures)
	{
		vResult.push_back(tex.pTexture);
	}

	return vResult;
}

MTexture* MDeferredRenderProgram::GetShadowmapTexture()
{
	if (m_pShadowMapWork)
	{
		return m_pShadowMapWork->GetShadowMap();
	}

	return nullptr;
}

void MDeferredRenderProgram::DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand, std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>>& tMaterialGroup)
{
	for (auto& pr : tMaterialGroup)
	{
		std::shared_ptr<MMaterial> pMaterial = pr.first;
		std::vector<MRenderableMeshComponent*>& vMesh = pr.second;

		pCommand->SetUseMaterial(pMaterial);
		pCommand->SetShaderParamSet(&m_frameParamSet);

		for (MRenderableMeshComponent* pMeshComponent : vMesh)
		{
			if (std::shared_ptr<MSkeletonInstance> pSkeletonIns = pMeshComponent->GetSkeletonInstance())
			{
				pCommand->SetShaderParamSet(pSkeletonIns->GetShaderParamSet());
			}
			pCommand->SetShaderParamSet(pMeshComponent->GetShaderMeshParamSet());
			pCommand->DrawMesh(pMeshComponent->GetMesh());
		}
	}
}

void MDeferredRenderProgram::OnCreated()
{
	Super::OnCreated();

	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pRenderGraph = pObjectSystem->CreateObject<MTaskGraph>();
	m_pShadowMapWork = pObjectSystem->CreateObject<MShadowMapRenderWork>();
//	m_pTransparentWork = pObjectSystem->CreateObject<MTransparentRenderWork>();

	MTaskNode* pRenderReadyTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Ready");
	pRenderReadyTask->SetThreadType(METhreadType::EAny);
	pRenderReadyTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderReady, this));

 	MTaskNode* pRenderShadowTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Shadowmap");
 	pRenderShadowTask->SetThreadType(METhreadType::EAny);
 	pRenderShadowTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderShadow, this));

	MTaskNode* pRenderGBufferTask = m_pRenderGraph->AddNode<MTaskNode>("Render_GBuffer");
	pRenderGBufferTask->SetThreadType(METhreadType::EAny);
	pRenderGBufferTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderGBuffer, this));

	MTaskNode* pRenderLightningTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Lightning");
	pRenderLightningTask->SetThreadType(METhreadType::EAny);
	pRenderLightningTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderLightning, this));

	MTaskNode* pRenderForwardTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Forward");
	pRenderForwardTask->SetThreadType(METhreadType::EAny);
	pRenderForwardTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderForward, this));

	MTaskNode* pRenderTransparentTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Transparent");
	pRenderTransparentTask->SetThreadType(METhreadType::EAny);
	pRenderTransparentTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderTransparent, this));

	MTaskNode* pRenderDebugTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Debug");
	pRenderDebugTask->SetThreadType(METhreadType::EAny);
	pRenderDebugTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderDebug, this));

	/*
		RenderReady --> RenderShadowmap --> pRenderEnvironmentTask --> RenderGBuffer --> RenderLightning --> RenderForward --> RenderTransparent --> RenderDebug --> output				
	*/

 	pRenderReadyTask->AppendOutput()->LinkTo(pRenderShadowTask->AppendInput());
	pRenderShadowTask->AppendOutput()->LinkTo(pRenderGBufferTask->AppendInput());
	pRenderGBufferTask->AppendOutput()->LinkTo(pRenderLightningTask->AppendInput());
	pRenderLightningTask->AppendOutput()->LinkTo(pRenderForwardTask->AppendInput());
	pRenderForwardTask->AppendOutput()->LinkTo(pRenderTransparentTask->AppendInput());
	pRenderTransparentTask->AppendOutput()->LinkTo(pRenderDebugTask->AppendInput());

	InitializeFrameShaderParams();
	InitializeRenderPass();
	InitializeMaterial();
	InitializeMesh();
}

void MDeferredRenderProgram::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	Super::OnDelete();

	m_pRenderGraph->DeleteLater();
	m_pRenderGraph = nullptr;

	m_pShadowMapWork->DeleteLater();
	m_pShadowMapWork = nullptr;

	ReleaseFrameShaderParams();
	ReleaseRenderPass();
	ReleaseMaterial();
	ReleaseMesh();
}

void MDeferredRenderProgram::InitializeRenderPass()
{
	Vector2 v2Size = Vector2(512.0f, 512.0f);

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	const std::vector<std::pair<MString, MPassTargetDescription>> vTextureDesc = {
		{"f3Albedo_fMetallic", {true, MColor::Black_T} },
		{"U_mat_f3Normal_fRoughness", {true, MColor::Black_T} },
		{"U_mat_f3Position_fAmbientOcc", {true, MColor::Black_T} },
	};

	for (auto& desc : vTextureDesc)
	{
		MTexture* pRenderTarget = MTexture::CreateRenderTargetGBuffer();
		pRenderTarget->SetName(desc.first);
		pRenderTarget->SetSize(v2Size);
		pRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());
		m_gbufferRenderPass.AddBackTexture(pRenderTarget, desc.second);
	}

	MTexture* pDepthTexture = MTexture::CreateShadowMap();
	pDepthTexture->SetSize(v2Size);
	pDepthTexture->GenerateBuffer(pRenderSystem->GetDevice());
	
	m_gbufferRenderPass.SetDepthTexture(pDepthTexture, { true, MColor::White });
	m_gbufferRenderPass.GenerateBuffer(pRenderSystem->GetDevice());



	MTexture* pLightningRenderTarget = MTexture::CreateRenderTarget();
	pLightningRenderTarget->SetName("Lightning Output");
	pLightningRenderTarget->SetSize(v2Size);
	pLightningRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());
	m_lightningRenderPass.AddBackTexture(pLightningRenderTarget, { true, MColor::Black_T });
	m_lightningRenderPass.GenerateBuffer(pRenderSystem->GetDevice());



	m_forwardRenderPass.AddBackTexture(pLightningRenderTarget, { false, true, MColor::Black_T });
	m_forwardRenderPass.SetDepthTexture(pDepthTexture, { false, true, MColor::White });
	m_forwardRenderPass.GenerateBuffer(pRenderSystem->GetDevice());


	m_debugRenderPass.AddBackTexture(pLightningRenderTarget, { false, true, MColor::Black_T });
	m_debugRenderPass.SetDepthTexture(pDepthTexture, { false, true, MColor::White });
	m_debugRenderPass.GenerateBuffer(pRenderSystem->GetDevice());


	m_pFinalOutputTexture = pLightningRenderTarget;
}

void MDeferredRenderProgram::ReleaseRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	pRenderSystem->ReleaseRenderpass(m_gbufferRenderPass, true);
	pRenderSystem->ReleaseRenderpass(m_lightningRenderPass, true);

	pRenderSystem->ReleaseRenderpass(m_forwardRenderPass, false);
	pRenderSystem->ReleaseRenderpass(m_debugRenderPass, false);
}

void MDeferredRenderProgram::InitializeFrameShaderParams()
{
	m_frameParamSet.InitializeShaderParamSet(GetEngine());


	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> pBrdfTexture = pResourceSystem->LoadResource("/Texture/ibl_brdf_lut.png");

	if (std::shared_ptr<MTextureResource> pTexture = MTypeClass::DynamicCast<MTextureResource>(pBrdfTexture))
	{
		m_BrdfTexture.SetResource(pBrdfTexture);
		m_frameParamSet.SetBrdfMapTexture(pTexture->GetTextureTemplate());
	}
}

void MDeferredRenderProgram::ReleaseFrameShaderParams()
{
	m_frameParamSet.ReleaseShaderParamSet(GetEngine());
}

void MDeferredRenderProgram::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("./Shader/post_process_basic.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("./Shader/model_deferred.mps");


	m_pLightningMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pLightningMaterial->LoadVertexShader(vs);
	m_pLightningMaterial->LoadPixelShader(ps);
	m_pLightningMaterial->AddRef();

	if (MShaderParamSet* pParams = m_pLightningMaterial->GetMaterialParamSet())
	{
		pParams->SetValue("U_mat_f3Albedo_fMetallic", m_gbufferRenderPass.m_vBackTextures[0].pTexture);
		pParams->SetValue("U_mat_f3Normal_fRoughness", m_gbufferRenderPass.m_vBackTextures[1].pTexture);
		pParams->SetValue("U_mat_f3Position_fAmbientOcc", m_gbufferRenderPass.m_vBackTextures[2].pTexture);
//		pParams->SetValue("U_mat_DepthMap", m_gbufferRenderPass.m_vBackTextures[3]);
	}


	std::shared_ptr<MResource> skyboxVS = pResourceSystem->LoadResource("./Shader/skybox.mvs");
	std::shared_ptr<MResource> skyboxPS = pResourceSystem->LoadResource("./Shader/skybox.mps");
	m_pSkyBoxMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pSkyBoxMaterial->SetRasterizerType(MERasterizerType::ECullNone);
	m_pSkyBoxMaterial->LoadVertexShader(skyboxVS);
	m_pSkyBoxMaterial->LoadPixelShader(skyboxPS);
	m_pSkyBoxMaterial->AddRef();
}

void MDeferredRenderProgram::ReleaseMaterial()
{
	m_pLightningMaterial->SubRef();
	m_pLightningMaterial = nullptr;

	m_pSkyBoxMaterial->SubRef();
	m_pSkyBoxMaterial = nullptr;
}

void MDeferredRenderProgram::InitializeMesh()
{
	{
		MMesh<Vector2>& mesh = m_ScreenDrawMesh;
		mesh.ResizeVertices(4);
		Vector2* vVertices = (Vector2*)mesh.GetVertices();

		vVertices[0] = Vector2(-1, -1);
		vVertices[1] = Vector2(1, -1);
		vVertices[2] = Vector2(-1, 1);
		vVertices[3] = Vector2(1, 1);

		mesh.ResizeIndices(2, 3);
		uint32_t* vIndices = mesh.GetIndices();

		vIndices[0] = 0;
		vIndices[1] = 2;
		vIndices[2] = 1;

		vIndices[3] = 2;
		vIndices[4] = 3;
		vIndices[5] = 1;
	}

	{
		MMesh<Vector3>& mesh = m_SkyBoxDrawMesh;
		mesh.ResizeVertices(8);
		mesh.ResizeIndices(12, 3);

		Vector3* vVertices = (Vector3*)mesh.GetVertices();
		vVertices[0] = Vector3(-1.0, -1.0, 1.0);
		vVertices[1] = Vector3(-1.0, 1.0, 1.0);
		vVertices[2] = Vector3(1.0, 1.0, 1.0);
		vVertices[3] = Vector3(1.0, -1.0, 1.0);
		vVertices[4] = Vector3(-1.0, -1.0, -1.0);
		vVertices[5] = Vector3(-1.0, 1.0, -1.0);
		vVertices[6] = Vector3(1.0, 1.0, -1.0);
		vVertices[7] = Vector3(1.0, -1.0, -1.0);

		const uint32_t indices[] = {
			3, 2, 6, 3, 6, 7,//right
			0, 1, 5, 0, 5, 4,//left
			5, 1, 2, 5, 2, 6,//top
			4, 0, 3, 4, 3, 7,//bottom
			0, 1, 2, 0, 2, 3,//front
			4, 5, 6, 4, 6, 7,//back
		};

		memcpy(mesh.GetIndices(), indices, sizeof(indices));
	}
}

void MDeferredRenderProgram::ReleaseMesh()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_ScreenDrawMesh.DestroyBuffer(pRenderSystem->GetDevice());
	m_SkyBoxDrawMesh.DestroyBuffer(pRenderSystem->GetDevice());
}

void MDeferredRenderProgram::UpdateFrameParams(MRenderInfo& info)
{
	m_frameParamSet.UpdateShaderSharedParams(info);
}

void MDeferredRenderProgram::CollectRenderMesh(MRenderInfo& info)
{
	MViewport* pViewport = info.pViewport;

	MScene* pScene = pViewport->GetScene();

	Vector3 v3BoundsMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3BoundsMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	MComponentGroup<MRenderableMeshComponent>* pMeshComponents = pScene->FindComponents<MRenderableMeshComponent>();

	if (!pMeshComponents)
		return;

	for (MRenderableMeshComponent& meshComp : pMeshComponents->m_vComponents)
	{
		std::shared_ptr<MMaterial> pMaterial = meshComp.GetMaterial();
		if (!pMaterial)
			continue;

		MSceneComponent* pSceneComponent = meshComp.GetEntity()->GetComponent<MSceneComponent>();

		if (!pSceneComponent->GetVisibleRecursively())
			continue;

		const MBoundsAABB* pBounds = meshComp.GetBoundsAABB();

		if (MCameraFrustum::EOUTSIDE == pViewport->GetCameraFrustum().ContainTest(*pBounds))
			continue;

		if (pMaterial->GetMaterialType() == MEMaterialType::EDepthPeel)
		{
			auto& meshes = info.m_tTransparentGroupMesh[pMaterial];
			meshes.push_back(&meshComp);
		}
		else if (pMaterial->GetMaterialType() == MEMaterialType::EDeferred)
		{
			auto& meshes = info.m_tDeferredMaterialGroupMesh[pMaterial];
			meshes.push_back(&meshComp);
		}
		else
		{
			auto& meshes = info.m_tMaterialGroupMesh[pMaterial];
			meshes.push_back(&meshComp);
		}


		if (meshComp.GetShadowType() != MRenderableMeshComponent::MEShadowType::ENone)
		{
			pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
		}
	}

	info.cCaclSceneRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}
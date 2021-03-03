#include "MEngine.h"
#include "MIRenderer.h"
#include "MIRenderView.h"
#include "MViewport.h"
#include "MIRenderTarget.h"
#include "MRenderStatistics.h"

#include "MLogManager.h"
#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
#include "MDirectX11Device.h"
#include "MDirectX11Renderer.h"
#elif (RENDER_GRAPHICS == MORTY_VULKAN)
#include "MVulkanDevice.h"
#include "MVulkanRenderer.h"
#endif

#ifdef MORTY_WIN
#include "DirectX11/MWindowsRenderView.h"
#endif

#include "MNode.h"
#include "MScene.h"
#include "MMaterial.h"
#include "Timer/MTimer.h"
#include "MResourceManager.h"  
#include "Model/MMeshResource.h"
#include "Texture/MTextureResource.h"
#include "Material/MMaterialResource.h"
#include "Texture/MTextureCubeResource.h"

#include "MInputManager.h"
#include "MIRenderProgram.h"


MEngine::MEngine()
	: m_unFrameIdx(0)
	, m_pObjectManager(nullptr)
	, m_pResourceManager(nullptr)
	, m_pScene(nullptr)
	, m_pDevice(nullptr)
	, m_pRenderer(nullptr)
	, m_cTickInfo(500)
{
}

MEngine::~MEngine()
{
}

bool MEngine::OpenProject(const MString& strProjectPath)
{
	m_Project.SetWorkPath(strProjectPath);

	m_pResourceManager->SetSearchPath({strProjectPath });

	return true;
}

bool MEngine::Initialize(const MString& strSearchPath)
{

	if (nullptr == m_pRenderer)
	{
#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
		MDirectX11Device* pDevice = new MDirectX11Device();
		pDevice->Initialize();
		m_pDevice = pDevice;

		MDirectX11Renderer* pDx11Renderer = new MDirectX11Renderer(pDevice);
		m_pRenderer = pDx11Renderer;
#elif (RENDER_GRAPHICS == MORTY_VULKAN)
		MVulkanDevice* pDevice = new MVulkanDevice();
		pDevice->Initialize();
		m_pDevice = pDevice;

		m_pRenderer = new MVulkanRenderer(pDevice);
#else
		m_pRenderer = nullptr;
#endif
	}

	if (!m_pRenderer->Initialize())
	{
		delete m_pRenderer;
		m_pRenderer = nullptr;
		return false;
	}

	m_pObjectManager = new MObjectManager();
	m_pObjectManager->SetOwnerEngine(this);

	m_pResourceManager = new MResourceManager();
	m_pResourceManager->SetOwnerEngine(this);
	m_pResourceManager->SetSearchPath({ strSearchPath });

	//初始化默认资源
	InitializeDefaultResource();

	return true;

}

void MEngine::AddView(MIRenderView* pView)
{
	pView->m_pEngine = this;
	m_vView.push_back(pView);
}

void MEngine::Release()
{
	for (auto pView : m_vView)
	{
		pView->Release();
		
		delete pView;
		pView = nullptr;
	}

	m_vView.clear();

	m_pObjectManager->CleanRemoveObject();
	delete m_pObjectManager;


	delete m_pResourceManager;


	ReleaseDefaultResource();

	if (m_pRenderer)
	{
		m_pRenderer->Release();
		delete m_pRenderer;
		m_pRenderer = nullptr;
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
		delete m_pDevice;
		m_pDevice = nullptr;
	}
}

void MEngine::Tick(float fDelta)
{
//	MLogManager::GetInstance()->Log("fps:  %d", (int)(1.0f / fDelta));

//	MLogManager::GetInstance()->Log("t:  %u", MRenderStatistics::GetInstance()->unTriangleCount);
	MRenderStatistics::GetInstance()->unTriangleCount = 0;
	if (m_pScene)
	{
		m_pScene->Tick(fDelta);
	}

	m_pObjectManager->CleanRemoveObject();
}

void MEngine::SetMaxFPS(const int& nFPS)
{
	if (nFPS > 0)
	{
		m_cTickInfo.nMaxFPS = nFPS;
		m_cTickInfo.fTickInterval = 1.0f / m_cTickInfo.nMaxFPS;
	}
}

bool MEngine::InitializeDefaultResource()
{
	MResource* pMeshVSResource = GetResourceManager()->LoadResource("./Shader/model.mvs");
	MResource* pMeshPSResource = GetResourceManager()->LoadResource("./Shader/model.mps");

	MMaterialResource* pStaticMeshMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_MODEL_STATIC_MESH);
	pStaticMeshMaterialRes->LoadVertexShader(pMeshVSResource);
	pStaticMeshMaterialRes->LoadPixelShader(pMeshPSResource);
	pStaticMeshMaterialRes->AddRef();

	MMaterialResource* pSkinnedMeshMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_MODEL_SKELETON_MESH);
	pSkinnedMeshMaterialRes->GetShaderMacro()->SetInnerMacro(MATERIAL_MACRO_SKELETON_ENABLE, "1");
	pSkinnedMeshMaterialRes->LoadVertexShader(pMeshVSResource);
	pSkinnedMeshMaterialRes->LoadPixelShader(pMeshPSResource);
	pSkinnedMeshMaterialRes->AddRef();

	MResource* pDraw2DVSResource = GetResourceManager()->LoadResource("./Shader/draw.mvs");
	MResource* pDraw2DPSResource = GetResourceManager()->LoadResource("./Shader/draw.mps");
	MMaterialResource* pDraw2DMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW2D);
	pDraw2DMaterialRes->SetMaterialType(MEMaterialType::EDefault);
	pDraw2DMaterialRes->SetRasterizerType(MERasterizerType::ECullNone);
	pDraw2DMaterialRes->LoadVertexShader(pDraw2DVSResource);
	pDraw2DMaterialRes->LoadPixelShader(pDraw2DPSResource);
	pDraw2DMaterialRes->AddRef();

	MResource* pDraw3DVSResource = GetResourceManager()->LoadResource("./Shader/draw3D.mvs");
	MResource* pDraw3DPSResource = GetResourceManager()->LoadResource("./Shader/draw3D.mps");
	MMaterialResource* pDraw3DMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	pDraw3DMaterialRes->LoadVertexShader(pDraw3DVSResource);
	pDraw3DMaterialRes->LoadPixelShader(pDraw3DPSResource);
	pDraw3DMaterialRes->AddRef();

	MResource* pEmptyVSResource = GetResourceManager()->LoadResource("./Shader/empty.mvs");
	MResource* pEmptyPSResource = GetResourceManager()->LoadResource("./Shader/empty.mps");
	MMaterialResource* pShadowStaticMaterial = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_STATIC);
	pShadowStaticMaterial->LoadVertexShader(pEmptyVSResource);
	pShadowStaticMaterial->LoadPixelShader(pEmptyPSResource);
	pShadowStaticMaterial->SetRasterizerType(MERasterizerType::ECullFront);
	pShadowStaticMaterial->AddRef();

	MMaterialResource* pShadowSkeletonMaterial = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_SKELETON);
	pShadowSkeletonMaterial->GetShaderMacro()->SetInnerMacro(MATERIAL_MACRO_SKELETON_ENABLE, "1");
	pShadowSkeletonMaterial->LoadVertexShader(pEmptyVSResource);
	pShadowSkeletonMaterial->LoadPixelShader(pEmptyPSResource);
	pShadowSkeletonMaterial->SetRasterizerType(MERasterizerType::ECullFront);
	pShadowSkeletonMaterial->AddRef();


	MTextureResource* pWhiteTextureRes = GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_WHITE);
	MTexture* pWhiteTexture = pWhiteTextureRes->GetTextureTemplate();
	pWhiteTexture->SetSize(Vector2(1, 1));
	pWhiteTexture->FillColor(MColor(1, 1, 1, 1));
	pWhiteTexture->GenerateBuffer(GetDevice());
	pWhiteTextureRes->AddRef();

	MTextureResource* pBlackTextureRes = GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_BLACK);
	MTexture* pBlackTexture = pBlackTextureRes->GetTextureTemplate();
	pBlackTexture->SetSize(Vector2(1, 1));
	pBlackTexture->FillColor(MColor(0, 0, 0, 1));
	pBlackTexture->GenerateBuffer(GetDevice());
	pBlackTextureRes->AddRef();

	MTextureResource* pNormalMapRes = GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_NORMALMAP);
	MTexture* pNormalMap = pNormalMapRes->GetTextureTemplate();
	pNormalMap->SetSize(Vector2(1, 1));
	pNormalMap->FillColor(MColor(0.5f, 0.5f, 1, 1));
	pNormalMap->GenerateBuffer(GetDevice());
	pNormalMapRes->AddRef();


	MResource* pDPVSResource = GetResourceManager()->LoadResource("./Shader/depth_peel_blend.mvs");
	MResource* pDPBPSResource = GetResourceManager()->LoadResource("./Shader/depth_peel_blend.mps");
	MResource* pDPFPSResource = GetResourceManager()->LoadResource("./Shader/depth_peel_fill.mps");

	MMaterialResource* pTextureMaterial = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DEPTH_PEEL_BLEND);
	pTextureMaterial->SetMaterialType(MEMaterialType::ETransparentBlend);
	pTextureMaterial->LoadVertexShader(pDPVSResource);
	pTextureMaterial->LoadPixelShader(pDPBPSResource);
	pTextureMaterial->AddRef();


	MMaterialResource* pDepthPeelFillMaterial = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DEPTH_PEEL_FILL);
	pDepthPeelFillMaterial->SetMaterialType(MEMaterialType::EDepthPeel);
	pDepthPeelFillMaterial->LoadVertexShader(pDPVSResource);
	pDepthPeelFillMaterial->LoadPixelShader(pDPFPSResource);
	pDepthPeelFillMaterial->AddRef();


	MMesh<Vector2>* pScreenMesh = new MMesh<Vector2>;
	pScreenMesh->ResizeVertices(4);
	Vector2* vVertices = (Vector2*)pScreenMesh->GetVertices();

	vVertices[0] = Vector2(-1, -1);
	vVertices[1] = Vector2(1, -1);
	vVertices[2] = Vector2(-1, 1);
	vVertices[3] = Vector2(1, 1);

	pScreenMesh->ResizeIndices(2, 3);
	uint32_t* vIndices = pScreenMesh->GetIndices();

	vIndices[0] = 0;
	vIndices[1] = 2;
	vIndices[2] = 1;

	vIndices[3] = 2;
	vIndices[4] = 3;
	vIndices[5] = 1;
	MMeshResource* pScreenMeshRes = GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->m_pMesh = pScreenMesh;
	pScreenMeshRes->AddRef();

	return true;
}

void MEngine::ReleaseDefaultResource()
{
}

void MEngine::WaitRenderFinished(const uint32_t& unFrameIdx)
{

}

bool MEngine::MainLoop()
{
	long long currentTime = MTimer::GetCurTime();

	if (0 == m_cTickInfo.lPrevTickTime)
		m_cTickInfo.lPrevTickTime = currentTime;

	float fTimeDelta = (float)(currentTime - m_cTickInfo.lPrevTickTime) / 1000;

	for (std::vector<MIRenderView*>::iterator iter = m_vView.begin(); iter != m_vView.end();)
	{
		MIRenderView* pView = *iter;
		if (!pView->MainLoop(fTimeDelta))
		{
			iter = m_vView.erase(iter);

			pView->Release();
			delete pView;
			pView = nullptr;
		}
		else
		{
			++iter;
		}
	}

	if (fTimeDelta - m_cTickInfo.fTickInterval  >= 0)
	{
		m_cTickInfo.fTimeDelta = fTimeDelta;

		//tick
		Tick(fTimeDelta);
		m_cTickInfo.lPrevTickTime = currentTime;

		// prev frame finished.
		WaitRenderFinished(m_unFrameIdx);
		m_unFrameIdx = (m_unFrameIdx + 1) % M_BUFFER_NUM;

		// render
		for (MIRenderView* pView : m_vView)
		{
			RenderToView(pView, fTimeDelta);
			break; //TODO mutil view.
		}
	}

	return !m_vView.empty();
}

void MEngine::RenderToView(MIRenderView* pView, const float& fDelta)
{
	if (pView->GetMinimized())
		return;

	pView->Render();

}

void MEngine::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

MEngine::TickInfo::TickInfo(int nFps)
{
	if (nFps == 0)
		nFps = 60;
	nMaxFPS = nFps;
	fTickInterval = 1.0f / nFps;
	lPrevTickTime = 0;
}

#include "MEngine.h"
#include "MIRenderer.h"
#include "MIRenderView.h"
#include "MViewport.h"

#include "MLogManager.h"
#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
#include "MDirectX11Device.h"
#include "MDirectX11Renderer.h"
#endif

#ifdef MORTY_WIN
#include "MWindowsRenderView.h"
#endif

#include "Timer/MTimer.h"
#include "MNode.h"
#include "MScene.h"
#include "MMaterial.h"
#include "MResourceManager.h"  
#include "Material/MMaterialResource.h"
#include "Texture/MTextureResource.h"
#include "Texture/MTextureCubeResource.h"

#include "MInputManager.h"


MEngine::MEngine()
	: m_pObjectManager(nullptr)
	, m_pResourceManager(nullptr)
	, m_pScene(nullptr)
	, m_pDevice(nullptr)
	, m_pRenderer(nullptr)
	, m_cTickInfo(30)
{
}

MEngine::~MEngine()
{
}

bool MEngine::Initialize()
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
		m_pRenderer = nullptr;
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

	//允许资源重加载
	m_pResourceManager->SetReloadEnabled(true);
	
	//初始化默认资源
	InitializeDefaultResource();

	return true;

}

MIRenderView* MEngine::CreateView()
{

	MIRenderView* pNewView = nullptr;

#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
	if (MDirectX11Renderer* pRenderer = dynamic_cast<MDirectX11Renderer*>(m_pRenderer))
	{
		MWindowsRenderView* pWindowsView = new MWindowsRenderView();
		pWindowsView->Initialize(this, "Morty");
		pWindowsView->m_pEngine = this;
		AddView(pWindowsView);

		pNewView = pWindowsView;
	}

#endif

	MViewport* pViewport = GetObjectManager()->CreateObject<MViewport>();
	pNewView->AppendViewport(pViewport);

	return pNewView;

}

void MEngine::AddView(MIRenderView* pView)
{
	pView->m_pEngine = this;
	m_vView.push_back(pView);
}

void MEngine::Release()
{
	if (m_pRenderer)
	{
		m_pRenderer->Release();
		delete m_pRenderer;
		m_pRenderer = nullptr;
	}

	for (auto pView : m_vView)
	{
		pView->Release();
		
		delete pView;
		pView = nullptr;
	}

	m_vView.clear();

	delete m_pObjectManager;
	delete m_pResourceManager;


	ReleaseDefaultResource();

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
	MResource* pStaticVSResource = GetResourceManager()->LoadResource("./Shader/staticModel.mvs");
	MResource* pSkinnedVSResource = GetResourceManager()->LoadResource("./Shader/animationModel.mvs");
	MResource* pMeshPSResource = GetResourceManager()->LoadResource("./Shader/model.mps");

	MMaterialResource* pStaticMeshMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_STATIC);
	pStaticMeshMaterialRes->LoadVertexShader(pStaticVSResource);
	pStaticMeshMaterialRes->LoadPixelShader(pMeshPSResource);

	MMaterialResource* pSkinnedMeshMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SKINNED);
	pSkinnedMeshMaterialRes->LoadVertexShader(pSkinnedVSResource);
	pSkinnedMeshMaterialRes->LoadPixelShader(pMeshPSResource);

	MResource* pDraw2DVSResource = GetResourceManager()->LoadResource("./Shader/draw.mvs");
	MResource* pDraw2DPSResource = GetResourceManager()->LoadResource("./Shader/draw.mps");
	MMaterialResource* pDraw2DMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW2D);
	pDraw2DMaterialRes->LoadVertexShader(pDraw2DVSResource);
	pDraw2DMaterialRes->LoadPixelShader(pDraw2DPSResource);

	MResource* pDraw3DVSResource = GetResourceManager()->LoadResource("./Shader/draw3D.mvs");
	MResource* pDraw3DPSResource = GetResourceManager()->LoadResource("./Shader/draw3D.mps");
	MMaterialResource* pDraw3DMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	pDraw3DMaterialRes->LoadVertexShader(pDraw3DVSResource);
	pDraw3DMaterialRes->LoadPixelShader(pDraw3DPSResource);

	MResource* pSkyBoxVSResource = GetResourceManager()->LoadResource("./Shader/skybox.mvs");
	MResource* pSkyBoxPSResource = GetResourceManager()->LoadResource("./Shader/skybox.mps");
	MMaterialResource* pSkyBoxMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SKYBOX);
	pSkyBoxMaterialRes->LoadVertexShader(pSkyBoxVSResource);
	pSkyBoxMaterialRes->LoadPixelShader(pSkyBoxPSResource);

	MResource* pSphereResource = GetResourceManager()->LoadResource("./Model/Sphere.fbx");


	const MString vTexturePath[6] = {
		"ashcanyon_lf.tga",
		"ashcanyon_rt.tga",
		"ashcanyon_up.tga",
		"ashcanyon_dn.tga",
		"ashcanyon_ft.tga",
		"ashcanyon_bk.tga",
	};

	MTextureResource* vTextureRes[6];
	for (int i = 0; i < 6; ++i)
	{
		vTextureRes[i] = static_cast<MTextureResource*>(GetResourceManager()->LoadResource("./Texture/skybox/" + vTexturePath[i]));
	}

	MTextureCubeResource* pTextureCubeRes = GetResourceManager()->CreateResource<MTextureCubeResource>();
	pTextureCubeRes->SetTextures(vTextureRes);

	pSkyBoxMaterialRes->SetTexutreParam("SkyTexCube", pTextureCubeRes);


	MResource* pEmptyVSResource = GetResourceManager()->LoadResource("./Shader/empty.mvs");
	MResource* pAnimVSResource = GetResourceManager()->LoadResource("./Shader/empty_anim.mvs");
	MResource* pEmptyPSResource = GetResourceManager()->LoadResource("./Shader/empty.mps");
	MMaterialResource* pShadowMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW);
	pShadowMaterialRes->LoadVertexShader(pEmptyVSResource);
	pShadowMaterialRes->LoadPixelShader(pEmptyPSResource);
	pShadowMaterialRes->SetRasterizerType(MERasterizerType::ECullFront);

	MMaterialResource* pShadowWithAnimMaterialRes = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_ANIM);
	pShadowWithAnimMaterialRes->LoadVertexShader(pAnimVSResource);
	pShadowWithAnimMaterialRes->LoadPixelShader(pEmptyPSResource);
	pShadowWithAnimMaterialRes->SetRasterizerType(MERasterizerType::ECullFront);



	MTextureResource* pWhiteTextureRes = GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_WHITE);
	MTexture* pWhiteTexture = pWhiteTextureRes->GetTextureTemplate();
	pWhiteTexture->SetSize(Vector2(1, 1));
	pWhiteTexture->FillColor(MColor(1, 1, 1, 1));

	MTextureResource* pBlackTextureRes = GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_BLACK);
	MTexture* pBlackTexture = pBlackTextureRes->GetTextureTemplate();
	pBlackTexture->SetSize(Vector2(1, 1));
	pBlackTexture->FillColor(MColor(0, 0, 0, 1));

	MTextureResource* pNormalMapRes = GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_NORMALMAP);
	MTexture* pNormalMap = pNormalMapRes->GetTextureTemplate();
	pNormalMap->SetSize(Vector2(1, 1));
	pNormalMap->FillColor(MColor(0.5f, 0.5f, 1, 1));

	MResource* pDPVSResource = GetResourceManager()->LoadResource("./Shader/depthPeeling.mvs");
	MResource* pDPPSResource = GetResourceManager()->LoadResource("./Shader/depthPeeling.mps");
	MMaterialResource* pTextureMaterial = GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DEPTH_PEELING);
	pTextureMaterial->LoadVertexShader(pDPVSResource);
	pTextureMaterial->LoadPixelShader(pDPPSResource);
	pTextureMaterial->SetMaterialType(MEMaterialType::EBlendTransparent);

	return true;
}

void MEngine::ReleaseDefaultResource()
{
	for (MShaderParam* param : MShaderBuffer::s_vShaderParams)
	{
		if(param)
			GetDevice()->DestroyShaderParamBuffer(param);
	}
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

		//Tick
		Tick(fTimeDelta);
		m_cTickInfo.lPrevTickTime = currentTime;

		//Render
		for (MIRenderView* pView : m_vView)
			RenderToView(pView);

 		int nTime = (int)(m_cTickInfo.fTickInterval * 1000) * 0.75 - (MTimer::GetCurTime() - currentTime);
 		if (nTime > 0)
 		{
 			Sleep(nTime);
 		}
	}

	return !m_vView.empty();
}
void MEngine::RenderToView(MIRenderView* pView)
{
	if (MIRenderTarget* pRenderTarget = pView->GetRenderTarget())
	{
		m_pRenderer->Render(pRenderTarget);
	}
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

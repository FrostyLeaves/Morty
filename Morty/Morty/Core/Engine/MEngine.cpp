#include "MEngine.h"
#include "MIRenderer.h"
#include "MIRenderView.h"
#include "MIViewport.h"

#include "MLogManager.h"
#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
#include "MDirectX11Device.h"
#include "MDirectX11Renderer.h"
#endif

#ifdef MORTY_WIN
#include "MWindowsRenderView.h"
#endif

#include "MTimer.h"
#include "MNode.h"
#include "MResourceManager.h"

#include "MInputManager.h"


MEngine::MEngine()
	: m_pObjectManager(nullptr)
	, m_pResourceManager(nullptr)
	, m_pRootNode(nullptr)
	, m_pDevice(nullptr)
	, m_pRenderer(nullptr)
	, m_cTickInfo(50)
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
#elif (RENDER_GRAPHICS == MORTY_OPENGLES)
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

	MIViewport* pViewport = GetObjectManager()->CreateObject<MIViewport>();
	pNewView->AppendViewport(pViewport);

	return pNewView;

}

void MEngine::AddView(MIRenderView* pView)
{
	pView->m_pEngine = this;

	m_pRenderer->AddOutputView(pView);

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

	if (m_pDevice)
	{
		m_pDevice->Release();
		delete m_pDevice;
		m_pDevice = nullptr;
	}
}

void MEngine::Tick(float fDelta)
{
	if (m_pRootNode)
	{
		m_pRootNode->Tick(fDelta);
	}
}

void MEngine::SetMaxFPS(const int& nFPS)
{
	if (nFPS > 0)
	{
		m_cTickInfo.nMaxFPS = nFPS;
		m_cTickInfo.fTickInterval = 1.0f / m_cTickInfo.nMaxFPS;
	}
}

bool MEngine::MainLoop()
{
	long long currentTime = MTimer::GetCurTime();

	if (0 == m_cTickInfo.lPrevTickTime)
		m_cTickInfo.lPrevTickTime = currentTime;

	float fTimeDelta = (float)(currentTime - m_cTickInfo.lPrevTickTime) / 1000;

	if (fTimeDelta >= m_cTickInfo.fTickInterval)
	{
		m_cTickInfo.fTimeDelta = fTimeDelta;
//		MLogManager::GetInstance()->Log("fps: %f", 1.0f / lTimeDelta);

		Tick(fTimeDelta);
		m_cTickInfo.lPrevTickTime = currentTime;

		for (std::vector<MIRenderView*>::iterator iter = m_vView.begin(); iter != m_vView.end();)
		{
			MIRenderView* pView = (*iter);
			if (pView->MainLoop())
			{
				m_pRenderer->RenderToView(pView);
				++iter;
			}
			else
			{
				m_pRenderer->RemoveOutputView(*iter);
				iter = m_vView.erase(iter);
				
				pView->Release();
				delete pView;
				pView = nullptr;
			}
		}

		int nTime = (int)(m_cTickInfo.fTickInterval * 1000) * 0.75 - (MTimer::GetCurTime() - currentTime);
		if (nTime > 0)
		{
			Sleep(nTime);
		}
	}

	return !m_vView.empty();
}

void MEngine::SetRootNode(MNode* pNode)
{
	m_pRootNode = pNode;
}

MEngine::TickInfo::TickInfo(int nFps)
{
	if (nFps == 0)
		nFps = 60;
	nMaxFPS = nFps;
	fTickInterval = 1.0f / nFps;
	lPrevTickTime = 0;
}

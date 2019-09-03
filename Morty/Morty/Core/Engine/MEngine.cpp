#include "MEngine.h"
#include "MIRenderer.h"
#include "MIRenderView.h"

#include "MLogManager.h"
#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
#include "MDirectX11Renderer.h"
#endif

#ifdef MORTY_WIN
#include "MWindowsRenderView.h"
#endif

#include "MTimer.h"
#include "MNode.h"
#include "MResourceManager.h"


MEngine::MEngine()
	: m_pObjectManager(nullptr)
	, m_pResourceManager(nullptr)
	, m_pRootNode(nullptr)
	, m_pRenderer(nullptr)
	, m_cTickInfo(60)
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
		MDirectX11Renderer* pDx11Renderer = new MDirectX11Renderer();
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

	return true;

}

void MEngine::CreateView()
{

#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
	if (MDirectX11Renderer* pRenderer = dynamic_cast<MDirectX11Renderer*>(m_pRenderer))
	{
		MWindowsRenderView* pWindowsView = new MWindowsRenderView();
		pWindowsView->Initialize("Morty");
		pRenderer->AddOutputView(pWindowsView);
		pWindowsView->SetResizeCallback([=](const int& nWidth, const int& nHeight)
		{
			m_pRenderer->OnResize(pWindowsView, nWidth, nHeight);
			m_pRenderer->RenderNodeToView(pWindowsView->GetRootNode(), pWindowsView->GetCamera(), pWindowsView);
		});

		m_vView.push_back(pWindowsView);
	}

#endif
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
#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
		if (MWindowsRenderView* pWindowsView = dynamic_cast<MWindowsRenderView*>(pView))
		{
			pWindowsView->Release();
		}
#elif (RENDER_GRAPHICS == MORTY_OPENGLES)
		m_pRenderer = nullptr;
#else
		m_pRenderer = nullptr;
#endif
		
		delete pView;
		pView = nullptr;
	}

	delete m_pObjectManager;
	delete m_pResourceManager;

	m_vView.clear();
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

MIRenderer* MEngine::GetRenderer()
{
	return m_pRenderer;
}

bool MEngine::MainLoop()
{
	long long currentTime = MTimer::GetCurTime();

	if (0 == m_cTickInfo.lPrevTickTime)
		m_cTickInfo.lPrevTickTime = currentTime;

	float lTimeDelta = (float)(currentTime - m_cTickInfo.lPrevTickTime) / 1000;

//	if (lTimeDelta >= m_cTickInfo.fTickInterval)
	{
		Tick(lTimeDelta);
		m_cTickInfo.lPrevTickTime = currentTime;

//		MLogManager::GetInstance()->Log("fps: %f", 1.0f / lTimeDelta);

		for (std::vector<MIRenderView*>::iterator iter = m_vView.begin(); iter != m_vView.end();)
		{
			MIRenderView* pView = (*iter);
			if (pView->MainLoop())
			{
				m_pRenderer->RenderNodeToView(m_pRootNode, pView->GetCamera(), pView);
				++iter;
			}

			else
			{
				m_pRenderer->RemoveOutputView(*iter);
				iter = m_vView.erase(iter);
			}
		}
	}

	Sleep(1000 / (m_cTickInfo.nMaxFPS + 10));

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

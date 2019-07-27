#include "MEngine.h"
#include "MDirectX11Renderer.h"
#include "MWindowsRenderView.h"
#include "MTimer.h"


MEngine::MEngine()
	: m_pRenderer(nullptr)
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
		pRenderer = nullptr;
#else
		pRenderer = nullptr;
#endif
	}

	if (m_pRenderer->Initialize())
		return true;

	delete m_pRenderer;
	m_pRenderer = nullptr;
	
	return false;
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
			m_pRenderer->RenderNodeToView(pWindowsView->GetRootNode(), pWindowsView);
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
		pRenderer = nullptr;
#else
		pRenderer = nullptr;
#endif
		
		delete pView;
		pView = nullptr;
	}

	m_vView.clear();
}

void MEngine::Tick(float fDelta)
{

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

	if (lTimeDelta >= m_cTickInfo.fTickInterval)
	{
		Tick(lTimeDelta);
		m_cTickInfo.lPrevTickTime = currentTime;
	}

	for (std::vector<MIRenderView*>::iterator iter = m_vView.begin(); iter != m_vView.end(); )
	{
		MIRenderView* pView = (*iter);
		if (pView->MainLoop())
		{
			m_pRenderer->RenderNodeToView(pView->GetRootNode(), pView);
			++iter;
		}

		else
		{
			m_pRenderer->RemoveOutputView(*iter);
			iter = m_vView.erase(iter);
		}
	}

	return !m_vView.empty();
}

MEngine::TickInfo::TickInfo(int nFps)
{
	if (nFps == 0)
		nFps = 60;
	nMaxFPS = nFps;
	fTickInterval = 1.0f / nFps;
	lPrevTickTime = 0;
}

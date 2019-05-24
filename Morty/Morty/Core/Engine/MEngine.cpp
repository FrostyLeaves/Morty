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
	MWindowsRenderView* pWindowsView = new MWindowsRenderView();
	pWindowsView->Initialize("Morty");
	
	m_pView = pWindowsView;

	if (nullptr == m_pRenderer)
	{
#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
		MDirectX11Renderer* pDx11Renderer = new MDirectX11Renderer();
		pDx11Renderer->Initialize();
		pDx11Renderer->AddOutputWindow(m_pView);

		m_pRenderer = pDx11Renderer;
#elif (RENDER_GRAPHICS == MORTY_OPENGLES)
		pRenderer = nullptr;
#else
		pRenderer = nullptr;
#endif
	}

	m_pView->SetResizeCallback([=](const int& nWidth, const int& nHeight)
	{
		this->Render();
	});


	return true;
}

void MEngine::Release()
{
	if (m_pRenderer)
	{
		m_pRenderer->Release();
		delete m_pRenderer;
		m_pRenderer = nullptr;
	}

	if (m_pView)
	{
#if (RENDER_GRAPHICS == MORTY_DIRECTX_11)
		if (MWindowsRenderView* pWindowsView = dynamic_cast<MWindowsRenderView*>(m_pView))
		{
			pWindowsView->Release();
		}
#elif (RENDER_GRAPHICS == MORTY_OPENGLES)
		pRenderer = nullptr;
#else
		pRenderer = nullptr;
#endif
		
		delete m_pView;
		m_pView = nullptr;
	}
}

void MEngine::Run()
{
	m_cTickInfo.lPrevTickTime = MTimer::GetCurTime();

	MainLoop();
}

void MEngine::Tick(float fDelta)
{

}

void MEngine::Render()
{
	if (m_pRenderer)
	{
		m_pRenderer->Render();
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

void MEngine::MainLoop()
{
	bool bLoop = true;
	while (bLoop)
	{
		long long currentTime = MTimer::GetCurTime();

		float lTimeDelta = (float)(currentTime - m_cTickInfo.lPrevTickTime) / 1000;

		if (lTimeDelta >= m_cTickInfo.fTickInterval)
		{
			Tick(lTimeDelta);
			Render();         
			m_cTickInfo.lPrevTickTime = currentTime;
		}
		
		bLoop = m_pView->MainLoop();
	}
}

MEngine::TickInfo::TickInfo(int nFps)
{
	if (nFps == 0)
		nFps = 60;
	nMaxFPS = nFps;
	fTickInterval = 1.0f / nFps;
	lPrevTickTime = 0;
}

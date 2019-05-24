#include "MEngine.h"
#include "MDirectX11Renderer.h"
#include "MWindowsRenderView.h"
#include "MTimer.h"


MEngine::MEngine()
	: m_pRenderer(nullptr)
	, m_nMaxFPS(60)
	, m_fTickInterval(1.0f / 60)
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


	static float nTickTime = 0;
	m_pView->SetTickFunction([=](float fDelta)
	{
		bool bTicked = false;
		float tickInterval = 1.0f / m_nMaxFPS;
		nTickTime += fDelta;
		while (nTickTime > tickInterval)
		{
			nTickTime -= tickInterval;

			this->Tick(tickInterval);
			bTicked = true;
		}
		if (bTicked)
		{
			this->Render();
		}
		
	});

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
	if (m_pView)
		m_pView->Run();
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
		m_nMaxFPS = nFPS;
		m_fTickInterval = 1.0f / m_nMaxFPS;
	}
}

MIRenderer* MEngine::GetRenderer()
{
	return m_pRenderer;
}

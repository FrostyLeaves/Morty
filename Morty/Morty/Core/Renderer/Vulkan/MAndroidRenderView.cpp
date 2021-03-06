﻿#include "MAndroidRenderView.h"

#ifdef MORTY_ANDROID

#include <functional>
#include <string>

#include "MEngine.h"
#include "MVulkanRenderTarget.h"

MAndroidRenderView::MAndroidRenderView()
	: MIRenderView()
	, m_nWidth(640)
	, m_nHeight(480)
	, m_pNativeWindow(nullptr)
{

}

MAndroidRenderView::~MAndroidRenderView()
{

}

bool MAndroidRenderView::Initialize(MEngine* pEngine, const char* svWindowName)
{
	MIRenderView::Initialize(pEngine, svWindowName);

	m_pEngine = pEngine;

#if RENDER_GRAPHICS == MORTY_VULKAN
	MVulkanRenderTarget::CreateForAndroidView(pEngine->GetDevice(), this);
#endif

	return true;
}

void MAndroidRenderView::OnResize(const int& nWidth, const int& nHeight)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	if (MVulkanRenderTarget* pRt = dynamic_cast<MVulkanRenderTarget*>(m_pRenderTarget))
		pRt->Resize(nWidth, nHeight);
#endif
}

void MAndroidRenderView::SetRenderTarget(MIRenderTarget* pRenderTarget)
{
	MIRenderView::SetRenderTarget(pRenderTarget);
}

bool MAndroidRenderView::MainLoop(const float& fDelta)
{
	return true;
}

void MAndroidRenderView::SetWindowTitle(const MString& strTilte)
{
}

void MAndroidRenderView::Input(MInputEvent* pEvent)
{

}

void MAndroidRenderView::Release()
{
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Release(m_pEngine->GetDevice());
		delete m_pRenderTarget;
		m_pRenderTarget = nullptr;
	}

	MIRenderView::Release();
}

void MAndroidRenderView::SetSize(const Vector2& v2Size)
{
	m_nWidth = v2Size.x;
	m_nHeight = v2Size.y;
}

#endif
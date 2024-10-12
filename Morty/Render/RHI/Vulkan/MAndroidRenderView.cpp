#include "MAndroidRenderView.h"

//#ifdef MORTY_ANDROID
#if false

#include <functional>
#include <string>

#include "Engine/MEngine.h"
#include "RHI/Vulkan/MVulkanRenderTarget.h"

using namespace morty;

MAndroidRenderView::MAndroidRenderView()
    : MRenderView()
    , m_width(640)
    , m_height(480)
    , m_nativeWindow(nullptr)
{

}

MAndroidRenderView::~MAndroidRenderView()
{

}

bool MAndroidRenderView::Initialize(MEngine* pEngine, const char* svWindowName)
{
    MRenderView::Initialize(pEngine, svWindowName);

    m_engine = pEngine;

#if RENDER_GRAPHICS == MORTY_VULKAN
    MVulkanRenderTarget::CreateForAndroidView(pEngine->GetDevice(), this);
#endif

    return true;
}

void MAndroidRenderView::OnResize(const int& nWidth, const int& nHeight)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    if (MVulkanRenderTarget* pRt = dynamic_cast<MVulkanRenderTarget*>(m_renderTarget))
        pRt->Resize(nWidth, nHeight);
#endif
}

void MAndroidRenderView::SetRenderTarget(MIRenderTarget* pRenderTarget)
{
    MRenderView::SetRenderTarget(pRenderTarget);
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
    if (m_renderTarget)
    {
        m_renderTarget->Release(m_engine->GetDevice());
        delete m_renderTarget;
        m_renderTarget = nullptr;
    }

    MRenderView::Release();
}

void MAndroidRenderView::SetSize(const Vector2& v2Size)
{
    m_width = v2Size.x;
    m_height = v2Size.y;
}

#endif
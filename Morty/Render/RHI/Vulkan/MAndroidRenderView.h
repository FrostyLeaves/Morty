/**
 * @File         MAndroidRenderView
 * 
 * @Created      2020-09-18 18:52:47
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

//#ifdef MORTY_ANDROID
#if false

#include "MRenderView.h"
#include "Utility/MString.h"

namespace morty
{

class MORTY_API MAndroidRenderView : public MRenderView
{
public:
    MAndroidRenderView();
    virtual ~MAndroidRenderView();

public:
    virtual bool Initialize(MEngine* pEngine, const char* svWindowName) override;
    virtual void Release() override;

    void SetSize(const Vector2& v2Size);
    virtual int GetViewWidth() override { return m_width; }
    virtual int GetViewHeight() override { return m_height; }

    virtual bool GetMinimized() override { return false; }

    virtual void OnResize(const int& nWidth, const int& nHeight) override;
    virtual void SetRenderTarget(MIRenderTarget* pRenderTarget) override;
    virtual bool MainLoop(const float& fDelta) override;

    void SetWindowTitle(const MString& strTilte);

    virtual void Input(MInputEvent* pEvent) override;

    void SetNativeWindow(class ANativeWindow* pWindow) { m_nativeWindow = pWindow; }
    class ANativeWindow* GetNativeWindow() { return m_nativeWindow; }

protected:
    int m_width;
    int m_height;

    class ANativeWindow* m_nativeWindow;
};

}

#endif

/**
 * @File         MAndroidRenderView
 * 
 * @Created      2020-09-18 18:52:47
 *
 * @Author       DoubleYe
**/

#ifndef _M_ANDROID_RENDERVIEW_H_
#define _M_ANDROID_RENDERVIEW_H_
#include "MGlobal.h"

#ifdef MORTY_ANDROID

#include "MIRenderView.h"
#include "MString.h"


class MORTY_API MAndroidRenderView : public MIRenderView
{
public:
	MAndroidRenderView();
    virtual ~MAndroidRenderView();

public:
	virtual bool Initialize(MEngine* pEngine, const char* svWindowName) override;
	virtual void Release() override;

	void SetSize(const Vector2& v2Size);
	virtual int GetViewWidth() override { return m_nWidth; }
	virtual int GetViewHeight() override { return m_nHeight; }

	virtual bool GetMinimized() override { return false; }

	virtual void OnResize(const int& nWidth, const int& nHeight) override;
	virtual void SetRenderTarget(MIRenderTarget* pRenderTarget) override;
	virtual bool MainLoop(const float& fDelta) override;

	void SetWindowTitle(const MString& strTilte);

	virtual void Input(MInputEvent* pEvent) override;

	void SetNativeWindow(class ANativeWindow* pWindow) { m_pNativeWindow = pWindow; }
	class ANativeWindow* GetNativeWindow() { return m_pNativeWindow; }

protected:
	int m_nWidth;
	int m_nHeight;

	class ANativeWindow* m_pNativeWindow;
};


#endif


#endif
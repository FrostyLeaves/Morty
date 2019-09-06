/**
 * @File         MWindowsRenderView
 * 
 * @Created      2019-05-12 22:03:17
 *
 * @Author       Morty
**/

#ifndef _M_MWINDOWSRENDERVIEW_H_
#define _M_MWINDOWSRENDERVIEW_H_
#include "MGlobal.h"
#include "MIRenderView.h"

#include <map>
#include <windows.h>


class MORTY_CLASS MWindowsRenderView : public MIRenderView
{
public:
    MWindowsRenderView();
    virtual ~MWindowsRenderView();

public:
	virtual bool Initialize(MEngine* pEngine, const char* svWindowName) override;
	virtual void Release() override;

	bool IsClosed(){ return m_bIsClosed; }

	HWND GetHWND() { return m_hwnd; }

	virtual int GetViewWidth() override { return m_nWidth; }
	virtual int GetViewHeight() override { return m_nHeight; }

	virtual void SetResizeCallback(const ResizeCallback& func) override;

	virtual bool MainLoop() override;

public:
	static LRESULT CALLBACK ProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	static bool RegisterClass();

protected:
	HWND m_hwnd;

	int m_nWidth;
	int m_nHeight;


	long long m_lEnginePrevTickTime;
	bool m_bIsClosed;

	ResizeCallback m_pResizeCallback;

protected:
	static HINSTANCE s_hInstance;
	static std::map<HWND, MWindowsRenderView*> s_tViewTable;
	static bool s_bIsRegisterWindow;
};


#endif

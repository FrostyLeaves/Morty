/**
 * @File         MWindowsRenderView
 * 
 * @Created      2019-05-12 22:03:17
 *
 * @Author       Pobrecito
**/

#ifndef _M_MWINDOWSRENDERVIEW_H_
#define _M_MWINDOWSRENDERVIEW_H_
#include "MGlobal.h"

#ifdef MORTY_WIN

#include "MIRenderView.h"
#include "MString.h"

#include <map>
#include <vector>
#include <windows.h>


class MORTY_API MWindowsRenderView : public MIRenderView
{
public:
    MWindowsRenderView();
    virtual ~MWindowsRenderView();

public:
	virtual bool Initialize(MEngine* pEngine, const char* svWindowName) override;
	virtual void Release() override;

	bool IsClosed(){ return m_bIsClosed; }

	HWND GetHWND() { return m_hwnd; }
	HINSTANCE GetHINSTANCE() { return s_hInstance; }

	virtual int GetViewWidth() override { return m_nWidth; }
	virtual int GetViewHeight() override { return m_nHeight; }

	virtual bool GetMinimized() override { return m_bMinimized; }

	virtual void OnResize(const int& nWidth, const int& nHeight) override;
	virtual void SetRenderTarget(MIRenderTarget* pRenderTarget) override;
	virtual bool MainLoop(const float& fDelta) override;

	void SetWindowTitle(const MString& strTilte);

	virtual void Input(MInputEvent* pEvent) override;

public:

	
	struct MKeyState
	{
		uint32_t unKey;
		MEKeyState eState;
	};

	void KeyBoardChanged(const uint32_t& unKey, const MEKeyState& eState);
	void MouseBtnChanged(const uint32_t& unMouseBtn, const MEKeyState& eState);

	static LRESULT CALLBACK ProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


	virtual LRESULT CALLBACK ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	static bool RegisterClass();

	void CheckInputEvent();

protected:
	HWND m_hwnd;

	int m_nWidth;
	int m_nHeight;


	long long m_lEnginePrevTickTime;
	bool m_bIsClosed;

	bool m_bMinimized;

	std::vector<MKeyState> m_vKeyQueue;
	std::vector<MKeyState> m_vMouseBtnQueue;

	float m_fCheckInputDelta;

protected:
	static HINSTANCE s_hInstance;
	static std::map<HWND, MWindowsRenderView*> s_tViewTable;
	static bool s_bIsRegisterWindow;
};


#endif


#endif
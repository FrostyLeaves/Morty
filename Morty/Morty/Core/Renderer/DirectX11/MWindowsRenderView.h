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
#include "MIRenderView.h"
#include "MString.h"

#include <map>
#include <vector>
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

	virtual void OnResize(const int& nWidth, const int& nHeight) override;

	virtual bool MainLoop() override;

	void SetWindowTitle(const MString& strTilte);

public:

	
	struct MKeyState
	{
		unsigned int unKey;
		MEKeyState eState;
	};

	void KeyBoardChanged(const unsigned int& unKey, const MEKeyState& eState);
	void MouseBtnChanged(const unsigned int& unMouseBtn, const MEKeyState& eState);

	static LRESULT CALLBACK ProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


	virtual LRESULT CALLBACK ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	static bool RegisterClass();

protected:
	HWND m_hwnd;

	int m_nWidth;
	int m_nHeight;


	long long m_lEnginePrevTickTime;
	bool m_bIsClosed;

	std::vector<MKeyState> m_vKeyQueue;
	std::vector<MKeyState> m_vMouseBtnQueue;

protected:
	static HINSTANCE s_hInstance;
	static std::map<HWND, MWindowsRenderView*> s_tViewTable;
	static bool s_bIsRegisterWindow;
};


#endif

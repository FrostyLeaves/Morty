#include "MWindowsRenderView.h"
#include <windows.h>
#include <functional>
#include <string>

#include "MLogManager.h"
#include "MTimer.h"
#include "MEngine.h"
#include "MInputManager.h"
#include "MIRenderer.h"

std::map<HWND, MWindowsRenderView*> MWindowsRenderView::s_tViewTable = std::map<HWND, MWindowsRenderView*>();
HINSTANCE MWindowsRenderView::s_hInstance = 0;
bool MWindowsRenderView::s_bIsRegisterWindow = false;

MWindowsRenderView::MWindowsRenderView()
	: m_bIsClosed(false)
	, m_nWidth(640)
	, m_nHeight(480)
	, m_lEnginePrevTickTime(0)
{

}

MWindowsRenderView::~MWindowsRenderView()
{

}

bool MWindowsRenderView::RegisterClass()
{
	s_hInstance = GetModuleHandle(NULL);

	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = MWindowsRenderView::ProcessFunction;
	wndClass.hInstance = s_hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "Morty";

	if (!RegisterClassEx(&wndClass))
		return false;


	s_bIsRegisterWindow = true;
	return true;
}

bool MWindowsRenderView::Initialize(MEngine* pEngine, const char* svWindowName)
{
	if (false == s_bIsRegisterWindow && !RegisterClass())
	{
		MLogManager::GetInstance()->Error("Register class 'Morty' failed!");
		return false;
	}

	if (svWindowName == NULL)
	{
		MLogManager::GetInstance()->Error("Window name can`t be empty!");
		return false;
	}


	m_pEngine = pEngine;

	RECT rc = { 0, 0, m_nWidth, m_nHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	m_hwnd = CreateWindow("Morty", svWindowName, WS_OVERLAPPEDWINDOW, 0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, s_hInstance, NULL);

	if (!m_hwnd)
	{
		auto err = GetLastError();
		MLogManager::GetInstance()->Error("Create Window failed! error code: %l", err);
		return false;
	}

	s_tViewTable[m_hwnd] = this;

	ShowWindow(m_hwnd, SW_SHOW);
	UpdateWindow(m_hwnd);

	return true;
}

bool MWindowsRenderView::MainLoop()
{
	MSG msg = { 0 };

	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.message != WM_QUIT;
}

void MWindowsRenderView::Release()
{
	s_tViewTable.erase(m_hwnd);
}

void MWindowsRenderView::SetResizeCallback(const ResizeCallback& func)
{
	m_pResizeCallback = func;
}

LRESULT CALLBACK MWindowsRenderView::ProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MWindowsRenderView* pView = s_tViewTable[hwnd];

	if (pView)
		return pView->ViewProcessFunction(hwnd, message, wParam, lParam);
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK MWindowsRenderView::ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		m_bIsClosed = true;
		Release();
		break;

	case WM_ERASEBKGND:
		m_pEngine->GetRenderer()->RenderViewportToView(GetViewport(), this);
		break;
	case WM_SIZE:
	{
		m_nWidth = LOWORD(lParam);
		m_nHeight = HIWORD(lParam);

		if (m_pResizeCallback)
		{
			m_pResizeCallback(LOWORD(lParam), HIWORD(lParam));
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	case WM_KEYDOWN:
		if ((HIWORD(lParam) & KF_REPEAT) == 0)
			m_pEngine->GetInputManager()->Input(new MKeyBoardInputEvent(wParam, MKeyBoardInputEvent::KeyBoardDown));
		break;

	case WM_KEYUP:
		m_pEngine->GetInputManager()->Input(new MKeyBoardInputEvent(wParam, MKeyBoardInputEvent::KeyBoardUp));
		break;

	case WM_MOUSEMOVE:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		m_pEngine->GetInputManager()->Input(new MMouseInputEvent(Vector2(x, y)));
		break;
	}

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}


	return 0;
}

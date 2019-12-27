#include "MWindowsRenderView.h"
#include <windows.h>
#include <functional>
#include <string>

#include "MLogManager.h"
#include "MTimer.h"
#include "MEngine.h"
#include "MInputManager.h"
#include "MIRenderer.h"
#include "MIViewport.h"
#include "MTimer.h"

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

	int nScreenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYFULLSCREEN);

	RECT rc = { (nScreenWidth - m_nWidth) * 0.5, (nScreenHeight - m_nHeight) * 0.5, (nScreenWidth + m_nWidth) * 0.5, (nScreenHeight + m_nHeight) * 0.5 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	m_hwnd = CreateWindow("Morty", svWindowName, WS_OVERLAPPEDWINDOW, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, s_hInstance, NULL);

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

void MWindowsRenderView::OnResize(const int& nWidth, const int& nHeight)
{
	m_pEngine->GetRenderer()->OnResize(this, nWidth, nHeight);

	for (MIViewport* pViewport : m_vViewport)
	{
		pViewport->SetSize(Vector2(nWidth, nHeight));
	}
}

bool MWindowsRenderView::MainLoop()
{
	MSG msg = { 0 };

	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	static POINT point = { -1, -1 };
	POINT newPoint;
	TCHAR s[10];
	GetCursorPos(&newPoint);
	ScreenToClient(m_hwnd, &newPoint);

	if (point.x != newPoint.x || point.y != newPoint.y)
	{
		Vector2 v2Addi;
		if (point.x == -1 && point.y == -1)
			v2Addi = Vector2(0, 0);
		else
			v2Addi = Vector2(newPoint.x - point.x, newPoint.y - point.y);

		MMouseInputEvent event(Vector2(newPoint.x, newPoint.y), v2Addi);


		for (MIViewport* pViewport : m_vViewport)
		{
			MMouseInputEvent eventClone(event);
			pViewport->Input(&eventClone);
		}
		point = newPoint;
	}

	for (const MKeyState& state : m_vKeyQueue)
	{
		MKeyBoardInputEvent event(state.unKey, state.eState);

		for (MIViewport* pViewport : m_vViewport)
		{
			MKeyBoardInputEvent eventClone(event);
			pViewport->Input(&eventClone);
		}
	}

	for (const MKeyState& state : m_vMouseBtnQueue)
	{
		MMouseInputEvent event((MMouseInputEvent::MEMouseDownButton)state.unKey, state.eState == MEKeyState::DOWN ? MMouseInputEvent::MEMouseInputType::ButtonDown : MMouseInputEvent::MEMouseInputType::ButtonUp);

		for (MIViewport* pViewport : m_vViewport)
		{
			MMouseInputEvent eventClone(event);
			pViewport->Input(&event);
		}
	}

	m_vKeyQueue.clear();
	m_vMouseBtnQueue.clear();

	return msg.message != WM_QUIT;
}

void MWindowsRenderView::SetWindowTitle(const MString& strTilte)
{
	SetWindowText(m_hwnd, strTilte.c_str());
}

void MWindowsRenderView::Release()
{
	s_tViewTable.erase(m_hwnd);
}

void MWindowsRenderView::KeyBoardChanged(const unsigned int& unKey, const MEKeyState& eState)
{
	MKeyState state;
	state.unKey = unKey;
	state.eState = eState;
	m_vKeyQueue.push_back(state);
}

void MWindowsRenderView::MouseBtnChanged(const unsigned int& unMouseBtn, const MEKeyState& eState)
{
	MKeyState state;
	state.unKey = unMouseBtn;
	state.eState = eState;
	m_vMouseBtnQueue.push_back(state);
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
		break;

	case WM_ERASEBKGND:
		m_pEngine->GetRenderer()->RenderToView(this);
		break;
	case WM_SIZE:
	{
		OnResize(LOWORD(lParam), HIWORD(lParam));

		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	case WM_KEYDOWN:
		if ((HIWORD(lParam) & KF_REPEAT) == 0)
		{
			KeyBoardChanged(wParam, MEKeyState::DOWN);
		}
		break;

	case WM_KEYUP:
		KeyBoardChanged(wParam, MEKeyState::UP);
		break;

	case WM_LBUTTONDOWN:
	{
		MouseBtnChanged(MMouseInputEvent::MEMouseDownButton::LeftButton, MEKeyState::DOWN);
		break;
	}
	case WM_LBUTTONUP:
	{
		MouseBtnChanged(MMouseInputEvent::MEMouseDownButton::LeftButton, MEKeyState::UP);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		MouseBtnChanged(MMouseInputEvent::MEMouseDownButton::RightButton, MEKeyState::DOWN);
		break;
	}
	case WM_RBUTTONUP:
	{
		MouseBtnChanged(MMouseInputEvent::MEMouseDownButton::RightButton, MEKeyState::UP);
		break;
	}

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}

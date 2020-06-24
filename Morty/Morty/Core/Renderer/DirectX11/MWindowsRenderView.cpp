#include "MWindowsRenderView.h"
#include <windows.h>
#include <functional>
#include <string>

#include "MLogManager.h"
#include "Timer/MTimer.h"
#include "MEngine.h"
#include "MInputManager.h"
#include "MIRenderer.h"
#include "MViewport.h"
#include "MIRenderTarget.h"

std::map<HWND, MWindowsRenderView*> MWindowsRenderView::s_tViewTable = std::map<HWND, MWindowsRenderView*>();
HINSTANCE MWindowsRenderView::s_hInstance = 0;
bool MWindowsRenderView::s_bIsRegisterWindow = false;

MWindowsRenderView::MWindowsRenderView()
	: MIRenderView()
	, m_bIsClosed(false)
	, m_nWidth(640)
	, m_nHeight(480)
	, m_lEnginePrevTickTime(0)
	, m_fCheckInputDelta(0.0f)
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

void MWindowsRenderView::CheckInputEvent()
{
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
		Input(&event);
		
		point = newPoint;
	}

	for (const MKeyState& state : m_vKeyQueue)
	{
		MKeyBoardInputEvent event(state.unKey, state.eState);
		Input(&event);
	}

	for (const MKeyState& state : m_vMouseBtnQueue)
	{
		MMouseInputEvent event((MMouseInputEvent::MEMouseDownButton)state.unKey, state.eState == MEKeyState::DOWN ? MMouseInputEvent::MEMouseInputType::ButtonDown : MMouseInputEvent::MEMouseInputType::ButtonUp);
		Input(&event);
	}

	m_vKeyQueue.clear();
	m_vMouseBtnQueue.clear();
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

	RECT rc = { (nScreenWidth - m_nWidth) / 2, (nScreenHeight - m_nHeight) / 2, (nScreenWidth + m_nWidth) / 2, (nScreenHeight + m_nHeight) / 2 };
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

	m_pEngine->GetRenderer()->AddOutputView(this);

	return true;
}

void MWindowsRenderView::OnResize(const int& nWidth, const int& nHeight)
{
	if(m_pRenderTarget)
		m_pRenderTarget->OnResize(nWidth, nHeight);
}

void MWindowsRenderView::SetRenderTarget(MIRenderTarget* pRenderTarget)
{
	MIRenderView::SetRenderTarget(pRenderTarget);
}

bool MWindowsRenderView::MainLoop(const float& fDelta)
{
	MSG msg = { 0 };

	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	m_fCheckInputDelta += fDelta;

	if (m_fCheckInputDelta > 1.0f / 30.0f)
	{
		m_fCheckInputDelta -= 1.0f / 30.0f;
		CheckInputEvent();
	}

	return msg.message != WM_QUIT;
}

void MWindowsRenderView::SetWindowTitle(const MString& strTilte)
{
	SetWindowText(m_hwnd, strTilte.c_str());
}

void MWindowsRenderView::Input(MInputEvent* pEvent)
{
	for (MViewport* pViewport : m_vViewport)
	{
		pViewport->Input(pEvent);
	}
}

void MWindowsRenderView::Release()
{
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Release(m_pEngine->GetDevice());
		delete m_pRenderTarget;
		m_pRenderTarget = nullptr;
	}
	s_tViewTable.erase(m_hwnd);
}

void MWindowsRenderView::KeyBoardChanged(const uint32_t& unKey, const MEKeyState& eState)
{
	MKeyState state;
	state.unKey = unKey;
	state.eState = eState;
	m_vKeyQueue.push_back(state);
}

void MWindowsRenderView::MouseBtnChanged(const uint32_t& unMouseBtn, const MEKeyState& eState)
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
		m_pEngine->RenderToView(this);
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

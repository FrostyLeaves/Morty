#include "MInputManager.h"

unsigned int MMouseInputEvent::s_unMouseDownButton = 0;
Vector2 MMouseInputEvent::s_v2MousePosition(-1, -1);
bool MKeyBoardInputEvent::s_vKeyDownMap[256] = { 0 };

MInputEvent::MInputEvent()
	: m_bIsAccepted(false)
{

}

MInputEvent::~MInputEvent()
{

}

MMouseInputEvent::MMouseInputEvent(const MEMouseDownButton& eMouseDownButton, const MEMouseInputType& eInputType)
	: m_eEventButton(eMouseDownButton)
{
	if (MEMouseInputType::ButtonDown == eInputType)
	{
		s_unMouseDownButton |= eMouseDownButton;
	}
	else if (MEMouseInputType::ButtonUp == eInputType)
	{
		s_unMouseDownButton &= ~eMouseDownButton;
	}

	m_eInputType = eInputType;

	m_v2MousePositionAddition = Vector2(0, 0);
}

MMouseInputEvent::MMouseInputEvent(const Vector2& v2MousePosition)
	: m_eEventButton(MEMouseDownButton::NoneButton)
	, m_eInputType(MEMouseInputType::MouseMove)
{
	if (s_v2MousePosition.x != -1 && s_v2MousePosition.y != -1)
	{
		m_v2MousePositionAddition = v2MousePosition - s_v2MousePosition;
	}
	else
	{
		m_v2MousePositionAddition = Vector2(0, 0);
	}
	s_v2MousePosition = v2MousePosition;
}

MKeyBoardInputEvent::MKeyBoardInputEvent(const unsigned int& unKeyIndex, const MEKeyBoardInputType& eInputType)
	: m_unKeyIndex(0)
{
	if (unKeyIndex < 256)
	{
		s_vKeyDownMap[unKeyIndex] = MEKeyBoardInputType::KeyBoardDown == eInputType;

		m_unKeyIndex = unKeyIndex;

	}

	m_eInputType = eInputType;
}

MInputListener::MInputListener()
	: m_function(nullptr)
{

}

MInputManager::MInputManager()
{

}

MInputManager::~MInputManager()
{
	for (MInputListener* pListener : m_vListenerList)
		delete pListener;
	
	m_vListenerList.clear();
}

void MInputManager::Input(MInputEvent* pEvent)
{
	for (MInputListener* pListener : m_vListenerList)
	{
		if (pListener->m_function)
		{
			pListener->m_function(pEvent);
		}
	}

	delete pEvent;
	pEvent = nullptr;
}

void MInputManager::AddListener(MInputListener* pListener)
{
	for (MInputListener* p : m_vListenerList)
		if (p == pListener)
			return;
	
	m_vListenerList.push_back(pListener);
}

void MInputManager::RemoveListener(MInputListener* pListener)
{
	for (std::vector<MInputListener*>::iterator iter = m_vListenerList.begin(); iter != m_vListenerList.end(); ++iter)
	{
		if (*iter == pListener)
		{
			m_vListenerList.erase(iter);
			break;
		}
	}
}


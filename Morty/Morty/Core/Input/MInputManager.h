/**
 * @File         MInputManager
 * 
 * @Created      2019-09-03 18:22:20
 *
 * @Author       Pobrecito
**/

#ifndef _M_MINPUTMANAGER_H_
#define _M_MINPUTMANAGER_H_
#include "MGlobal.h"
#include "Vector.h"

#include <vector>
#include <functional>

class MViewport;
class MORTY_CLASS MInputEvent
{
public:
	MInputEvent();
	virtual ~MInputEvent();

	void Accepted() { m_bIsAccepted = true; }
	bool IsAccepted() { return m_bIsAccepted; }


private:
	bool m_bIsAccepted;
};

class MORTY_CLASS MMouseInputEvent : public MInputEvent
{
public:

	enum MEMouseDownButton
	{
		NoneButton = 0,
		LeftButton = 1 << 0,
		RightButton = 1 << 1,
		ScrollButton = 1 << 2,
	};

	enum MEMouseInputType
	{
		ButtonDown = 1,
		ButtonUp = 2,
		MouseMove = 3,
	};

	MMouseInputEvent(const MEMouseDownButton& eMouseDownButton, const MEMouseInputType& eInputType);

	MMouseInputEvent(const Vector2& v2MousePosition, const Vector2& v2MousePositionAddition);

	Vector2 GetMosuePosition() { return s_v2MousePosition; }
	Vector2 GetMouseAddition(){ return m_v2MousePositionAddition; }
	MEMouseDownButton GetButton(){ return m_eEventButton; }
	MEMouseInputType GetType(){ return m_eInputType; }
		
protected:

	MEMouseDownButton m_eEventButton;
	Vector2 m_v2MousePositionAddition;

	static unsigned int s_unMouseDownButton;
	static Vector2 s_v2MousePosition;

	MEMouseInputType m_eInputType;

};

class MORTY_CLASS MKeyBoardInputEvent : public MInputEvent
{
public:

	MKeyBoardInputEvent(const unsigned int& unKeyIndex, const MEKeyState& eInputType);

	unsigned int GetKey() { return m_unKeyIndex; }
	MEKeyState GetType(){ return m_eInputType; }

protected:

	static bool s_vKeyDownMap[256];

	unsigned int m_unKeyIndex;
	MEKeyState m_eInputType;
};

#endif

/**
 * @File         MInputManager
 * 
 * @Created      2019-09-03 18:22:20
 *
 * @Author       Morty
**/

#ifndef _M_MINPUTMANAGER_H_
#define _M_MINPUTMANAGER_H_
#include "MGlobal.h"
#include "Vector.h"

#include <vector>
#include <functional>

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

protected:

	static unsigned int s_unMouseDownButton;
	static Vector2 m_v2MousePosition;

	MEMouseInputType m_eInputType;

};

class MORTY_CLASS MKeyBoardInputEvent : public MInputEvent
{
public:
	enum MEKeyBoardInputType
	{
		KeyBoardDown = 1,
		KeyBoardUp = 2,
	};


	MKeyBoardInputEvent(const unsigned int& unKeyIndex, const MEKeyBoardInputType& eInputType);

	unsigned int GetKey() { return m_unKeyIndex; }
	MEKeyBoardInputType GetType(){ return m_eInputType; }

protected:

	static bool s_vKeyDownMap[256];

	unsigned int m_unKeyIndex;
	MEKeyBoardInputType m_eInputType;
};

class MORTY_CLASS MInputListener
{
public:
	typedef std::function<void(MInputEvent*)> MInputEventFunction;

	MInputListener();

	MInputEventFunction m_function;

};

class MORTY_CLASS MInputManager
{
public:
    MInputManager();
    virtual ~MInputManager();

public:

	void Input(MInputEvent* pEvent);


	void AddListener(MInputListener* pListener);
	void RemoveListener(MInputListener* pListener);

private:
	std::vector<MInputListener*> m_vListenerList;
};


#endif

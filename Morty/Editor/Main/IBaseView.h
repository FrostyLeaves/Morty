#ifndef _I_BASE_VIEW_H_
#define _I_BASE_VIEW_H_

#include "Utility/MGlobal.h"

class MEngine;
class MInputEvent;
class IBaseView
{
public:
	IBaseView() {}
	virtual ~IBaseView() {}

	virtual void Render() = 0;

	virtual void Initialize(MEngine* pEngine) = 0;
	virtual void Release() = 0;

	virtual void Input(MInputEvent* pEvent) = 0;

	MString GetName() const { return m_strViewName; }
	bool GetVisible() const { return m_bVisiable; }
	void SetVisible(bool bVisible) { m_bVisiable = bVisible; }


protected:
	MString m_strViewName = "";
	bool m_bVisiable = false;
};




#endif
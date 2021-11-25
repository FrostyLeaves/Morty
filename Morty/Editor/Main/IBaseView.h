#ifndef _I_BASE_VIEW_H_
#define _I_BASE_VIEW_H_

#include "MGlobal.h"

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


	bool m_bVisiable = false;
};




#endif
/**
 * @File         MIRenderView
 * 
 * @Created      2019-05-12 21:58:21
 *
 * @Author       Morty
**/

#ifndef _M_MIRENDERVIEW_H_
#define _M_MIRENDERVIEW_H_
#include "MGlobal.h"
#include <functional>

#include "Vector.h"

class MIViewport;
class MEngine;
class MORTY_CLASS MIRenderView
{
public:
    MIRenderView();
    virtual ~MIRenderView();

	typedef std::function<void(int, int)> ResizeCallback;

public:

	virtual bool Initialize(MEngine* pEngine, const char* svWindowName) = 0;
	virtual void Release() = 0;

	virtual int GetViewWidth() = 0;
	virtual int GetViewHeight() = 0;

	virtual Vector2 GetRenderRectTopLeft() { return VECTOR2_ZERO; }
	virtual Vector2 GetRenderRectSize() { return Vector2(GetViewWidth(), GetViewHeight()); }

	virtual void SetResizeCallback(const ResizeCallback& func) = 0;

	virtual bool MainLoop() = 0;

	virtual void OnRenderBegin() {}
	virtual void OnRenderEnd() {}

	void SetViewport(MIViewport* pViewport);
	MIViewport* GetViewport(){ return m_pViewport; }


protected:

	friend class MEngine;
	MIViewport* m_pViewport;

	MEngine* m_pEngine;

};


#endif

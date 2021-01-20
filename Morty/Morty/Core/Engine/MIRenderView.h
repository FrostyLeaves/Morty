/**
 * @File         MIRenderView
 * 
 * @Created      2019-05-12 21:58:21
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIRENDERVIEW_H_
#define _M_MIRENDERVIEW_H_
#include "MGlobal.h"
#include <functional>
#include <vector>

#include "Vector.h"
#include "Type/MColor.h"

class MEngine;
class MViewport;
class MInputEvent;
class MIRenderTarget;
class MORTY_API MIRenderView
{
public:
    MIRenderView();
    virtual ~MIRenderView();

	typedef std::function<void(int, int)> ResizeCallback;

public:

	virtual bool Initialize(MEngine* pEngine, const char* svWindowName);
	virtual void Release();

	virtual int GetViewWidth() = 0;
	virtual int GetViewHeight() = 0;

	virtual void OnResize(const int& nWidth, const int& nHeight) = 0;

	virtual bool MainLoop(const float& fDelta) = 0;

	virtual void Input(MInputEvent* pEvent) = 0;

	virtual void OnRenderBegin() {}
	virtual void OnRenderEnd() {}

	virtual bool GetMinimized() = 0;

	void AppendViewport(MViewport* pViewport);
	void RemoveViewport(MViewport* pViewport);
	std::vector<MViewport*>& GetViewports(){ return m_vViewport; }

	MIRenderTarget* GetRenderTarget() { return m_pRenderTarget; }
	virtual void SetRenderTarget(MIRenderTarget* pRenderTarget) {m_pRenderTarget = pRenderTarget;}

	void SetBackColor(const MColor& color) { m_BackColor = color; }
	MColor GetBackColor()const { return m_BackColor; }

protected:

	friend class MEngine;
	std::vector<MViewport*> m_vViewport;

	MEngine* m_pEngine;

	MIRenderTarget* m_pRenderTarget;

	MColor m_BackColor;
};


#endif

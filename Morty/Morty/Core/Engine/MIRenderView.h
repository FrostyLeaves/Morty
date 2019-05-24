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

class MORTY_CLASS MIRenderView
{
public:
    MIRenderView();
    virtual ~MIRenderView();

	typedef std::function<void(int, int)> ResizeCallback;

public:

	virtual int GetViewWidth() = 0;
	virtual int GetViewHeight() = 0;

	virtual void SetResizeCallback(const ResizeCallback& func) = 0;

	virtual bool MainLoop() = 0;

private:

};


#endif

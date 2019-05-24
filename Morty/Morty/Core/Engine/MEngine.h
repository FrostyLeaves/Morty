/**
 * @File         MEngine
 * 
 * @Created      2019-05-11 23:19:14
 *
 * @Author       Morty
**/

#ifndef _M_MENGINE_H_
#define _M_MENGINE_H_
#include "MGlobal.h"


class MIRenderer;
class MIRenderView;
class MORTY_CLASS MEngine
{
public:
    MEngine();
    virtual ~MEngine();

public:

	virtual bool Initialize();
	virtual void Release();

	virtual void Run();

	virtual void Tick(float fDelta);
	virtual void Render();

public:

	void SetMaxFPS(const int& nFPS);

	MIRenderer* GetRenderer();
	


protected:

	void MainLoop();

private:

	MIRenderer* m_pRenderer;
	MIRenderView* m_pView;

	struct TickInfo
	{
		int nMaxFPS;
		float fTickInterval;
		long long lPrevTickTime;

		TickInfo(int nFps);

	} m_cTickInfo;
};


#endif

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

#include <vector>

class MIRenderer;
class MIRenderView;
class MNode;
class MORTY_CLASS MEngine
{
public:
    MEngine();
    virtual ~MEngine();

public:

	virtual bool Initialize();
	virtual void Release();

	bool MainLoop();

	virtual void Tick(float fDelta);


	void CreateView();

public:

	void SetMaxFPS(const int& nFPS);

	MIRenderer* GetRenderer();
	


protected:


private:

	MIRenderer* m_pRenderer;
	
	std::vector<MIRenderView*> m_vView;

	struct TickInfo
	{
		int nMaxFPS;
		float fTickInterval;
		long long lPrevTickTime;

		TickInfo(int nFps);

	} m_cTickInfo;
};


#endif

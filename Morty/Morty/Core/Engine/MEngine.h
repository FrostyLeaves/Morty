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
class MObjectManager;
class MResourceManager;
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

	void SetRootNode(MNode* pNode);

	void CreateView();

	MObjectManager* GetObjectManager() { return m_pObjectManager; }
	MResourceManager* GetResourceManager() { return m_pResourceManager; }

public:

	void SetMaxFPS(const int& nFPS);

	MIRenderer* GetRenderer();

protected:


private:

	MObjectManager* m_pObjectManager;
	MResourceManager* m_pResourceManager;

	MNode* m_pRootNode;

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

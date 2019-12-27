/**
 * @File         MEngine
 * 
 * @Created      2019-05-11 23:19:14
 *
 * @Author       Pobrecito
**/

#ifndef _M_MENGINE_H_
#define _M_MENGINE_H_
#include "MGlobal.h"

#include <vector>

class MNode;
class MIDevice;
class MIRenderer;
class MIRenderView;
class MObjectManager;
class MResourceManager;
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

	MIRenderView* CreateView();
	void AddView(MIRenderView* pView);

	MObjectManager* GetObjectManager() { return m_pObjectManager; }
	MResourceManager* GetResourceManager() { return m_pResourceManager; }

public:

	void SetMaxFPS(const int& nFPS);
	float GetInstantFPS() { return 1.0f / m_cTickInfo.fTimeDelta; }

	MIDevice* GetDevice() { return m_pDevice; }
	MIRenderer* GetRenderer() { return m_pRenderer; }

protected:

	bool InitializeDefaultResource();
	void ReleaseDefaultResource();

private:

	MObjectManager* m_pObjectManager;
	MResourceManager* m_pResourceManager;

	MNode* m_pRootNode;

	MIDevice* m_pDevice;
	MIRenderer* m_pRenderer;
	
	std::vector<MIRenderView*> m_vView;

	struct TickInfo
	{
		int nMaxFPS;
		float fTickInterval;
		float fTimeDelta;
		long long lPrevTickTime;

		TickInfo(int nFps);

	} m_cTickInfo;
};


#endif

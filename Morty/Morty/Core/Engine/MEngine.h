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
#include "MProject.h"
#include "MTypedClass.h"
#include "MIRenderProgram.h"

#include <vector>

class MNode;
class MScene;
class MIDevice;
class MIRenderer;
class MIRenderView;
class MObjectManager;
class MIRenderProgram;
class MResourceManager;
class MORTY_API MEngine
{
public:
    MEngine();
    virtual ~MEngine();

public:

	bool OpenProject(const MString& strProjectPath);

public:

	virtual bool Initialize(const MString& strSearchPath);
	virtual void Release();

	bool MainLoop();

	virtual void Tick(float fDelta);

	void SetScene(MScene* pScene);

	void AddView(MIRenderView* pView);

	void RenderToView(MIRenderView* pView, const float& fDelta);

	MObjectManager* GetObjectManager() { return m_pObjectManager; }
	MResourceManager* GetResourceManager() { return m_pResourceManager; }

public:

	void SetMaxFPS(const int& nFPS);
	float GetInstantFPS() { return 1.0f / m_cTickInfo.fTimeDelta; }
	float GetInstantDelta() { return m_cTickInfo.fTimeDelta; }

	MIDevice* GetDevice() { return m_pDevice; }
	MIRenderer* GetRenderer() { return m_pRenderer; }

protected:

	bool InitializeDefaultResource();
	void ReleaseDefaultResource();

private:

	MProject m_Project;

	MObjectManager* m_pObjectManager;
	MResourceManager* m_pResourceManager;

	MScene* m_pScene;

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

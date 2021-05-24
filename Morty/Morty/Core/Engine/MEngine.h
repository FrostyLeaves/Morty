/**
 * @File         MEngine
 * 
 * @Created      2019-05-11 23:19:14
 *
 * @Author       DoubleYe
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
class MIPlugin;
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

	uint32_t GetFrameIdx() { return m_unFrameIdx; }

public:

	void SetMaxFPS(const int& nFPS);
	float GetInstantFPS() { return 1.0f / m_cTickInfo.fTimeDelta; }
	float GetInstantDelta() { return m_cTickInfo.fTimeDelta; }

	MIDevice* GetDevice() { return m_pDevice; }
	MIRenderer* GetRenderer() { return m_pRenderer; }


public:

	template<class T>
	void RegisterPlugin();
	void RegisterPlugin(MTypeIdentifierConstPointer type);

	template<class T>
	T* GetPlugin() { return static_cast<T*>(GetPlugin(T::GetClassTypeIdentifier())); }
	MIPlugin* GetPlugin(MTypeIdentifierConstPointer pComponentType);

protected:

	bool InitializeDefaultResource();
	void ReleaseDefaultResource();

	void WaitRenderFinished(const uint32_t& unFrameIdx);

private:

	MProject m_Project;

	//TODO 这个idx值，应该是上层无感知的
	uint32_t m_unFrameIdx;

	MObjectManager* m_pObjectManager;
	MResourceManager* m_pResourceManager;

	MScene* m_pScene;
	MIDevice* m_pDevice;
	MIRenderer* m_pRenderer;


	std::vector<MIPlugin*> m_vPlugins;
	
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

template<class T>
void MEngine::RegisterPlugin()
{
	RegisterPlugin(T::GetClassTypeIdentifier());
}

#endif

/**
 * @File         MRenderSystem
 * 
 * @Created      2021-07-15 11:10:16
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERSYSTEM_H_
#define _M_MRENDERSYSTEM_H_
#include "MRenderGlobal.h"
#include "MSystem.h"


class MIDevice;
class MTaskNode;
class MIRenderCommand;
class MORTY_API MRenderSystem : public MISystem
{
public:
    MORTY_CLASS(MRenderSystem)
public:
    MRenderSystem();
    virtual ~MRenderSystem();

public:

    void Update(MTaskNode* pNode);
    

    MIRenderCommand* NextFrame();

public:

    MIDevice* GetDevice();


    void CheckFrameFinish();
    void WiteAllFrameFinish();

public:

    virtual void Initialize();
    virtual void Release();

private:

    MIDevice* m_pDevice;
    uint32_t m_unFrameCount;

    std::vector<MIRenderCommand*> m_vWaitRenderCommand;
};


#endif

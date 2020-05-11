/**
 * @File         MISystem
 * 
 * @Created      2020-04-02 13:03:24
 *
 * @Author       Pobrecito
**/

#ifndef _M_MISYSTEM_H_
#define _M_MISYSTEM_H_
#include "MGlobal.h"
#include "MObject.h"

class MScene;
class MViewport;
class MIRenderer;
class MIRenderTarget;
class MORTY_CLASS MISystem : public MObject
{
public:
	M_OBJECT(MISystem);
    MISystem();
    virtual ~MISystem();

public:

    virtual void Tick(const float& fDelta) {};
    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport, MScene* pScene, MIRenderTarget* pRenderTarget) {};

private:
};

#endif

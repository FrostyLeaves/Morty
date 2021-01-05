/**
 * @File         MIPostProcessWork
 * 
 * @Created      2020-11-30 16:08:08
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIPOSTPROCESSWORK_H_
#define _M_MIPOSTPROCESSWORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "Vector.h"

class MITexture;
class MViewport;
class MIRenderer;
class MIRenderProgram;
class MIRenderBackTexture;
class MTextureRenderTarget;
struct MORTY_CLASS MPostProcessRenderInfo
{
    float fDelta;
    uint32_t unFrameIndex;
    MIRenderer* pRenderer;
    MViewport* pViewport;
    MIRenderBackTexture* pPrevLevelOutput;
    MIRenderBackTexture* pPrevLevelOutput1;
};

class MORTY_CLASS MIPostProcessWork : public MObject
{
public:
	M_OBJECT(MIPostProcessWork);
    MIPostProcessWork();
    virtual ~MIPostProcessWork();

public:

    virtual void Initialize(MIRenderProgram* pRenderProgram) {};
    virtual void Release() {};

    virtual void CheckRenderTargetSize(const Vector2& v2Size) {}

    virtual void Render(MPostProcessRenderInfo& info) {}

    virtual MTextureRenderTarget* GetRenderTarget() { return nullptr; }


	virtual void OnDelete();

private:
};

#endif

/**
 * @File         MBasicRenderSystem
 * 
 * @Created      2020-04-19 16:05:49
 *
 * @Author       Pobrecito
**/

#ifndef _M_MBASICRENDERSYSTEM_H_
#define _M_MBASICRENDERSYSTEM_H_
#include "MGlobal.h"
#include "MMesh.h"
#include "Vector.h"
#include "MBounds.h"
#include "MISystem.h"
#include "MMaterialGroup.h"
#include "MShadowTextureRenderTarget.h"

#include <vector>

class MScene;
class MCamera;
class MIMeshInstance;
class MModelInstance;
class MSkeletonInstance;
class MDirectionalLight;
class MIModelMeshInstance;
class MORTY_CLASS MBasicRenderSystem : public MISystem
{
public:
	M_OBJECT(MBasicRenderSystem);
    MBasicRenderSystem();
    virtual ~MBasicRenderSystem();

public:

    virtual void Tick(const float& fDelta) override;
    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport, MScene* pScene, MIRenderTarget* pRenderTarget) override;

    virtual void OnCreated() override;
    virtual void OnDelete() override;

protected:
    MMesh<Vector2> m_DrawMesh;
    MMaterial* m_pDrawMaterial;
};

#endif

/**
 * @File         MDebugRenderComponent
 * 
 * @Created      2022-07-30 21:54:49
 *
 * @Author       Pobrecito
**/

#ifndef _M_MDEBUGRENDERCOMPONENT_H_
#define _M_MDEBUGRENDERCOMPONENT_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Math/Vector.h"
#include "Render/MMesh.h"

#include "Component/MComponent.h"

struct MORTY_API MDebugDrawLine
{
public:

    Vector3 m_v3StartPosition;
    Vector3 m_v3EndPosition;
};

struct MDebugDrawLineVertex
{
    Vector3 v3Position;
};

class MORTY_API MDebugRenderComponent : public MComponent
{
public:
	MORTY_CLASS(MDebugRenderComponent);
    MDebugRenderComponent();
    virtual ~MDebugRenderComponent();

    MMesh<MDebugDrawLineVertex>& GetDrawLineMesh();

public:

    void AddLine(const MDebugDrawLine& drawLine);

private:


    std::vector<MDebugDrawLine> m_vDrawLine;

    bool m_bDrawLineDirty;
    MMesh<MDebugDrawLineVertex> m_cDrawLineMesh;

};

#endif

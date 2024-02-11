/**
 * @File         MDebugRenderComponent
 * 
 * @Created      2022-07-30 21:54:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Math/Vector.h"
#include "Render/MMesh.h"

#include "Component/MComponent.h"

MORTY_SPACE_BEGIN

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

MORTY_SPACE_END
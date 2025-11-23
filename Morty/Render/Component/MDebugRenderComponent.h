/**
 * @File         MDebugRenderComponent
 * 
 * @Created      2022-07-30 21:54:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Mesh/MMesh.h"
#include "Object/MObject.h"

#include "Component/MComponent.h"

namespace morty
{

struct MORTY_API MDebugDrawLine {
public:
    Vector3 m_startPosition;
    Vector3 m_endPosition;
};

struct MDebugDrawLineVertex {
    Vector3                   v3Position;

    static size_t             AttributeProtectMask() { return 0; }
    static std::vector<float> SimplifyWeight() { return {}; }
};

class MORTY_API MDebugRenderComponent : public MComponent
{
public:
                                 MORTY_CLASS(MDebugRenderComponent);

                                 MDebugRenderComponent();

    virtual ~                    MDebugRenderComponent();

    MMesh<MDebugDrawLineVertex>& GetDrawLineMesh();

public:
    void AddLine(const MDebugDrawLine& drawLine);

private:
    std::vector<MDebugDrawLine> m_drawLine;

    bool                        m_drawLineDirty;
    MMesh<MDebugDrawLineVertex> m_drawLineMesh;
};

}// namespace morty
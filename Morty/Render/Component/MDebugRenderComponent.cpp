#include "Component/MDebugRenderComponent.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDebugRenderComponent, MComponent)

MDebugRenderComponent::MDebugRenderComponent()
    : MComponent()
    , m_drawLineDirty(false)
{}

MDebugRenderComponent::~MDebugRenderComponent() {}

void MDebugRenderComponent::AddLine(const MDebugDrawLine& drawLine)
{
    m_drawLine.push_back(drawLine);
    m_drawLineDirty = true;
}

MMesh<MDebugDrawLineVertex>& MDebugRenderComponent::GetDrawLineMesh()
{
    if (m_drawLineDirty)
    {
        m_drawLineMesh.CreateVertices(static_cast<uint32_t>(m_drawLine.size()) * 2);
        m_drawLineMesh.CreateIndices(static_cast<uint32_t>(m_drawLine.size()) * 2, 1);

        for (uint32_t nLineIdx = 0; nLineIdx < static_cast<uint32_t>(m_drawLine.size()); ++nLineIdx)
        {
            m_drawLineMesh.GetVertices()[nLineIdx * 2 + 0].v3Position = m_drawLine[nLineIdx].m_startPosition;
            m_drawLineMesh.GetVertices()[nLineIdx * 2 + 1].v3Position = m_drawLine[nLineIdx].m_endPosition;

            m_drawLineMesh.GetIndices()[nLineIdx * 2 + 0] = nLineIdx * 2 + 0;
            m_drawLineMesh.GetIndices()[nLineIdx * 2 + 1] = nLineIdx * 2 + 1;
        }
    }

    return m_drawLineMesh;
}

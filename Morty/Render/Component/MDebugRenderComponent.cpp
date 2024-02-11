#include "Component/MDebugRenderComponent.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDebugRenderComponent, MComponent)

MDebugRenderComponent::MDebugRenderComponent()
    : MComponent()
    , m_bDrawLineDirty(false)
{
}

MDebugRenderComponent::~MDebugRenderComponent()
{
}

void MDebugRenderComponent::AddLine(const MDebugDrawLine& drawLine)
{
    m_vDrawLine.push_back(drawLine);
    m_bDrawLineDirty = true;
}

MMesh<MDebugDrawLineVertex>& MDebugRenderComponent::GetDrawLineMesh()
{
    if (m_bDrawLineDirty)
    {
        m_cDrawLineMesh.CreateVertices(static_cast<uint32_t>(m_vDrawLine.size()) * 2);
        m_cDrawLineMesh.CreateIndices(static_cast<uint32_t>(m_vDrawLine.size()) * 2, 1);

        for (uint32_t nLineIdx = 0; nLineIdx < static_cast<uint32_t>(m_vDrawLine.size()); ++nLineIdx)
        {
            m_cDrawLineMesh.GetVertices()[nLineIdx * 2 + 0].v3Position = m_vDrawLine[nLineIdx].m_v3StartPosition;
            m_cDrawLineMesh.GetVertices()[nLineIdx * 2 + 1].v3Position = m_vDrawLine[nLineIdx].m_v3EndPosition;

            m_cDrawLineMesh.GetIndices()[nLineIdx * 2 + 0] = nLineIdx * 2 + 0;
            m_cDrawLineMesh.GetIndices()[nLineIdx * 2 + 1] = nLineIdx * 2 + 1;
        }
    }

    return m_cDrawLineMesh;
}

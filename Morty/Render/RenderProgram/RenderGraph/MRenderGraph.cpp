#include "MRenderGraph.h"

#include "MRenderTargetManager.h"
#include "Engine/MEngine.h"
#include "System/MObjectSystem.h"


MORTY_CLASS_IMPLEMENT(MRenderGraph, MTaskGraph)

MRenderGraph::MRenderGraph(MEngine* pEngine)
    : Super()
{
    m_pEngine = pEngine;

    auto pObjectSystem = m_pEngine->FindSystem<MObjectSystem>();

    m_pRenderTargetManager = pObjectSystem->CreateObject<MRenderTargetManager>();
}

MRenderGraph::~MRenderGraph()
{
    m_pRenderTargetManager->DeleteLater();
    m_pRenderTargetManager = nullptr;
}

MRenderTaskTarget* MRenderGraph::FindRenderTaskTarget(const MStringId& name)
{
    return m_pRenderTargetManager->FindRenderTarget(name);
}

void MRenderGraph::Resize(const Vector2i& size)
{
    if (m_n2Size == size)
    {
        return;
    }

    m_n2Size = size;

    m_pRenderTargetManager->ResizeRenderTarget(size);

    for (auto pTaskNode : GetOrderedNodes())
    {
        pTaskNode->DynamicCast<MRenderTaskNode>()->Resize(size);
    }
}

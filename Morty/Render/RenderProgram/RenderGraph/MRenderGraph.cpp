#include "MRenderGraph.h"

#include "Engine/MEngine.h"
#include "MRenderGraphSetting.h"
#include "MRenderTargetManager.h"
#include "System/MObjectSystem.h"
#include "Utility/MFunction.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderGraph, MTaskGraph)

MRenderGraph::MRenderGraph(MEngine* pEngine)
    : Super()
{
    m_engine = pEngine;

    auto pObjectSystem = m_engine->FindSystem<MObjectSystem>();

    m_renderTargetManager = pObjectSystem->CreateObject<MRenderTargetManager>();

    m_renderGraphSetting = std::make_shared<MRenderGraphSetting>();
}

MRenderGraph::~MRenderGraph()
{
    m_renderTargetManager->DeleteLater();
    m_renderTargetManager = nullptr;
    m_renderGraphSetting  = nullptr;
}

MRenderTaskTarget* MRenderGraph::FindRenderTaskTarget(const MStringId& name)
{
    return m_renderTargetManager->FindRenderTarget(name);
}

void MRenderGraph::Resize(const Vector2i& size)
{
    if (m_size == size) { return; }

    m_size = size;

    m_renderTargetManager->ResizeRenderTarget(size);

    for (auto pTaskNode: GetOrderedNodes()) { pTaskNode->DynamicCast<MRenderTaskNode>()->Resize(size); }
}

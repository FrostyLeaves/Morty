#include "MRenderGraph.h"

#include "Engine/MEngine.h"
#include "MRenderGraphSetting.h"
#include "System/MObjectSystem.h"
#include "Utility/MFunction.h"
#include "MRenderGraph_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderGraph, MTaskGraph)

MRenderGraph::MRenderGraph(MEngine* pEngine)
    : Super()
{
    m_engine             = pEngine;
    m_renderGraphSetting = std::make_shared<MRenderGraphSetting>();
}

MRenderGraph::~MRenderGraph() { m_renderGraphSetting = nullptr; }

void MRenderGraph::Resize(const Vector2i& size)
{
    if (m_size == size) { return; }

    m_size = size;

    for (auto pTaskNode: GetOrderedNodes()) { pTaskNode->DynamicCast<MRenderTaskNode>()->Resize(size); }
}

MRenderTaskNode* MRenderGraph::FindRenderNode(size_t renderNodeId) const
{
    return FindTaskNode(renderNodeId)->DynamicCast<MRenderTaskNode>();
}

bool MRenderGraph::AddNode(const MStringId& name, MTaskNode* pRenderNode)
{
    const auto findResult = m_taskNodeTable.find(name);
    if (findResult != m_taskNodeTable.end()) { return false; }

    Super::AddNode(name, pRenderNode);

    m_taskNodeTable[pRenderNode->GetNodeName()] = pRenderNode->DynamicCast<MRenderTaskNode>();

    return true;
}
flatbuffers::Offset<void> MRenderGraph::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto                     fbSuper = MTaskGraph::Serialize(fbb);
    fbs::MRenderGraphBuilder builder(fbb);
    builder.add_super(fbSuper.o);

    return builder.Finish().Union();
}

void MRenderGraph::Deserialize(const void* pBufferPointer)
{
    const fbs::MRenderGraph* fbRenderGraph = reinterpret_cast<const fbs::MRenderGraph*>(pBufferPointer);
    auto                     fbSuper       = fbRenderGraph->super();

    MTaskGraph::Deserialize(fbSuper);
}

void MRenderGraph::OnPreCompile() {}

void MRenderGraph::OnPostCompile()
{
    m_renderTargetBinding = std::make_unique<MRenderTargetBindingWalker>(GetEngine());
    m_renderTargetBinding->SetForceExclusive(true);

    //Bind Render Target
    (*m_renderTargetBinding)(this);
}
#include "TaskGraphView.h"

#include "Engine/MEngine.h"
#include "GraphEditor.h"

#include "Scene/MEntity.h"
#include "TaskGraph/MTaskGraph.h"

#include "Render/ImGui/imnodes.h"

using namespace morty;

TaskGraphView::TaskGraphView(const MString& viewName)
    : BaseWidget()
{
    m_strViewName = viewName;
}

int GetDepthTable(MTaskGraph* pTaskGraph, std::map<MTaskNode*, int>& output)
{
    int  nMaxDepth = 0;
    auto vNodes    = pTaskGraph->GetFinalNodes();

    for (MTaskNode* node: vNodes) { output[node] = 0; }

    while (!vNodes.empty())
    {
        MTaskNode* node = vNodes.back();
        vNodes.pop_back();

        for (size_t nInputIdx = 0; nInputIdx < node->GetInputSize(); ++nInputIdx)
        {
            MTaskNode* pPrevNode = node->GetInput(nInputIdx)->GetLinkedNode();

            if (output.find(pPrevNode) == output.end()) { output[pPrevNode] = output[node] + 1; }
            else if (output[pPrevNode] < output[node] + 1) { output[pPrevNode] = output[node] + 1; }

            vNodes.push_back(pPrevNode);

            if (nMaxDepth < output[pPrevNode]) { nMaxDepth = output[pPrevNode]; }
        }
    }

    return nMaxDepth;
}

void TaskGraphView::Render()
{
    if (!m_taskGraph) { return; }

    std::map<MTaskNode*, int> tDepthTable;
    const int                 nMaxDepth = GetDepthTable(m_taskGraph, tDepthTable);
    std::vector<int>          vTaskColumn(nMaxDepth + 1, 0);

    auto                      vAllNodes = m_taskGraph->GetAllNodes();

    ImNodes::BeginNodeEditor();

    for (auto& node: vAllNodes)
    {
        const int imNodeId = static_cast<int>(reinterpret_cast<std::intptr_t>(node));
        (ImNodes::BeginNode(imNodeId));
        {
            //first initialize position.
            ImVec2 size = ImNodes::GetNodeDimensions(imNodeId);
            if (size.x + size.y <= MGlobal::M_FLOAT_BIAS)
            {
                ImVec2 initialPosition;
                initialPosition.x = -tDepthTable[node] * 300;
                initialPosition.y = vTaskColumn[tDepthTable[node]] * 200;
                ImNodes::SetNodeEditorSpacePos(imNodeId, initialPosition);

                vTaskColumn[tDepthTable[node]]++;
            }

            //title
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(node->GetNodeName().ToString().c_str());
            ImNodes::EndNodeTitleBar();

            //input
            for (size_t nIdx = 0; nIdx < node->GetInputSize(); ++nIdx)
            {
                auto pNodeInput = node->GetInput(nIdx);
                ImNodes::BeginInputAttribute(static_cast<int>(reinterpret_cast<std::intptr_t>(pNodeInput)));
                ImGui::Text("%s", pNodeInput->GetName().c_str());
                ImNodes::EndInputAttribute();
            }

#if MORTY_DEBUG
            //Context
            ImGui::Text("avg time: %f", (static_cast<float>(node->m_debugTime) / 1000.0f));
#endif
            //output
            for (size_t nIdx = 0; nIdx < node->GetOutputSize(); ++nIdx)
            {
                auto pNodeOutput = node->GetOutput(nIdx);
                ImNodes::BeginOutputAttribute(static_cast<int>(reinterpret_cast<std::intptr_t>(pNodeOutput)));
                ImGui::Text("%s", pNodeOutput->GetName().ToString().c_str());
                ImNodes::EndOutputAttribute();
            }
        }
        ImNodes::EndNode();
    }

    //link
    for (auto& node: vAllNodes)
    {
        for (size_t nInputIdx = 0; nInputIdx < node->GetInputSize(); ++nInputIdx)
        {
            auto input  = node->GetInput(nInputIdx);
            auto output = input->GetLinkedOutput();
            if (!output) { continue; }

            const int inputId  = static_cast<int>(reinterpret_cast<std::intptr_t>(input));
            const int outputId = static_cast<int>(reinterpret_cast<std::intptr_t>(output));

            ImNodes::Link(inputId + outputId, outputId, inputId);
        }
    }


    ImNodes::EndNodeEditor();
}

void TaskGraphView::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void TaskGraphView::Release() {}

void TaskGraphView::SetTaskGraph(MTaskGraph* pGraph) { m_taskGraph = pGraph; }

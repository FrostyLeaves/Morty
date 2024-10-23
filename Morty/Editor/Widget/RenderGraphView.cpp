#include "RenderGraphView.h"

#include "Engine/MEngine.h"
#include "GraphEditor.h"
#include "ImGuiFileDialog.h"
#include "Property/PropertyBase.h"
#include "Render/ImGui/imnodes.h"
#include "Render/MIRenderProgram.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Render/RenderGraph/MRenderGraphSetting.h"
#include "Render/RenderGraph/MRenderTaskNodeInput.h"
#include "Render/RenderGraph/MRenderTaskNodeOutput.h"
#include "Render/RenderNode/MDebugRenderNode.h"
#include "RenderGraph/EditRenderTaskNodeBase.h"
#include "RenderGraph/MRenderGraphNodeList.h"
#include "Scene/MEntity.h"

using namespace morty;

const int NodeBit       = 8;
const int InputSlotBit  = 4;
const int OutputSlotBit = 0;

int       GetInputSlotId(MTaskNodeInput* pInput)
{
    return static_cast<int>((pInput->GetTaskNode()->GetNodeID() << NodeBit) | (pInput->GetIndex() << InputSlotBit));
}

int GetOutputSlotId(MTaskNodeOutput* pOutput)
{
    return static_cast<int>(
            (pOutput->GetTaskNode()->GetNodeID() << NodeBit) | (pOutput->GetIndex() << OutputSlotBit) + 1
    );
}

RenderGraphView::RenderGraphView(const MString& viewName)
    : BaseWidget()
{
    m_strViewName = viewName;
}

int RenderGraphView::GetDepthTable(MRenderGraph* pTaskGraph, std::map<MTaskNode*, int>& output)
{
    int  nMaxDepth = 0;
    auto vNodes    = pTaskGraph->GetFinalNodes();

    for (MTaskNode* pNode: vNodes) { output[pNode] = 0; }

    while (!vNodes.empty())
    {
        MTaskNode* pNode = vNodes.back();
        vNodes.pop_back();

        if (!pNode) continue;

        for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
        {
            MTaskNode* pPrevNode = pNode->GetInput(nInputIdx)->GetLinkedNode();

            if (output.find(pPrevNode) == output.end()) { output[pPrevNode] = output[pNode] + 1; }
            else if (output[pPrevNode] < output[pNode] + 1) { output[pPrevNode] = output[pNode] + 1; }

            vNodes.push_back(pPrevNode);

            if (nMaxDepth < output[pPrevNode]) { nMaxDepth = output[pPrevNode]; }
        }
    }

    return nMaxDepth;
}

void RenderGraphView::DrawProperty()
{
    auto pRenderGraph = m_renderProgram->GetRenderGraph();
    auto vAllNodes    = pRenderGraph->GetAllNodes();

    ImGui::Columns(2);
    ImGui::Separator();
    for (auto node: vAllNodes)
    {
        auto renderNode = node->DynamicCast<MRenderTaskNode>();
        if (ImNodes::IsNodeSelected(static_cast<int>(node->GetNodeID())))
        {
            if (m_editNodeTable[renderNode->GetNodeID()] == nullptr)
            {
                const auto nodeTypeName = MStringId(renderNode->GetTypeName());
                auto       findFactory  = MRenderGraphNodeList::EditFactory.find(nodeTypeName);
                if (findFactory != MRenderGraphNodeList::EditFactory.end())
                {
                    m_editNodeTable[renderNode->GetNodeID()] = findFactory->second();
                }
            }

            if (m_editNodeTable[renderNode->GetNodeID()])
            {

                m_editNodeTable[renderNode->GetNodeID()]->EditRenderTaskNode(renderNode);
            }
        }
    }
    ImGui::Separator();
    ImGui::Columns(1);
}

void RenderGraphView::DrawGraphView()
{
    auto                      pRenderGraph = m_renderProgram->GetRenderGraph();
    std::map<MTaskNode*, int> tDepthTable;
    const int                 nMaxDepth = GetDepthTable(pRenderGraph, tDepthTable);
    std::vector<int>          vTaskColumn(nMaxDepth + 1, 0);
    auto                      vAllNodes = pRenderGraph->GetAllNodes();


    ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
    ImNodes::BeginNodeEditor();

    //Warning: use int32 to restore all node | slot | conn, it can only record 8 bit info for node id.
    MORTY_ASSERT(vAllNodes.size() < 256);

    for (auto& pNode: vAllNodes)
    {
        const int imNodeId = static_cast<int>(pNode->GetNodeID());
        (ImNodes::BeginNode(imNodeId));
        {
            //first initialize position.
            ImVec2 size = ImNodes::GetNodeDimensions(imNodeId);
            if (size.x + size.y <= MGlobal::M_FLOAT_BIAS)
            {
                //ImVec2 initialPosition;
                //initialPosition.x = -tDepthTable[pNode] * 300;
                //initialPosition.y = vTaskColumn[tDepthTable[pNode]] * 200;
                //ImNodes::SetNodeEditorSpacePos(imNodeId, initialPosition);

                vTaskColumn[tDepthTable[pNode]]++;
            }

            //title
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(pNode->GetNodeName().ToString().c_str());
            ImNodes::EndNodeTitleBar();

            //input
            for (size_t nIdx = 0; nIdx < pNode->GetInputSize(); ++nIdx)
            {
                auto pNodeInput = pNode->GetInput(nIdx);
                ImNodes::BeginInputAttribute(GetInputSlotId(pNodeInput));
                ImGui::Text("%s", pNodeInput->GetName().c_str());
                ImNodes::EndInputAttribute();
            }

            auto         property = pRenderGraph->GetRenderGraphSetting()->GetPropertyVariant(pNode->GetNodeName());
            PropertyBase prop;
            prop.EditMVariant(pNode->GetNodeName().ToString(), property);

            //output
            for (size_t nIdx = 0; nIdx < pNode->GetOutputSize(); ++nIdx)
            {
                auto pNodeOutput = pNode->GetOutput(nIdx);
                ImNodes::BeginOutputAttribute(GetOutputSlotId(pNodeOutput));
                bool check = pNode->GetNodeID() == m_finalOutputNodeId && nIdx == m_finalOutputSlotId;
                if (ImGui::Checkbox(pNodeOutput->GetName().ToString().c_str(), &check))
                {
                    m_finalOutputNodeId = pNode->GetNodeID();
                    m_finalOutputSlotId = nIdx;
                    pRenderGraph->RequireCompile();
                }

                ImNodes::EndOutputAttribute();
            }
        }
        ImNodes::EndNode();
    }

    //link
    for (auto& pNode: vAllNodes)
    {
        for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
        {
            auto pInput  = pNode->GetInput(nInputIdx);
            auto pOutput = pInput->GetLinkedOutput();
            if (!pOutput) { continue; }

            int inputId  = GetInputSlotId(pInput);
            int outputId = GetOutputSlotId(pOutput);

            ImNodes::Link((inputId << 16) | outputId, outputId, inputId);
        }
    }


    ImNodes::MiniMap();
    ImNodes::EndNodeEditor();
    ImNodes::PopAttributeFlag();

    int destroyedLinkId;
    if (ImNodes::IsLinkDestroyed(&destroyedLinkId))
    {
        size_t inputNodeId  = (destroyedLinkId >> 24) & 255;
        size_t inputSlotId  = (destroyedLinkId >> 20) & 15;
        size_t outputNodeId = (destroyedLinkId >> 8) & 255;
        size_t outputSlotId = ((destroyedLinkId >> 0) & 15) - 1;

        auto   pInputNode  = pRenderGraph->FindTaskNode(inputNodeId);
        auto   pOutputNode = pRenderGraph->FindTaskNode(outputNodeId);
        auto   pInput      = pInputNode->GetInput(inputSlotId);
        auto   pOutput     = pOutputNode->GetOutput(outputSlotId);

        pOutput->UnLink(pInput);
        pRenderGraph->RequireCompile();
    }

    int createLinkId[4];
    if (ImNodes::IsLinkCreated(createLinkId, createLinkId + 1, createLinkId + 2, createLinkId + 3))
    {
        size_t outputNodeId = createLinkId[0];
        size_t outputSlotId = (createLinkId[1] & 15) - 1;
        size_t inputNodeId  = createLinkId[2];
        size_t inputSlotId  = (createLinkId[3] >> 4) & 15;

        auto   pInputNode  = pRenderGraph->FindTaskNode(inputNodeId);
        auto   pOutputNode = pRenderGraph->FindTaskNode(outputNodeId);
        auto   pInput      = pInputNode->GetInput(inputSlotId)->DynamicCast<MRenderTaskNodeInput>();
        auto   pOutput     = pOutputNode->GetOutput(outputSlotId)->DynamicCast<MRenderTaskNodeOutput>();

        if (pInput && pOutput)
        {
            if (pOutput->LinkTo(pInput)) { pRenderGraph->RequireCompile(); }
        }
    }
}

void RenderGraphView::Render()
{
    if (!m_renderProgram) { return; }

    DrawMenu();
    ProcessDialog();

    ImVec2 WindowSize = ImGui::GetWindowSize();
    if (ImGui::BeginChild("Graph Node Property", ImVec2(0.25f * WindowSize.x, 0))) { DrawProperty(); }
    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::BeginChild("Graph View", ImVec2(0.75f * WindowSize.x, 0))) { DrawGraphView(); }
    ImGui::EndChild();
}

void RenderGraphView::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void RenderGraphView::Release() {}

void RenderGraphView::SetRenderProgram(MIRenderProgram* pRenderProgram)
{
    m_renderProgram = pRenderProgram;

    for (const auto& node: m_renderProgram->GetRenderGraph()->GetAllNodes())
    {
        if (node->DynamicCast<MDebugRenderNode>()) { m_finalOutputNodeId = node->GetNodeID(); }
    }
}

void RenderGraphView::DrawMenu()
{
    auto pRenderGraph = m_renderProgram->GetRenderGraph();

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Render Graph File", ""))
            {
                ImGuiFileDialog::Instance()
                        ->OpenModal(m_graphOpenDialogId, "Open Render Graph", m_graphFileSuffix.c_str(), ".");
            }

            if (ImGui::MenuItem("Save Render Graph File To", ""))
            {
                ImGuiFileDialog::Instance()
                        ->OpenModal(m_graphSaveDialogId, "Save Render Graph", m_graphFileSuffix.c_str(), ".");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Node"))
        {
            for (const auto& Name: MRenderGraphNodeList::Names)
            {
                if (ImGui::MenuItem(Name.c_str(), ""))
                {
                    auto    renderNode = MTypeClass::New(Name.c_str())->DynamicCast<MRenderTaskNode>();

                    size_t  nameIdx           = 0;
                    MString newRenderNodeName = renderNode->GetTypeName();
                    while (!pRenderGraph->AddNode(MStringId(newRenderNodeName), renderNode))
                    {
                        newRenderNodeName = renderNode->GetTypeName() + MStringUtil::ToString(++nameIdx);
                    }
                }
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void RenderGraphView::ProcessDialog()
{
    if (ImGuiFileDialog::Instance()->Display(m_graphOpenDialogId))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string        filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::vector<MByte> buffer;
            if (MFileHelper::ReadData(filePathName, buffer)) { LoadGraph(buffer); }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display(m_graphSaveDialogId))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            m_saveToFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();

            SaveGraph(m_saveBuffer);

            ImGui::OpenPopup(m_graphSaveResultId.c_str());
        }
        ImGuiFileDialog::Instance()->Close();
    }


    ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Always,
            ImVec2(0.5f, 0.5f)
    );
    if (ImGui::BeginPopupModal(m_graphSaveResultId.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        bool result = MFileHelper::WriteData(m_saveToFilePathName, m_saveBuffer);
        ImGui::Text(result ? "Save success." : "Save failed.");
        ImGui::Separator();

        if (ImGui::Button("OK")) { ImGui::CloseCurrentPopup(); }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}

void RenderGraphView::LoadGraph(const std::vector<MByte>& buffer) { m_renderProgram->LoadGraph(buffer); }

void RenderGraphView::SaveGraph(std::vector<MByte>& buffer) { m_renderProgram->SaveGraph(buffer); }

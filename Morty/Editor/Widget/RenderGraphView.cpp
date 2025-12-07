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

int       GetInputSlotId(MTaskNodeInput* input)
{
    return static_cast<int>((input->GetTaskNode()->GetNodeID() << NodeBit) | (input->GetIndex() << InputSlotBit));
}

int GetOutputSlotId(MTaskNodeOutput* output)
{
    return static_cast<int>(
            (output->GetTaskNode()->GetNodeID() << NodeBit) | (output->GetIndex() << OutputSlotBit) + 1
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

    for (MTaskNode* node: vNodes) { output[node] = 0; }

    while (!vNodes.empty())
    {
        MTaskNode* node = vNodes.back();
        vNodes.pop_back();

        if (!node) continue;

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

                m_editNodeTable[renderNode->GetNodeID()]->EditRenderTaskNode(GetEngine(), renderNode);
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

    for (auto& node: vAllNodes)
    {
        const int imNodeId = static_cast<int>(node->GetNodeID());
        (ImNodes::BeginNode(imNodeId));
        {
            //first initialize position.
            ImVec2 size = ImNodes::GetNodeDimensions(imNodeId);
            if (size.x + size.y <= MGlobal::M_FLOAT_BIAS)
            {
                //ImVec2 initialPosition;
                //initialPosition.x = -tDepthTable[node] * 300;
                //initialPosition.y = vTaskColumn[tDepthTable[node]] * 200;
                //ImNodes::SetNodeEditorSpacePos(imNodeId, initialPosition);

                vTaskColumn[tDepthTable[node]]++;
            }

            //title
            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(node->GetNodeName().ToString().c_str());
            ImNodes::EndNodeTitleBar();

            float nodeWidth = GetNodeWidth(static_cast<MRenderTaskNode*>(node));

            //input
            for (size_t nIdx = 0; nIdx < node->GetInputSize(); ++nIdx)
            {
                auto pNodeInput = static_cast<MRenderTaskNodeInput*>(node->GetInput(nIdx));
                SetupLinkStyle(pNodeInput);
                ImNodes::BeginInputAttribute(GetInputSlotId(pNodeInput));
                ImGui::Text("%s", pNodeInput->GetName().c_str());
                ImNodes::EndInputAttribute();
                ResetLinkStyle();
            }

            auto         property = pRenderGraph->GetRenderGraphSetting()->GetPropertyVariant(node->GetNodeName());
            PropertyBase prop;
            prop.EditMVariant(node->GetNodeName().ToString(), property);

            //output
            for (size_t nIdx = 0; nIdx < node->GetOutputSize(); ++nIdx)
            {
                auto pNodeOutput = static_cast<MRenderTaskNodeOutput*>(node->GetOutput(nIdx));
                SetupLinkStyle(pNodeOutput);
                ImNodes::BeginOutputAttribute(GetOutputSlotId(pNodeOutput));
                bool check = node->GetNodeID() == pRenderGraph->GetFinalOutputNodeIdx() &&
                             nIdx == pRenderGraph->GetFinalOutputSlotIdx();

                ImGui::SetCursorPosX(
                        ImGui::GetCursorPosX() + nodeWidth - ImGui::CalcTextSize(pNodeOutput->GetName().c_str()).x - 4
                );
                ImGui::TextUnformatted(pNodeOutput->GetName().ToString().c_str());
                ImGui::SameLine(nodeWidth);
                if (ImGui::Checkbox("", &check))
                {
                    pRenderGraph->SetFinalOutput(node->GetNodeID(), nIdx);
                    pRenderGraph->RequireCompile();
                }

                ImNodes::EndOutputAttribute();
                ResetLinkStyle();
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

            int inputId  = GetInputSlotId(input);
            int outputId = GetOutputSlotId(output);

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
        auto   input       = pInputNode->GetInput(inputSlotId);
        auto   output      = pOutputNode->GetOutput(outputSlotId);

        output->UnLink(input);
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
        auto   input       = pInputNode->GetInput(inputSlotId)->DynamicCast<MRenderTaskNodeInput>();
        auto   output      = pOutputNode->GetOutput(outputSlotId)->DynamicCast<MRenderTaskNodeOutput>();

        if (input && output && !pRenderGraph->CheckCycle(pOutputNode, pInputNode))
        {
            if (output->LinkTo(input)) { pRenderGraph->RequireCompile(); }
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

void RenderGraphView::SetRenderProgram(MIRenderProgram* pRenderProgram) { m_renderProgram = pRenderProgram; }

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
                    auto   renderNode = MTypeClass::New(Name)->DynamicCast<MRenderTaskNode>();

                    size_t nameIdx           = 0;
                    auto   newRenderNodeName = renderNode->GetTypeName();
                    while (!pRenderGraph->AddNode(newRenderNodeName, renderNode))
                    {
                        newRenderNodeName =
                                MStringId(renderNode->GetTypeName().ToString() + MStringUtil::ToString(++nameIdx));
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

void    RenderGraphView::LoadGraph(const std::vector<MByte>& buffer) { m_renderProgram->LoadGraph(buffer); }

void    RenderGraphView::SaveGraph(std::vector<MByte>& buffer) { m_renderProgram->SaveGraph(buffer); }

ImColor GetColorFromTextureFormat(METextureFormat format)
{

    static ImColor                                      AllowEmptyColor  = ImColor(230, 155, 3);
    static ImColor                                      DefaultLinkColor = ImColor(225, 238, 210);

    static std::unordered_map<METextureFormat, ImColor> ColorTable = {
            {METextureFormat ::Unknow, DefaultLinkColor},
            {METextureFormat ::Depth, ImColor(255, 227, 132)},
            {METextureFormat ::UNorm_R8, ImColor(240, 128, 128)},
            {METextureFormat ::UNorm_RGBA8, ImColor(152, 251, 152)},
            {METextureFormat ::Float_R32, ImColor(220, 20, 60)},
            {METextureFormat ::Float_RGBA16, ImColor(124, 252, 0)},
            {METextureFormat ::Float_RGBA32, ImColor(124, 252, 0)},
            {METextureFormat ::SRGB_R8, ImColor(208, 32, 144)},
            {METextureFormat ::SRGB_R8G8B8A8, ImColor(50, 205, 50)},
    };

    return FIND_OR_DEFAULT(ColorTable, format, DefaultLinkColor);
}

void RenderGraphView::SetupLinkStyle(MRenderTaskNodeInput* input)
{
    ImNodes::PushColorStyle(ImNodesCol_Pin, GetColorFromTextureFormat(input->GetFormat()));
}

void RenderGraphView::SetupLinkStyle(MRenderTaskNodeOutput* output)
{
    ImNodes::PushColorStyle(ImNodesCol_Pin, GetColorFromTextureFormat(output->GetFormat()));
}

void  RenderGraphView::ResetLinkStyle() { ImNodes::PopColorStyle(); }

float RenderGraphView::GetNodeWidth(MRenderTaskNode* node)
{
    const float emptyWidth = 30;

    float       width = ImGui::CalcTextSize(node->GetNodeName().c_str()).x;
    for (size_t nIdx = 0; nIdx < node->GetInputSize(); ++nIdx)
    {
        auto pNodeInput = static_cast<MRenderTaskNodeInput*>(node->GetInput(nIdx));
        width           = std::max(width, ImGui::CalcTextSize(pNodeInput->GetName().c_str()).x);
    }

    //output
    for (size_t nIdx = 0; nIdx < node->GetOutputSize(); ++nIdx)
    {
        auto pNodeOutput = static_cast<MRenderTaskNodeOutput*>(node->GetOutput(nIdx));
        width            = std::max(width, ImGui::CalcTextSize(pNodeOutput->GetName().c_str()).x);
    }

    return width + emptyWidth;
}

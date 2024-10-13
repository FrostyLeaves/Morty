#include "RenderGraphView.h"

#include "Engine/MEngine.h"
#include "GraphEditor.h"
#include "ImGuiFileDialog.h"
#include "Property/PropertyBase.h"
#include "Render/ImGui/imnodes.h"
#include "Render/MIRenderProgram.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Render/RenderGraph/MRenderGraphSetting.h"
#include "Render/RenderNode/MDebugRenderNode.h"
#include "Scene/MEntity.h"

using namespace morty;

RenderGraphView::RenderGraphView(const MString& viewName)
    : BaseWidget()
{
    m_strViewName       = viewName;
    m_finalOutputNodeId = MStringId(MDebugRenderNode::GetClassTypeName());
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

void RenderGraphView::Render()
{
    if (!m_renderProgram) { return; }

    DrawMenu();
    ProcessDialog();

    auto                      pRenderGraph = m_renderProgram->GetRenderGraph();
    std::map<MTaskNode*, int> tDepthTable;
    const int                 nMaxDepth = GetDepthTable(pRenderGraph, tDepthTable);
    std::vector<int>          vTaskColumn(nMaxDepth + 1, 0);

    auto                      vAllNodes = pRenderGraph->GetAllNodes();

    ImNodes::BeginNodeEditor();

    for (auto& pNode: vAllNodes)
    {
        const int imNodeId = static_cast<int>(reinterpret_cast<std::intptr_t>(pNode));
        (ImNodes::BeginNode(imNodeId));
        {
            //first initialize position.
            ImVec2 size = ImNodes::GetNodeDimensions(imNodeId);
            if (size.x + size.y <= MGlobal::M_FLOAT_BIAS)
            {
                ImVec2 initialPosition;
                initialPosition.x = -tDepthTable[pNode] * 300;
                initialPosition.y = vTaskColumn[tDepthTable[pNode]] * 200;
                ImNodes::SetNodeEditorSpacePos(imNodeId, initialPosition);

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
                ImNodes::BeginInputAttribute(static_cast<int>(reinterpret_cast<std::intptr_t>(pNodeInput)));
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
                ImNodes::BeginOutputAttribute(static_cast<int>(reinterpret_cast<std::intptr_t>(pNodeOutput)));
                bool check = pNode->GetNodeName() == m_finalOutputNodeId && nIdx == m_finalOutputSlotId;
                if (ImGui::Checkbox(pNodeOutput->GetName().ToString().c_str(), &check))
                {
                    m_finalOutputNodeId = pNode->GetNodeName();
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

            const int inputId  = static_cast<int>(reinterpret_cast<std::intptr_t>(pInput));
            const int outputId = static_cast<int>(reinterpret_cast<std::intptr_t>(pOutput));

            ImNodes::Link(inputId + outputId, outputId, inputId);
        }
    }


    ImNodes::EndNodeEditor();
}

void RenderGraphView::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void RenderGraphView::Release() {}

void RenderGraphView::SetRenderProgram(MIRenderProgram* pRenderProgram) { m_renderProgram = pRenderProgram; }

void RenderGraphView::DrawMenu()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", ""))
            {
                ImGuiFileDialog::Instance()
                        ->OpenModal(m_graphOpenDialogId, "Open Render Graph", m_graphFileSuffix.c_str(), ".");
            }

            if (ImGui::MenuItem("Save", ""))
            {
                ImGuiFileDialog::Instance()
                        ->OpenModal(m_graphSaveDialogId, "Save Render Graph", m_graphFileSuffix.c_str(), ".");
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

MTexturePtr RenderGraphView::GetFinalOutput()
{
    auto finalTaskNode = m_renderProgram->GetRenderGraph()->FindTaskNode(m_finalOutputNodeId);
    if (!finalTaskNode) { return nullptr; }
    auto finalOutput = finalTaskNode->GetOutput(m_finalOutputSlotId);
    if (!finalOutput) { return nullptr; }
    auto finalRenderOutput = finalOutput->DynamicCast<MRenderTaskNodeOutput>();
    if (!finalRenderOutput) { return nullptr; }

    return finalRenderOutput->GetTexture();
}

void RenderGraphView::LoadGraph(const std::vector<MByte>& buffer) { m_renderProgram->LoadGraph(buffer); }

void RenderGraphView::SaveGraph(std::vector<MByte>& buffer) { m_renderProgram->SaveGraph(buffer); }

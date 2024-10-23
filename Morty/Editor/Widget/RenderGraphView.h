#pragma once

#include "Utility/MRenderGlobal.h"
#include "Main/BaseWidget.h"

namespace morty
{

class MTaskNode;
class MRenderGraph;
class MIRenderProgram;
class MRenderTaskNode;
class EditRenderTaskNodeBase;
class RenderGraphView : public BaseWidget
{

public:
    explicit RenderGraphView(const MString& viewName);
    ~RenderGraphView() override = default;

    void                 Render() override;

    void                 Initialize(MainEditor* pMainEditor) override;

    void                 Release() override;

    void                 SetRenderProgram(MIRenderProgram* pRenderProgram);

    ImGuiWindowFlags     GetWindowFlags() override { return BaseWidget::GetWindowFlags() | ImGuiWindowFlags_MenuBar; }

    [[nodiscard]] size_t GetFinalOutputNodeId() const { return m_finalOutputNodeId; }
    [[nodiscard]] size_t GetFinalOutputSlotId() const { return m_finalOutputSlotId; }


private:
    void DrawMenu();
    void DrawProperty();
    void DrawGraphView();
    void ProcessDialog();

private:
    void               LoadGraph(const std::vector<MByte>& buffer);
    void               SaveGraph(std::vector<MByte>& buffer);

    static int         GetDepthTable(MRenderGraph* pTaskGraph, std::map<MTaskNode*, int>& output);

    MIRenderProgram*   m_renderProgram = nullptr;

    MString            m_graphOpenDialogId = "Render Graph Open Dialog";
    MString            m_graphSaveDialogId = "Render Graph Save Dialog";
    MString            m_graphSaveResultId = "Render Graph Save Popup";
    MString            m_graphFileSuffix   = ".mrg\0\0";

    std::string        m_saveToFilePathName;
    std::vector<MByte> m_saveBuffer;

    size_t             m_finalOutputNodeId = 0;
    uint32_t           m_finalOutputSlotId = 0;

    std::map<size_t, EditRenderTaskNodeBase*> m_editNodeTable;
};

}// namespace morty
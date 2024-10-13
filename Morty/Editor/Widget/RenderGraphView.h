#pragma once

#include "Utility/MRenderGlobal.h"
#include "Main/BaseWidget.h"

namespace morty
{

class MTaskNode;
class MRenderGraph;
class MIRenderProgram;
class RenderGraphView : public BaseWidget
{
public:
    explicit RenderGraphView(const MString& viewName);
    ~RenderGraphView() override = default;

    void             Render() override;

    void             Initialize(MainEditor* pMainEditor) override;

    void             Release() override;

    void             SetRenderProgram(MIRenderProgram* pRenderProgram);

    ImGuiWindowFlags GetWindowFlags() override { return BaseWidget::GetWindowFlags() | ImGuiWindowFlags_MenuBar; }

    MTexturePtr      GetFinalOutput();

private:
    void DrawMenu();
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

    MStringId          m_finalOutputNodeId;
    uint32_t           m_finalOutputSlotId = 0;
};

}// namespace morty
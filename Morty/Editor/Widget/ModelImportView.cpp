#include "ModelImportView.h"

#include "Utility/MGlobal.h"
#include "ImGuiFileDialog.h"
#include "imgui.h"
#include "imgui_stdlib.h"

#include "Engine/MEngine.h"
#include "Thread/MThreadPool.h"
#include "Tools/MModelImporter.h"
#include "Utility/MFileHelper.h"

#include "Property/PropertyBase.h"

using namespace morty;

static const char* svModelFilter = ".fbx,.obj,.blend\0\0";

const char*        svImportModelID  = "Import Model File";
const char*        svOutputFolderID = "Output Model Entity Dir";

ModelImportView::ModelImportView()
    : BaseWidget()
{
    m_strViewName = "ModelImporter";
}

void ModelImportView::Render()
{
    PropertyBase prop;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::Columns(2);
    ImGui::Separator();

    prop.ShowValueBegin("Source File");
    float fWidth = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button(m_strSourcePath.c_str(), ImVec2(fWidth, 0)))
    {
        ImGuiFileDialog::Instance()->OpenModal(svImportModelID, "Import", svModelFilter, "");
    }
    prop.ShowValueEnd();

    prop.ShowValueBegin("Output Dir");
    if (ImGui::Button(m_strOutputDir.c_str(), ImVec2(fWidth, 0)))
    {
        ImGuiFileDialog::Instance()->OpenDialog(svOutputFolderID, "Choose Output Folder", nullptr, ".");
    }
    prop.ShowValueEnd();

    prop.ShowValueBegin("Import Camera");
    ImGui::Checkbox("##ImportCamera", &m_bImportCamera);
    prop.ShowValueEnd();

    prop.ShowValueBegin("Import Lights");
    ImGui::Checkbox("##ImportLights", &m_bImportLights);
    prop.ShowValueEnd();

    prop.ShowValueBegin("Enable Nanite");
    ImGui::Checkbox("##EnableNanite", &m_bEnableNanite);
    prop.ShowValueEnd();

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::PopStyleVar();


    {
        float fWidth = ImGui::GetContentRegionAvail().x;

        ImGui::BeginDisabled(m_bIsConverting);
        if (ImGui::Button("Import Model", ImVec2(fWidth, 0.0f)))
        {
            if (!m_strSourcePath.empty() && !m_strOutputDir.empty())
            {
                MModelConvertInfo info;
                info.strResourcePath = m_strSourcePath;
                info.strOutputDir    = m_strOutputDir;
                info.bImportCamera   = m_bImportCamera;
                info.bImportLights   = m_bImportLights;
                info.bEnableNanite   = m_bEnableNanite;

                m_convertQueue.push(info);
                m_bIsConverting    = true;
                m_strStatusMessage = "Importing: " + m_strSourcePath + "...";
            }
            else { m_strStatusMessage = "Error: Please fill in all required fields!"; }
        }
        ImGui::EndDisabled();

        // Status message
        if (!m_strStatusMessage.empty())
        {
            ImVec4 color = m_bIsConverting ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f)
                                           : (m_strStatusMessage.find("Error") != std::string::npos
                                                      ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f)
                                                      : ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::TextColored(color, "%s", m_strStatusMessage.c_str());
        }
    }

    if (ImGuiFileDialog::Instance()->Display(svImportModelID))
    {
        if (ImGuiFileDialog::Instance()->IsOk() == true)
        {
            std::map<std::string, std::string>&& files = ImGuiFileDialog::Instance()->GetSelection();

            for (auto pr: files)
            {
                m_strSourcePath = pr.second;
                break;
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }


    if (ImGuiFileDialog::Instance()->Display(svOutputFolderID))
    {
        if (ImGuiFileDialog::Instance()->IsOk() == true)
        {
            m_strOutputDir = ImGuiFileDialog::Instance()->GetCurrentPath();
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (!m_convertQueue.empty())
    {
        std::queue<MModelConvertInfo> convertQueue = m_convertQueue;
        m_convertQueue                             = {};

        MThreadWork work(METhreadType::ECurrentThread);
        work.funcWorkFunction = std::bind(&ModelImportView::Convert, this, convertQueue);

        GetEngine()->GetThreadPool()->AddWork(work);
    }
}

void ModelImportView::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void ModelImportView::Release() {}

void ModelImportView::Convert(std::queue<MModelConvertInfo> convertQueue)
{
    while (!convertQueue.empty())
    {
        MModelConvertInfo info = convertQueue.front();

        MModelImporter    importer(GetEngine());
        bool              success = importer.Import(info);

        if (success) { m_strStatusMessage = "Successfully imported: " + info.strResourcePath; }
        else { m_strStatusMessage = "Error: Failed to import " + info.strResourcePath; }

        convertQueue.pop();
    }

    m_bIsConverting = false;
}

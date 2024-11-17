#include "ResourceView.h"
#include "Engine/MEngine.h"
#include "ImGuiFileDialog.h"
#include "Main/MainEditor.h"
#include "MaterialView.h"
#include "Resource/MMaterialResourceData.h"
#include "Resource/MResource.h"
#include "System/MResourceSystem.h"

using namespace morty;

ResourceView::ResourceView()
    : BaseWidget()
{
    m_strViewName = "Resource";
}

enum class ResourceColumnID
{
    PATH = 0,
    TYPE,
    COUNT,
};

void ResourceView::Render()
{
    DrawMenu();
    ProcessDialog();


    auto        pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    const auto& resources   = pResourceSystem->GetAllResources();
    size_t      ITEMS_COUNT = resources.size();

    if (ImGui::BeginTable("resource table", 3 /*, ImGuiTableFlags_Sortable*/))
    {
        ImGui::TableSetupColumn(
                "Path",
                ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide,
                0.0f,
                static_cast<int>(ResourceColumnID::PATH)
        );
        ImGui::TableSetupColumn(
                "Name",
                ImGuiTableColumnFlags_WidthFixed,
                0.0f,
                static_cast<int>(ResourceColumnID::TYPE)
        );
        ImGui::TableSetupColumn(
                "Count",
                ImGuiTableColumnFlags_WidthFixed,
                0.0f,
                static_cast<int>(ResourceColumnID::COUNT)
        );

        //ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs();
        //if (sort_specs && sort_specs->SpecsDirty && ITEMS_COUNT > 1) { sort_specs->SpecsDirty = false; }

        ImGui::TableHeadersRow();

        ImGuiListClipper clipper(static_cast<int>(ITEMS_COUNT));// Also demonstrate using the clipper for large list
        while (clipper.Step())
        {
            auto iter = resources.begin();
            for (int i = 0; i < clipper.DisplayStart; ++i) { ++iter; }

            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None);
                ImGui::TableSetColumnIndex(0);

                std::shared_ptr<MResource> pResource = iter->second;

                if (ImGui::Selectable(
                            pResource->GetResourcePath().c_str(),
                            m_selectedResourceId == pResource->GetResourceID(),
                            ImGuiSelectableFlags_SpanAllColumns
                    ))
                {
                    m_selectedResourceId = pResource->GetResourceID();

                    if (auto pMaterial = MTypeClass::DynamicCast<MMaterialResource>(pResource))
                    {
                        GetMainEditor()->FindWidget<MaterialView>()->SetMaterial(pMaterial);
                    }
                }
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", pResource->GetTypeName().c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%ld", pResource.use_count());

                ++iter;
            }
        }

        ImGui::EndTable();
    }
}

void ResourceView::DrawMenu()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Create Material", ""))
            {
                MString suffixList;
                for (const MString& suffix: MMaterialResourceLoader::GetSuffixList())
                {
                    suffixList += "." + suffix + ",";
                }
                suffixList += '\0';

                ImGuiFileDialog::Instance()
                        ->OpenModal(m_createResourceDialogId, "Create Material", suffixList.c_str(), ".");

                m_createResourceType = MMaterialResource::GetClassType();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void ResourceView::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void ResourceView::Release() {}

void ResourceView::ProcessDialog()
{
    if (ImGuiFileDialog::Instance()->Display(m_createResourceDialogId))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            m_createResourcePath = ImGuiFileDialog::Instance()->GetFilePathName();

            auto resourceSystem = GetEngine()->FindSystem<MResourceSystem>();

            auto newResource = resourceSystem->CreateResource(m_createResourceType);
            resourceSystem->MoveTo(newResource, m_createResourcePath);
            resourceSystem->SaveResource(newResource);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

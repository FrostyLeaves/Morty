#include "Widget/MenuBar.h"

#include "Engine/MEngine.h"
#include "ImGuiFileDialog.h"
#include "Main/MainEditor.h"
#include "Resource/MEntityResource.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"
#include "Widget/PropertyViewManager.h"
#include "imgui.h"

using namespace morty;

MenuBar::MenuBar() { m_strViewName = "MenuBar"; }

void MenuBar::Initialize(MainEditor* pMainEditor)
{
    BaseWidget::Initialize(pMainEditor);
    m_renderInHidden = true;
}

void MenuBar::Release() {}

void MenuBar::Render()
{
    if (ImGui::BeginMainMenuBar())
    {
        RenderFileMenu();
        RenderViewMenu();
        RenderWindowMenu();
        RenderEditMenu();
        RenderToolMenu();

        ImGui::EndMainMenuBar();
    }

    RenderFileDialogs();
}

void MenuBar::RenderFileMenu()
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Open", ""))
        {
            ImGuiFileDialog::Instance()->OpenModal("OpenFile", "Open", ".entity\0\0", ".");
        }

        if (ImGui::MenuItem("Save", "")) {}

        if (ImGui::MenuItem("Save as", ""))
        {
            ImGuiFileDialog::Instance()->OpenModal("Save As", "Open", ".entity\0\0", "new");
        }

        ImGui::EndMenu();
    }
}

void MenuBar::RenderViewMenu()
{
    if (ImGui::BeginMenu("View"))
    {
        for (BaseWidget* pView: m_children)
        {
            bool bVisible = pView->GetVisible();
            if (ImGui::MenuItem(pView->GetName().c_str(), "", &bVisible)) {}
            pView->SetVisible(bVisible);
        }

        ImGui::EndMenu();
    }
}

void MenuBar::RenderWindowMenu()
{
    if (ImGui::BeginMenu("Window"))
    {
        if (ImGui::MenuItem("New Property View"))
        {
            // This will be handled by MainEditor
            GetMainEditor()->FindWidget<PropertyViewManager>()->AddPropertyViewPanel();
        }

        ImGui::EndMenu();
    }
}

void MenuBar::RenderEditMenu()
{
    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Import model")) {}

        if (ImGui::MenuItem("Load model")) {}

        ImGui::EndMenu();
    }
}

void MenuBar::RenderToolMenu()
{
    if (ImGui::BeginMenu("Tool")) { ImGui::EndMenu(); }
}

void MenuBar::RenderFileDialogs()
{
    if (ImGuiFileDialog::Instance()->Display("OpenFile"))
    {
        if (ImGuiFileDialog::Instance()->IsOk() == true)
        {
            std::map<std::string, std::string>&& files = ImGuiFileDialog::Instance()->GetSelection();
            HandleOpenFile(files);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("Save As"))
    {
        if (ImGuiFileDialog::Instance()->IsOk() == true)
        {
            std::string strFilePathName    = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string strCurrentFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
            HandleSaveFile(strFilePathName, strCurrentFileName);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("Convert Model")) {}
}

void MenuBar::HandleOpenFile(const std::map<std::string, std::string>& files)
{
    for (const auto& [fileName, filePath]: files)
    {
        if (LoadEntityFile(filePath))
        {
            GetEngine()->GetLogger()->Information("Entity file loaded successfully: {}", filePath);
        }
        else { GetEngine()->GetLogger()->Error("Failed to load entity file: {}", filePath); }
    }
}

void MenuBar::HandleSaveFile(const std::string& filePath, const std::string& fileName)
{
    MORTY_UNUSED(filePath);
    MORTY_UNUSED(fileName);
    // TODO: Implement save functionality
}

bool MenuBar::LoadEntityFile(const std::string& filePath)
{
    MScene* scene = GetScene();
    if (!scene)
    {
        GetEngine()->GetLogger()->Error("No scene available to load entity");
        return false;
    }

    MResourceSystem* resourceSystem = GetEngine()->GetSystem<MResourceSystem>();
    if (!resourceSystem)
    {
        GetEngine()->GetLogger()->Error("ResourceSystem not found");
        return false;
    }

    MEntitySystem* pEntitySystem = GetEngine()->GetSystem<MEntitySystem>();
    if (!pEntitySystem)
    {
        GetEngine()->GetLogger()->Error("EntitySystem not found");
        return false;
    }

    // Load entity resource
    std::shared_ptr<MResource> pResource = resourceSystem->LoadResource(filePath);
    if (!pResource)
    {
        GetEngine()->GetLogger()->Error("Failed to load resource from file: {}", filePath);
        return false;
    }

    // Load entities into scene
    std::vector<MEntity*> vEntities = pEntitySystem->LoadEntity(scene, pResource);
    if (vEntities.empty())
    {
        GetEngine()->GetLogger()->Warning("No entities loaded from file: {}", filePath);
        return false;
    }

    GetEngine()->GetLogger()->Information("Loaded {} entities from file", vEntities.size());
    return true;
}

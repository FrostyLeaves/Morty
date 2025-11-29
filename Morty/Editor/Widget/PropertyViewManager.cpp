#include "PropertyViewManager.h"
#include "Main/MainEditor.h"
#include "PropertyViewPanel.h"
#include "Utility/IniConfig.h"
#include "Widget/MenuBar.h"
#include "imgui.h"
#include "imgui_internal.h"

using namespace morty;

PropertyViewManager::PropertyViewManager()
    : BaseWidget()
{
    m_strViewName    = "PropertyViewManager";
    m_renderInHidden = true;
}

PropertyViewManager::~PropertyViewManager()
{
    for (auto* panel: m_panels)
    {
        if (panel)
        {
            panel->Release();
            delete panel;
        }
    }
    m_panels.clear();
}

void PropertyViewManager::Render()
{
    // Clear the removal list at the start of each frame
    m_panelsToRemove.clear();

    for (auto* panel: m_panels) { ShowPanel(panel); }

    // After rendering, remove panels that were closed
    for (int panelID: m_panelsToRemove) { RemovePropertyViewPanel(panelID); }

    // After rendering, capture the first panel's dock node ID for future use
    if (!m_panels.empty() && m_targetDockID == 0)
    {
        ImGuiWindow* firstWindow = ImGui::FindWindowByName(m_panels[0]->GetName().c_str());
        if (firstWindow && firstWindow->DockNode) { m_targetDockID = firstWindow->DockNode->ID; }
    }
}

void PropertyViewManager::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void PropertyViewManager::Release()
{
    for (auto* panel: m_panels)
    {
        if (panel)
        {
            panel->Release();
            delete panel;
        }
    }
    m_panels.clear();
}

void PropertyViewManager::SaveConfig(IniConfig* pConfig)
{
    BaseWidget::SaveConfig(pConfig);

    // Collect panel IDs into a vector
    std::vector<int> panelIDs;
    panelIDs.reserve(m_panels.size());
    for (auto* panel: m_panels)
    {
        if (panel) { panelIDs.push_back(panel->GetPanelID()); }
    }

    // Save panel IDs as an array
    pConfig->SetArray<int>(GetName().c_str(), "PanelIDs", panelIDs);

    // Save each panel's configuration
    for (auto* panel: m_panels)
    {
        if (panel) { panel->SaveConfig(pConfig); }
    }
}

void PropertyViewManager::LoadConfig(IniConfig* pConfig)
{
    BaseWidget::LoadConfig(pConfig);

    // Load panel IDs from array
    std::vector<int> panelIDs = pConfig->GetArray<int>(GetName().c_str(), "PanelIDs");

    // If we have saved panels, restore them
    if (!panelIDs.empty())
    {
        // Clear default panel
        for (auto* panel: m_panels)
        {
            if (panel)
            {
                panel->Release();
                delete panel;
            }
        }
        m_panels.clear();

        auto* menuBar = m_mainEditor->GetMenuBar();
        // Recreate saved panels
        for (int panelID: panelIDs)
        {
            auto* newPanel = new PropertyViewPanel(panelID);
            newPanel->Initialize(m_mainEditor);
            newPanel->LoadConfig(pConfig);
            m_panels.push_back(newPanel);
            if (menuBar) { menuBar->AddWidget(newPanel); }
        }
    }
}

void PropertyViewManager::AddPropertyViewPanel()
{
    int panelId = 0;
    while (FindPanel(panelId)) { ++panelId; }
    auto* newPanel = new PropertyViewPanel(panelId);
    newPanel->Initialize(m_mainEditor);
    newPanel->SetVisible(true);

    m_panels.push_back(newPanel);

    // Add the new panel to the menu bar
    auto* menuBar = m_mainEditor->GetMenuBar();
    if (menuBar) { menuBar->AddWidget(newPanel); }

    // Schedule the new panel to be docked on the next frame
    if (m_panels.size() > 1) { m_pendingDockWindowName = newPanel->GetName(); }
}

void PropertyViewManager::RemovePropertyViewPanel(int panelID)
{
    auto* menuBar = m_mainEditor->GetMenuBar();
    for (auto it = m_panels.begin(); it != m_panels.end(); ++it)
    {
        if ((*it)->GetPanelID() == panelID)
        {
            // Remove from menu bar
            if (menuBar) { menuBar->RemoveWidget(*it); }

            (*it)->Release();
            delete (*it);
            m_panels.erase(it);
            return;
        }
    }
}

PropertyViewPanel* PropertyViewManager::FindPanel(int panelID)
{
    for (auto* panel: m_panels)
    {
        if (panel->GetPanelID() == panelID) { return panel; }
    }
    return nullptr;
}

void PropertyViewManager::ShowPanel(PropertyViewPanel* pPanel)
{
    bool bVisible = pPanel->GetVisible();

    if (bVisible)
    {
        // If this is a pending dock window, set the dock ID before Begin
        if (!m_pendingDockWindowName.empty() && pPanel->GetName() == m_pendingDockWindowName && m_targetDockID != 0)
        {
            ImGui::SetNextWindowDockID(m_targetDockID, ImGuiCond_FirstUseEver);
            m_pendingDockWindowName.clear();
        }

        if (ImGui::Begin(pPanel->GetName().c_str(), &bVisible, pPanel->GetWindowFlags())) { pPanel->Render(); }

        // If the window was closed (bVisible became false), mark panel for removal
        if (!bVisible)
        {
            // Only allow closing if we have more than one panel (keep at least one)
            if (m_panels.size() > 1) { m_panelsToRemove.push_back(pPanel->GetPanelID()); }
            else
            {
                // Restore visibility for the last panel (prevent closing it)
                bVisible = true;
            }
        }

        pPanel->SetVisible(bVisible);
        ImGui::End();
    }
    else if (pPanel->GetRenderInHidden()) { pPanel->Render(); }
}

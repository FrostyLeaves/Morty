#pragma once

#include "Main/BaseWidget.h"
#include <vector>

namespace morty
{

class PropertyViewPanel;

class PropertyViewManager : public BaseWidget
{
public:
    PropertyViewManager();
    ~PropertyViewManager() override;

    void                            Render() override;

    void                            Initialize(MainEditor* pMainEditor) override;

    void                            Release() override;

    void                            SaveConfig(IniConfig* pConfig) override;

    void                            LoadConfig(IniConfig* pConfig) override;

    void                            AddPropertyViewPanel();

    void                            RemovePropertyViewPanel(int panelID);

    PropertyViewPanel*              FindPanel(int panelID);

    std::vector<PropertyViewPanel*> GetPanels() const { return m_panels; }

protected:
    void ShowPanel(PropertyViewPanel* pPanel);

private:
    std::vector<PropertyViewPanel*> m_panels;
    MString                         m_pendingDockWindowName;  // Window to dock on next frame
    ImGuiID                         m_targetDockID = 0;       // Target dock node ID
    std::vector<int>                m_panelsToRemove;         // Panels to remove after rendering
};

}// namespace morty

#pragma once

#include "Main/BaseWidget.h"
#include "Math/Vector.h"
#include "Utility/MColor.h"
#include "Utility/MString.h"
#include "Utility/MStringId.h"
#include "Utility/MTransform.h"
#include "Utility/SelectionManager.h"
#include <deque>
#include <functional>
#include <map>
#include <memory>

namespace morty
{

class MEntity;
class MComponentProperty;
class MaterialPropertyRenderer;
class MMaterialResource;

class PropertyViewPanel : public BaseWidget
{
public:
    PropertyViewPanel(int panelID);

    ~PropertyViewPanel() override;

    void Render() override;

    void Initialize(MainEditor* pMainEditor) override;

    void Release() override;

    void Input(MInputEvent* pEvent) override;

    ImGuiWindowFlags GetWindowFlags() override;

    void SetLocked(bool locked) { m_locked = locked; }
    bool IsLocked() const { return m_locked; }

    int GetPanelID() const { return m_panelID; }

protected:
    void OnSelectionChanged(const Selection& selection);

    void RenderEntityProperties(MEntity* pEntity);
    void RenderMaterialProperties(std::shared_ptr<MMaterialResource> material);

    void UpdatePropertyList(MEntity* pEntity);

    void RenderLockButton();

    void RenderPathHeader();

    void OnPathClicked();

private:
    int                                                       m_panelID = 0;
    bool                                                      m_locked  = false;

    Selection                                                 m_currentSelection;
    MEntity*                                                  m_entity = nullptr;
    std::deque<MComponentProperty*>                           m_propertyList;

    std::map<MStringId, std::function<MComponentProperty*()>> m_createPropertyFactory;

    std::unique_ptr<MaterialPropertyRenderer>                 m_materialRenderer;
};

}// namespace morty

#pragma once

#include "Main/BaseWidget.h"
#include "Render/SceneTexture.h"

#include "Property/PropertyBase.h"
#include "Resource/MResource.h"

namespace morty
{

class MScene;
class MEntity;
class MEngine;
class MMaterialResource;
class MInputEvent;
class MaterialView : public BaseWidget
{
public:
    MaterialView();

    ~MaterialView() = default;

public:
    void SetMaterial(std::shared_ptr<MMaterialResource> pMaterial);

    void Initialize(MainEditor* pMainEditor) override;

    void Release() override;

    void Input(MInputEvent* pEvent) override;

    void Render() override;


private:
    std::shared_ptr<MMaterialResource> m_material = nullptr;
    PropertyBase                       m_propertyBase;

    MScene*                            m_scene = nullptr;

    MEntity*                           m_staticSphereMeshNode   = nullptr;
    MEntity*                           m_skeletonSphereMeshNode = nullptr;

    std::shared_ptr<SceneTexture>      m_sceneTexture = nullptr;
};

}// namespace morty
#pragma once

#include "Property/PropertyBase.h"
#include "Render/SceneViewer.h"
#include "Resource/MMaterialResource.h"
#include <memory>

namespace morty
{

class MScene;
class MEntity;
class MEngine;
class MainEditor;

class MaterialPropertyRenderer
{
public:
    MaterialPropertyRenderer();
    ~MaterialPropertyRenderer();

    void         Initialize(MainEditor* pMainEditor);
    void         Release(MainEditor* pMainEditor);

    void         RenderMaterialProperties(std::shared_ptr<MMaterialResource> material);

    SceneViewer* GetSceneViewer() const { return m_sceneViewer; }

private:
    void SetMaterial(std::shared_ptr<MMaterialResource> material);

private:
    std::shared_ptr<MMaterialResource> m_material = nullptr;
    PropertyBase                       m_propertyBase;

    MScene*                            m_scene = nullptr;

    MEntity*                           m_staticSphereMeshNode   = nullptr;
    MEntity*                           m_skeletonSphereMeshNode = nullptr;

    SceneViewer*                       m_sceneViewer = nullptr;
};

}// namespace morty

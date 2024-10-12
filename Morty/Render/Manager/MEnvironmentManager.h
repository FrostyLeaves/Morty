#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Material/MMaterial.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"

namespace morty
{

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderPropertyBlock;
class MRenderMeshComponent;
struct MShaderConstantParam;

class MSkyBoxComponent;
class MORTY_API MEnvironmentManager : public IManager
{
public:
    MORTY_CLASS(MEnvironmentManager)

public:
    void                       Initialize() override;

    void                       Release() override;

    std::set<const MType*>     RegisterComponentType() const override;

    void                       RegisterComponent(MComponent* pComponent) override;

    void                       UnregisterComponent(MComponent* pComponent) override;

    void                       OnSkyBoxTextureChanged(MComponent* pComponent);

    void                       OnDiffuseEnvTextureChanged(MComponent* pComponent);

    void                       OnSpecularEnvTextureChanged(MComponent* pComponent);

    void                       UpdateSkyBoxMaterial(MSkyBoxComponent* pComponent);

    bool                       HasEnvironmentComponent() const;

    std::shared_ptr<MMaterial> GetMaterial() const;

protected:
    void InitializeMaterial();

    void ReleaseMaterial();

private:
    MSkyBoxComponent*           m_currentSkyBoxComponent = nullptr;

    std::set<MSkyBoxComponent*> m_allSkyBoxComponent;
    std::shared_ptr<MMaterial>  m_skyBoxMaterial = nullptr;
    MResourceRef                m_materialResource;
};

}// namespace morty
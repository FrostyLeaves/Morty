#ifndef _M_ENVIRONMENT_MANAGER_H_
#define _M_ENVIRONMENT_MANAGER_H_

#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"
#include "Material/MMaterial.h"

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderConstantParam;
class MShaderPropertyBlock;
class MRenderableMeshComponent;

class MSkyBoxComponent;
class MORTY_API MEnvironmentManager : public IManager
{
public:
	MORTY_CLASS(MEnvironmentManager)

public:

	void Initialize() override;
	void Release() override;

	std::set<const MType*> RegisterComponentType() const override;

	void RegisterComponent(MComponent* pComponent) override;
	void UnregisterComponent(MComponent* pComponent) override;

	void OnSkyBoxTextureChanged(MComponent* pComponent);
	void OnDiffuseEnvTextureChanged(MComponent* pComponent);
	void OnSpecularEnvTextureChanged(MComponent* pComponent);


	void UpdateSkyBoxMaterial(MSkyBoxComponent* pComponent);

	std::shared_ptr<MMaterial> GetMaterial() const { return m_pSkyBoxMaterial; }

protected:

	void InitializeMaterial();
	void ReleaseMaterial();

private:

	std::shared_ptr<MMaterial> m_pSkyBoxMaterial = nullptr;
	MSkyBoxComponent* m_pCurrentSkyBoxComponent = nullptr;

	std::set<MSkyBoxComponent*> m_tAllSkyBoxComponent;

};

#endif
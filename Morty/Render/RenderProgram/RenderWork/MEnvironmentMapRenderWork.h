/**
 * @File         MEnvironmentMapRenderWork
 * 
 * @Created      2022-01-19 09:24:03
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterial.h"
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Render/MRenderPass.h"

#include "Resource/MResource.h"

MORTY_SPACE_BEGIN

class MMeshResource;
class MIRenderCommand;
class MSkyBoxComponent;
class MORTY_API MEnvironmentMapRenderWork : public MObject
{
	MORTY_CLASS(MEnvironmentMapRenderWork)
public:
    MEnvironmentMapRenderWork();
    ~MEnvironmentMapRenderWork() override = default;

public:

	void OnCreated() override;
	void OnDelete() override;

public:

	void MarkUpdateEnvironment();

	void RenderEnvironment(MIRenderCommand* pRenderCommand, MSkyBoxComponent* pSkyBoxComponent);

	std::shared_ptr<MResource> GetDiffuseOutputTexture() const;

protected:

	void RenderDiffuse(MIRenderCommand* pRenderCommand, MSkyBoxComponent* pSkyBoxComponent);

	void RenderSpecular(MIRenderCommand* pRenderCommand, MSkyBoxComponent* pSkyBoxComponent);

protected:

	void InitializeResource();
	void ReleaseResource();

	void InitializeMaterial();
	void ReleaseMaterial();

	void InitializeRenderPass();
	void ReleaseRenderPass();

private:

	bool m_bUpdateNextFrame;

	std::shared_ptr<MMaterial> m_DiffuseMaterial;
	std::vector<std::shared_ptr<MMaterial>> m_vSpecularMaterial;

	std::shared_ptr<MMeshResource> m_pCubeMesh;

	MResourceRef m_DiffuseEnvironmentMap;
	MResourceRef m_SpecularEnvironmentMap;

	MRenderPass m_DiffuseRenderPass;
	std::vector<MRenderPass> m_vSpecularRenderPass;
};

MORTY_SPACE_END
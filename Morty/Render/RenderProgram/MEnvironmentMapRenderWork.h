/**
 * @File         MEnvironmentMapRenderWork
 * 
 * @Created      2022-01-19 09:24:03
 *
 * @Author       Pobrecito
**/

#ifndef _M_MENVIRONMENTMAPRENDERWORK_H_
#define _M_MENVIRONMENTMAPRENDERWORK_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "MRenderInfo.h"
#include "Render/MRenderPass.h"
#include "MShadowMapShaderParamSet.h"

#include "Resource/MResource.h"

class MMeshResource;
class MIRenderCommand;
class MSkyBoxComponent;
class MORTY_API MEnvironmentMapRenderWork : public MObject
{
	MORTY_CLASS(MEnvironmentMapRenderWork)
public:
    MEnvironmentMapRenderWork();
    virtual ~MEnvironmentMapRenderWork();

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

public:

	void MarkUpdateEnvironment();

	void RenderEnvironment(MIRenderCommand* pRenderCommand, MSkyBoxComponent* pSkyBoxComponent);

	std::shared_ptr<MResource> GetDiffuseOutputTexture();

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

	MResourceKeeper m_DiffuseEnvironmentMap;
	MResourceKeeper m_SpecularEnvironmentMap;

	MRenderPass m_DiffuseRenderPass;
	std::vector<MRenderPass> m_vSpecularRenderPass;
};

#endif

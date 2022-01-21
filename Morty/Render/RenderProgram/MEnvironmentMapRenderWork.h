/**
 * @File         MEnvironmentMapRenderWork
 * 
 * @Created      2022-01-19 09:24:03
 *
 * @Author       Pobrecito
**/

#ifndef _M_MENVIRONMENTMAPRENDERWORK_H_
#define _M_MENVIRONMENTMAPRENDERWORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "MRenderInfo.h"
#include "MRenderPass.h"
#include "MShadowMapShaderParamSet.h"

#include "MResource.h"

class MMeshResource;
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

	void RenderEnvironment(MRenderInfo& info);

protected:

	void InitializeResource();
	void ReleaseResource();

	void InitializeMaterial();
	void ReleaseMaterial();

	void InitializeRenderPass();
	void ReleaseRenderPass();

private:

	bool m_bUpdateNextFrame;

	MMaterial* m_DiffuseMaterial;
	MMaterial* m_SpecularMaterial;

	MMeshResource* m_pCubeMesh;

	MResourceKeeper m_DiffuseEnvironmentMap;
	MResourceKeeper m_SpecularEnvironmentMap;

	MRenderPass m_DiffuseRenderPass;
	MRenderPass m_SpecularRenderPass;

};

#endif

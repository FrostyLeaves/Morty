/**
 * @File         MMaterialResource
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMATERIALRESOURCE_H_
#define _M_MMATERIALRESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

class MMaterial;
class MShader;
class MShaderResource;
class MORTY_CLASS MMaterialResource : public MResource
{
public:
	enum EResReloadType
	{
		EVertex = MResource::EResReloadType::EUserDef + 1,
		EPixel = MResource::EResReloadType::EUserDef + 2,
	};
public:
    MMaterialResource();
    virtual ~MMaterialResource();

	MShader* GetVertexShader() { return m_pVertexShader; }
	MShader* GetPixelShader() { return m_pPixelShader; }

	bool LoadVertexShader(MResource* pResource);
	bool LoadPixelShader(MResource* pResource);

	MResource* GetVertexShaderResource() { return m_pVertexResource ? m_pVertexResource->GetResource() : nullptr; }
	MResource* GetPixelShaderResource() { return m_pPixelResource ? m_pPixelResource->GetResource() : nullptr; }

	MMaterial* GetMaterialTemplate() { return m_pMaterial; }

public:
	virtual void OnCreated() override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

private:

	MShader* m_pVertexShader;
	MShader* m_pPixelShader;

	MResourceHolder* m_pVertexResource;
	MResourceHolder* m_pPixelResource;

	MMaterial* m_pMaterial;
};


#endif

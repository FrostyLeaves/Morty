/**
 * @File         MComputerDispatcher
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#ifndef _M_MCOMPUTER_DISPATCHER_H_
#define _M_MCOMPUTER_DISPATCHER_H_
#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Resource/MResource.h"
#include "Resource/MTextureResource.h"
#include "Object/MObject.h"

#include "MShaderMacro.h"
#include "MShaderGroup.h"
#include "MShaderBuffer.h"
#include "Material/MShaderParamSet.h"

#include <vector>

class MORTY_API MComputeDispatcher : public MObject
{
public:
	MORTY_CLASS(MComputeDispatcher)
		MComputeDispatcher();
    virtual ~MComputeDispatcher();
	
public:

	MShader* GetComputeShader() { return m_shaderGroup.GetComputeShader(); }

	bool LoadComputeShader(std::shared_ptr<MResource> pResource);
	bool LoadComputeShader(const MString& strResource);

public:

	std::shared_ptr<MResource> GetComputeShaderResource() { return m_shaderGroup.GetComputeShaderResource(); }

	MShaderMacro& GetShaderMacro() { return m_shaderGroup.GetShaderMacro(); }

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;


private:

	MShaderGroup m_shaderGroup;
};

#endif

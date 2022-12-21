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
#include "MShaderProgram.h"
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

	bool LoadComputeShader(std::shared_ptr<MResource> pResource);
	bool LoadComputeShader(const MString& strResource);

public:

	std::shared_ptr<MResource> GetComputeShaderResource() { return m_pShaderProgram->GetComputeShaderResource(); }
	std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() { return  m_pShaderProgram->GetShaderParamSets(); }
	MShader* GetComputeShader();

	MShaderMacro& GetShaderMacro() { return m_pShaderProgram->GetShaderMacro(); }
	std::shared_ptr<MShaderProgram> GetShaderProgram() const { return m_pShaderProgram; }

	void SetDispatcherID(const uint32_t& nID) { m_unDispatcherID = nID; }
	uint32_t GetDispatcherID() const { return m_unDispatcherID; }

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;


private:

	std::shared_ptr<MShaderProgram> m_pShaderProgram;

	uint32_t m_unDispatcherID;

public:


	void GeneratePipeline();
	void DestroyPipeline();


#if RENDER_GRAPHICS == MORTY_VULKAN

	VkPipeline m_VkPipeline;

#endif
};

#endif

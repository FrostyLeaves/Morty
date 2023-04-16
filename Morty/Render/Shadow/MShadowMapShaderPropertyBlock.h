/**
 * @File         MShadowMapShaderPropertyBlock
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_SHADOW_MAP_SHADER_PROPERTY_BLOCK_H_
#define _M_SHADOW_MAP_SHADER_PROPERTY_BLOCK_H_
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "RenderProgram/MRenderInfo.h"
#include "Material/MShaderParamSet.h"
#include "RenderProgram/RenderWork/MRenderWork.h"


class MEngine;
class MORTY_API MShadowMapShaderPropertyBlock
	: public IPropertyBlockAdapter
	, public IComputeDispatcherAdapter
	, public IDrawIndirectAdapter
{
public:

	void Initialize(MEngine* pEngine);
	void Release(MEngine* pEngine);
	void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial);

	std::vector<std::shared_ptr<MShaderPropertyBlock>> GetPropertyBlock() const override;

	MComputeDispatcher* GetComputeDispatcher() const override { return m_pCullingDispatcher; }
	std::array<size_t, 3> GetComputeGroup() const override { return { GetCount() / 16 + (GetCount() % 16 ? 1 : 0), 1, 1 }; }
	std::vector<const MBuffer*> GetBarrierBuffer() const override { return { &m_drawIndirectBuffer }; }

	std::shared_ptr<MMaterial> GetMaterial() const override { return m_pMaterial; }
	const MBuffer* GetDrawIndirectBuffer() const override { return &m_drawIndirectBuffer; }
	const size_t GetOffset() const override { return 0; }
	const size_t GetCount() const override { return m_nInstanceNum; }

	void UpdateShaderSharedParams(MRenderInfo& info) ;

protected:
	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pMeshInstancePropertyBlock = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pWorldMatrixParam = nullptr;

	MComputeDispatcher* m_pCullingDispatcher = nullptr;
	size_t m_nInstanceNum = 0;
	MBuffer m_drawIndirectBuffer;
	MBuffer m_outputBuffer;
};

#endif

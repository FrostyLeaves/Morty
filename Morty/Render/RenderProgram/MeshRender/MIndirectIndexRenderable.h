/**
 * @File         MIndirectIndexRenderable
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

class IMaterialFilter;
class MScene;
class MInstanceCulling;
class MMaterialBatchGroup;
class MORTY_API MIndirectIndexRenderable : public IRenderable
{
public:

	void SetMaterial(const std::shared_ptr<MMaterial>& pMaterial) { m_pMaterial = pMaterial; }
	void SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter) { m_vPropertyAdapter = vAdapter; }
	void SetIndirectIndexBuffer(const MBuffer* pBuffer) { m_pBuffer = pBuffer; }
	void SetMeshBuffer(const std::shared_ptr<MMeshBufferAdapter>& pMeshBuffer) { m_pMeshBuffer = pMeshBuffer; }
	
	void Render(MIRenderCommand* pCommand) override;

	//override to use other material.
	virtual const std::shared_ptr<MMaterial>& GetMaterial() const { return m_pMaterial; }
	virtual const MBuffer* GetIndirectBuffer() const { return m_pBuffer; }

private:

	std::vector<std::shared_ptr<IPropertyBlockAdapter>> m_vPropertyAdapter;
	std::shared_ptr<MMeshBufferAdapter> m_pMeshBuffer = nullptr;
	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	const MBuffer* m_pBuffer = nullptr;
};

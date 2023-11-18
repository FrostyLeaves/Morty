/**
 * @File         MPostProcessNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#include "RenderProgram/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"

class MRenderPass;

class MORTY_API MPostProcessNode : public MTaskNode
{
public:

	void SetMaterial(const std::shared_ptr<MMaterial>& pMaterial);
	std::shared_ptr<MMaterial> GetMaterial() const { return m_pMaterial; }

	void SetRenderPass(MRenderPass* pRenderPass) { m_pRenderPass = pRenderPass; }
	MRenderPass* GetRenderPass() const { return m_pRenderPass; }

private:

	std::shared_ptr<MMaterial> m_pMaterial = nullptr;

	MRenderPass* m_pRenderPass = nullptr;
};

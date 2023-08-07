/**
 * @File         MDeferredLightingRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "MRenderWork.h"
#include "RenderProgram/MRenderInfo.h"

class MORTY_API MDeferredLightingRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MDeferredLightingRenderWork)

	void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;

	void Render(MRenderInfo& info);

	void SetGBuffer(const std::shared_ptr<IGBufferAdapter>& pAdapter);
	void SetShadowMap(const std::shared_ptr<ITextureInputAdapter>& pAdapter);
	void SetFrameProperty(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter);

private:

	std::shared_ptr<IPropertyBlockAdapter> m_pFramePropertyAdapter = nullptr;
	std::shared_ptr<IGBufferAdapter> m_pGBufferAdapter = nullptr;
	std::shared_ptr<ITextureInputAdapter> m_pShadowMapAdapter = nullptr;

	std::shared_ptr<MMaterial> m_pLightningMaterial = nullptr;
};

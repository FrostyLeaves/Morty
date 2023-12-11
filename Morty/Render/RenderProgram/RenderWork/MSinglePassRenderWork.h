/**
 * @File         MSinglePassRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "RenderProgram/MRenderInfo.h"
#include "MRenderWork.h"
#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"

class MORTY_API ISinglePassRenderWork : public IRenderWork
{
	MORTY_INTERFACE(ISinglePassRenderWork)
public:

    void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;

	void Resize(Vector2i size) override;

	MEngine* GetEngine() const { return m_pEngine; }

	void SetRenderTarget(const MRenderTargetGroup& renderTarget);

	std::vector<std::shared_ptr<MTexture>> GetBackTextures() const;
	std::shared_ptr<MTexture> GetDepthTexture() const;

	std::shared_ptr<IGetTextureAdapter> CreateOutput() const;

protected:

	MEngine* m_pEngine = nullptr;
	MRenderPass m_renderPass;
};

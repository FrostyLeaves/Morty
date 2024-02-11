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

MORTY_SPACE_BEGIN

class MORTY_API ISinglePassRenderWork : public MRenderTaskNode
{
	MORTY_INTERFACE(ISinglePassRenderWork)
public:

    void Initialize(MEngine* pEngine) override;
	void Release() override;

	void Resize(Vector2i size) override;

	MEngine* GetEngine() const { return m_pEngine; }

	void SetRenderTarget(const MRenderTargetGroup& renderTarget);

	std::vector<std::shared_ptr<MTexture>> GetBackTextures() const;
	std::shared_ptr<MTexture> GetDepthTexture() const;

	std::shared_ptr<IGetTextureAdapter> CreateOutput() const;

protected:

	MRenderTargetGroup AutoBindTarget();
	MRenderTargetGroup AutoBindTargetWithVRS();
	void AutoBindBarrierTexture();
	void AutoSetTextureBarrier(MIRenderCommand* pCommand);

	MEngine* m_pEngine = nullptr;
	MRenderPass m_renderPass;
	std::unordered_map<METextureBarrierStage, std::vector<MTexture*>> m_vBarrierTexture;
};

MORTY_SPACE_END
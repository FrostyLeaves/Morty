/**
 * @File         MVRSTextureRenderWork
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

class MORTY_API MVRSTextureRenderWork : public IRenderWork
{
	MORTY_INTERFACE(MVRSTextureRenderWork)
public:

    void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;

	void Resize(Vector2i size) override;

	void Render(MRenderInfo& info, const std::shared_ptr<IGetTextureAdapter>& pEdgeTexture);

	MEngine* GetEngine() const { return m_pEngine; }

	std::shared_ptr<MTexture> GetVRSTexture() const;

protected:

	MEngine* m_pEngine = nullptr;

	std::shared_ptr<MTexture> m_pVRSTexture = nullptr;

	MComputeDispatcher* m_pVRSGenerator = nullptr;

	Vector2i m_n2TexelSize = { 8, 8 };
};

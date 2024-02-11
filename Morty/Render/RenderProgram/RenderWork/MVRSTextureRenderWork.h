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

class MORTY_API MVRSTextureRenderWork : public MRenderTaskNode
{
	MORTY_CLASS(MVRSTextureRenderWork)
	static const MStringId VRS_TEXTURE;
public:

    void Initialize(MEngine* pEngine) override;
	void Release() override;

	void Resize(Vector2i size) override;

	void Render(const MRenderInfo& info) override;

	MEngine* GetEngine() const { return m_pEngine; }


protected:

	void BindTarget() override;

	std::vector<MRenderTaskInputDesc> InitInputDesc() override;

	std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

	MEngine* m_pEngine = nullptr;

	MComputeDispatcher* m_pVRSGenerator = nullptr;

	Vector2i m_n2TexelSize = { 8, 8 };
};

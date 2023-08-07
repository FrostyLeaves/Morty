#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "Render/MMesh.h"
#include "Render/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"

class MTaskNode;
class MIRenderCommand;
class MORTY_API MPostProcessRenderWork : public ISinglePassRenderWork
{
	MORTY_CLASS(MPostProcessRenderWork)

public:

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release(MEngine* pEngine) override;

	void Render(MRenderInfo& info);


	void SetInputTexture(const std::shared_ptr<ITextureInputAdapter>& pAdapter);
public:

	void InitializeMaterial();

	void ReleaseMaterial();

private:

	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<ITextureInputAdapter> m_pInputAdapter = nullptr;
};

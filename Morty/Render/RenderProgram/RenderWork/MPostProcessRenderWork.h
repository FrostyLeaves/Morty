#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "Render/MMesh.h"
#include "Render/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"
#include "TaskGraph/MTaskGraph.h"
#include "PostProcess/MPostProcessRenderTargetBinding.h"

class MTaskNode;
class MIRenderCommand;
class MORTY_API MPostProcessRenderWork : public IRenderWork
{
	MORTY_CLASS(MPostProcessRenderWork)

public:

	void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;

	void Render(MRenderInfo& info);
	void Resize(Vector2i size) override;

	void SetInputTexture(const std::shared_ptr<IGetTextureAdapter>& pAdapter);
	void SetRenderTarget(const MRenderTarget& backTexture);

	MEngine* GetEngine() const { return m_pEngine; }

	std::shared_ptr<IGetTextureAdapter> GetOutput(const MStringId& strNodeName) const;

	void InitializeMaterial();

	void ReleaseMaterial();

private:

	MEngine* m_pEngine = nullptr;
	MTaskGraph m_postProcessGraph;
	std::unique_ptr<MPostProcessRenderTargetBinding> m_pRenderTargetBinding = nullptr;

	std::shared_ptr<IGetTextureAdapter> m_pInputAdapter = nullptr;
};

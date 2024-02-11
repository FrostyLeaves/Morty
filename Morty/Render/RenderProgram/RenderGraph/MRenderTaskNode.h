/**
 * @File         MRenderTaskNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Basic/MTexture.h"
#include "Render/MRenderPass.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MGlobal.h"

#include "RenderProgram/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MStringId.h"
#include "MRenderTaskNodeOutput.h"
#include "Render/MRenderCommand.h"
#include "RenderProgram/RenderGraph/MRenderTargetManager.h"

MORTY_SPACE_BEGIN

class MRenderTaskNodeInput;
class IShaderPropertyUpdateDecorator;
class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;
class MRenderTaskNodeOutput;

class MORTY_API MRenderTaskTarget
{
public:

	enum class ResizePolicy
	{
		Fixed = 0,
		Scale = 1,
	};

	enum class SharedPolicy
	{
	    Shared = 0,
		Exclusive = 1,
	};


	MRenderTaskTarget* InitResizePolicy(ResizePolicy ePolicy, float fScale = 1.0f, size_t nTexelSize = 1);
	MRenderTaskTarget* InitSharedPolicy(SharedPolicy ePolicy);
	MRenderTaskTarget* InitTextureDesc(const MTextureDesc& desc);

	void SetTexture(const std::shared_ptr<MTexture>& pTexture);
	const std::shared_ptr<MTexture>& GetTexture() const { return m_pTexture; }
	SharedPolicy GetSharedPolicy() const { return m_eSharedPolicy; }
	ResizePolicy GetResizePolicy() const { return m_eResizePolicy; }
	MTextureDesc GetTextureDesc() const { return m_textureDesc; }

	float GetScale() const { return m_fScale; }
	size_t GetTexelSize() const { return m_nTexelSize; }

private:

	std::shared_ptr<MTexture> m_pTexture = nullptr;
	MTextureDesc m_textureDesc = {};
	SharedPolicy m_eSharedPolicy = SharedPolicy::Shared;
	ResizePolicy m_eResizePolicy = ResizePolicy::Scale;
	float m_fScale = 1.0f;
	size_t m_nTexelSize = 1;
};

struct MRenderTaskInputDesc
{
	MStringId name;
	METextureBarrierStage barrier = METextureBarrierStage::EPixelShaderSample;
};

struct MRenderTaskOutputDesc
{
	MStringId name;
	MPassTargetDescription renderDesc;
};

class MORTY_API MRenderTaskNode : public MTaskNode
{
	MORTY_CLASS(MRenderTaskNode)
public:

	virtual void Initialize(MEngine* pEngine) { MORTY_UNUSED(pEngine); }
	virtual void Release() {}
	virtual void Render(const MRenderInfo& info) { MORTY_UNUSED(info); }
	virtual void RenderSetup(const MRenderInfo& info) { MORTY_UNUSED(info); }
	virtual void BindTarget() {}
	virtual void RegisterSetting() {}
	virtual void Resize(Vector2i size) { MORTY_UNUSED(size); }
	virtual std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() { return nullptr; }

	virtual std::vector<MRenderTaskInputDesc> InitInputDesc() { return {}; }
	virtual std::vector<MRenderTaskOutputDesc> InitOutputDesc() { return {}; }

	void OnCreated() override;
	void OnDelete() override;


	MRenderGraph* GetRenderGraph() const;
	MRenderTargetManager* GetRenderTargetManager() const;

	std::shared_ptr<MTexture> GetInputTexture(const size_t& nIdx);
	std::shared_ptr<MTexture> GetOutputTexture(const size_t& nIdx);
	std::shared_ptr<MTexture> GetInputTexture(const MStringId& nIdx);
	std::shared_ptr<MTexture> GetOutputTexture(const MStringId& nIdx);


	MRenderTaskNodeOutput* GetRenderOutput(const size_t& nIdx);

	std::unordered_map<MStringId, MRenderTaskNodeInput*> m_tInput;
	std::unordered_map<MStringId, MRenderTaskNodeOutput*> m_tOutput;
};

MORTY_SPACE_END
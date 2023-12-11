/**
 * @File         MIRenderCommand
 * 
 * @Created      2021-07-14 18:08:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MIDevice.h"
#include "Render/MRenderGlobal.h"

#include "Math/Vector.h"
#include "Utility/MColor.h"

class MIMesh;
class MBuffer;
class MTexture;
class MMaterial;
class MRenderPass;
class MShaderPropertyBlock;
class MComputeDispatcher;
class MMaterialPipelineLayoutData;

enum class METextureBarrierStage
{
    EUnknow = 0,
	EPixelShaderSample,
	EPixelShaderWrite,
	EComputeShaderWrite,
	EComputeShaderRead,
	EShadingRateMask,
};

struct MORTY_API MViewportInfo
{
	MViewportInfo();
	MViewportInfo(const float& _x, const float& _y, const float& _width, const float& _height);

	float x;
	float y;
	float width;
	float height;
	float minz;
	float maxz;
};

struct MORTY_API MScissorInfo
{
	MScissorInfo();
	MScissorInfo(const float& _x, const float& _y, const float& _width, const float& _height);

	float x;
	float y;
	float width;
	float height;
};

struct MORTY_API MRenderPassStage
{
	MRenderPassStage();
	MRenderPassStage(MRenderPass* p, const uint32_t& n);

	MRenderPass* pRenderPass;
	uint32_t nSubpassIdx;
};

class MORTY_API MIRenderCommand
{
public:

	MIRenderCommand();
	virtual ~MIRenderCommand() {}

	virtual void SetViewport(const MViewportInfo& viewport) = 0;
	virtual void SetScissor(const MScissorInfo& scissor) = 0;

	virtual void RenderCommandBegin() = 0;
	virtual void RenderCommandEnd() = 0;

	virtual void BeginRenderPass(MRenderPass* pRenderPass) = 0;
	virtual void NextSubPass() = 0;
	virtual void EndRenderPass() = 0;

	virtual void DrawMesh(MIMesh* pMesh) = 0;
	virtual void DrawMesh(MIMesh* pMesh, const uint32_t& nIdxOffset, const uint32_t& nIdxCount, const uint32_t& nVrtOffset) = 0;
	virtual void DrawMesh(const MBuffer* pVertexBuffer, const MBuffer* pIndexBuffer, const size_t nVertexOffset, const size_t nIndexOffset, const size_t nIndexCount) = 0;
	virtual void DrawIndexedIndirect(const MBuffer* pVertexBuffer, const MBuffer* pIndexBuffer, const MBuffer* pCommandsBuffer, const size_t& offset, const size_t& count) = 0;

	virtual bool SetUseMaterial(std::shared_ptr<MMaterial> pMaterial) = 0;
	virtual bool SetGraphPipeline(std::shared_ptr<MMaterial> pMaterial) = 0;
	virtual void SetShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) = 0;

	virtual void PushShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) = 0;
	virtual void PopShaderPropertyBlock() = 0;

	virtual bool DispatchComputeJob(MComputeDispatcher* pMaterial, const uint32_t& nGroupX, const uint32_t& nGroupY, const uint32_t& nGroupZ) = 0;

	virtual bool AddRenderToTextureBarrier(const std::vector<MTexture*> vTextures, METextureBarrierStage dstStage) = 0;
	virtual bool AddBufferMemoryBarrier(const std::vector<const MBuffer*> vBuffers, MEBufferBarrierStage srcStage, MEBufferBarrierStage dstStage) = 0;

	virtual bool DownloadTexture(MTexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback) = 0;
	virtual bool CopyImageBuffer(MTexture* pSource, MTexture* pDest) = 0;
	virtual void UpdateMipmaps(MTexture* pBuffer) = 0;
	virtual void ResetBuffer(const MBuffer* pBuffer) = 0;
	virtual void FillTexture(MTexture* pBuffer, MColor color) = 0;

	virtual void SetShadingRate(Vector2i i2ShadingSize, std::array<MEShadingRateCombinerOp, 2> combineOp) = 0;

	virtual MIRenderCommand* CreateChildCommand() { return nullptr; }
	virtual MIRenderCommand* GetChildCommand(const size_t& nIndex) { 
		MORTY_UNUSED(nIndex);
		return nullptr;
	}

	virtual void ExecuteChildCommand() {}


	virtual bool IsFinished() { return false; }
	virtual void CheckFinished() {}

	virtual void addFinishedCallback(std::function<void()> func) = 0;

	uint32_t GetFrameIndex() { return m_unFrameIndex; }

	size_t GetDrawCallCount() const { return m_nDrawCallCount; }

public:
	uint32_t m_unFrameIndex;
	size_t m_nDrawCallCount = 0;

};

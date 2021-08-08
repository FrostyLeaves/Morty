/**
 * @File         MIRenderCommand
 * 
 * @Created      2021-07-14 18:08:04
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIRenderCommand_H_
#define _M_MIRenderCommand_H_
#include "MRenderGlobal.h"

#include "Vector.h"

class MIMesh;
class MTexture;
class MMaterial;
class MRenderPass;
class MShaderParamSet;
class MMaterialPipelineLayoutData;


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
	virtual void NextSubpass() = 0;
	virtual void EndRenderPass() = 0;

	virtual void DrawMesh(MIMesh* pMesh) = 0;
	virtual void DrawMesh(MIMesh* pMesh, const uint32_t& nIdxOffset, const uint32_t& nIdxCount, const uint32_t& nVrtOffset) = 0;

	virtual bool SetUseMaterial(MMaterial* pMaterial) = 0;
	virtual void SetShaderParamSet(MShaderParamSet* pParamSet) = 0;

	virtual bool SetRenderToTextureBarrier(const std::vector<MTexture*> vTextures) = 0;
	virtual bool DownloadTexture(MTexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback) = 0;
	virtual bool CopyImageBuffer(MTexture* pSource, MTexture* pDest) = 0;
	virtual void UpdateMipmaps(MTexture* pBuffer) = 0;

	virtual bool IsFinished() = 0;
	virtual void CheckFinished() = 0;

	virtual void AddDependCommand(MIRenderCommand* pDependCommand) = 0;

	uint32_t GetFrameIndex() { return m_unFrameIndex; }

public:
	uint32_t m_unFrameIndex;
	std::vector<std::function<void()>> m_aRenderFinishedCallback;

};


#endif

/**
 * @File         MIDevice
 * 
 * @Created      2019-09-21 23:08:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MIDEVICE_H_
#define _M_MIDEVICE_H_
#include "MGlobal.h"

#include "MShader.h"
#include "MRenderPass.h"

class MVertexBuffer;
class MIMesh;
class MTextureBuffer;
class MRenderTextureBuffer;
class MDepthTextureBuffer;
class MTexture;
class MRenderPass;
class MIRenderTarget;
class MTextureRenderTarget;
struct MShaderConstantParam;
class MMaterial;
class MIRenderCommand;

class MORTY_API MIDevice
{;
public:
	MIDevice(): m_pEngine(nullptr) { }
	virtual ~MIDevice() {}

	void SetEngine(MEngine* pEngine) { m_pEngine = pEngine; }
	MEngine* GetEngine() { return m_pEngine; }

public:
	virtual bool Initialize() = 0;
	virtual void Release() = 0;


public:
	virtual void GenerateVertex(MVertexBuffer* ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) = 0;
	virtual void DestroyVertex(MVertexBuffer* ppVertexBuffer) = 0;
	virtual void UploadVertex(MVertexBuffer* ppVertexBuffer, MIMesh* pMesh) = 0;

	virtual void GenerateTexture(MTexture* pTexture, MByte* pData = nullptr) = 0;
	virtual void DestroyTexture(MTexture* pTexture) = 0;

	virtual bool CompileShader(MShader* pShader) = 0;
	virtual void CleanShader(MShader* pShader) = 0;

	virtual bool GenerateShaderParamSet(MShaderParamSet* pParamSet) = 0;
	virtual void DestroyShaderParamSet(MShaderParamSet* pParamSet) = 0;

	virtual bool GenerateShaderParamBuffer(MShaderConstantParam* pParam) = 0;
	virtual void DestroyShaderParamBuffer(MShaderConstantParam* pParam) = 0;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) = 0;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) = 0;

	virtual bool GenerateFrameBuffer(MRenderPass* pRenderPass) = 0;
	virtual void DestroyFrameBuffer(MRenderPass* pRenderPass) = 0;

	virtual bool RegisterMaterial(MMaterial* pMaterial) { return true; };
	virtual bool UnRegisterMaterial(MMaterial* pMaterial) { return true; };

	virtual MIRenderCommand* CreateRenderCommand() = 0;
	virtual void RecoveryRenderCommand(MIRenderCommand* pCommand) = 0;

	virtual bool IsFinishedCommand(MIRenderCommand* pCommand) = 0;

	virtual void NewFrame(const uint32_t& nIdx) {}
	virtual void FrameFinish(const uint32_t& nIdx) {}

private:

	MEngine* m_pEngine;
};

#endif

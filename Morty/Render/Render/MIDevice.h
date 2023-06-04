﻿/**
 * @File         MIDevice
 * 
 * @Created      2019-09-21 23:08:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MIDEVICE_H_
#define _M_MIDEVICE_H_
#include "Utility/MGlobal.h"

#include "Material/MShader.h"
#include "Render/MRenderPass.h"

class MVertexBuffer;
class MIMesh;
class MBuffer;
class MTextureBuffer;
class MRenderTextureBuffer;
class MDepthTextureBuffer;
class MTexture;
class MRenderPass;
class MIRenderTarget;
class MTextureRenderTarget;
struct MShaderConstantParam;
class MMaterial;
class MComputeDispatcher;
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

	virtual void GenerateBuffer(MBuffer* pBuffer, const MByte* initialData, const size_t& unDataSize) = 0;
	virtual void DestroyBuffer(MBuffer* pBuffer) = 0;
	virtual void UploadBuffer(MBuffer* pBuffer, const size_t& unBeginOffset, const MByte* data, const size_t& unDataSize) = 0;
	virtual void DownloadBuffer(MBuffer* pBuffer, MByte* outputData, const size_t& nSize) = 0;

	virtual void GenerateTexture(MTexture* pTexture, const MByte* pData = nullptr) = 0;
	virtual void DestroyTexture(MTexture* pTexture) = 0;

	virtual bool CompileShader(MShader* pShader) = 0;
	virtual void CleanShader(MShader* pShader) = 0;

	virtual bool GenerateShaderParamSet(const std::shared_ptr<MShaderPropertyBlock>& pParamSet) = 0;
	virtual void DestroyShaderParamSet(const std::shared_ptr<MShaderPropertyBlock>& pParamSet) = 0;

	virtual bool GenerateShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam) = 0;
	virtual void DestroyShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam) = 0;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) = 0;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) = 0;

	virtual bool GenerateFrameBuffer(MRenderPass* pRenderPass) = 0;
	virtual void DestroyFrameBuffer(MRenderPass* pRenderPass) = 0;

	virtual bool GenerateShaderProgram(MShaderProgram* pShaderProgram) = 0;
	virtual void DestroyShaderProgram(MShaderProgram* pShaderProgram) = 0;

	virtual bool RegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher) { return true; };
	virtual bool UnRegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher) { return true; };

	virtual MIRenderCommand* CreateRenderCommand(const MString& strCommandName) = 0;
	virtual void RecoveryRenderCommand(MIRenderCommand* pCommand) = 0;

	virtual bool IsFinishedCommand(MIRenderCommand* pCommand) = 0;

	virtual void SubmitCommand(MIRenderCommand* pCommand) = 0;

	virtual void Update() {}

private:

	MEngine* m_pEngine;
};

#endif

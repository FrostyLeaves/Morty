﻿/**
 * @File         MIDevice
 * 
 * @Created      2019-09-21 23:08:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MIDEVICE_H_
#define _M_MIDEVICE_H_
#include "MGlobal.h"
#include "MString.h"
#include "MRenderPass.h"
#include "MRenderStructure.h"
#include "Shader/MShaderMacro.h"

class MVertexBuffer;
class MIMesh;
class MShaderParamSet;
class MTextureBuffer;
class MRenderTextureBuffer;
class MDepthTextureBuffer;
class MTexture;
class MIRenderTexture;
class MRenderPass;
class MShaderBuffer;
class MIRenderTarget;
class MTextureRenderTarget;
struct MShaderConstantParam;
class MMaterial;


class MORTY_API MIDevice
{;
public:
	MIDevice() {}
	virtual ~MIDevice() {}

public:
	virtual bool Initialize() = 0;
	virtual void Release() = 0;


public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) = 0;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) = 0;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) = 0;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture) = 0;
	virtual void GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap = true) = 0;
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) = 0;

	virtual bool GenerateRenderTextureBuffer(MRenderTextureBuffer* ppTextureBuffer, MIRenderTexture* pTexture) = 0;
	virtual void DestroyRenderTextureBuffer(MRenderTextureBuffer* ppTextureBuffer) = 0;

	virtual bool CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro) = 0;
	virtual void CleanShader(MShaderBuffer** ppShader) = 0;

	virtual bool GenerateShaderParamSet(MShaderParamSet* pParamSet) = 0;
	virtual void DestroyShaderParamSet(MShaderParamSet* pParamSet) = 0;

	virtual bool GenerateRenderTarget(MRenderPass* pRenderPass, MIRenderTarget* pRenderTarget) = 0;
	virtual void DestroyRenderTarget(MIRenderTarget* pRenderTarget) = 0;

	virtual bool GenerateShaderParamBuffer( MShaderConstantParam* pParam) = 0;
	virtual void DestroyShaderParamBuffer(MShaderConstantParam* pParam) = 0;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) = 0;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) = 0;

	virtual bool GenerateFrameBuffer(MRenderPass* pRenderPass) = 0;
	virtual void DestroyFrameBuffer(MRenderPass* pRenderPass) = 0;

	virtual bool GenerateRenderTargetView(MRenderTextureBuffer* pTextureBuffer) = 0;
	virtual void DestroyRenderTargetView(MRenderTextureBuffer* pTextureBuffer) = 0;

	virtual bool RegisterMaterial(MMaterial* pMaterial) { return true; };
	virtual bool UnRegisterMaterial(MMaterial* pMaterial) { return true; };

	virtual MRenderCommand* CreateRenderCommand() = 0;
	virtual void RecoveryRenderCommand(MRenderCommand* pCommand) = 0;

};

#endif

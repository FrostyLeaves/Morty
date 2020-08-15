/**
 * @File         MIDevice
 * 
 * @Created      2019-09-21 23:08:49
 *
 * @Author       Pobrecito
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
class MTextureBuffer;
class MRenderTextureBuffer;
class MDepthTextureBuffer;
class MTexture;
class MRenderPass;
class MShaderBuffer;
class MIRenderTarget;
class MTextureRenderTarget;
struct MShaderConstantParam;



class MORTY_CLASS MIDevice
{;
public:
	MIDevice() {}
	virtual ~MIDevice() {}

public:
	virtual bool Initialize() = 0;
	virtual void Release() = 0;

public:

	virtual void Tick(const float& fDelta) {}

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) = 0;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) = 0;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) = 0;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGeneerateMipmap = true) = 0;
	virtual void GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap = true) = 0;
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) = 0;

	virtual bool GenerateRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer, const METextureLayout& eType, const uint32_t& unWidth, const unsigned& unHeight) = 0;
	virtual void DestroyRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer) = 0;

	virtual void GenerateDepthTexture(MDepthTextureBuffer** ppTextureBuffer, const uint32_t& unWidth, const uint32_t& unHeight) = 0;
	virtual void DestroyDepthTexture(MDepthTextureBuffer** ppTextureBuffer) = 0;

	virtual bool CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const uint32_t& eShaderType, const MShaderMacro& macro) = 0;
	virtual void CleanShader(MShaderBuffer** ppShader) = 0;

	virtual bool GenerateRenderTarget(MIRenderTarget* pRenderTarget, uint32_t nWidth, uint32_t nHeight) = 0;
	virtual void DestroyRenderTarget(MIRenderTarget* pRenderTarget) = 0;

	virtual bool GenerateShaderParamBuffer( MShaderConstantParam* pParam) = 0;
	virtual void DestroyShaderParamBuffer(MShaderConstantParam* pParam) = 0;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) = 0;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) = 0;

	virtual void RegisterMaterial(MMaterial* pMaterial) {};
	virtual void UnRegisterMaterial(MMaterial* pMaterial) {};

};

#endif

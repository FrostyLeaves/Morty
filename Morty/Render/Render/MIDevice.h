/**
 * @File         MIDevice
 * 
 * @Created      2019-09-21 23:08:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#include "Shader/MShader.h"
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

enum class MEDeviceFeature
{
	EConservativeRasterization,
	EHLSLFunctionality,
	EVariableRateShading,
};

enum class MEBufferBarrierStage
{
	EUnknow = 0,
	EComputeShaderWrite,
	EComputeShaderRead,
	EPixelShaderWrite,
	EPixelShaderRead,
	EDrawIndirectRead,
	EShadingRateRead,
};

enum class MEShadingRateCombinerOp
{
	Keep = 0,
	Replace,
	Min,
	Max,
	Mul,
};

struct MShadingRateType {
	static constexpr MByte Rate_1x1 = 0;
	static constexpr MByte Rate_1X2 = 1;
	static constexpr MByte Rate_2X1 = 4;
	static constexpr MByte Rate_2X2 = 5;
	static constexpr MByte Rate_2X4 = 6;
	static constexpr MByte Rate_4X2 = 9;
	static constexpr MByte Rate_4X4 = 10;
};

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

	virtual bool GenerateShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) = 0;
	virtual void DestroyShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) = 0;

	virtual bool GenerateShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam) = 0;
	virtual void DestroyShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam) = 0;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) = 0;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) = 0;

	virtual bool GenerateFrameBuffer(MRenderPass* pRenderPass) = 0;
	virtual void DestroyFrameBuffer(MRenderPass* pRenderPass) = 0;

	virtual bool RegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher) { 
		MORTY_UNUSED(pComputeDispatcher); 
		return true; 
	};

	virtual bool UnRegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher) {
		MORTY_UNUSED(pComputeDispatcher); 
		return true;
	};

	virtual MIRenderCommand* CreateRenderCommand(const MString& strCommandName) = 0;
	virtual void RecoveryRenderCommand(MIRenderCommand* pCommand) = 0;

	virtual bool IsFinishedCommand(MIRenderCommand* pCommand) = 0;

	virtual void SubmitCommand(MIRenderCommand* pCommand) = 0;

	virtual void Update() {}

	virtual bool GetDeviceFeatureSupport(MEDeviceFeature feature) const = 0;

	virtual Vector2i GetShadingRateTextureTexelSize() const = 0;

private:

	MEngine* m_pEngine;
};

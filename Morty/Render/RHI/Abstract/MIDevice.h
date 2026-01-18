/**
 * @File         MIDevice
 * 
 * @Created      2019-09-21 23:08:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#include "RHI/MRenderPass.h"
#include "Shader/MShader.h"

namespace morty
{

class MVertexBuffer;
class MIMesh;
class MMaterialPass;
class MBuffer;
class MTextureBuffer;
class MRenderTextureBuffer;
class MDepthTextureBuffer;
class MTexture;
class MRenderPass;
class MIRenderTarget;
class MTextureRenderTarget;
struct MShaderUniformParam;
class MGraphicsPipeline;
class MMaterialTemplate;
class MMaterial;
class IShaderProgram;
class MComputeDispatcher;
class IRenderCommand;

class MORTY_API MIDevice
{
public:
    MIDevice()
        : m_engine(nullptr)
    {}

    virtual ~MIDevice() {}

    void     SetEngine(MEngine* engine) { m_engine = engine; }

    MEngine* GetEngine() const { return m_engine; }

public:
    virtual bool Initialize() = 0;

    virtual void Release() = 0;


public:
    virtual void GenerateBuffer(MBuffer* buffer, const MByte* initialData, const size_t& unDataSize) = 0;

    virtual void DestroyBuffer(MBuffer* buffer) = 0;

    virtual void
    UploadBuffer(MBuffer* buffer, const size_t& unBeginOffset, const MByte* data, const size_t& unDataSize) = 0;

    virtual void DownloadBuffer(MBuffer* buffer, MByte* outputData, const size_t& nSize) = 0;

    virtual void GenerateTexture(MTexture* texture, const std::vector<std::vector<MByte>>& buffer) = 0;

    virtual void DestroyTexture(MTexture* texture) = 0;

    // Copy image data between textures
    // Handles both regular textures and texture arrays automatically
    // @param source Source texture
    // @param target Target texture
    // @param srcMip Source mip level
    // @param srcSlice Source array slice (0 for non-array textures)
    // @param dstMip Destination mip level
    // @param dstSlice Destination array slice (0 for non-array textures)
    // @param width Copy width (at source mip level)
    // @param height Copy height (at source mip level)
    // @param command Optional render command to use (nullptr = create internal command)
    virtual void CopyImage(
            MTexture*       source,
            MTexture*       target,
            uint32_t        srcMip,
            uint32_t        srcSlice,
            uint32_t        dstMip,
            uint32_t        dstSlice,
            uint32_t        width,
            uint32_t        height,
            IRenderCommand* command = nullptr
    ) = 0;

    // Resize a texture array to a new layer count
    // Creates a new texture array with the specified layer count and copies existing data
    // @param texture The texture array to resize (must be ETexture2DArray type)
    // @param newLayerCount The new number of layers (must be >= current layer count)
    // @param command Optional render command to use (nullptr = create internal command)
    virtual void ResizeTextureArray(MTexture* texture, uint32_t newLayerCount, IRenderCommand* command = nullptr) = 0;

    virtual bool CompileShader(MShader* pShader) = 0;

    virtual void CleanShader(MShader* pShader) = 0;

    virtual bool SyncParameterSet(MShaderParameterSet* propertyBlock) = 0;

    virtual bool GenerateShaderParameterSet(MShaderParameterSet* pParameterSet) = 0;

    virtual void DestroyShaderParameterSet(MShaderParameterSet* pParameterSet) = 0;

    virtual bool GenerateShaderParamBuffer(MShaderUniformParam* param) = 0;

    virtual void DestroyShaderParamBuffer(MShaderUniformParam* param) = 0;

    virtual bool GenerateRenderPass(MRenderPass* pRenderPass) = 0;

    virtual void DestroyRenderPass(MRenderPass* pRenderPass) = 0;

    virtual bool GenerateFrameBuffer(MRenderPass* pRenderPass) = 0;

    virtual void DestroyFrameBuffer(MRenderPass* pRenderPass) = 0;

    virtual std::shared_ptr<MGraphicsPipeline>
    FindOrCreateGraphicsPipeline(const MMaterialPass* materialPass, const MRenderPass* pRenderPass) = 0;

    virtual IRenderCommand* CreateRenderCommand(const MString& strCommandName) = 0;

    virtual void            RecoveryRenderCommand(IRenderCommand* pCommand) = 0;

    virtual bool            IsFinishedCommand(IRenderCommand* pCommand) = 0;

    virtual void            SubmitCommand(IRenderCommand* pCommand) = 0;

    virtual void            Update() {}

    virtual bool            GetDeviceFeatureSupport(MEDeviceFeature feature) const = 0;

    virtual Vector2i        GetShadingRateTextureTexelSize() const = 0;

private:
    MEngine* m_engine = nullptr;
};

}// namespace morty
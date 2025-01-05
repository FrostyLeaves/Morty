#pragma once

#include "Utility/MGlobal.h"
#include "MRenderPassCmdDefine.h"
#include "Math/Vector.h"
#include "RHI/Abstract/MIDevice.h"

namespace morty
{

class MIMesh;
class MIDevice;
class MBuffer;
class MPipeline;
class MMaterial;
class MGraphicsPipeline;

class MRenderPassCmd
{
public:
    explicit MRenderPassCmd(MIDevice* device, MRenderPass* renderPass);
    ~MRenderPassCmd();

    [[nodiscard]] MRenderPass* GetRenderPass() const { return m_renderPass; }


public:
    void SetViewport(const MSetViewportCmd& viewport);
    void SetScissor(const MSetScissorCmd& scissor);

    void SetViewportAndScissor(const MSetViewportCmd& viewport);

    void DrawMesh(
            const MBuffer* pVertexBuffer,
            const MBuffer* pIndexBuffer,
            size_t         nVertexOffset,
            size_t         nIndexOffset,
            size_t         nIndexCount
    );

    void DrawMesh(MIMesh* mesh, size_t nIndexOffset, size_t nIndexCount, size_t nVertexOffset);

    void DrawMesh(MIMesh* mesh);

    void DrawIndexedIndirect(
            const MBuffer* pVertexBuffer,
            const MBuffer* pIndexBuffer,
            const MBuffer* pCommandsBuffer,
            size_t         offset,
            size_t         count
    );

    void SetGraphPipeline(const MGraphicsPipeline* pipeline, size_t subPassIdx);
    void SetGraphPipeline(const MMaterialTemplate* materialTemplate);
    void SetMaterial(const MMaterial* material);
    void SetMaterial(const MMaterialTemplate* materialTemplate);


    void SetShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& block);
    void SetShaderPropertyBlock(MShaderPropertyBlock* pPropertyBlock);

    void PushShaderPropertyBlock(MShaderPropertyBlock* pPropertyBlock);

    void PopShaderPropertyBlock();

    void AddTextureBarrier(const std::vector<MTexture*>& vTextures, METextureBarrierStage dstStage);

    void NextSubPass();

    void SetShadingRate(Vector2i i2ShadingSize, const std::array<MEShadingRateCombinerOp, 2>& combineOp);

    [[nodiscard]] const std::vector<IPipelineCmd*>& GetCommand() const { return m_commandQueue; }

private:
    void UpdateBuffer(MBuffer* pBuffer, const MByte* data, const size_t& size);

private:
    MIDevice*                          m_device        = nullptr;
    MPipeline*                         m_usingPipeline = nullptr;
    MRenderPass*                       m_renderPass    = nullptr;
    uint32_t                           m_subPassIdx    = 0;

    std::vector<IPipelineCmd*>         m_commandQueue;
    std::vector<MShaderPropertyBlock*> m_propertyBlockStack;
};

}// namespace morty

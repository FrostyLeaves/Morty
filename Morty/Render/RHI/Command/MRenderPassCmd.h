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
class MMaterialPass;
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
            const MBuffer* vertexBuffer,
            const MBuffer* indexBuffer,
            const MBuffer* commandsBuffer,
            size_t         offset,
            size_t         count
    );

    void DrawIndexedIndirectCount(
            const MBuffer* vertexBuffer,
            const MBuffer* indexBuffer,
            const MBuffer* commandsBuffer,
            const MBuffer* countBuffer,
            size_t         commandOffset,
            size_t         countOffset,
            size_t         maxCount
    );

    void SetGraphPipeline(const MGraphicsPipeline* pipeline, size_t subPassIdx);
    void SetGraphPipeline(const MMaterialPass* pass);

    void SetMaterial(const MMaterial* material, const MStringId& passName);
    void SetMaterial(const MMaterial* material, const MMaterialPass* pass);

    void SetShaderParameterSet(const std::shared_ptr<MShaderParameterSet>& block);
    void SetShaderParameterSet(MShaderParameterSet* pParameterSet);

    void PushShaderParameterSet(MShaderParameterSet* pParameterSet);
    void PopShaderParameterSet();
    void ApplyPushedShaderParameterSets();

    void AddTextureBarrier(const std::vector<MTexture*>& vTextures, METextureBarrierStage dstStage);

    void NextSubPass();

    void SetShadingRate(Vector2i i2ShadingSize, const std::array<MEShadingRateCombinerOp, 2>& combineOp);

    [[nodiscard]] const std::vector<IPipelineCmd*>& GetCommand() const { return m_commandQueue; }

private:
    void UpdateBuffer(MBuffer* buffer, const MByte* data, const size_t& size);

private:
    MIDevice*                         m_device        = nullptr;
    MPipeline*                        m_usingPipeline = nullptr;
    MRenderPass*                      m_renderPass    = nullptr;
    uint32_t                          m_subPassIdx    = 0;

    std::vector<IPipelineCmd*>        m_commandQueue;
    std::vector<MShaderParameterSet*> m_propertyBlockStack;
};

}// namespace morty

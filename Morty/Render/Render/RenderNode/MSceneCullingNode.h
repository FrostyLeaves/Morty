/**
 * @File         MSceneCullingNode
 *
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MBuffer.h"
#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

namespace morty
{

// Frustum plane for GPU culling (matches FrustumUtils.Plane in shader)
struct MFrustumPlaneData
{
    Vector3 normal   = Vector3(0.0f, 0.0f, 0.0f);
    float   distance = 0.0f;
};

// NaniteRenderData for GPU culling (matches NaniteRenderData in shader)
struct MNaniteRenderData
{
    Matrix4           viewProjMatrix;
    Matrix4           viewMatrix;
    Vector3           cameraPositionWS = Vector3(0.0f, 0.0f, 0.0f);
    float             padding1         = 0.0f;
    Vector2           screenSize       = Vector2(0.0f, 0.0f);
    float             nearClip         = 0.0f;
    float             farClip          = 0.0f;
    MFrustumPlaneData frustumPlanes[6];
};

// CandidateCluster output structure (matches CandidateCluster in shader)
struct MCandidateCluster
{
    uint32_t clusterIndex;
    uint32_t meshInstanceIndex;
    float    distanceToCamera;
    uint32_t padding;
};

REFL_RENDER_NODE_CLASS MSceneCullingNode : public MRenderTaskNode
{
    MORTY_CLASS(MSceneCullingNode)

public:
    void OnCreated() override;
    void OnDelete() override;
    void Execute(const MRenderInfo& info, IRenderCommand* primaryCommand) override;

    const MBuffer* GetCandidateClustersBuffer() const { return &m_candidateClustersBuffer; }
    const MBuffer* GetCandidateCountBuffer() const { return &m_candidateCountBuffer; }

protected:
    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

    MComputeDispatcher*                m_cullingDispatcher = nullptr;

    // Output buffers for culling results
    MBuffer                            m_candidateClustersBuffer;
    MBuffer                            m_candidateCountBuffer;

    // Max candidate clusters for buffer sizing
    static constexpr size_t            MaxCandidateClusters = 1024 * 64;
};

}// namespace morty
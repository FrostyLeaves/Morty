#include "MSceneCullingNode.h"

#include "Basic/MPlane.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Batch/Mesh/MMeshInstanceManager.h"
#include "Engine/MEngine.h"
#include "Material/MComputeDispatcher.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSceneCullingNode, MRenderTaskNode)

// Shader parameter names
static const MStringId InstanceDataNameId       = MStringId("meshInstances");
static const MStringId MeshResourcesNameId      = MStringId("meshResources");
static const MStringId ClusterGroupsNameId      = MStringId("clusterGroups");
static const MStringId ClustersNameId           = MStringId("clusters");
static const MStringId CandidateClustersNameId  = MStringId("outCandidateClusters");
static const MStringId CandidateCountNameId     = MStringId("outCandidateCount");
static const MStringId InstanceCountNameId      = MStringId("instanceCount");
static const MStringId CullingEntryNameId       = MStringId("NaniteInstanceCullingCS");

// NaniteRenderData uniform member names
static const MStringId ViewProjMatrixNameId     = MStringId("viewProjMatrix");
static const MStringId ViewMatrixNameId         = MStringId("viewMatrix");
static const MStringId CameraPositionNameId     = MStringId("cameraPositionWS");
static const MStringId ScreenSizeNameId         = MStringId("screenSize");
static const MStringId NearClipNameId           = MStringId("nearClip");
static const MStringId FarClipNameId            = MStringId("farClip");

void                   MSceneCullingNode::OnCreated()
{
    Super::OnCreated();

    auto objectSystem   = GetEngine()->GetSystem<MObjectSystem>();
    auto resourceSystem = GetEngine()->GetSystem<MResourceSystem>();
    auto renderSystem   = GetEngine()->GetSystem<MRenderSystem>();

    m_cullingDispatcher = objectSystem->CreateObject<MComputeDispatcher>();

    auto cullingShader = resourceSystem->LoadResource("ShaderSlang/Culling/MeshInstanceCullingModule.slang");
    m_cullingDispatcher->LoadComputeShader(cullingShader, CullingEntryNameId);

    // Initialize output buffers
    m_candidateClustersBuffer = MBuffer::CreateStorageBuffer("MSceneCullingNode::CandidateClusters");
    m_candidateClustersBuffer.ReallocMemory(MaxCandidateClusters * sizeof(MCandidateCluster));
    m_candidateClustersBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);

    m_candidateCountBuffer = MBuffer::CreateStorageBuffer("MSceneCullingNode::CandidateCount");
    m_candidateCountBuffer.ReallocMemory(sizeof(uint32_t));
    m_candidateCountBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);
}

void MSceneCullingNode::OnDelete()
{
    auto renderSystem = GetEngine()->GetSystem<MRenderSystem>();

    m_candidateClustersBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_candidateCountBuffer.DestroyBuffer(renderSystem->GetDevice());

    m_cullingDispatcher->DeleteLater();
    m_cullingDispatcher = nullptr;
}

void MSceneCullingNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    // Get managers
    auto instanceManager = info.scene->GetManager<MMeshInstanceManager>();
    auto meshManager     = info.scene->GetManager<MMeshManager>();

    if (!instanceManager || !meshManager) { return; }

    // Get instance count
    const auto& renderProxies = instanceManager->GetBatchGroups();
    uint32_t    instanceCount = 0;
    for (const auto& group: renderProxies)
    {
        if (group) { instanceCount += static_cast<uint32_t>(group->GetInstanceCount()); }
    }

    if (instanceCount == 0) { return; }

    // Build NaniteRenderData from MRenderInfo
    MNaniteRenderData renderData;

    // View matrix is inverse of camera transform
    renderData.viewMatrix     = info.m4CameraTransform.Inverse();
    renderData.viewProjMatrix = info.m4ProjectionMatrix * renderData.viewMatrix;

    // Camera position is the translation part of camera transform
    renderData.cameraPositionWS = Vector3(
            info.m4CameraTransform.m[3][0],
            info.m4CameraTransform.m[3][1],
            info.m4CameraTransform.m[3][2]
    );

    // Screen size from viewport rect
    renderData.screenSize = Vector2(
            static_cast<float>(info.viewportRect.GetWidth()),
            static_cast<float>(info.viewportRect.GetHeight())
    );

    // Near/far clip planes
    renderData.nearClip = info.f2CameraNearFar.x;
    renderData.farClip  = info.f2CameraNearFar.y;

    // Extract frustum planes from MCameraFrustum
    for (size_t i = 0; i < 6; ++i)
    {
        MPlane plane                    = info.cameraFrustum.GetPlane(i);
        renderData.frustumPlanes[i].normal   = Vector3(plane.m_plane.x, plane.m_plane.y, plane.m_plane.z);
        renderData.frustumPlanes[i].distance = plane.m_plane.w;
    }

    // Get shader parameter set
    auto parameterSet = m_cullingDispatcher->GetShaderParameterSet(0);

    // Set uniform values for NaniteRenderData struct
    parameterSet->SetValue(ViewProjMatrixNameId, renderData.viewProjMatrix);
    parameterSet->SetValue(ViewMatrixNameId, renderData.viewMatrix);
    parameterSet->SetValue(CameraPositionNameId, renderData.cameraPositionWS);
    parameterSet->SetValue(ScreenSizeNameId, renderData.screenSize);
    parameterSet->SetValue(NearClipNameId, renderData.nearClip);
    parameterSet->SetValue(FarClipNameId, renderData.farClip);

    // TODO: frustumPlanes array needs to be set - may require extending shader param system
    // For now, frustum culling in shader will use these planes from the uniform buffer

    // Set instance count uniform
    parameterSet->SetValue(InstanceCountNameId, instanceCount);

    // Set input buffers
    parameterSet->SetBuffer(InstanceDataNameId, instanceManager->GetInstanceBuffer());
    parameterSet->SetBuffer(MeshResourcesNameId, meshManager->GetMeshResourceBuffer());
    parameterSet->SetBuffer(ClusterGroupsNameId, meshManager->GetClusterGroupBuffer());
    parameterSet->SetBuffer(ClustersNameId, meshManager->GetClusterBuffer());

    // Set output buffers
    parameterSet->SetBuffer(CandidateClustersNameId, &m_candidateClustersBuffer);
    parameterSet->SetBuffer(CandidateCountNameId, &m_candidateCountBuffer);

    // Calculate thread group count (64 threads per group as defined in shader)
    constexpr uint32_t ThreadsPerGroup = 64;
    uint32_t           groupCountX     = (instanceCount + ThreadsPerGroup - 1) / ThreadsPerGroup;

    // Dispatch compute shader
    primaryCommand->DispatchComputeJob(m_cullingDispatcher, CullingEntryNameId, groupCountX, 1, 1);
}

std::vector<MRenderTaskOutputDesc> MSceneCullingNode::InitOutputDesc()
{
    static const auto outputCullingResultId = MStringId("Culling output");

    return {
            MRenderTaskNodeOutput::CreateBuffer(outputCullingResultId),
    };
}

#include "MSceneCullingNode.h"

#include "Basic/MPlane.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Batch/Mesh/MMeshInstanceManager.h"
#include "Engine/MEngine.h"
#include "Material/MComputeDispatcher.h"
#include "Mesh/MMeshManager.h"
#include "Mesh/MVertex.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderer.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSceneCullingNode, MRenderTaskNode)

static const MStringId CullingEntryNameId       = MStringId("NaniteInstanceCullingCS");
static const MStringId BuildDrawCallEntryNameId = MStringId("BuildDrawCallFillCS");

// Shader parameter names for NaniteCulling (MeshInstanceCullingModule.slang)
// Uses global ParameterBlock: instanceCullingInput
static const MStringId CullingMeshInstancesNameId    = MStringId("instanceCullingInput.meshInstances");
static const MStringId CullingMeshResourcesNameId    = MStringId("instanceCullingInput.meshResources");
static const MStringId CullingClusterGroupsNameId    = MStringId("instanceCullingInput.clusterGroups");
static const MStringId CullingClustersNameId         = MStringId("instanceCullingInput.clusters");
static const MStringId CullingGroupLinksNameId       = MStringId("instanceCullingInput.groupLinks");
static const MStringId CullingOutCandidateClustersId = MStringId("instanceCullingInput.outCandidateClusters");
static const MStringId CullingOutCandidateCountId    = MStringId("instanceCullingInput.outCandidateCount");

// Shader parameter names for BuildDrawCall (BuildDrawCallModule.slang)
// Uses global ParameterBlock: buildDrawCallInput
static const MStringId BuildCandidateClustersNameId = MStringId("buildDrawCallInput.candidateClusters");
static const MStringId BuildCandidateCountNameId    = MStringId("buildDrawCallInput.candidateCount");
static const MStringId BuildMeshInstancesNameId     = MStringId("buildDrawCallInput.meshInstances");
static const MStringId BuildClustersNameId          = MStringId("buildDrawCallInput.clusters");
static const MStringId BuildClusterGroupsNameId     = MStringId("buildDrawCallInput.clusterGroups");
static const MStringId BuildOutDrawIndirectNameId   = MStringId("buildDrawCallInput.outDrawIndirect");
static const MStringId BuildOutDrawGroupCountNameId = MStringId("buildDrawCallInput.outDrawGroupCount");
static const MStringId BuildDrawGroupCountUniformId = MStringId("drawGroupCount");// Inside buildDrawCallParams struct

// Culling params constant buffer member names (for recursive struct member lookup)
static const MStringId NaniteCullingParamsNameId = MStringId("naniteCullingParams");
static const MStringId ViewProjMatrixNameId      = MStringId("viewProjMatrix");
static const MStringId ViewMatrixNameId          = MStringId("viewMatrix");
static const MStringId CameraPositionNameId      = MStringId("cameraPositionWS");
static const MStringId ScreenSizeNameId          = MStringId("screenSize");
static const MStringId NearClipNameId            = MStringId("nearClip");
static const MStringId FarClipNameId             = MStringId("farClip");
static const MStringId FrustumNameId             = MStringId("frustum");
static const MStringId PlaneNameId               = MStringId("planes");
static const MStringId InstanceCountNameId       = MStringId("instanceCount");

void                   MSceneCullingNode::OnCreated()
{
    Super::OnCreated();

    auto objectSystem   = GetEngine()->GetSystem<MObjectSystem>();
    auto resourceSystem = GetEngine()->GetSystem<MResourceSystem>();
    auto renderSystem   = GetEngine()->GetSystem<MRenderSystem>();

    m_cullingDispatcher       = objectSystem->CreateObject<MComputeDispatcher>();
    m_buildDrawCallDispatcher = objectSystem->CreateObject<MComputeDispatcher>();
    m_renderer                = std::make_unique<MIndexedIndirectCountRenderer>();

    auto cullingShader = resourceSystem->LoadResource("ShaderSlang/Culling/MeshInstanceCullingModule.slang");
    m_cullingDispatcher->LoadComputeShader(cullingShader, CullingEntryNameId);

    auto buildDrawCallShader = resourceSystem->LoadResource("ShaderSlang/Culling/BuildDrawCallModule.slang");
    m_buildDrawCallDispatcher->LoadComputeShader(buildDrawCallShader, BuildDrawCallEntryNameId);

    // Initialize output buffers
    m_candidateClustersBuffer = MBuffer::CreateStorageBuffer("MSceneCullingNode::CandidateClusters");
    m_candidateClustersBuffer.ReallocMemory(MaxCandidateClusters * sizeof(MCandidateCluster));
    m_candidateClustersBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);

    m_candidateCountBuffer = MBuffer::CreateStorageBuffer("MSceneCullingNode::CandidateCount");
    m_candidateCountBuffer.ReallocMemory(sizeof(uint32_t));
    m_candidateCountBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);

    m_drawIndirectBuffer = MBuffer::CreateIndirectDrawBuffer("MSceneCullingNode::DrawIndirect");
    m_drawIndirectBuffer.ReallocMemory(MaxDrawCallsPerGroup * sizeof(MDrawIndexedIndirectData));
    m_drawIndirectBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_drawIndirectBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);

    m_drawCallGroupBuffer = MBuffer::CreateBuffer(MBuffer::MMemoryType::EHostVisible, MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect, "MSceneCullingNode::DrawCallGroupCount");
}

void MSceneCullingNode::OnDelete()
{
    auto renderSystem = GetEngine()->GetSystem<MRenderSystem>();

    m_candidateClustersBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_candidateCountBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_drawIndirectBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_drawCallGroupBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_cullingDispatcher->DeleteLater();
    m_cullingDispatcher = nullptr;

    m_buildDrawCallDispatcher->DeleteLater();
    m_buildDrawCallDispatcher = nullptr;

    m_renderer = nullptr;
}

void MSceneCullingNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    auto instanceManager = info.scene->GetManager<MMeshInstanceManager>();
    auto meshManager     = info.scene->GetManager<MMeshManager>();

    NaniteCulling(info, primaryCommand);

    {
        std::vector<const MBufferRHI*> barrierBuffers;
        if (m_candidateClustersBuffer.m_bufferRHI) { barrierBuffers.push_back(m_candidateClustersBuffer.m_bufferRHI.get()); }
        if (m_candidateCountBuffer.m_bufferRHI) { barrierBuffers.push_back(m_candidateCountBuffer.m_bufferRHI.get()); }
        if (!barrierBuffers.empty()) { primaryCommand->AddBufferMemoryBarrier(barrierBuffers, MEBufferBarrierStage::EComputeShaderWrite, MEBufferBarrierStage::EComputeShaderRead); }
    }

    BuildDrawCall(info, primaryCommand);


    if (m_drawIndirectBuffer.m_bufferRHI)
    {
        primaryCommand->AddBufferMemoryBarrier({m_drawIndirectBuffer.m_bufferRHI.get()}, MEBufferBarrierStage::EComputeShaderWrite, MEBufferBarrierStage::EDrawIndirectRead);
    }
    if (m_drawCallGroupBuffer.m_bufferRHI)
    {
        primaryCommand->AddBufferMemoryBarrier({m_drawCallGroupBuffer.m_bufferRHI.get()}, MEBufferBarrierStage::EComputeShaderWrite, MEBufferBarrierStage::EDrawIndirectRead);
    }

    if (m_renderer)
    {
        m_renderer->vertexBuffer   = meshManager->GetVertexBuffer();
        m_renderer->indexBuffer    = meshManager->GetIndexBuffer();
        m_renderer->indirectBuffer = &m_drawIndirectBuffer;
        m_renderer->countBuffer    = &m_drawCallGroupBuffer;

        const auto& batchGroups = instanceManager->GetBatchGroups();
        m_renderer->drawCalls.resize(batchGroups.size());
        std::transform(batchGroups.begin(), batchGroups.end(), m_renderer->drawCalls.begin(), [](const MMaterialBatchGroup* group) {
            return MIndexedIndirectCountRenderer::DrawCall{
                    .pass          = group->GetMaterialTemplate()->GetDefaultPass(),
                    .parameterSet  = group->GetParameterSet().get(),
                    .commandOffset = group->GetBatchId() * MaxDrawCallsPerGroup,
                    .countOffset   = group->GetBatchId(),
                    .maxCount      = MaxDrawCallsPerGroup
            };
        });
        GetRenderOutput(0)->SetData(m_renderer.get());
    }
}

void MSceneCullingNode::NaniteCulling(const MRenderInfo& info, IRenderCommand* primaryCommand)
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

    // Build NaniteCullingParams from MRenderInfo
    MNaniteCullingParams cullingParams;

    // View matrix is inverse of camera transform
    cullingParams.renderData.viewMatrix     = info.m4CameraTransform.Inverse();
    cullingParams.renderData.viewProjMatrix = info.m4ProjectionMatrix * cullingParams.renderData.viewMatrix;

    // Camera position is the translation part of camera transform
    cullingParams.renderData.cameraPositionWS = Vector3(info.m4CameraTransform.m[3][0], info.m4CameraTransform.m[3][1], info.m4CameraTransform.m[3][2]);

    // Screen size from viewport rect
    cullingParams.renderData.screenSize = Vector2(static_cast<float>(info.viewportRect.GetWidth()), static_cast<float>(info.viewportRect.GetHeight()));

    // Near/far clip planes
    cullingParams.renderData.nearClip = info.f2CameraNearFar.x;
    cullingParams.renderData.farClip  = info.f2CameraNearFar.y;

    // Extract frustum planes from MCameraFrustum
    for (size_t i = 0; i < 6; ++i)
    {
        MPlane plane                                       = info.cameraFrustum.GetPlane(i);
        cullingParams.renderData.frustumPlanes[i].normal   = Vector3(plane.m_plane.x, plane.m_plane.y, plane.m_plane.z);
        cullingParams.renderData.frustumPlanes[i].distance = plane.m_plane.w;
    }

    // Set instance count
    cullingParams.instanceCount = instanceCount;

    // Get shader parameter set
    auto parameterSet = m_cullingDispatcher->GetShaderParameterSet(0);

    // Set values for the params constant buffer (using recursive struct member lookup)
    // The shader has: ConstantBuffer<NaniteCullingParams> params
    // The SetValue function will recursively find members in nested structs
    parameterSet->SetValue(ViewProjMatrixNameId, cullingParams.renderData.viewProjMatrix);
    parameterSet->SetValue(ViewMatrixNameId, cullingParams.renderData.viewMatrix);
    parameterSet->SetValue(CameraPositionNameId, cullingParams.renderData.cameraPositionWS);
    parameterSet->SetValue(ScreenSizeNameId, cullingParams.renderData.screenSize);
    parameterSet->SetValue(NearClipNameId, cullingParams.renderData.nearClip);
    parameterSet->SetValue(FarClipNameId, cullingParams.renderData.farClip);
    parameterSet->SetValue(InstanceCountNameId, cullingParams.instanceCount);

    auto planes = parameterSet->FindValue(PlaneNameId).GetValue<MVariantArray>();
    for (size_t i = 0; i < 6; ++i)
    {
        Vector4 param = {cullingParams.renderData.frustumPlanes[i].normal, cullingParams.renderData.frustumPlanes[i].distance};
        planes.SetVariant(i, param);
    }

    // Set input buffers
    parameterSet->SetBuffer(CullingMeshInstancesNameId, instanceManager->GetInstanceBuffer());
    parameterSet->SetBuffer(CullingMeshResourcesNameId, meshManager->GetMeshResourceBuffer());
    parameterSet->SetBuffer(CullingClusterGroupsNameId, meshManager->GetClusterGroupBuffer());
    parameterSet->SetBuffer(CullingClustersNameId, meshManager->GetClusterBuffer());
    parameterSet->SetBuffer(CullingGroupLinksNameId, meshManager->GetGroupLinksBuffer());

    // Set output buffers
    parameterSet->SetBuffer(CullingOutCandidateClustersId, &m_candidateClustersBuffer);
    parameterSet->SetBuffer(CullingOutCandidateCountId, &m_candidateCountBuffer);

    // Calculate thread group count (64 threads per group as defined in shader)
    constexpr uint32_t ThreadsPerGroup = 64;
    uint32_t           groupCountX     = (instanceCount + ThreadsPerGroup - 1) / ThreadsPerGroup;


    primaryCommand->FillBuffer(&m_candidateCountBuffer, 0);
    primaryCommand->AddBufferMemoryBarrier({m_candidateCountBuffer.m_bufferRHI.get()}, MEBufferBarrierStage::ETransferWrite, MEBufferBarrierStage::EComputeShaderWrite);

    // Dispatch compute shader
    primaryCommand->DispatchComputeJob(m_cullingDispatcher, CullingEntryNameId, groupCountX, 1, 1);
}

void MSceneCullingNode::BuildDrawCall(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    auto instanceManager = info.scene->GetManager<MMeshInstanceManager>();
    auto meshManager     = info.scene->GetManager<MMeshManager>();
    if (!instanceManager || !meshManager || !m_buildDrawCallDispatcher) { return; }

    const auto&  batchGroups = instanceManager->GetBatchGroups();
    const size_t groupCount  = batchGroups.size();
    if (groupCount == 0) { return; }

    auto         renderSystem = GetEngine()->GetSystem<MRenderSystem>();

    const size_t drawBufferSize = groupCount * MaxDrawCallsPerGroup * sizeof(MDrawIndexedIndirectData);
    if (m_drawIndirectBuffer.GetSize() < drawBufferSize)
    {
        m_drawIndirectBuffer.ReallocMemory(drawBufferSize);
        m_drawIndirectBuffer.DestroyBuffer(renderSystem->GetDevice());
        m_drawIndirectBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);
    }

    const size_t groupBufferSize = groupCount * sizeof(uint32_t);
    if (m_drawCallGroupBuffer.GetSize() != groupBufferSize)
    {
        m_drawCallGroupBuffer.ReallocMemory(groupBufferSize);
        m_drawCallGroupBuffer.DestroyBuffer(renderSystem->GetDevice());
        m_drawCallGroupBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);
    }
    std::vector<uint32_t> groupCounts(groupCount);
    m_drawCallGroupBuffer.UploadBuffer(renderSystem->GetDevice(), reinterpret_cast<const MByte*>(groupCounts.data()), groupBufferSize);

    auto setupParameters = [&](const std::shared_ptr<MShaderParameterSet>& parameterSet) {
        parameterSet->SetBuffer(BuildCandidateClustersNameId, &m_candidateClustersBuffer);
        parameterSet->SetBuffer(BuildCandidateCountNameId, &m_candidateCountBuffer);
        parameterSet->SetBuffer(BuildMeshInstancesNameId, instanceManager->GetInstanceBuffer());
        parameterSet->SetBuffer(BuildClustersNameId, meshManager->GetClusterBuffer());
        parameterSet->SetBuffer(BuildClusterGroupsNameId, meshManager->GetClusterGroupBuffer());
        parameterSet->SetBuffer(BuildOutDrawIndirectNameId, &m_drawIndirectBuffer);
        parameterSet->SetBuffer(BuildOutDrawGroupCountNameId, &m_drawCallGroupBuffer);
        parameterSet->SetValue(BuildDrawGroupCountUniformId, static_cast<uint32_t>(groupCount));
    };

    setupParameters(m_buildDrawCallDispatcher->GetShaderParameterSet(0));

    constexpr uint32_t ThreadsPerGroup = 64;
    uint32_t           groupCountX     = (MaxCandidateClusters + ThreadsPerGroup - 1) / ThreadsPerGroup;


    primaryCommand->FillBuffer(&m_drawCallGroupBuffer, 0);
    primaryCommand->AddBufferMemoryBarrier({m_drawCallGroupBuffer.m_bufferRHI.get()}, MEBufferBarrierStage::ETransferWrite, MEBufferBarrierStage::EComputeShaderWrite);


    primaryCommand->DispatchComputeJob(m_buildDrawCallDispatcher, BuildDrawCallEntryNameId, groupCountX, 1, 1);

    if (m_drawIndirectBuffer.m_bufferRHI)
    {
        primaryCommand->AddBufferMemoryBarrier({m_drawIndirectBuffer.m_bufferRHI.get()}, MEBufferBarrierStage::EComputeShaderWrite, MEBufferBarrierStage::EDrawIndirectRead);
    }
    if (m_drawCallGroupBuffer.m_bufferRHI)
    {
        primaryCommand->AddBufferMemoryBarrier({m_drawCallGroupBuffer.m_bufferRHI.get()}, MEBufferBarrierStage::EComputeShaderWrite, MEBufferBarrierStage::EDrawIndirectRead);
    }
}

std::vector<MRenderTaskOutputDesc> MSceneCullingNode::InitOutputDesc()
{
    static const auto outputCullingResultId = MStringId("Renderer");

    return {
            MRenderTaskNodeOutput::CreateData<IRenderer>(outputCullingResultId),
    };
}

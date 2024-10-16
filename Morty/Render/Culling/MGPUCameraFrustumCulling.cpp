#include "MGPUCameraFrustumCulling.h"

#include "Utility/MGlobal.h"
#include "Batch/MMaterialBatchGroup.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Material/MComputeDispatcher.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"

using namespace morty;

void MGPUCameraFrustumCulling::Initialize(MEngine* pEngine)
{
    m_engine                     = pEngine;
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_cullingInputBuffer  = MBuffer::CreateStorageBuffer("Culling Input Buffer");
    m_drawIndirectBuffer  = MBuffer::CreateIndirectDrawBuffer("GPU Culling Draw Instance Buffer");
    m_cullingOutputBuffer = MBuffer::CreateStorageBuffer("Culling Output Buffer");

    constexpr size_t nOutputDataSize = sizeof(MMergeInstanceDrawCallOutput);
    m_cullingOutputBuffer.ReallocMemory(nOutputDataSize);
    m_cullingOutputBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nOutputDataSize);


    MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
    m_cullingComputeDispatcher   = pObjectSystem->CreateObject<MComputeDispatcher>();
    m_cullingComputeDispatcher->LoadComputeShader("Shader/Culling/cull.mcs");


    const std::shared_ptr<MShaderPropertyBlock>& params = m_cullingComputeDispatcher->GetShaderPropertyBlock(0);

    if (std::shared_ptr<MShaderStorageParam> pStorageParam =
                params->FindStorageParam(MShaderPropertyName::CULLING_INSTANCE_DATA))
    {
        pStorageParam->pBuffer = &m_cullingInputBuffer;
        pStorageParam->SetDirty();
    }

    if (std::shared_ptr<MShaderStorageParam> pStorageParam =
                params->FindStorageParam(MShaderPropertyName::CULLING_OUTPUT_DRAW_DATA))
    {
        pStorageParam->pBuffer = &m_drawIndirectBuffer;
        pStorageParam->SetDirty();
    }

    if (std::shared_ptr<MShaderStorageParam>&& pStorageParam =
                params->FindStorageParam(MShaderPropertyName::CULLING_OUTPUT_DRAW_COUNT))
    {
        pStorageParam->pBuffer = &m_cullingOutputBuffer;
        pStorageParam->SetDirty();
    }
}

void MGPUCameraFrustumCulling::Release()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    m_cullingInputBuffer.DestroyBuffer(pRenderSystem->GetDevice());
    m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
    m_cullingOutputBuffer.DestroyBuffer(pRenderSystem->GetDevice());

    m_cullingComputeDispatcher->DeleteLater();
    m_cullingComputeDispatcher = nullptr;
}

void MGPUCameraFrustumCulling::UpdateCullingCamera()
{
    const std::shared_ptr<MShaderPropertyBlock>& params = m_cullingComputeDispatcher->GetShaderPropertyBlock(0);

    std::shared_ptr<MShaderConstantParam>        pConstantParam =
            params->FindConstantParam(MShaderPropertyName::CULLING_CBUFFER_FRAME_DATA);
    if (!pConstantParam)
    {
        MORTY_ASSERT(pConstantParam);
        return;
    }

    MVariantStruct& sut = pConstantParam->var.GetValue<MVariantStruct>();
    sut.SetVariant(MShaderPropertyName::CULLING_CAMERA_POSITION, Vector4(m_cameraPosition, 1.0f));
    MVariantArray& cFrustumArray = sut.GetVariant<MVariantArray>(MShaderPropertyName::CULLING_FRUSTUM_PLANES);
    {
        for (size_t planeIdx = 0; planeIdx < 6; ++planeIdx)
        {
            const Vector4 plane = m_cameraFrustum.GetPlane(planeIdx).m_plane;
            cFrustumArray[planeIdx].SetValue(plane / Vector3(plane.x, plane.y, plane.z).Length());
        }
    }

    pConstantParam->SetDirty();
}

void MGPUCameraFrustumCulling::Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup)
{
    if (!m_command)
    {
        MORTY_ASSERT(m_command);
        return;
    }

    UpdateCullingCamera();
    m_cullingInstanceGroup.clear();


    MRenderSystem*                      pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    std::vector<MMergeInstanceCullData> vInstanceCullingData;

    auto                                createNewGroupFunc = [&](const std::shared_ptr<MMaterial>& pMaterial,
                                  MInstanceBatchGroup*              pInstanceBatchGroup) {
        size_t     nIndirectBeginIdx = vInstanceCullingData.size();
        const auto pMeshProperty     = pInstanceBatchGroup->GetMeshProperty();
        pMeshProperty->SetValue(MShaderPropertyName::MESH_INSTANCE_BEGIN_INDEX, nIndirectBeginIdx);

        m_cullingInstanceGroup.push_back({});
        MMaterialCullingGroup* pMaterialCullingGroup  = &m_cullingInstanceGroup.back();
        pMaterialCullingGroup->pMaterial              = pMaterial;
        pMaterialCullingGroup->nIndirectBeginIdx      = vInstanceCullingData.size();
        pMaterialCullingGroup->pMeshTransformProperty = pMeshProperty;
    };

    const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    for (MMaterialBatchGroup* pMaterialGroup: vInstanceGroup)
    {
        if (pMaterialGroup->GetMaterial() == nullptr)
        {
            MORTY_ASSERT(pMaterialGroup->GetMaterial());
            continue;
        }

        for (MInstanceBatchGroup* pInstanceGroup: pMaterialGroup->GetInstanceBatchGroup())
        {
            createNewGroupFunc(pMaterialGroup->GetMaterial(), pInstanceGroup);

            pInstanceGroup->InstanceExecute([&](const MMeshInstanceRenderProxy& instance, size_t nIdx) {
                MORTY_UNUSED(nIdx);

                const MMeshManager::MMeshData& data = pMeshManager->FindMesh(instance.pMesh);

                MMergeInstanceCullData         cullData;
                cullData.position = instance.boundsWithTransform.m_centerPoint;
                cullData.radius   = instance.boundsWithTransform.m_halfLength.Length();
                for (size_t nLodIdx = 0; nLodIdx < MRenderGlobal::MESH_LOD_LEVEL_RANGE; ++nLodIdx)
                {
                    //TODO LOD
                    cullData.lods[nLodIdx].distance   = 500 * (1 + nLodIdx) / MRenderGlobal::MESH_LOD_LEVEL_RANGE;
                    cullData.lods[nLodIdx].firstIndex = data.indexInfo.begin;
                    cullData.lods[nLodIdx].indexCount = data.indexInfo.size;
                }
                vInstanceCullingData.push_back(cullData);
            });

            if (!m_cullingInstanceGroup.empty())
            {
                m_cullingInstanceGroup.back().nIndirectCount =
                        vInstanceCullingData.size() - m_cullingInstanceGroup.back().nIndirectBeginIdx;
            }
        }
    }

    if (vInstanceCullingData.empty())
    {
        m_cullingInstanceGroup.clear();
        return;
    }

    const size_t unCullingBufferSize = vInstanceCullingData.size() * sizeof(MMergeInstanceCullData);
    if (m_cullingInputBuffer.GetSize() < unCullingBufferSize)
    {
        m_cullingInputBuffer.ReallocMemory(unCullingBufferSize);
        m_cullingInputBuffer.DestroyBuffer(pRenderSystem->GetDevice());
        m_cullingInputBuffer.GenerateBuffer(
                pRenderSystem->GetDevice(),
                reinterpret_cast<MByte*>(vInstanceCullingData.data()),
                unCullingBufferSize
        );
    }
    else
    {
        m_cullingInputBuffer.UploadBuffer(
                pRenderSystem->GetDevice(),
                reinterpret_cast<MByte*>(vInstanceCullingData.data()),
                unCullingBufferSize
        );
    }

    const size_t nDrawIndirectBufferSize = vInstanceCullingData.size() * sizeof(MDrawIndexedIndirectData);
    if (m_drawIndirectBuffer.GetSize() < nDrawIndirectBufferSize)
    {
        m_drawIndirectBuffer.ReallocMemory(nDrawIndirectBufferSize);
        m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
        m_drawIndirectBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nDrawIndirectBufferSize);
    }

    m_command->AddBufferMemoryBarrier(
            {&m_drawIndirectBuffer},
            MEBufferBarrierStage::EDrawIndirectRead,
            MEBufferBarrierStage::EComputeShaderWrite
    );

    m_command->DispatchComputeJob(
            m_cullingComputeDispatcher,
            static_cast<uint32_t>(vInstanceCullingData.size()) / 16 +
                    (static_cast<uint32_t>(vInstanceCullingData.size()) % 16 ? 1 : 0),
            1,
            1
    );

    m_command->AddBufferMemoryBarrier(
            {&m_drawIndirectBuffer},
            MEBufferBarrierStage::EComputeShaderWrite,
            MEBufferBarrierStage::EDrawIndirectRead
    );
}

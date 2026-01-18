#include "MFrameParamNode.h"
#include "Batch/Mesh/MMeshInstanceManager.h"
#include "Engine/MEngine.h"
#include "MFrameParameterSetAdapter.h"
#include "Material/MMaterialTemplate.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"
#include "Shader/MShaderParameterSet.h"
#include "System/MResourceSystem.h"
#include "Utility/MRenderGraphName.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MFrameParamNode, MRenderTaskNode)

// Shader parameter names for FrameData (set = 1)
static const MStringId ViewMatrixNameId     = MStringId("viewMatrix");
static const MStringId CameraPositionNameId = MStringId("positionWS");
static const MStringId InstanceProxyNameId  = MStringId("GlobalData.scene.instanceProxy");

void                   MFrameParamNode::OnCreated()
{
    Super::OnCreated();

    auto                 resourceSystem = GetEngine()->GetSystem<MResourceSystem>();

    // Create MaterialTemplate and load shader that contains GlobalFrameData
    static const MString FrameParamTemplateName = "FrameParamTemplate";
    m_materialTemplate = resourceSystem->FindResource<MMaterialTemplate>(FrameParamTemplateName);

    if (!m_materialTemplate)
    {
        m_materialTemplate = resourceSystem->CreateResource<MMaterialTemplate>(FrameParamTemplateName);

        // Load Forward shader which uses GlobalFrameData (set = 1)
        auto shaderResource = resourceSystem->LoadResource("ShaderSlang/Main/DeferredGBuffer.slang");
        m_materialTemplate->LoadShader(shaderResource);

        // Set default pass entry points
        m_materialTemplate->SetPass(
                MRenderGlobal::DEFAULT_PASS_NAME,
                MRenderGlobal::DEFAULT_VERTEX_ENTRY,
                MRenderGlobal::DEFAULT_PIXEL_ENTRY
        );
    }

    // Create ParameterSet for set = 1 (FrameData)
    // set 0 = per-object data, set 1 = per-frame data, set 2 = material data, set 3 = other
    constexpr size_t FrameDataSetIndex = 1;
    m_frameParameterSet                = m_materialTemplate->CreateParameterSet(FrameDataSetIndex);

    // Create adapter for passing through render graph
    m_adapter = std::make_shared<MFrameParameterSetAdapter>(m_frameParameterSet);
}

void MFrameParamNode::OnDelete()
{
    m_adapter           = nullptr;
    m_frameParameterSet = nullptr;
    m_materialTemplate  = nullptr;

    Super::OnDelete();
}

void MFrameParamNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    MORTY_UNUSED(primaryCommand);

    UpdateFrameParameters(info);

    // Output the adapter containing the ParameterSet
    GetRenderOutput(0)->SetData(m_adapter.get());
}

void MFrameParamNode::UpdateFrameParameters(const MRenderInfo& info)
{
    if (!m_frameParameterSet) { return; }
    auto    instanceManager = info.scene->GetManager<MMeshInstanceManager>();

    // Camera transform is world-to-view matrix
    Matrix4 viewMatrix = info.m4CameraTransform.Inverse();

    // Update camera view matrix (CameraData.viewMatrix)
    m_frameParameterSet->SetValue(ViewMatrixNameId, viewMatrix);

    // Extract camera world position from transform matrix
    Vector3 cameraPosition = info.m4CameraTransform.GetTranslation();

    // Update camera world position (CameraData.positionWS)
    m_frameParameterSet->SetValue(CameraPositionNameId, cameraPosition);

    auto instanceBuffer = instanceManager->GetInstanceBuffer();
    m_frameParameterSet->SetBuffer(InstanceProxyNameId, instanceBuffer);
}

std::vector<MRenderTaskOutputDesc> MFrameParamNode::InitOutputDesc()
{
    return {
            MRenderTaskNodeOutput::CreateData<MFrameParameterSetAdapter>(MRenderGraphName::FrameParamSet),
    };
}

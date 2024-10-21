#include "MRenderTaskNode.h"

#include "MRenderGraph.h"
#include "MRenderTaskNodeInput.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNode, MTaskNode)

METextureFormat MRenderTaskNode::DefaultLinearSpaceFormat = METextureFormat::Float_RGBA16;

void            MRenderTaskNode::OnCreated()
{
    for (const auto& input: InitInputDesc())
    {
        auto pInput = AppendInput<MRenderTaskNodeInput>();
        pInput->SetName(input.name);
        m_input[input.name] = pInput;
    }

    for (const auto& output: InitOutputDesc())
    {
        auto pOutput = AppendOutput<MRenderTaskNodeOutput>();
        pOutput->SetName(output.name);
        m_output[output.name] = pOutput;
    }

    SetThreadType(METhreadType::ERenderThread);
}

void                  MRenderTaskNode::OnDelete() { Release(); }

MEngine*              MRenderTaskNode::GetEngine() const { return static_cast<MRenderGraph*>(GetGraph())->GetEngine(); }

MRenderGraph*         MRenderTaskNode::GetRenderGraph() const { return GetGraph()->DynamicCast<MRenderGraph>(); }

MRenderTargetManager* MRenderTaskNode::GetRenderTargetManager() const
{
    return GetRenderGraph()->GetRenderTargetManager();
}

MTexturePtr MRenderTaskNode::GetInputTexture(const size_t& nIdx)
{
    return GetInput(nIdx)->GetLinkedOutput()->DynamicCast<MRenderTaskNodeOutput>()->GetTexture();
}

MTexturePtr MRenderTaskNode::GetOutputTexture(const size_t& nIdx)
{
    return GetOutput(nIdx)->DynamicCast<MRenderTaskNodeOutput>()->GetTexture();
}

MTexturePtr MRenderTaskNode::GetInputTexture(const MStringId& nIdx)
{
    auto findResult = m_input.find(nIdx);
    if (findResult == m_input.end()) { return nullptr; }

    return findResult->second->GetLinkedOutput()->DynamicCast<MRenderTaskNodeOutput>()->GetTexture();
}

MTexturePtr MRenderTaskNode::GetOutputTexture(const MStringId& nIdx)
{
    auto findResult = m_output.find(nIdx);
    if (findResult == m_output.end()) { return nullptr; }

    return findResult->second->GetTexture();
}

MRenderTaskNodeOutput* MRenderTaskNode::GetRenderOutput(const size_t& nIdx)
{
    return GetOutput(nIdx)->DynamicCast<MRenderTaskNodeOutput>();
}
flatbuffers::Offset<void> MRenderTaskNode::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    /*
    std::vector<flatbuffers::Offset<flatbuffers::String>> outputTargetName(GetOutputSize());
    for (size_t nOutputIdx = 0; nOutputIdx < GetOutputSize(); ++nOutputIdx)
    {
        auto taskOutput = GetOutput(nOutputIdx);
        if (!taskOutput) continue;
        auto renderOutput = taskOutput->DynamicCast<MRenderTaskNodeOutput>();
        if (!renderOutput) continue;

        if (auto renderTarget = renderOutput->GetRenderTarget())
        {
            outputTargetName[nOutputIdx] = fbb.CreateString(renderTarget->GetName().ToString());
        }
        else { outputTargetName[nOutputIdx] = fbb.CreateString(""); }
    }

    auto                        fbOutputTargetName = fbb.CreateVector(outputTargetName);
*/

    auto                        fbSuper = MTaskNode::Serialize(fbb);
    auto                        fbName  = fbb.CreateString(GetNodeName().c_str());

    fbs::MRenderTaskNodeBuilder builder(fbb);
    builder.add_super(fbSuper.o);
    builder.add_name(fbName);

    return builder.Finish().Union();
}
void MRenderTaskNode::Deserialize(const void* flatbuffer)
{
    const auto* fbRenderNode = reinterpret_cast<const fbs::MRenderTaskNode*>(flatbuffer);

    MTaskNode::Deserialize(fbRenderNode->super());
    m_strNodeName = MStringId(fbRenderNode->name()->c_str());

    /*
    if (!fbRenderNode->output_name()) continue;

    const size_t outputNum = fbRenderNode->output_name()->size();
    for (size_t nOutputIdx = 0; nOutputIdx < outputNum; ++nOutputIdx)
    {
        if (nOutputIdx >= GetOutputSize()) continue;

        auto outputName = fbRenderNode->output_name()->Get(nOutputIdx);
        if (!outputName) continue;
        auto renderTarget = rtManager->FindRenderTarget(MStringId(outputName->c_str()));
        if (!renderTarget) continue;

        renderNode->GetRenderOutput(nOutputIdx)->SetRenderTarget(renderTarget);
    }
*/
}

void               MRenderTaskTarget::SetTexture(const MTexturePtr& pTexture) { m_texture = pTexture; }

MRenderTaskTarget* MRenderTaskTarget::InitName(const MStringId& name)
{
    m_name = name;
    return this;
}

MRenderTaskTarget* MRenderTaskTarget::InitResizePolicy(MEResizePolicy ePolicy, float fScale, size_t nTexelSize)
{
    m_resizePolicy = ePolicy;
    m_scale        = fScale;
    m_texelSize    = nTexelSize;

    return this;
}

MRenderTaskTarget* MRenderTaskTarget::InitSharedPolicy(MESharedPolicy ePolicy)
{
    m_sharedPolicy = ePolicy;
    return this;
}

MRenderTaskTarget* MRenderTaskTarget::InitTextureDesc(const MTextureDesc& desc)
{
    m_textureDesc = desc;
    return this;
}

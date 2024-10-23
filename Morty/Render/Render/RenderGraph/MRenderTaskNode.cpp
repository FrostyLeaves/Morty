#include "MRenderTaskNode.h"
#include "Engine/MEngine.h"
#include "MRenderGraph.h"
#include "MRenderTaskNodeInput.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNode, MTaskNode)

METextureFormat MRenderTaskNode::DefaultLinearSpaceFormat = METextureFormat::Float_RGBA16;

void            MRenderTaskNode::OnCreated()
{
    for (const auto& input: InitInputDesc())
    {
        auto pInput = AppendInput<MRenderTaskNodeInput>();
        pInput->SetInputDesc(input);
    }

    for (const auto& output: InitOutputDesc())
    {
        auto pOutput = AppendOutput<MRenderTaskNodeOutput>();
        pOutput->SetOutputDesc(output);
    }

    SetThreadType(METhreadType::ERenderThread);
}

void          MRenderTaskNode::OnDelete() { Release(); }

MEngine*      MRenderTaskNode::GetEngine() const { return static_cast<MRenderGraph*>(GetGraph())->GetEngine(); }

MRenderGraph* MRenderTaskNode::GetRenderGraph() const { return GetGraph()->DynamicCast<MRenderGraph>(); }

MTexturePtr   MRenderTaskNode::GetInputTexture(const size_t& nIdx) const
{
    auto pConnOutput = GetInput(nIdx)->GetLinkedOutput();
    if (!pConnOutput) return nullptr;

    return static_cast<MRenderTaskNodeOutput*>(pConnOutput)->GetRenderTexture();
}

MTexturePtr MRenderTaskNode::GetOutputTexture(const size_t& nIdx) const
{
    return GetOutput(nIdx)->DynamicCast<MRenderTaskNodeOutput>()->GetRenderTexture();
}

MRenderTaskNodeOutput* MRenderTaskNode::GetRenderOutput(const size_t& nIdx) const
{
    return GetOutput(nIdx)->DynamicCast<MRenderTaskNodeOutput>();
}

void MRenderTaskNode::Resize(Vector2i size)
{
    const auto pDevice = GetEngine()->FindSystem<MRenderSystem>()->GetDevice();

    for (auto pOutput: m_output)
    {
        auto        pRenderOutput = static_cast<MRenderTaskNodeOutput*>(pOutput);
        auto        pTexture      = pRenderOutput->GetRenderTexture();
        const auto& desc          = pRenderOutput->GetOutputDesc();
        if (desc.allocPolicy == METextureSourceType::Allocate && desc.resizePolicy == MEResizePolicy::Scale && pTexture)
        {
            Vector2i n2TexelFormatSize{};
            n2TexelFormatSize.x = size.x * desc.scale + ((size.x % desc.texelSize) == 0 ? 0 : 1);
            n2TexelFormatSize.y = size.y * desc.scale + ((size.y % desc.texelSize) == 0 ? 0 : 1);

            pTexture->Resize(pDevice, Vector3i(n2TexelFormatSize.x, n2TexelFormatSize.y, pTexture->GetSize().z));
        }
    }
}

flatbuffers::Offset<void> MRenderTaskNode::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
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
}
bool MRenderTaskNode::IsValidRenderNode()
{
    if (m_validCacheFlag != ValidCacheFlag::EUnknow) { return m_validCacheFlag == ValidCacheFlag::EValid; }

    for (size_t idx = 0; idx < GetInputSize(); ++idx)
    {
        auto pInput = static_cast<MRenderTaskNodeInput*>(GetInput(idx));
        if (!pInput->GetInputDesc().allowEmpty)
        {
            auto pConnOutput = static_cast<MRenderTaskNodeOutput*>(pInput->GetLinkedOutput());
            if (pConnOutput == nullptr)
            {
                m_validCacheFlag = ValidCacheFlag::EInvalid;
                return false;
            }

            auto pConnNode = pConnOutput->GetTaskNode();
            if (!static_cast<MRenderTaskNode*>(pConnNode)->IsValidRenderNode())
            {
                m_validCacheFlag = ValidCacheFlag::EInvalid;
                return false;
            }
        }
    }

    m_validCacheFlag = ValidCacheFlag::EValid;
    return true;
}

void MRenderTaskNode::OnPreCompile()
{
    MTaskNode::OnPreCompile();
    m_validCacheFlag = ValidCacheFlag::EUnknow;
}

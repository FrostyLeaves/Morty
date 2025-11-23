#include "MMaterialPass.h"
#include "MMaterialTemplate.h"
#include "System/MShaderProgramSystem.h"
#include "Utility/MUtils.h"
#include "Flatbuffer/MMaterialPass_generated.h"

using namespace morty;

MMaterialPass::MMaterialPass(MMaterialTemplate* param)
    : m_template(param)
{
    m_programSystem = m_template->GetEngine()->FindSystemWeak<MShaderProgramSystem>();
}

MMaterialPass::~MMaterialPass() { ReleaseProgram(); }

void            MMaterialPass::ReleaseProgram() const
{
    if (auto system = m_programSystem.lock()) system->ReleaseShaderProgram(this);
}

IShaderProgram* MMaterialPass::GetShaderProgram() const
{
    if (auto system = m_programSystem.lock()) return system->CreateShaderProgram(this);

    return nullptr;
}

void MMaterialPass::SetEntry(const MStringId& entry, MEShaderType type)
{
    m_entryNames[static_cast<int>(type)] = entry;
    ReleaseProgram();
}

void MMaterialPass::SetBlendState(size_t renderTargetId, const MBlendState& state)
{
    if (renderTargetId >= m_blendState.size()) { m_blendState.resize(renderTargetId + 1); }
    m_blendState[renderTargetId] = state;
    ReleaseProgram();
}

void MMaterialPass::SetDepthStencilState(const MDepthStencilState& state)
{
    m_depthStencilState = state;
    ReleaseProgram();
}
void MMaterialPass::SetRenderQueue(uint32_t queue)
{
    m_renderQueue = queue;
    ReleaseProgram();
}

void MMaterialPass::SetConservativeRasterizationEnable(bool bEnable)
{
    m_conservativeRasterizationEnable = bEnable;
    ReleaseProgram();
}

void MMaterialPass::SetEnableShadingRate(bool enabled)
{
    m_shadingRateEnable = enabled;
    ReleaseProgram();
}

void MMaterialPass::SetShadingRate(Vector2i n2ShadingRate)
{
    m_shadingRate = n2ShadingRate;
    ReleaseProgram();
}

MBlendState MMaterialPass::GetBlendState(size_t renderTargetId) const
{
    if (renderTargetId < m_blendState.size()) { return m_blendState[renderTargetId]; }

    return MBlendState{};
}

flatbuffers::Offset<void> MMaterialPass::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    // Serialize pass name
    auto                                                  passNameOffset = fbb.CreateString(m_passName.c_str());

    // Serialize entry names
    std::vector<flatbuffers::Offset<flatbuffers::String>> entryNameOffsets;
    for (const auto& entryName: m_entryNames) { entryNameOffsets.push_back(fbb.CreateString(entryName.c_str())); }
    auto                                   entryNamesVector = fbb.CreateVector(entryNameOffsets);

    // Serialize blend states
    std::vector<flatbuffers::Offset<void>> blendStateOffsets;
    blendStateOffsets.reserve(m_blendState.size());
    for (const auto& blendState: m_blendState) { blendStateOffsets.push_back(blendState.Serialize(fbb)); }
    auto         blendStatesVector = fbb.CreateVector(blendStateOffsets).o;

    // Serialize depth stencil state
    auto         depthStencilStateOffset = m_depthStencilState.Serialize(fbb).o;

    // Create shading rate vector
    fbs::Vector2 shadingRateVec(static_cast<float>(m_shadingRate.x), static_cast<float>(m_shadingRate.y));

    // Create the main object
    auto         materialPassOffset = fbs::CreateMMaterialPass(
            fbb,
            passNameOffset,
            m_conservativeRasterizationEnable,
            m_shadingRateEnable,
            &shadingRateVec,
            static_cast<fbs::MECullMode>(m_cullMode),
            entryNamesVector,
            blendStatesVector,
            depthStencilStateOffset
    );

    return materialPassOffset.Union();
}

void MMaterialPass::Deserialize(const void* pBufferPointer)
{
    const auto* materialPass = fbs::GetMMaterialPass(pBufferPointer);
    if (!materialPass) return;

    // Deserialize pass name
    if (materialPass->pass_name()) { m_passName = MStringId(materialPass->pass_name()->c_str()); }

    // Deserialize conservative rasterization
    m_conservativeRasterizationEnable = materialPass->conservative_rasterization();

    // Deserialize shading rate
    m_shadingRateEnable = materialPass->shading_rate_enable();
    if (materialPass->shading_rate())
    {
        m_shadingRate.x = static_cast<int>(materialPass->shading_rate()->x());
        m_shadingRate.y = static_cast<int>(materialPass->shading_rate()->y());
    }

    // Deserialize cull mode
    m_cullMode = static_cast<MECullMode>(materialPass->cull_mode());

    // Deserialize entry names
    if (materialPass->entry_names())
    {
        const auto* entryNamesVector = materialPass->entry_names();
        size_t      count            = std::min(static_cast<size_t>(entryNamesVector->size()), m_entryNames.size());
        for (size_t i = 0; i < count; ++i)
        {
            const auto* entryName = entryNamesVector->Get(static_cast<uint32_t>(i));
            if (entryName) { m_entryNames[i] = MStringId(entryName->c_str()); }
        }
    }

    // Deserialize blend states
    if (materialPass->blend_state())
    {
        const auto* blendStatesVector = materialPass->blend_state();
        m_blendState.clear();
        m_blendState.reserve(blendStatesVector->size());

        for (uint32_t i = 0; i < blendStatesVector->size(); ++i)
        {
            const auto* fbsBlendState = blendStatesVector->Get(i);
            if (fbsBlendState)
            {
                MBlendState blendState;
                blendState.Deserialize(fbsBlendState);
                m_blendState.push_back(blendState);
            }
        }
    }

    // Deserialize depth stencil state
    if (materialPass->depth_stencil_state()) { m_depthStencilState.Deserialize(materialPass->depth_stencil_state()); }

    // Release any cached shader program since properties may have changed
    ReleaseProgram();
}

MHashCode MMaterialPass::GetHashCode() const
{
    MHashCode hash = 0;

    if (m_template) { MUtils::HashCombine(hash, m_template->GetHashCode()); }

    MUtils::HashCombine(hash, m_conservativeRasterizationEnable);
    MUtils::HashCombine(hash, m_shadingRateEnable);
    MUtils::HashCombine(hash, m_shadingRate.x);
    MUtils::HashCombine(hash, m_shadingRate.y);
    MUtils::HashCombine(hash, (int) m_cullMode);

    for (const auto& state: m_blendState) { MUtils::HashCombine(hash, state.GetHashCode()); }

    MUtils::HashCombine(hash, m_depthStencilState.GetHashCode());

    return hash;
}

MHashCode MBlendState::GetHashCode() const
{
    MHashCode hash;

    MUtils::HashCombine(hash, BlendEnable);
    MUtils::HashCombine(hash, (int) SrcColorBlend);
    MUtils::HashCombine(hash, (int) DstColorBlend);
    MUtils::HashCombine(hash, (int) ColorBlendOp);
    MUtils::HashCombine(hash, (int) SrcAlphaBlend);
    MUtils::HashCombine(hash, (int) DstAlphaBlend);
    MUtils::HashCombine(hash, (int) AlphaBlendOp);
    MUtils::HashCombine(hash, ColorWriteMask);

    return hash;
}

flatbuffers::Offset<void> MBlendState::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    return fbs::CreateMBlendState(
                   fbb,
                   BlendEnable,
                   static_cast<fbs::MEBlendFactor>(SrcColorBlend),
                   static_cast<fbs::MEBlendFactor>(DstColorBlend),
                   static_cast<fbs::MEBlendOp>(ColorBlendOp),
                   static_cast<fbs::MEBlendFactor>(SrcAlphaBlend),
                   static_cast<fbs::MEBlendFactor>(DstAlphaBlend),
                   static_cast<fbs::MEBlendOp>(AlphaBlendOp),
                   ColorWriteMask
    )
            .o;
}

void MBlendState::Deserialize(const fbs::MBlendState* fbsBlendState)
{
    if (!fbsBlendState) return;

    BlendEnable    = fbsBlendState->blend_enable();
    SrcColorBlend  = static_cast<MEBlendFactor>(fbsBlendState->src_color_blend());
    DstColorBlend  = static_cast<MEBlendFactor>(fbsBlendState->dst_color_blend());
    ColorBlendOp   = static_cast<MEBlendOp>(fbsBlendState->color_blend_op());
    SrcAlphaBlend  = static_cast<MEBlendFactor>(fbsBlendState->src_alpha_blend());
    DstAlphaBlend  = static_cast<MEBlendFactor>(fbsBlendState->dst_alpha_blend());
    AlphaBlendOp   = static_cast<MEBlendOp>(fbsBlendState->alpha_blend_op());
    ColorWriteMask = fbsBlendState->color_write_mask();
}

MHashCode MDepthStencilState::GetHashCode() const
{
    MHashCode hash;

    MUtils::HashCombine(hash, DepthTest);
    MUtils::HashCombine(hash, DepthWrite);
    MUtils::HashCombine(hash, (int) DepthFunc);
    MUtils::HashCombine(hash, (int) FrontDepthFail);
    MUtils::HashCombine(hash, (int) BackDepthFail);
    MUtils::HashCombine(hash, StencilTest);
    MUtils::HashCombine(hash, StencilReadMask);
    MUtils::HashCombine(hash, StencilWriteMask);
    MUtils::HashCombine(hash, FrontStencilRef);
    MUtils::HashCombine(hash, (int) FrontStencilFunc);
    MUtils::HashCombine(hash, (int) FrontStencilFail);
    MUtils::HashCombine(hash, (int) FrontStencilPass);
    MUtils::HashCombine(hash, BackStencilRef);
    MUtils::HashCombine(hash, (int) BackStencilFunc);
    MUtils::HashCombine(hash, (int) BackStencilFail);
    MUtils::HashCombine(hash, (int) BackStencilPass);

    return hash;
}

flatbuffers::Offset<void> MDepthStencilState::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    return fbs::CreateMDepthStencilState(
                   fbb,
                   DepthTest,
                   DepthWrite,
                   static_cast<fbs::MEDepthFunc>(DepthFunc),
                   static_cast<fbs::MEStencilOp>(FrontDepthFail),
                   static_cast<fbs::MEStencilOp>(BackDepthFail),
                   StencilTest,
                   StencilReadMask,
                   StencilWriteMask,
                   FrontStencilRef,
                   static_cast<fbs::MEDepthFunc>(FrontStencilFunc),
                   static_cast<fbs::MEStencilOp>(FrontStencilFail),
                   static_cast<fbs::MEStencilOp>(FrontStencilPass),
                   BackStencilRef,
                   static_cast<fbs::MEDepthFunc>(BackStencilFunc),
                   static_cast<fbs::MEStencilOp>(BackStencilFail),
                   static_cast<fbs::MEStencilOp>(BackStencilPass)
    )
            .o;
}

void MDepthStencilState::Deserialize(const fbs::MDepthStencilState* fbsDepthStencilState)
{
    if (!fbsDepthStencilState) return;

    DepthTest        = fbsDepthStencilState->depth_test();
    DepthWrite       = fbsDepthStencilState->depth_write();
    DepthFunc        = static_cast<MEDepthFunc>(fbsDepthStencilState->depth_func());
    FrontDepthFail   = static_cast<MEStencilOp>(fbsDepthStencilState->front_depth_fail());
    BackDepthFail    = static_cast<MEStencilOp>(fbsDepthStencilState->back_depth_fail());
    StencilTest      = fbsDepthStencilState->stencil_test();
    StencilReadMask  = fbsDepthStencilState->stencil_read_mask();
    StencilWriteMask = fbsDepthStencilState->stencil_write_mask();
    FrontStencilRef  = fbsDepthStencilState->front_stencil_ref();
    FrontStencilFunc = static_cast<MEDepthFunc>(fbsDepthStencilState->front_stencil_func());
    FrontStencilFail = static_cast<MEStencilOp>(fbsDepthStencilState->front_stencil_fail());
    FrontStencilPass = static_cast<MEStencilOp>(fbsDepthStencilState->front_stencil_pass());
    BackStencilRef   = fbsDepthStencilState->back_stencil_ref();
    BackStencilFunc  = static_cast<MEDepthFunc>(fbsDepthStencilState->back_stencil_func());
    BackStencilFail  = static_cast<MEStencilOp>(fbsDepthStencilState->back_stencil_fail());
    BackStencilPass  = static_cast<MEStencilOp>(fbsDepthStencilState->back_stencil_pass());
}
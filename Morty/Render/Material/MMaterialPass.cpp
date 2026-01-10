#include "MMaterialPass.h"
#include "MMaterialTemplate.h"
#include "System/MShaderProgramSystem.h"
#include "Utility/MUtils.h"
#include "yaml-cpp/yaml.h"
#include "Flatbuffer/MMaterialPass_generated.h"

using namespace morty;

// Helper functions for enum to/from string conversion
namespace
{
const char* EnumToString(MEBlendFactor factor)
{
    switch (factor)
    {
        case MEBlendFactor::Zero: return "Zero";
        case MEBlendFactor::One: return "One";
        case MEBlendFactor::SrcColor: return "SrcColor";
        case MEBlendFactor::OneMinusSrcColor: return "OneMinusSrcColor";
        case MEBlendFactor::DstColor: return "DstColor";
        case MEBlendFactor::OneMinusDstColor: return "OneMinusDstColor";
        case MEBlendFactor::SrcAlpha: return "SrcAlpha";
        case MEBlendFactor::OneMinusSrcAlpha: return "OneMinusSrcAlpha";
        case MEBlendFactor::DstAlpha: return "DstAlpha";
        case MEBlendFactor::OneMinusDstAlpha: return "OneMinusDstAlpha";
        case MEBlendFactor::ConstantColor: return "ConstantColor";
        case MEBlendFactor::OneMinusConstantColor: return "OneMinusConstantColor";
        case MEBlendFactor::ConstantAlpha: return "ConstantAlpha";
        case MEBlendFactor::OneMinusConstantAlpha: return "OneMinusConstantAlpha";
        default: return "Unknown";
    }
}

MEBlendFactor StringToBlendFactor(const std::string& str)
{
    if (str == "Zero") return MEBlendFactor::Zero;
    if (str == "One") return MEBlendFactor::One;
    if (str == "SrcColor") return MEBlendFactor::SrcColor;
    if (str == "OneMinusSrcColor") return MEBlendFactor::OneMinusSrcColor;
    if (str == "DstColor") return MEBlendFactor::DstColor;
    if (str == "OneMinusDstColor") return MEBlendFactor::OneMinusDstColor;
    if (str == "SrcAlpha") return MEBlendFactor::SrcAlpha;
    if (str == "OneMinusSrcAlpha") return MEBlendFactor::OneMinusSrcAlpha;
    if (str == "DstAlpha") return MEBlendFactor::DstAlpha;
    if (str == "OneMinusDstAlpha") return MEBlendFactor::OneMinusDstAlpha;
    if (str == "ConstantColor") return MEBlendFactor::ConstantColor;
    if (str == "OneMinusConstantColor") return MEBlendFactor::OneMinusConstantColor;
    if (str == "ConstantAlpha") return MEBlendFactor::ConstantAlpha;
    if (str == "OneMinusConstantAlpha") return MEBlendFactor::OneMinusConstantAlpha;
    return MEBlendFactor::One;
}

const char* EnumToString(MEBlendOp op)
{
    switch (op)
    {
        case MEBlendOp::Add: return "Add";
        case MEBlendOp::Subtract: return "Subtract";
        case MEBlendOp::ReverseSubtract: return "ReverseSubtract";
        case MEBlendOp::Min: return "Min";
        case MEBlendOp::Max: return "Max";
        default: return "Unknown";
    }
}

MEBlendOp StringToBlendOp(const std::string& str)
{
    if (str == "Add") return MEBlendOp::Add;
    if (str == "Subtract") return MEBlendOp::Subtract;
    if (str == "ReverseSubtract") return MEBlendOp::ReverseSubtract;
    if (str == "Min") return MEBlendOp::Min;
    if (str == "Max") return MEBlendOp::Max;
    return MEBlendOp::Add;
}

const char* EnumToString(MEDepthFunc func)
{
    switch (func)
    {
        case MEDepthFunc::Never: return "Never";
        case MEDepthFunc::Less: return "Less";
        case MEDepthFunc::Equal: return "Equal";
        case MEDepthFunc::LessEqual: return "LessEqual";
        case MEDepthFunc::Greater: return "Greater";
        case MEDepthFunc::NotEqual: return "NotEqual";
        case MEDepthFunc::GreaterEqual: return "GreaterEqual";
        case MEDepthFunc::Always: return "Always";
        default: return "Unknown";
    }
}

MEDepthFunc StringToDepthFunc(const std::string& str)
{
    if (str == "Never") return MEDepthFunc::Never;
    if (str == "Less") return MEDepthFunc::Less;
    if (str == "Equal") return MEDepthFunc::Equal;
    if (str == "LessEqual") return MEDepthFunc::LessEqual;
    if (str == "Greater") return MEDepthFunc::Greater;
    if (str == "NotEqual") return MEDepthFunc::NotEqual;
    if (str == "GreaterEqual") return MEDepthFunc::GreaterEqual;
    if (str == "Always") return MEDepthFunc::Always;
    return MEDepthFunc::Less;
}

const char* EnumToString(MEStencilOp op)
{
    switch (op)
    {
        case MEStencilOp::Keep: return "Keep";
        case MEStencilOp::Zero: return "Zero";
        case MEStencilOp::Replace: return "Replace";
        case MEStencilOp::Increment: return "Increment";
        case MEStencilOp::IncrementWrap: return "IncrementWrap";
        case MEStencilOp::Decrement: return "Decrement";
        case MEStencilOp::DecrementWrap: return "DecrementWrap";
        case MEStencilOp::Invert: return "Invert";
        default: return "Unknown";
    }
}

MEStencilOp StringToStencilOp(const std::string& str)
{
    if (str == "Keep") return MEStencilOp::Keep;
    if (str == "Zero") return MEStencilOp::Zero;
    if (str == "Replace") return MEStencilOp::Replace;
    if (str == "Increment") return MEStencilOp::Increment;
    if (str == "IncrementWrap") return MEStencilOp::IncrementWrap;
    if (str == "Decrement") return MEStencilOp::Decrement;
    if (str == "DecrementWrap") return MEStencilOp::DecrementWrap;
    if (str == "Invert") return MEStencilOp::Invert;
    return MEStencilOp::Keep;
}

const char* EnumToString(MECullMode mode)
{
    switch (mode)
    {
        case MECullMode::EWireframe: return "Wireframe";
        case MECullMode::ECullNone: return "CullNone";
        case MECullMode::ECullBack: return "CullBack";
        case MECullMode::ECullFront: return "CullFront";
        default: return "Unknown";
    }
}

MECullMode StringToCullMode(const std::string& str)
{
    if (str == "Wireframe") return MECullMode::EWireframe;
    if (str == "CullNone") return MECullMode::ECullNone;
    if (str == "CullBack") return MECullMode::ECullBack;
    if (str == "CullFront") return MECullMode::ECullFront;
    return MECullMode::ECullBack;
}

const char* EnumToString(MEShaderType type)
{
    switch (type)
    {
        case MEShaderType::EVertex: return "Vertex";
        case MEShaderType::EPixel: return "Pixel";
        case MEShaderType::ECompute: return "Compute";
        case MEShaderType::EGeometry: return "Geometry";
        default: return "Unknown";
    }
}

MEShaderType StringToShaderType(const std::string& str)
{
    if (str == "Vertex") return MEShaderType::EVertex;
    if (str == "Pixel") return MEShaderType::EPixel;
    if (str == "Compute") return MEShaderType::ECompute;
    if (str == "Geometry") return MEShaderType::EGeometry;
    return MEShaderType::EVertex;
}

std::string MaskToString(uint32_t mask, int bits = 4)
{
    std::string result(bits, '0');
    for (int i = 0; i < bits; ++i) { result[bits - 1 - i] = (mask & (1 << i)) ? '1' : '0'; }
    return result;
}

uint32_t StringToMask(const std::string& str)
{
    uint32_t mask = 0;
    int      len  = static_cast<int>(str.length());
    for (int i = 0; i < len; ++i)
    {
        if (str[len - 1 - i] == '1') { mask |= (1 << i); }
    }
    return mask;
}

}// namespace

MMaterialPass::~MMaterialPass() { ReleaseProgram(); }

void MMaterialPass::Initialize(morty::MMaterialTemplate* temp)
{
    MORTY_ASSERT(temp);

    m_template      = temp;
    m_programSystem = m_template->GetEngine()->FindSystemWeak<MShaderProgramSystem>();
}

void MMaterialPass::ReleaseProgram() const
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
    MHashCode hash = 0;

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
    MHashCode hash = 0;

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

YAML::Node MBlendState::SerializeYaml() const
{
    YAML::Node node;
    node["BlendEnable"]    = BlendEnable;
    node["SrcColorBlend"]  = EnumToString(SrcColorBlend);
    node["DstColorBlend"]  = EnumToString(DstColorBlend);
    node["ColorBlendOp"]   = EnumToString(ColorBlendOp);
    node["SrcAlphaBlend"]  = EnumToString(SrcAlphaBlend);
    node["DstAlphaBlend"]  = EnumToString(DstAlphaBlend);
    node["AlphaBlendOp"]   = EnumToString(AlphaBlendOp);
    node["ColorWriteMask"] = MaskToString(ColorWriteMask, 4);
    return node;
}

void MBlendState::DeserializeYaml(const YAML::Node& node)
{
    if (!node.IsMap()) return;

    if (node["BlendEnable"]) BlendEnable = node["BlendEnable"].as<bool>();
    if (node["SrcColorBlend"])
    {
        SrcColorBlend = node["SrcColorBlend"].IsScalar() ? StringToBlendFactor(node["SrcColorBlend"].as<std::string>())
                                                         : static_cast<MEBlendFactor>(node["SrcColorBlend"].as<int>());
    }
    if (node["DstColorBlend"])
    {
        DstColorBlend = node["DstColorBlend"].IsScalar() ? StringToBlendFactor(node["DstColorBlend"].as<std::string>())
                                                         : static_cast<MEBlendFactor>(node["DstColorBlend"].as<int>());
    }
    if (node["ColorBlendOp"])
    {
        ColorBlendOp = node["ColorBlendOp"].IsScalar() ? StringToBlendOp(node["ColorBlendOp"].as<std::string>())
                                                       : static_cast<MEBlendOp>(node["ColorBlendOp"].as<int>());
    }
    if (node["SrcAlphaBlend"])
    {
        SrcAlphaBlend = node["SrcAlphaBlend"].IsScalar() ? StringToBlendFactor(node["SrcAlphaBlend"].as<std::string>())
                                                         : static_cast<MEBlendFactor>(node["SrcAlphaBlend"].as<int>());
    }
    if (node["DstAlphaBlend"])
    {
        DstAlphaBlend = node["DstAlphaBlend"].IsScalar() ? StringToBlendFactor(node["DstAlphaBlend"].as<std::string>())
                                                         : static_cast<MEBlendFactor>(node["DstAlphaBlend"].as<int>());
    }
    if (node["AlphaBlendOp"])
    {
        AlphaBlendOp = node["AlphaBlendOp"].IsScalar() ? StringToBlendOp(node["AlphaBlendOp"].as<std::string>())
                                                       : static_cast<MEBlendOp>(node["AlphaBlendOp"].as<int>());
    }
    if (node["ColorWriteMask"])
    {
        ColorWriteMask = node["ColorWriteMask"].IsScalar() ? StringToMask(node["ColorWriteMask"].as<std::string>())
                                                           : node["ColorWriteMask"].as<uint32_t>();
    }
}

YAML::Node MDepthStencilState::SerializeYaml() const
{
    YAML::Node node;
    node["DepthTest"]        = DepthTest;
    node["DepthWrite"]       = DepthWrite;
    node["DepthFunc"]        = EnumToString(DepthFunc);
    node["FrontDepthFail"]   = EnumToString(FrontDepthFail);
    node["BackDepthFail"]    = EnumToString(BackDepthFail);
    node["StencilTest"]      = StencilTest;
    node["StencilReadMask"]  = MaskToString(StencilReadMask, 8);
    node["StencilWriteMask"] = MaskToString(StencilWriteMask, 8);
    node["FrontStencilRef"]  = FrontStencilRef;
    node["FrontStencilFunc"] = EnumToString(FrontStencilFunc);
    node["FrontStencilFail"] = EnumToString(FrontStencilFail);
    node["FrontStencilPass"] = EnumToString(FrontStencilPass);
    node["BackStencilRef"]   = BackStencilRef;
    node["BackStencilFunc"]  = EnumToString(BackStencilFunc);
    node["BackStencilFail"]  = EnumToString(BackStencilFail);
    node["BackStencilPass"]  = EnumToString(BackStencilPass);
    return node;
}

void MDepthStencilState::DeserializeYaml(const YAML::Node& node)
{
    if (!node.IsMap()) return;

    if (node["DepthTest"]) DepthTest = node["DepthTest"].as<bool>();
    if (node["DepthWrite"]) DepthWrite = node["DepthWrite"].as<bool>();
    if (node["DepthFunc"])
    {
        DepthFunc = node["DepthFunc"].IsScalar() ? StringToDepthFunc(node["DepthFunc"].as<std::string>())
                                                 : static_cast<MEDepthFunc>(node["DepthFunc"].as<int>());
    }
    if (node["FrontDepthFail"])
    {
        FrontDepthFail = node["FrontDepthFail"].IsScalar() ? StringToStencilOp(node["FrontDepthFail"].as<std::string>())
                                                           : static_cast<MEStencilOp>(node["FrontDepthFail"].as<int>());
    }
    if (node["BackDepthFail"])
    {
        BackDepthFail = node["BackDepthFail"].IsScalar() ? StringToStencilOp(node["BackDepthFail"].as<std::string>())
                                                         : static_cast<MEStencilOp>(node["BackDepthFail"].as<int>());
    }
    if (node["StencilTest"]) StencilTest = node["StencilTest"].as<bool>();
    if (node["StencilReadMask"])
    {
        StencilReadMask = node["StencilReadMask"].IsScalar()
                                  ? static_cast<uint8_t>(StringToMask(node["StencilReadMask"].as<std::string>()))
                                  : node["StencilReadMask"].as<uint8_t>();
    }
    if (node["StencilWriteMask"])
    {
        StencilWriteMask = node["StencilWriteMask"].IsScalar()
                                   ? static_cast<uint8_t>(StringToMask(node["StencilWriteMask"].as<std::string>()))
                                   : node["StencilWriteMask"].as<uint8_t>();
    }
    if (node["FrontStencilRef"]) FrontStencilRef = node["FrontStencilRef"].as<uint32_t>();
    if (node["FrontStencilFunc"])
    {
        FrontStencilFunc = node["FrontStencilFunc"].IsScalar()
                                   ? StringToDepthFunc(node["FrontStencilFunc"].as<std::string>())
                                   : static_cast<MEDepthFunc>(node["FrontStencilFunc"].as<int>());
    }
    if (node["FrontStencilFail"])
    {
        FrontStencilFail = node["FrontStencilFail"].IsScalar()
                                   ? StringToStencilOp(node["FrontStencilFail"].as<std::string>())
                                   : static_cast<MEStencilOp>(node["FrontStencilFail"].as<int>());
    }
    if (node["FrontStencilPass"])
    {
        FrontStencilPass = node["FrontStencilPass"].IsScalar()
                                   ? StringToStencilOp(node["FrontStencilPass"].as<std::string>())
                                   : static_cast<MEStencilOp>(node["FrontStencilPass"].as<int>());
    }
    if (node["BackStencilRef"]) BackStencilRef = node["BackStencilRef"].as<uint32_t>();
    if (node["BackStencilFunc"])
    {
        BackStencilFunc = node["BackStencilFunc"].IsScalar()
                                  ? StringToDepthFunc(node["BackStencilFunc"].as<std::string>())
                                  : static_cast<MEDepthFunc>(node["BackStencilFunc"].as<int>());
    }
    if (node["BackStencilFail"])
    {
        BackStencilFail = node["BackStencilFail"].IsScalar()
                                  ? StringToStencilOp(node["BackStencilFail"].as<std::string>())
                                  : static_cast<MEStencilOp>(node["BackStencilFail"].as<int>());
    }
    if (node["BackStencilPass"])
    {
        BackStencilPass = node["BackStencilPass"].IsScalar()
                                  ? StringToStencilOp(node["BackStencilPass"].as<std::string>())
                                  : static_cast<MEStencilOp>(node["BackStencilPass"].as<int>());
    }
}

YAML::Node MMaterialPass::SerializeYaml() const
{
    YAML::Node node;

    // Serialize pass name
    node["PassName"] = m_passName.ToString();

    // Serialize conservative rasterization
    node["ConservativeRasterization"] = m_conservativeRasterizationEnable;

    // Serialize shading rate
    node["ShadingRateEnable"] = m_shadingRateEnable;
    node["ShadingRate"]       = m_shadingRate.SerializeYaml();

    // Serialize cull mode
    node["CullMode"] = EnumToString(m_cullMode);

    // Serialize entry names as map
    node["EntryNames"] = YAML::Node(YAML::NodeType::Map);
    for (int i = 0; i < static_cast<int>(MEShaderType::TOTAL_NUM); ++i)
    {
        if (!m_entryNames[i].empty())
        {
            const char* shaderTypeName         = EnumToString(static_cast<MEShaderType>(i));
            node["EntryNames"][shaderTypeName] = m_entryNames[i].c_str();
        }
    }

    // Serialize blend states
    node["BlendStates"] = YAML::Node(YAML::NodeType::Sequence);
    for (const auto& blendState: m_blendState) { node["BlendStates"].push_back(blendState.SerializeYaml()); }

    // Serialize depth stencil state
    node["DepthStencilState"] = m_depthStencilState.SerializeYaml();

    return node;
}

void MMaterialPass::DeserializeYaml(const YAML::Node& node)
{
    if (!node.IsMap()) return;

    // Deserialize pass name
    if (node["PassName"]) { m_passName = MStringId(node["PassName"].as<std::string>()); }

    // Deserialize conservative rasterization
    if (node["ConservativeRasterization"])
    {
        m_conservativeRasterizationEnable = node["ConservativeRasterization"].as<bool>();
    }

    // Deserialize shading rate
    if (node["ShadingRateEnable"]) { m_shadingRateEnable = node["ShadingRateEnable"].as<bool>(); }
    if (node["ShadingRate"])
    {
        if (node["ShadingRate"].IsMap())
        {
            // New format: { x: 1, y: 1 }
            m_shadingRate.DeserializeYaml(node["ShadingRate"]);
        }
        else if (node["ShadingRate"].IsSequence() && node["ShadingRate"].size() >= 2)
        {
            // Old format: [1, 1] (for backward compatibility)
            m_shadingRate.x = node["ShadingRate"][0].as<int>();
            m_shadingRate.y = node["ShadingRate"][1].as<int>();
        }
    }

    // Deserialize cull mode
    if (node["CullMode"])
    {
        m_cullMode = node["CullMode"].IsScalar() ? StringToCullMode(node["CullMode"].as<std::string>())
                                                 : static_cast<MECullMode>(node["CullMode"].as<int>());
    }

    // Deserialize entry names
    if (node["EntryNames"])
    {
        if (node["EntryNames"].IsMap())
        {
            // New format: key-value pairs
            for (const auto& entry: node["EntryNames"])
            {
                const std::string typeName           = entry.first.as<std::string>();
                MEShaderType      type               = StringToShaderType(typeName);
                m_entryNames[static_cast<int>(type)] = MStringId(entry.second.as<std::string>());
            }
        }
        else if (node["EntryNames"].IsSequence())
        {
            // Old format: array (for backward compatibility)
            size_t count = std::min(node["EntryNames"].size(), m_entryNames.size());
            for (size_t i = 0; i < count; ++i) { m_entryNames[i] = MStringId(node["EntryNames"][i].as<std::string>()); }
        }
    }

    // Deserialize blend states
    if (node["BlendStates"] && node["BlendStates"].IsSequence())
    {
        m_blendState.clear();
        m_blendState.reserve(node["BlendStates"].size());
        for (const auto& blendStateNode: node["BlendStates"])
        {
            MBlendState blendState;
            blendState.DeserializeYaml(blendStateNode);
            m_blendState.push_back(blendState);
        }
    }

    // Deserialize depth stencil state
    if (node["DepthStencilState"]) { m_depthStencilState.DeserializeYaml(node["DepthStencilState"]); }

    // Release any cached shader program since properties may have changed
    ReleaseProgram();
}
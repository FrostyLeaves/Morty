/**
 * @File         MMaterialPass
 * 
 * @Created      2025-08-11 14:30:00
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Shader/MShaderMacro.h"
#include "Type/MType.h"

namespace morty
{
class IShaderProgram;
class MMaterialTemplate;
class MShaderProgramSystem;

// Stencil test function enumeration (same as depth test function)
using MEStencilFunc = MEDepthFunc;

struct MBlendState {
    bool                      BlendEnable    = true;
    MEBlendFactor             SrcColorBlend  = MEBlendFactor::One;
    MEBlendFactor             DstColorBlend  = MEBlendFactor::Zero;
    MEBlendOp                 ColorBlendOp   = MEBlendOp::Add;
    MEBlendFactor             SrcAlphaBlend  = MEBlendFactor::One;
    MEBlendFactor             DstAlphaBlend  = MEBlendFactor::Zero;
    MEBlendOp                 AlphaBlendOp   = MEBlendOp::Add;
    uint32_t                  ColorWriteMask = 0xF;

    MHashCode                 GetHashCode() const;
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const morty::fbs::MBlendState* fbsBlendState);

    YAML::Node                SerializeYaml() const;
    void                      DeserializeYaml(const YAML::Node& node);
};

struct MDepthStencilState {
    bool                      DepthTest        = true;
    bool                      DepthWrite       = true;
    MEDepthFunc               DepthFunc        = MEDepthFunc::Less;
    MEStencilOp               FrontDepthFail   = MEStencilOp::Keep;
    MEStencilOp               BackDepthFail    = MEStencilOp::Keep;
    bool                      StencilTest      = false;
    uint8_t                   StencilReadMask  = 0xFF;
    uint8_t                   StencilWriteMask = 0xFF;
    uint32_t                  FrontStencilRef  = 0;
    MEDepthFunc               FrontStencilFunc = MEDepthFunc::Always;
    MEStencilOp               FrontStencilFail = MEStencilOp::Keep;
    MEStencilOp               FrontStencilPass = MEStencilOp::Keep;
    uint32_t                  BackStencilRef   = 0;
    MEDepthFunc               BackStencilFunc  = MEDepthFunc::Always;
    MEStencilOp               BackStencilFail  = MEStencilOp::Keep;
    MEStencilOp               BackStencilPass  = MEStencilOp::Keep;

    MHashCode                 GetHashCode() const;
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const morty::fbs::MDepthStencilState* fbsDepthStencilState);

    YAML::Node                SerializeYaml() const;
    void                      DeserializeYaml(const YAML::Node& node);
};

/**
 * @brief Material Pass class - defines all states and properties for a single render pass
 */
class MORTY_API MMaterialPass : public MTypeClass
{
public:
    explicit MMaterialPass() = default;
    ~MMaterialPass() override;

public:
    [[nodiscard]] const MMaterialTemplate* GetTemplate() const { return m_template; }

    void                                   SetPassName(const MStringId& name) { m_passName = name; }
    [[nodiscard]] const MStringId&         GetPassName() const { return m_passName; }
    void                                   SetEntry(const MStringId& entry, MEShaderType type);
    [[nodiscard]] const MStringId& GetEntry(MEShaderType type) const { return m_entryNames[static_cast<int>(type)]; }
    [[nodiscard]] MEntryNames      GetEntryNames() const { return m_entryNames; }
    void                           SetCullMode(MECullMode mode) { m_cullMode = mode; }
    [[nodiscard]] MECullMode       GetCullMode() const { return m_cullMode; }
    void                           SetBlendState(size_t renderTargetId, const MBlendState& state);
    [[nodiscard]] MBlendState      GetBlendState(size_t renderTargetId) const;
    void                           SetDepthStencilState(const MDepthStencilState& state);
    [[nodiscard]] const MDepthStencilState& GetDepthStencilState() const { return m_depthStencilState; }
    void                                    SetEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool                      IsEnabled() const { return m_enabled; }
    void                                    SetRenderQueue(uint32_t queue);
    [[nodiscard]] uint32_t                  GetRenderQueue() const { return m_renderQueue; }
    [[nodiscard]] bool GetConservativeRasterizationEnable() const { return m_conservativeRasterizationEnable; }
    void               SetConservativeRasterizationEnable(bool bEnable);
    void               SetShadingRate(Vector2i n2ShadingRate);
    void               SetEnableShadingRate(bool enabled);
    [[nodiscard]] bool GetEnableShadingRate() const { return m_shadingRateEnable; }

    [[nodiscard]] IShaderProgram* GetShaderProgram() const;
    [[nodiscard]] Vector2i        GetShadingRate() const { return m_shadingRate; }
    [[nodiscard]] const char*     GetDebugName() const { return m_passName.c_str(); }
    [[nodiscard]] MHashCode       GetHashCode() const;

    flatbuffers::Offset<void>     Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                          Deserialize(const void* pBufferPointer);

    YAML::Node                    SerializeYaml() const;
    void                          DeserializeYaml(const YAML::Node& node);

    void                          Initialize(MMaterialTemplate* temp);
    void                          ReleaseProgram() const;

private:
    std::weak_ptr<MShaderProgramSystem> m_programSystem = {};
    MMaterialTemplate*                  m_template      = nullptr;
    // Basic pass information
    MStringId                           m_passName    = MRenderGlobal::DEFAULT_PASS_NAME;
    bool                                m_enabled     = true;
    uint32_t                            m_renderQueue = 2000;// Default opaque object queue

    bool                                m_conservativeRasterizationEnable = false;
    bool                                m_shadingRateEnable               = false;
    Vector2i                            m_shadingRate                     = {1, 1};

    MEntryNames                         m_entryNames;

    // Render states
    MECullMode                          m_cullMode          = MECullMode::ECullBack;
    std::vector<MBlendState>            m_blendState        = {};
    MDepthStencilState                  m_depthStencilState = {};
};

}// namespace morty

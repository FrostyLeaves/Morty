#include "MShaderMacro.h"

#include "Flatbuffer/MShaderMacro_generated.h"
#include "Mesh/MVertex.h"
#include "Utility/MFunction.h"

#include "Utility/MRenderGlobal.h"

using namespace morty;

enum class METransparentPolicy
{
    EDualDepthPeeling = 1,
};

const MString strBonesPerVertex              = MStringUtil::ToString(MRenderGlobal::BONES_PER_VERTEX);
const MString strPointLightMaxNumber         = MStringUtil::ToString(MRenderGlobal::POINT_LIGHT_MAX_NUMBER);
const MString strPointLightPixelNumber       = MStringUtil::ToString(MRenderGlobal::POINT_LIGHT_PIXEL_NUMBER);
const MString strSpotLightMaxNumber          = MStringUtil::ToString(MRenderGlobal::SPOT_LIGHT_MAX_NUMBER);
const MString strSpotLightPixelNumber        = MStringUtil::ToString(MRenderGlobal::SPOT_LIGHT_PIXEL_NUMBER);
const MString strCascadedShadowMapNumber     = MStringUtil::ToString(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
const MString strMeshLODLevelRangeNumber     = MStringUtil::ToString(MRenderGlobal::MESH_LOD_LEVEL_RANGE);
const MString strTransformInUniformMaxNumber = MStringUtil::ToString(MRenderGlobal::MESH_TRANSFORM_IN_UNIFORM_MAX_NUM);
const MString strTransparentPolicy           = MStringUtil::ToString((int) METransparentPolicy::EDualDepthPeeling);
const MString strVoxelClipMapNumber          = MStringUtil::ToString(MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM);
const MString strVoxelDiffuseConeCount       = MStringUtil::ToString(16);
const MString strVXGIEnable                  = MStringUtil::ToString(MORTY_VXGI_ENABLE);

std::unordered_map<MStringId, MString> MShaderMacro::s_vGlobalMacroParams = {
        {MStringId("MBONES_PER_VERTEX"), strBonesPerVertex},
        {MStringId("MPOINT_LIGHT_MAX_NUMBER"), strPointLightMaxNumber},
        {MStringId("MPOINT_LIGHT_PIXEL_NUMBER"), strPointLightPixelNumber},
        {MStringId("MSPOT_LIGHT_MAX_NUMBER"), strSpotLightMaxNumber},
        {MStringId("MSPOT_LIGHT_PIXEL_NUMBER"), strSpotLightPixelNumber},
        {MStringId("MTRANSPARENT_POLICY"), strTransparentPolicy},
        {MStringId("CASCADED_SHADOW_MAP_NUM"), strCascadedShadowMapNumber},
        {MStringId("MESH_LOD_LEVEL_RANGE"), strMeshLODLevelRangeNumber},
        {MStringId("MESH_TRANSFORM_IN_UNIFORM_MAX_NUM"), strTransformInUniformMaxNumber},
        {MStringId("VOXEL_GI_CLIP_MAP_NUM"), strVoxelClipMapNumber},
        {MStringId("VOXEL_DIFFUSE_CONE_COUNT"), strVoxelDiffuseConeCount},
        {MStringId("MORTY_VXGI_ENABLE"), strVXGIEnable},
};

void MShaderMacro::SetMacro(const MStringId& strKey, const MString& strValue)
{
    SetMacro(strKey, strValue, m_macroParams);
}

void MShaderMacro::SetMacro(
        const MStringId&                        strKey,
        const MString&                          strValue,
        std::unordered_map<MStringId, MString>& table
)
{
    table[strKey] = strValue;
}

void MShaderMacro::AddUnionMacro(const MStringId& strKey, const MString& strValue /*= ""*/)
{
    MORTY_ASSERT(m_macroParams.find(strKey) == m_macroParams.end());
    m_macroParams[strKey] = strValue;
}

void MShaderMacro::RemoveMacro(const MStringId& strKey) { m_macroParams.erase(strKey); }

bool MShaderMacro::HasMacro(const MStringId& strKey) const { return m_macroParams.find(strKey) != m_macroParams.end(); }

MString MShaderMacro::GetMacro(const MStringId& strKey) const
{
    const auto findResult = m_macroParams.find(strKey);
    if (findResult == m_macroParams.end()) { return MString(); }

    return findResult->second;
}

bool MShaderMacro::Compare(const MShaderMacro& macro)
{
    if (m_macroParams != macro.m_macroParams) { return false; }

    return true;
}

flatbuffers::Offset<void> MShaderMacro::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    std::vector<flatbuffers::Offset<fbs::MShaderMacroPair>> vMaterialMacroPairs;
    for (auto pairs: m_macroParams)
    {
        auto                         fbKey   = fbb.CreateString(pairs.first.ToString());
        auto                         fbValue = fbb.CreateString(pairs.second);
        fbs::MShaderMacroPairBuilder builder(fbb);
        builder.add_key(fbKey);
        builder.add_value(fbValue);
        vMaterialMacroPairs.push_back(builder.Finish().o);
    }

    const auto               fbMaterialMacro = fbb.CreateVector(vMaterialMacroPairs);
    fbs::MShaderMacroBuilder builder(fbb);

    builder.add_material_macro(fbMaterialMacro);

    return builder.Finish().Union();
}

void MShaderMacro::Deserialize(const void* pBufferPointer)
{
    const fbs::MShaderMacro* fbData = reinterpret_cast<const fbs::MShaderMacro*>(pBufferPointer);

    m_macroParams.clear();

    if (!pBufferPointer) { return; }

    if (fbData->material_macro())
    {
        for (auto pair: *fbData->material_macro())
        {
            m_macroParams[MStringId(pair->key()->c_str())] = pair->value()->c_str();
        }
    }
}

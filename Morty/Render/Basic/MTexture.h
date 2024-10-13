/**
 * @File         MTexture
 *
 * @Created      2019-09-11 16:12:31
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Math/Vector.h"
#include "RHI/Abstract/MTextureRHI.h"
#include "Flatbuffer/MTexture_generated.h"

namespace morty
{

class MIDevice;
using METextureType       = morty::fbs::METextureType;
using METextureFormat     = morty::fbs::METextureFormat;
using MEMipmapDataType    = morty::fbs::MEMipmapDataType;
using METextureWriteUsage = uint32_t;
using METextureReadUsage  = uint32_t;

class METextureWriteUsageBit
{
public:
    static constexpr uint32_t EUnknow        = 0;
    static constexpr uint32_t ERenderBack    = 1 << 1;
    static constexpr uint32_t ERenderDepth   = 1 << 2;
    static constexpr uint32_t ERenderPresent = 1 << 3;
    static constexpr uint32_t EStorageWrite  = 1 << 4;
};

class METextureReadUsageBit
{
public:
    static constexpr uint32_t EUnknow          = 0;
    static constexpr uint32_t ECpuReadable     = 1 << 1;
    static constexpr uint32_t EPixelSampler    = 1 << 2;
    static constexpr uint32_t EStorageRead     = 1 << 3;
    static constexpr uint32_t EShadingRateMask = 1 << 4;
};

struct MORTY_API MTextureDesc {
    MString             strName         = "Default";
    Vector3i            n3Size          = Vector3i(1, 1, 1);
    uint32_t            nLayer          = 1;
    METextureType       eTextureType    = METextureType::ETexture2D;
    METextureFormat     eFormat         = METextureFormat::UNorm_RGBA8;
    MEMipmapDataType    eMipmapDataType = MEMipmapDataType::Disable;
    METextureReadUsage  nReadUsage      = METextureReadUsageBit::EUnknow;
    METextureWriteUsage nWriteUsage     = METextureWriteUsageBit::EUnknow;

    MTextureDesc&       InitName(const MString& name)
    {
        strName = name;
        return *this;
    }

    MTextureDesc& InitSize(const Vector2i& n2Size)
    {
        n3Size = Vector3i(n2Size.x, n2Size.y, 1);
        return *this;
    }
};

class MORTY_API MTexture
{
public:
    MTexture();

    virtual ~MTexture();

public:
    [[nodiscard]] MString             GetName() const { return m_desc.strName; }

    [[nodiscard]] Vector3i            GetSize() const { return m_desc.n3Size; }

    [[nodiscard]] Vector2i            GetSize2D() const { return {m_desc.n3Size.x, m_desc.n3Size.y}; }

    [[nodiscard]] uint32_t            GetLayer() const { return m_desc.nLayer; }

    [[nodiscard]] METextureFormat     GetFormat() const { return m_desc.eFormat; }

    [[nodiscard]] uint32_t            GetWriteUsage() const { return m_desc.nWriteUsage; }

    [[nodiscard]] uint32_t            GetReadUsage() const { return m_desc.nReadUsage; }

    [[nodiscard]] METextureType       GetTextureType() const { return m_desc.eTextureType; }

    [[nodiscard]] MEMipmapDataType    GetMipmapDataType() const { return m_mipmapType; }

    [[nodiscard]] uint32_t            GetMipmapLevel() const { return m_mipmapLevel; }

    Vector2                           GetMipmapSize(const uint32_t& nMipmapLevel);

    [[nodiscard]] const MTextureDesc& GetTextureDesc() const { return m_desc; }

    template<typename T> T*           GetTextureRHI() const { return static_cast<T*>(m_textureRHI.get()); }

    void                              SetTextureRHI(std::unique_ptr<MTextureRHI>&& pRHITexture);

    void                              GenerateBuffer(MIDevice* pDevice);

    void                              GenerateBuffer(MIDevice* pDevice, const std::vector<std::vector<MByte>>& buffer);

    void                              DestroyBuffer(MIDevice* pDevice);

    void                              Resize(MIDevice* pDevice, const Vector2i& n2Size);

    void                              Resize(MIDevice* pDevice, const Vector3i& n3Size);

    static MTexturePtr                CreateTexture(const MTextureDesc& desc);

    static MTextureDesc               CreateDepthBuffer(const MString& name);

    static MTextureDesc CreateShadowMapArray(const MString& name, const int& nSize, const uint32_t& nArraySize);

    static MTextureDesc CreateRenderTarget(const MString& name, METextureFormat eLayout = METextureFormat::UNorm_RGBA8);

    static MTextureDesc CreateRenderTargetGBuffer(const MString& name);

    static MTextureDesc CreateTextureFbs(const morty::fbs::MTextureDesc& fbDesc);
    static flatbuffers::Offset<void> SerializeFbs(const MTextureDesc& desc, flatbuffers::FlatBufferBuilder& builder);

    static MTexturePtr               CreateRenderTargetFloat32();

    static MTextureDesc              CreateShadingRate();

    static MTexturePtr               CreateVXGIMap(Vector3i n3Size);

    static uint32_t                  GetImageMemorySize(const METextureFormat& layout);

private:
    MTextureDesc                 m_desc;

    MEMipmapDataType             m_mipmapType  = MEMipmapDataType::Disable;
    uint32_t                     m_mipmapLevel = 1;

    std::unique_ptr<MTextureRHI> m_textureRHI = nullptr;
};

}// namespace morty
/**
 * @File         MShaderResource
 * 
 * @Created      2019-08-26 20:26:13
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Resource/MResourceLoader.h"
#include "Shader/MShader.h"

namespace morty
{

class MORTY_API MShaderResourceData : public MResourceData
{
public:
    //RawData
    MEShaderType                     shaderType;
    MString                          shaderPath;
    MString                          entryName;


    void                             LoadBuffer(const std::vector<MByte>& buffer) override;

    [[nodiscard]] std::vector<MByte> SaveBuffer() const override;
};

class MORTY_API MShaderResource : public MResource
{
public:
    MORTY_CLASS(MShaderResource)

    MShaderResource() = default;

    ~MShaderResource() override = default;

public:
    MShader*                   GetShaderByIndex(const int& nIndex);

    int                        FindShaderByMacroParam(const MShaderMacro& macro);

    [[nodiscard]] MEShaderType GetShaderType() const;

    bool                       Load(std::unique_ptr<MResourceData>&& pResourceData) override;

    bool                       SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

    void                       OnDelete() override;

private:
    std::vector<MShader*>          m_shaders;
    std::unique_ptr<MResourceData> m_resourceData = nullptr;
};

class MORTY_API MShaderResourceLoader : public MResourceLoader
{
public:
    static MString              GetResourceTypeName() { return "Shader"; }

    static std::vector<MString> GetSuffixList()
    {
        return {
                MRenderGlobal::SUFFIX_VERTEX_SHADER,
                MRenderGlobal::SUFFIX_PIXEL_SHADER,
                MRenderGlobal::SUFFIX_COMPUTE_SHADER,
                MRenderGlobal::SUFFIX_GEOMETRY_SHADER,
        };
    }

    [[nodiscard]] const MType*     ResourceType() const override;

    std::unique_ptr<MResourceData> LoadResource(const MString& svFullPath) override;
};

}// namespace morty
#include "RHI/Vulkan/MVulkanShaderCompilerGlslang.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Utility/MGlobal.h"
#include "Engine/MEngine.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include "Resource/MResource.h"
#include "Utility/MFileHelper.h"
#include "Utility/MLogger.h"
#include "Utility/MString.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include <codecvt>
#include <locale>
#include <regex>
#include <string>

using namespace morty;

TBuiltInResource* GlslangDefaultResources()
{
    static TBuiltInResource Resources;

    Resources.maxLights                                 = 32;
    Resources.maxClipPlanes                             = 6;
    Resources.maxTextureUnits                           = 32;
    Resources.maxTextureCoords                          = 32;
    Resources.maxVertexAttribs                          = 64;
    Resources.maxVertexUniformComponents                = 4096;
    Resources.maxVaryingFloats                          = 64;
    Resources.maxVertexTextureImageUnits                = 32;
    Resources.maxCombinedTextureImageUnits              = 80;
    Resources.maxTextureImageUnits                      = 32;
    Resources.maxFragmentUniformComponents              = 4096;
    Resources.maxDrawBuffers                            = 32;
    Resources.maxVertexUniformVectors                   = 128;
    Resources.maxVaryingVectors                         = 8;
    Resources.maxFragmentUniformVectors                 = 16;
    Resources.maxVertexOutputVectors                    = 16;
    Resources.maxFragmentInputVectors                   = 15;
    Resources.minProgramTexelOffset                     = -8;
    Resources.maxProgramTexelOffset                     = 7;
    Resources.maxClipDistances                          = 8;
    Resources.maxComputeWorkGroupCountX                 = 65535;
    Resources.maxComputeWorkGroupCountY                 = 65535;
    Resources.maxComputeWorkGroupCountZ                 = 65535;
    Resources.maxComputeWorkGroupSizeX                  = 1024;
    Resources.maxComputeWorkGroupSizeY                  = 1024;
    Resources.maxComputeWorkGroupSizeZ                  = 64;
    Resources.maxComputeUniformComponents               = 1024;
    Resources.maxComputeTextureImageUnits               = 16;
    Resources.maxComputeImageUniforms                   = 8;
    Resources.maxComputeAtomicCounters                  = 8;
    Resources.maxComputeAtomicCounterBuffers            = 1;
    Resources.maxVaryingComponents                      = 60;
    Resources.maxVertexOutputComponents                 = 64;
    Resources.maxGeometryInputComponents                = 64;
    Resources.maxGeometryOutputComponents               = 128;
    Resources.maxFragmentInputComponents                = 128;
    Resources.maxImageUnits                             = 8;
    Resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
    Resources.maxCombinedShaderOutputResources          = 8;
    Resources.maxImageSamples                           = 0;
    Resources.maxVertexImageUniforms                    = 0;
    Resources.maxTessControlImageUniforms               = 0;
    Resources.maxTessEvaluationImageUniforms            = 0;
    Resources.maxGeometryImageUniforms                  = 0;
    Resources.maxFragmentImageUniforms                  = 8;
    Resources.maxCombinedImageUniforms                  = 8;
    Resources.maxGeometryTextureImageUnits              = 16;
    Resources.maxGeometryOutputVertices                 = 256;
    Resources.maxGeometryTotalOutputComponents          = 1024;
    Resources.maxGeometryUniformComponents              = 1024;
    Resources.maxGeometryVaryingComponents              = 64;
    Resources.maxTessControlInputComponents             = 128;
    Resources.maxTessControlOutputComponents            = 128;
    Resources.maxTessControlTextureImageUnits           = 16;
    Resources.maxTessControlUniformComponents           = 1024;
    Resources.maxTessControlTotalOutputComponents       = 4096;
    Resources.maxTessEvaluationInputComponents          = 128;
    Resources.maxTessEvaluationOutputComponents         = 128;
    Resources.maxTessEvaluationTextureImageUnits        = 16;
    Resources.maxTessEvaluationUniformComponents        = 1024;
    Resources.maxTessPatchComponents                    = 120;
    Resources.maxPatchVertices                          = 32;
    Resources.maxTessGenLevel                           = 64;
    Resources.maxViewports                              = 16;
    Resources.maxVertexAtomicCounters                   = 0;
    Resources.maxTessControlAtomicCounters              = 0;
    Resources.maxTessEvaluationAtomicCounters           = 0;
    Resources.maxGeometryAtomicCounters                 = 0;
    Resources.maxFragmentAtomicCounters                 = 8;
    Resources.maxCombinedAtomicCounters                 = 8;
    Resources.maxAtomicCounterBindings                  = 1;
    Resources.maxVertexAtomicCounterBuffers             = 0;
    Resources.maxTessControlAtomicCounterBuffers        = 0;
    Resources.maxTessEvaluationAtomicCounterBuffers     = 0;
    Resources.maxGeometryAtomicCounterBuffers           = 0;
    Resources.maxFragmentAtomicCounterBuffers           = 1;
    Resources.maxCombinedAtomicCounterBuffers           = 1;
    Resources.maxAtomicCounterBufferSize                = 16384;
    Resources.maxTransformFeedbackBuffers               = 4;
    Resources.maxTransformFeedbackInterleavedComponents = 64;
    Resources.maxCullDistances                          = 8;
    Resources.maxCombinedClipAndCullDistances           = 8;
    Resources.maxSamples                                = 4;
    Resources.maxMeshOutputVerticesNV                   = 256;
    Resources.maxMeshOutputPrimitivesNV                 = 512;
    Resources.maxMeshWorkGroupSizeX_NV                  = 32;
    Resources.maxMeshWorkGroupSizeY_NV                  = 1;
    Resources.maxMeshWorkGroupSizeZ_NV                  = 1;
    Resources.maxTaskWorkGroupSizeX_NV                  = 32;
    Resources.maxTaskWorkGroupSizeY_NV                  = 1;
    Resources.maxTaskWorkGroupSizeZ_NV                  = 1;
    Resources.maxMeshViewCountNV                        = 4;

    Resources.limits.nonInductiveForLoops                 = 1;
    Resources.limits.whileLoops                           = 1;
    Resources.limits.doWhileLoops                         = 1;
    Resources.limits.generalUniformIndexing               = 1;
    Resources.limits.generalAttributeMatrixVectorIndexing = 1;
    Resources.limits.generalVaryingIndexing               = 1;
    Resources.limits.generalSamplerIndexing               = 1;
    Resources.limits.generalVariableIndexing              = 1;
    Resources.limits.generalConstantMatrixVectorIndexing  = 1;

    return &Resources;
}

class MVulkanIncludeHandler : public glslang::TShader::Includer
{
public:
    MVulkanIncludeHandler() = default;

    void           SetSystemSearchPath(const std::vector<std::string>& paths) { m_searchPath = paths; }

    IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
    {
        MORTY_UNUSED(includerName);
        MORTY_UNUSED(inclusionDepth);

        MString* pCode = new MString();
        if (!MFileHelper::ReadString(headerName, *pCode)) return nullptr;

        glslang::TShader::Includer::IncludeResult* pResult =
                new glslang::TShader::Includer::IncludeResult(headerName, pCode->data(), pCode->length(), pCode);

        return pResult;
    }

    IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
    {
        MORTY_UNUSED(includerName);
        MORTY_UNUSED(inclusionDepth);

        MString* pCode = new MString();
        for (auto strLocalFolder: m_searchPath)
        {
            if (MFileHelper::ReadString(strLocalFolder + headerName, *pCode)) { break; }
        }

        if (pCode->size() == 0) { return nullptr; }

        glslang::TShader::Includer::IncludeResult* pResult =
                new glslang::TShader::Includer::IncludeResult(headerName, pCode->data(), pCode->length(), pCode);

        return pResult;
    }

    void releaseInclude(IncludeResult* pResult) override
    {
        if (pResult)
        {
            MString* pCode = (MString*) pResult->userData;
            delete pCode;
        }
    }

private:
    std::vector<std::string> m_searchPath;
};

MVulkanShaderCompilerGlslang::MVulkanShaderCompilerGlslang(MVulkanDevice* pDevice)
    : MVulkanShaderCompiler(pDevice)
{
    glslang::InitializeProcess();
}

MVulkanShaderCompilerGlslang::~MVulkanShaderCompilerGlslang()
{
    glslang::FinalizeProcess();// also test reference counting of users
}

bool MVulkanShaderCompilerGlslang::CompileShader(
        const MString&         strShaderPath,
        const MEShaderType&    eShaderType,
        const MShaderMacro&    macro,
        std::vector<uint32_t>& vSpirv
)
{
    static std::map<MEShaderType, EShLanguage> ShaderTypeTable = {
            {MEShaderType::EVertex, EShLangVertex},
            {MEShaderType::EPixel, EShLangFragment},
            {MEShaderType::ECompute, EShLangCompute},
            {MEShaderType::EGeometry, EShLangGeometry},
    };

    static std::map<MEShaderType, MString> ShaderEntryTable = {
            {MEShaderType::EVertex, "VS_MAIN"},
            {MEShaderType::EPixel, "PS_MAIN"},
            {MEShaderType::ECompute, "CS_MAIN"},
            {MEShaderType::EGeometry, "GS_MAIN"},
    };

    if (ShaderTypeTable.find(eShaderType) == ShaderTypeTable.end())
    {
        MORTY_ASSERT(false);
        return false;
    }

    if (ShaderEntryTable.find(eShaderType) == ShaderEntryTable.end())
    {
        MORTY_ASSERT(false);
        return false;
    }


    EShLanguage      eLanguageType = ShaderTypeTable[eShaderType];
    glslang::TShader shader        = glslang::TShader(eLanguageType);

    MString          strShaderCode;
    MFileHelper::ReadString(strShaderPath, strShaderCode);


    const char* svShaderCode = strShaderCode.c_str();
    const char* svShaderPath = strShaderPath.c_str();
    shader.setStringsWithLengthsAndNames(&svShaderCode, NULL, &svShaderPath, 1);
    shader.setEntryPoint(ShaderEntryTable[eShaderType].c_str());

    MPreamble UserPreamble;
    ConvertMacro(macro, UserPreamble);

    if (UserPreamble.IsValid()) { shader.setPreamble(UserPreamble.GetText()); }
    shader.addProcesses(UserPreamble.GetProcesses());

    shader.setNanMinMaxClamp(false);

    // 	shader.setFlattenUniformArrays((Options & EOptionFlattenUniformArrays) != 0);
    // 	if (Options & EOptionHlslIoMapping)
    // 		shader.setHlslIoMapping(true);
    //shader.setEnvTargetHlslFunctionality1();


    int         ClientInputSemanticsVersion = 120;
    EShMessages messages                    = EShMsgDefault;

    shader.setEnvInput(glslang::EShSourceHlsl, eLanguageType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
    messages = EShMessages(messages | EShMsgReadHlsl);

    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_2);

    MVulkanIncludeHandler includer;

    std::string           strShaderDirectory;
    const size_t          last_slash_idx = strShaderPath.rfind('/');
    if (std::string::npos != last_slash_idx) { strShaderDirectory = strShaderPath.substr(0, last_slash_idx); }

    includer.SetSystemSearchPath({MORTY_RESOURCE_PATH "/Shader/", strShaderDirectory + "/"});

    if (!shader.parse(GlslangDefaultResources(), 120, false, messages, includer))
    {
        GetDevice()->GetEngine()->GetLogger()->Error(
                "{}\n\n\n{}",
                MString(shader.getInfoLog()).c_str(),
                MString(shader.getInfoDebugLog()).c_str()
        );
        return false;
    }

    glslang::TProgram program;

    program.addShader(&shader);

    if (!program.link(messages)) { return false; }

    if (program.getIntermediate(eLanguageType))
    {
        spv::SpvBuildLogger logger;
        glslang::SpvOptions spvOptions;
#ifdef MORTY_DEBUG
        // crash
        //		 		spvOptions.generateDebugInfo = true;
        //		 		spvOptions.stripDebugInfo = true;
#endif
        glslang::GlslangToSpv(*program.getIntermediate(eLanguageType), vSpirv, &logger, &spvOptions);

        if (!logger.getAllMessages().empty())
        {
            GetDevice()->GetEngine()->GetLogger()->Information("{}", logger.getAllMessages().c_str());
        }

        return true;
    }

    return false;
}

#endif

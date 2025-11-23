//
// Created by DoubleYe on 2025/3/22.
//

#include "MSlangCompiler.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include "Utility/MFileHelper.h"
#include "Utility/MLogger.h"
#include "slang-com-ptr.h"

using namespace morty;
using namespace slang;

class MSlangCompilerSession : public ISlangCompilerSession {
public:
    MSlangCompilerSession() {
        if (!initialized) { createGlobalSession(session.writeRef()); }
    }

    ~MSlangCompilerSession() {
        if (initialized) { session->Release(); }
    }

    Slang::ComPtr<IGlobalSession> session;
    bool initialized = false;
};


/*
static METextureType GetTextureType(SlangResourceShape shape)
{
    switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
    {
        case SLANG_TEXTURE_2D: return METextureType::ETexture2D;
        case SLANG_TEXTURE_2D_ARRAY: return METextureType::ETexture2DArray;
        case SLANG_TEXTURE_3D: return METextureType::ETexture3D;
        case SLANG_TEXTURE_CUBE: return METextureType::ETextureCube;
        default: MORTY_ASSERT(false); return METextureType::MAX;
    }
}

static void ReflectionStorageFromSlang(VariableLayoutReflection* parameter, MShaderBuffer* shaderBuffer)
{
    auto                                 parameterType = parameter->getType();
    std::shared_ptr<MShaderStorageParam> param         = std::make_shared<MShaderStorageParam>();
    param->unSet                                       = parameter->getBindingSpace();
    param->unBinding = parameter->getOffset(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
    param->strName   = MStringId(parameter->getName());
    param->bWritable = parameterType->getResourceAccess() == SlangResourceAccess::SLANG_RESOURCE_ACCESS_READ_WRITE;
#if RENDER_GRAPHICS == MORTY_VULKAN
    param->m_vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
#endif

    shaderBuffer->m_shaderSets[param->unSet]->m_storages.push_back(param);
}

static void ReflectionResourceFromSlang(VariableLayoutReflection* parameter, MShaderBuffer* shaderBuffer)
{
    auto                                 parameterType = parameter->getType();
    std::shared_ptr<MShaderTextureParam> param         = std::make_shared<MShaderTextureParam>();
    param->unSet                                       = parameter->getBindingSpace();
    param->unBinding = parameter->getOffset(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
    param->strName   = MStringId(parameter->getName());
    param->eType     = GetTextureType(parameterType->getResourceShape());
#if RENDER_GRAPHICS == MORTY_VULKAN
    param->m_vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
#endif
    shaderBuffer->m_shaderSets[param->unSet]->m_textures.push_back(param);
}

*/


MEShaderType ConvertShaderType(SlangStage stage) {
    if (stage == SlangStage::SLANG_STAGE_VERTEX) return MEShaderType::EVertex;
    if (stage == SlangStage::SLANG_STAGE_FRAGMENT) return MEShaderType::EPixel;
    if (stage == SlangStage::SLANG_STAGE_COMPUTE) return MEShaderType::ECompute;
    if (stage == SlangStage::SLANG_STAGE_GEOMETRY) return MEShaderType::EGeometry;

    MORTY_ASSERT(false);
    return MEShaderType::EVertex;
}


static void ReflectionDefaultFromSlang(
    VariableLayoutReflection *parameter,
    MShaderBuffer *shaderBuffer,
    uint32_t reflectionDepth = 0
) {
    MORTY_UNUSED(shaderBuffer);

    MLogger logger;
    auto set = parameter->getBindingSpace();
    auto binding = parameter->getOffset(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
    auto name = parameter->getName();
    auto type = parameter->getType()->getName();
    auto offset = parameter->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);
    auto type_layout = parameter->getTypeLayout()->getName();

    auto space = std::string(reflectionDepth * 4, ' ');
    logger.Log(
        "{}name: {}, type: {}, set: {}, binding: {}, offest: {}, type layout: {}",
        space,
        name,
        type,
        set,
        binding,
        offset,
        type_layout
    );

    if (parameter->getType()->getKind() == slang::TypeReflection::Kind::ParameterBlock ||
        parameter->getType()->getKind() == slang::TypeReflection::Kind::ConstantBuffer) {
        auto elementTypeLayout = parameter->getTypeLayout()->getElementTypeLayout();
        auto fieldCount = elementTypeLayout->getFieldCount();
        for (auto idx = 0u; idx < fieldCount; ++idx) {
            ReflectionDefaultFromSlang(elementTypeLayout->getFieldByIndex(idx), shaderBuffer, reflectionDepth + 1);
        }
    }
}

static void
ReflectionDefaultFromSlang(TypeLayoutReflection *parameter, MShaderBuffer *shaderBuffer, uint32_t reflectionDepth = 0) {
    MORTY_UNUSED(shaderBuffer);

    MLogger logger;
    auto name = parameter->getName();
    auto type = parameter->getType()->getName();

    auto space = std::string(reflectionDepth * 4, ' ');
    if (nullptr != name) { logger.Log("{}name: {}, type: {}", space, name, type); }

    auto elementTypeLayout = parameter->getElementTypeLayout();
    auto fieldCount = elementTypeLayout->getFieldCount();
    for (auto idx = 0u; idx < fieldCount; ++idx) {
        ReflectionDefaultFromSlang(elementTypeLayout->getFieldByIndex(idx), shaderBuffer, reflectionDepth + 1);
    }
}

static void ReflectionDescriptorSetFromSlang(TypeLayoutReflection *typeLayout, MShaderBuffer *shaderBuffer) {
    MORTY_UNUSED(shaderBuffer);
    MLogger logger;


    int relativeSetIndex = 0;
    int descriptorRangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);
    for (int rangeIdx = 0; rangeIdx < descriptorRangeCount; ++rangeIdx) {
        slang::BindingType bindingType = typeLayout->getDescriptorSetDescriptorRangeType(relativeSetIndex, rangeIdx);
        auto descriptorCount = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(relativeSetIndex, rangeIdx);

        logger.Log("idx: {}, descriptorCount: {}, type: {}", rangeIdx, descriptorCount, (int) bindingType);
    }
}

static void ReflectionGlobalPropertyFromSlang(TypeLayoutReflection *typeLayout, MShaderBuffer *shaderBuffer) {
    MORTY_UNUSED(shaderBuffer);
    MLogger logger;

    for (int rangeIdx = 0; rangeIdx < typeLayout->getSubObjectRangeCount(); ++rangeIdx) {
        int bindingRangeIdx = typeLayout->getSubObjectRangeBindingRangeIndex(rangeIdx);
        auto bindingType = typeLayout->getBindingRangeType(bindingRangeIdx);

        logger.Log("range: {}, bindingRangeIdx: {}, bindingType: {}", rangeIdx, bindingRangeIdx, (int) bindingType);

        switch (bindingType) {
            case slang::BindingType::ParameterBlock:
            case slang::BindingType::ConstantBuffer: {
                auto parameterTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIdx);
                ReflectionDefaultFromSlang(parameterTypeLayout, shaderBuffer);
            }
            break;
            default: break;
        }
    }
}

static void ReflectionSlang(Slang::ComPtr<IComponentType> linkedProgram, MShaderBuffer *shaderBuffer) {
    if (nullptr == linkedProgram || nullptr == shaderBuffer) { return; }
    auto layout = linkedProgram->getLayout();

    auto globalScopeLayout = layout->getGlobalParamsTypeLayout();
    ReflectionDescriptorSetFromSlang(globalScopeLayout, shaderBuffer);
    ReflectionGlobalPropertyFromSlang(globalScopeLayout, shaderBuffer);

    for (auto rangeIdx = 0u; rangeIdx < globalScopeLayout->getSubObjectRangeCount(); ++rangeIdx) {
        auto bindingRangeIndex = globalScopeLayout->getSubObjectRangeBindingRangeIndex(rangeIdx);
        auto bindingType = globalScopeLayout->getBindingRangeType(bindingRangeIndex);

        switch (bindingType) {
            case slang::BindingType::ParameterBlock: {
                auto parameterBlockTypeLayout = globalScopeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                ReflectionDefaultFromSlang(parameterBlockTypeLayout, shaderBuffer);
            }
            break;
            default: break;
        }
        /*
switch (kind)
{
    case slang::TypeReflection::Kind::ShaderStorageBuffer:
        ReflectionStorageFromSlang(parameter, shaderBuffer);
        break;
    case slang::TypeReflection::Kind::Resource: ReflectionResourceFromSlang(parameter, shaderBuffer); break;
    default: ReflectionDefaultFromSlang(parameter, shaderBuffer); break;
}
 * */
    }
}

std::unique_ptr<ISlangCompilerSession> MSlangCompiler::s_globalSession = std::make_unique<MSlangCompilerSession>();

void MSlangCompiler::SetShaderPath(MStringView filePath) { m_filePath = filePath; }

bool MSlangCompiler::Compile() {
    auto globalSession = static_cast<MSlangCompilerSession *>(s_globalSession.get())->session;

    auto localFolder = MFileHelper::GetFileFolder(m_filePath);
    std::vector<std::string> searchPath = {
        MString(MORTY_RESOURCE_PATH) + "/ShaderSlang",
        localFolder,
    };
    std::vector<const char *> searchPathConst;
    searchPathConst.reserve(searchPath.size());
    for (const auto &path: searchPath) { searchPathConst.push_back(path.c_str()); }

    SessionDesc sessionDesc;
    sessionDesc.searchPaths = searchPathConst.data();
    sessionDesc.searchPathCount = searchPathConst.size();

    std::vector<TargetDesc> targets = {{.format = SLANG_SPIRV, .profile = globalSession->findProfile("sm_6_0")}};

    sessionDesc.targets = targets.data();
    sessionDesc.targetCount = targets.size();

    PreprocessorMacroDesc defineList = {"ENABLE_FANCY_FEATURE", "1"};
    sessionDesc.preprocessorMacros = &defineList;
    sessionDesc.preprocessorMacroCount = 1;

    Slang::ComPtr<ISession> session;
    auto createSessionResult = globalSession->createSession(sessionDesc, session.writeRef());
    MORTY_ASSERT(SLANG_SUCCEEDED(createSessionResult));

    auto moduleName = MFileHelper::GetFileName(m_filePath);
    Slang::ComPtr<IBlob> diagnostics;
    auto module = session->loadModule(moduleName.c_str(), diagnostics.writeRef());

    if (diagnostics) {
        MString diagnosticsLog = (const char *) diagnostics->getBufferPointer();
        INFO(diagnosticsLog.c_str());
    }
    MORTY_ASSERT(module);

    Slang::ComPtr<SlangCompileRequest> compileRequest;
    MORTY_ASSERT(SLANG_SUCCEEDED(session->createCompileRequest(compileRequest.writeRef())));


    std::vector<MString> allEntryNameArray;
    auto entryCount = module->getDefinedEntryPointCount();
    for (int32_t entryIdx = 0; entryIdx < entryCount; ++entryIdx) {
        Slang::ComPtr<IEntryPoint> entryPoint;
        module->getDefinedEntryPoint(entryIdx, entryPoint.writeRef());

        auto entryName = entryPoint->getFunctionReflection()->getName();
        allEntryNameArray.emplace_back(entryName);
    }

    std::vector<IComponentType *> components = {module};
    for (const auto &entryName: allEntryNameArray) {
        Slang::ComPtr<IEntryPoint> entryPoint;
        auto findEntryResult = module->findEntryPointByName(entryName.c_str(), entryPoint.writeRef());
        MORTY_ASSERT(SLANG_SUCCEEDED(findEntryResult));
        components.push_back(entryPoint);
    }

    Slang::ComPtr<IComponentType> program;
    MORTY_ASSERT(SLANG_SUCCEEDED(
        session->createCompositeComponentType(components.data(), components.size(), program.writeRef())
    ));

    Slang::ComPtr<IComponentType> linkedProgram;
    Slang::ComPtr<ISlangBlob> diagnosticBlob;
    MORTY_ASSERT(SLANG_SUCCEEDED(program->link(linkedProgram.writeRef(), diagnosticBlob.writeRef())));

    slang::ProgramLayout *layout = program->getLayout();
    m_output.resize(layout->getEntryPointCount());
    for (SlangUInt entryIdx = 0; entryIdx < layout->getEntryPointCount(); ++entryIdx) {
        auto entryPoint = layout->getEntryPointByIndex(entryIdx);
        MString entryName = entryPoint->getName();
        auto stage = entryPoint->getStage();

        int targetIndex = 0; // only one target
        Slang::ComPtr<IBlob> kernelBlob;
        MORTY_ASSERT(SLANG_SUCCEEDED(
            linkedProgram->getEntryPointCode(entryIdx, targetIndex, kernelBlob.writeRef(), diagnostics.writeRef())
        ));

        std::vector<uint32_t> buffer;
        buffer.resize((kernelBlob->getBufferSize() + sizeof(uint32_t) - 1) / sizeof(uint32_t));
        memcpy(buffer.data(), kernelBlob->getBufferPointer(), kernelBlob->getBufferSize());

        m_output[entryIdx].name = MStringId(entryName);
        m_output[entryIdx].type = ConvertShaderType(stage);
        m_output[entryIdx].buffer = std::move(buffer);
    }


    return true;
}

TEST_CASE("slang compile test") {
    /*
    MSlangCompiler compiler;
    compiler.SetShaderPath(MString(MORTY_RESOURCE_PATH) + "/ShaderSlang/Main/Test");
    compiler.SetModuleName("TestModule");
    compiler.EnableReflection(true);
    */
    ReflectionSlang(nullptr, nullptr);

    MSlangCompiler compiler;
    compiler.SetShaderPath(MString(MORTY_RESOURCE_PATH) + "/ShaderSlang/Main/ImGui/ImGuiModule.slang");


    CHECK(compiler.Compile());
}

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

class MSlangCompilerSession : public ISlangCompilerSession
{
public:
    MSlangCompilerSession()
    {
        if (!initialized) { createGlobalSession(session.writeRef()); }
    }

    ~MSlangCompilerSession()
    {
        if (initialized) { session->Release(); }
    }

    Slang::ComPtr<IGlobalSession> session;
    bool                          initialized = false;
};

MEShaderType ConvertShaderType(SlangStage stage)
{
    if (stage == SlangStage::SLANG_STAGE_VERTEX) return MEShaderType::EVertex;
    if (stage == SlangStage::SLANG_STAGE_FRAGMENT) return MEShaderType::EPixel;
    if (stage == SlangStage::SLANG_STAGE_COMPUTE) return MEShaderType::ECompute;
    if (stage == SlangStage::SLANG_STAGE_GEOMETRY) return MEShaderType::EGeometry;

    MORTY_ASSERT(false);
    return MEShaderType::EVertex;
}

static void ReflectionAttributes(MStringId name, slang::Attribute* attribute, MShaderPropertyBlock& output)
{
    auto attrName = attribute->getName();

    if (MString(attrName) == "Property")
    {
        size_t nameSize    = 0;
        int    paramType   = 0;
        auto   displayName = attribute->getArgumentValueString(0, &nameSize);
        attribute->getArgumentValueInt(1, &paramType);
        output.AddProperty({MStringId(displayName), name, static_cast<MShaderParamType>(paramType)});
    }
    else if (MString(attrName) == "Resource")
    {
        size_t nameSize    = 0;
        int    paramType   = 0;
        auto   displayName = attribute->getArgumentValueString(0, &nameSize);
        attribute->getArgumentValueInt(1, &paramType);
        output.AddResource({MStringId(displayName), name, static_cast<MShaderParamResourceType>(paramType)});
    }
    else if (MString(attrName) == "PerInstance")
    {

        int paramType = 0;
        attribute->getArgumentValueInt(0, &paramType);
        output.SetInstancingName(name, static_cast<MInstanceDataType>(paramType));
    }
}

static void ReflectionDefaultFromSlang(const MString& propertyBlockName, VariableLayoutReflection* parameter, MShaderPropertyBlock& output, uint32_t reflectionDepth = 0)
{
    auto name = parameter->getName();
    /*MLogger::GetInstance()->Information(
            "{} Reflecting TypeLayoutReflection name: {}",
            MStringUtil::Fill(' ', reflectionDepth * 4),
            name
    );
    */

    // Output user-defined attributes
    auto variable = parameter->getVariable();
    if (variable)
    {
        auto attributeCount = variable->getUserAttributeCount();
        if (attributeCount > 0)
        {
            //logger.Log("{}  User Attributes ({} total):", space, attributeCount);
            for (unsigned int attrIdx = 0; attrIdx < attributeCount; ++attrIdx)
            {
                auto attribute = variable->getUserAttributeByIndex(attrIdx);
                ReflectionAttributes(MStringId(propertyBlockName + "." + name), attribute, output);
            }
        }
    }

    auto typeKind = parameter->getType()->getKind();

    if (typeKind == slang::TypeReflection::Kind::ParameterBlock || typeKind == slang::TypeReflection::Kind::ConstantBuffer || typeKind == slang::TypeReflection::Kind::Resource)
    {
        auto elementTypeLayout = parameter->getTypeLayout()->getElementTypeLayout();
        auto fieldCount        = elementTypeLayout->getFieldCount();
        for (auto idx = 0u; idx < fieldCount; ++idx) { ReflectionDefaultFromSlang(propertyBlockName, elementTypeLayout->getFieldByIndex(idx), output, reflectionDepth + 1); }
    }
}

static void ReflectionParameterBlockTypeFromSlang(const MString& parameterBlockName, TypeLayoutReflection* parameter, MShaderPropertyBlock& output, uint32_t reflectionDepth = 0)
{
    if (parameter == nullptr) return;

    auto typeName = parameter->getName();
    MORTY_ASSERT(typeName);
    /*
    MLogger::GetInstance()->Information(
            "{} Reflecting TypeLayoutReflection type name: {}",
            MStringUtil::Fill(' ', reflectionDepth * 4),
            typeName
    );
    */
    if (auto type = parameter->getType())
    {
        //MLogger::GetInstance()->Information("Reflecting TypeLayoutReflection type layout: {}", type->getName());
        for (auto attrIdx = 0u; attrIdx < type->getUserAttributeCount(); ++attrIdx)
        {
            auto attribute = type->getUserAttributeByIndex(attrIdx);
            ReflectionAttributes(MStringId(parameterBlockName + "." + type->getName()), attribute, output);
        }
    }

    auto fieldCount = parameter->getFieldCount();
    for (auto idx = 0u; idx < fieldCount; ++idx) { ReflectionDefaultFromSlang(parameterBlockName, parameter->getFieldByIndex(idx), output, reflectionDepth + 1); }

    ReflectionParameterBlockTypeFromSlang(parameterBlockName, parameter->getElementTypeLayout(), output, reflectionDepth + 1);
}

static void ReflectionDescriptorSetFromSlang(TypeLayoutReflection* typeLayout, MShaderPropertyBlock& output)
{
    for (int rangeIdx = 0; rangeIdx < typeLayout->getSubObjectRangeCount(); ++rangeIdx)
    {
        int  bindingRangeIdx = typeLayout->getSubObjectRangeBindingRangeIndex(rangeIdx);
        auto bindingType     = typeLayout->getBindingRangeType(bindingRangeIdx);

        switch (bindingType)
        {
            case slang::BindingType::PushConstant: {
                // Push constants are not supported - report error early
                auto parameterTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIdx);
                auto typeName            = parameterTypeLayout ? parameterTypeLayout->getName() : "unknown";
                MLogger::GetInstance()->Error(
                        "Push constant '{}' detected in shader. "
                        "Push constants are not supported. "
                        "Use ParameterBlock<T> with [[vk::binding(N, M)]] instead of 'uniform' parameters.",
                        typeName ? typeName : "unknown"
                );
            }
            break;
            case slang::BindingType::ParameterBlock:
            case slang::BindingType::ConstantBuffer: {
                auto variable            = typeLayout->getBindingRangeLeafVariable(bindingRangeIdx);
                auto variableName        = variable->getName();
                auto parameterTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIdx);
                ReflectionParameterBlockTypeFromSlang(variableName, parameterTypeLayout, output);
            }
            break;
            default: break;
        }
    }
}

static void ReflectionSlang(const Slang::ComPtr<IComponentType>& linkedProgram, MShaderPropertyBlock& output)
{
    if (nullptr == linkedProgram) { return; }
    auto layout = linkedProgram->getLayout();

    auto globalScopeLayout = layout->getGlobalParamsTypeLayout();
    ReflectionDescriptorSetFromSlang(globalScopeLayout, output);
}

std::unique_ptr<ISlangCompilerSession> MSlangCompiler::s_globalSession = std::make_unique<MSlangCompilerSession>();

void                                   MSlangCompiler::SetShaderPath(MStringView filePath) { m_filePath = filePath; }

bool                                   MSlangCompiler::Compile()
{
    auto                     globalSession = static_cast<MSlangCompilerSession*>(s_globalSession.get())->session;

    auto                     localFolder = MFileHelper::GetFileFolder(m_filePath);
    std::vector<std::string> searchPath  = {
            MString(MORTY_RESOURCE_PATH) + "/ShaderSlang",
            localFolder,
    };
    std::vector<const char*> searchPathConst;
    searchPathConst.reserve(searchPath.size());
    for (const auto& path: searchPath) { searchPathConst.push_back(path.c_str()); }

    SessionDesc sessionDesc;
    sessionDesc.searchPaths     = searchPathConst.data();
    sessionDesc.searchPathCount = searchPathConst.size();


    std::vector<CompilerOptionEntry> compilerOptions;

#if MORTY_DEBUG
    // Emit full debug info so RenderDoc can step with proper source mapping
    compilerOptions.push_back({.name = CompilerOptionName::DebugInformation, .value = {.intValue0 = SLANG_DEBUG_INFO_LEVEL_MAXIMAL}});
    compilerOptions.push_back({.name = CompilerOptionName::DebugInformationFormat, .value = {.intValue0 = SLANG_DEBUG_INFO_FORMAT_DWARF}});
    // Disable optimization for better debugging experience
    compilerOptions.push_back({.name = CompilerOptionName::Optimization, .value = {.intValue0 = SLANG_OPTIMIZATION_LEVEL_NONE}});
    // Emit SPIRV directly from Slang IR (required for proper cross-file debug info)
    compilerOptions.push_back({.name = CompilerOptionName::EmitSpirvDirectly, .value = {.intValue0 = 1}});
    // Preserve parameters and keep OpLine for step-into
    //compilerOptions.push_back({.name = CompilerOptionName::PreserveParameters, .value = {.intValue0 = 1}});
    compilerOptions.push_back({.name = CompilerOptionName::LineDirectiveMode, .value = {.intValue0 = SLANG_LINE_DIRECTIVE_MODE_STANDARD}});
#endif

    std::vector<TargetDesc> targets = {{
            .format                   = SLANG_SPIRV,
            .profile                  = globalSession->findProfile("sm_6_0"),
            .compilerOptionEntries    = compilerOptions.empty() ? nullptr : compilerOptions.data(),
            .compilerOptionEntryCount = static_cast<uint32_t>(compilerOptions.size()),
    }};

    sessionDesc.targets     = targets.data();
    sessionDesc.targetCount = targets.size();

    PreprocessorMacroDesc defineList   = {"ENABLE_FANCY_FEATURE", "1"};
    sessionDesc.preprocessorMacros     = &defineList;
    sessionDesc.preprocessorMacroCount = 1;

    Slang::ComPtr<ISession> session;
    auto                    createSessionResult = globalSession->createSession(sessionDesc, session.writeRef());
    MORTY_ASSERT(SLANG_SUCCEEDED(createSessionResult));

    auto                 moduleName = MFileHelper::GetFileName(m_filePath);
    Slang::ComPtr<IBlob> diagnostics;
    auto                 module = session->loadModule(moduleName.c_str(), diagnostics.writeRef());

    if (diagnostics)
    {
        MString diagnosticsLog = (const char*) diagnostics->getBufferPointer();
        MLogger::GetInstance()->Information(diagnosticsLog.c_str());
    }
    if (module == nullptr) { return false; }

    Slang::ComPtr<SlangCompileRequest> compileRequest;
    MORTY_ASSERT(SLANG_SUCCEEDED(session->createCompileRequest(compileRequest.writeRef())));


    std::vector<MString> allEntryNameArray;
    auto                 entryCount = module->getDefinedEntryPointCount();
    for (int32_t entryIdx = 0; entryIdx < entryCount; ++entryIdx)
    {
        Slang::ComPtr<IEntryPoint> entryPoint;
        module->getDefinedEntryPoint(entryIdx, entryPoint.writeRef());

        auto entryName = entryPoint->getFunctionReflection()->getName();
        allEntryNameArray.emplace_back(entryName);
    }

    std::vector<IComponentType*> components = {module};
    for (const auto& entryName: allEntryNameArray)
    {
        Slang::ComPtr<IEntryPoint> entryPoint;
        auto                       findEntryResult = module->findEntryPointByName(entryName.c_str(), entryPoint.writeRef());
        MORTY_ASSERT(SLANG_SUCCEEDED(findEntryResult));
        components.push_back(entryPoint);
    }

    Slang::ComPtr<IComponentType> program;
    MORTY_ASSERT(SLANG_SUCCEEDED(session->createCompositeComponentType(components.data(), components.size(), program.writeRef())));

    Slang::ComPtr<IComponentType> linkedProgram;
    Slang::ComPtr<ISlangBlob>     diagnosticBlob;
    MORTY_ASSERT(SLANG_SUCCEEDED(program->link(linkedProgram.writeRef(), diagnosticBlob.writeRef())));

    slang::ProgramLayout* layout = program->getLayout();
    m_output.resize(layout->getEntryPointCount());
    for (SlangUInt entryIdx = 0; entryIdx < layout->getEntryPointCount(); ++entryIdx)
    {
        auto                 entryPoint = layout->getEntryPointByIndex(entryIdx);
        MString              entryName  = entryPoint->getName();
        auto                 stage      = entryPoint->getStage();

        int                  targetIndex = 0;// only one target
        Slang::ComPtr<IBlob> kernelBlob;
        if (!SLANG_SUCCEEDED(linkedProgram->getEntryPointCode(entryIdx, targetIndex, kernelBlob.writeRef(), diagnostics.writeRef())))
        {
            MLogger::GetInstance()->Error("Failed to get entry point code for {}, diagnostics: {}", entryName, diagnostics ? (const char*) diagnostics->getBufferPointer() : "none");
            return false;
        }

        std::vector<uint32_t> buffer;
        buffer.resize((kernelBlob->getBufferSize() + sizeof(uint32_t) - 1) / sizeof(uint32_t));
        memcpy(buffer.data(), kernelBlob->getBufferPointer(), kernelBlob->getBufferSize());

        m_output[entryIdx].name   = MStringId(entryName);
        m_output[entryIdx].type   = ConvertShaderType(stage);
        m_output[entryIdx].buffer = std::move(buffer);

    }

    //MLogger::GetInstance()->Log("==== Slang Reflection Result ====");
    ReflectionSlang(linkedProgram, m_reflection);
    //MLogger::GetInstance()->Log("==== Slang Reflection End ====");

    return true;
}

TEST_CASE("slang compile test")
{
    MSlangCompiler compiler;
    compiler.SetShaderPath(MString(MORTY_RESOURCE_PATH) + "/ShaderSlang/Main/DeferredGBuffer.slang");

    //auto reflection = compiler.GetReflection();

    CHECK(compiler.Compile());
}

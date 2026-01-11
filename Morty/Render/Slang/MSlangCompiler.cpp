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
        output.AddProperty(name, {displayName, static_cast<MShaderParamType>(paramType)});
    }
    else if (MString(attrName) == "Resource")
    {
        size_t nameSize    = 0;
        int    paramType   = 0;
        auto   displayName = attribute->getArgumentValueString(0, &nameSize);
        attribute->getArgumentValueInt(1, &paramType);
        output.AddResource(name, {displayName, static_cast<MShaderParamResourceType>(paramType)});
    }
    else if (MString(attrName) == "MeshInstance") { output.SetInstancingName(name); }
}

static void ReflectionDefaultFromSlang(
        VariableLayoutReflection* parameter,
        MShaderPropertyBlock&     output,
        uint32_t                  reflectionDepth = 0
)
{
    auto name = parameter->getName();

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
                ReflectionAttributes(MStringId(name), attribute, output);
            }
        }
    }

    if (parameter->getType()->getKind() == slang::TypeReflection::Kind::ParameterBlock ||
        parameter->getType()->getKind() == slang::TypeReflection::Kind::ConstantBuffer)
    {
        auto elementTypeLayout = parameter->getTypeLayout()->getElementTypeLayout();
        auto fieldCount        = elementTypeLayout->getFieldCount();
        for (auto idx = 0u; idx < fieldCount; ++idx)
        {
            ReflectionDefaultFromSlang(elementTypeLayout->getFieldByIndex(idx), output, reflectionDepth + 1);
        }
    }
}

static void
ReflectionDefaultFromSlang(TypeLayoutReflection* parameter, MShaderPropertyBlock& output, uint32_t reflectionDepth = 0)
{
    if (parameter == nullptr) return;

    if (auto type = parameter->getType())
    {
        //MLogger::GetInstance()->Information("Reflecting TypeLayoutReflection type layout: {}", type->getName());
        for (auto attrIdx = 0u; attrIdx < type->getUserAttributeCount(); ++attrIdx)
        {
            auto attribute = type->getUserAttributeByIndex(attrIdx);
            ReflectionAttributes(MStringId(type->getName()), attribute, output);
        }
    }

    auto fieldCount = parameter->getFieldCount();
    for (auto idx = 0u; idx < fieldCount; ++idx)
    {
        ReflectionDefaultFromSlang(parameter->getFieldByIndex(idx), output, reflectionDepth + 1);
    }

    ReflectionDefaultFromSlang(parameter->getElementTypeLayout(), output, reflectionDepth + 1);
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
                auto parameterTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIdx);
                ReflectionDefaultFromSlang(parameterTypeLayout, output);
            }
            break;
            default: break;
        }
    }

    auto setCount = typeLayout->getDescriptorSetCount();
    for (int relativeSetIndex = 0; relativeSetIndex < setCount; ++relativeSetIndex)
    {
        int descriptorRangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(relativeSetIndex);
        for (int rangeIdx = 0; rangeIdx < descriptorRangeCount; ++rangeIdx)
        {
            slang::BindingType bindingType =
                    typeLayout->getDescriptorSetDescriptorRangeType(relativeSetIndex, rangeIdx);

            switch (bindingType)
            {
                case slang::BindingType::PushConstant: {
                    // Push constants are not supported - report error early
                    MLogger::GetInstance()->Error(
                            "Push constant detected in descriptor set {} range {}. "
                            "Push constants are not supported. "
                            "Use ParameterBlock<T> with [[vk::binding(N, M)]] instead of 'uniform' parameters.",
                            relativeSetIndex,
                            rangeIdx
                    );
                }
                break;
                case slang::BindingType::RawBuffer: {
                    // RawBuffer is for StructuredBuffer and RWStructuredBuffer
                    ReflectionDefaultFromSlang(typeLayout, output);
                }
                break;
                case slang::BindingType::Texture:
                case slang::BindingType::Sampler: {
                    // For standalone textures/samplers, we need to get the corresponding variable
                    // Get the variable layout for this binding range
                    auto fieldCount = typeLayout->getFieldCount();
                    for (unsigned int fieldIdx = 0; fieldIdx < fieldCount; ++fieldIdx)
                    {
                        auto fieldLayout            = typeLayout->getFieldByIndex(fieldIdx);
                        auto fieldBindingRangeCount = fieldLayout->getCategoryCount();

                        // Check if this field corresponds to our binding range
                        for (unsigned int catIdx = 0; catIdx < fieldBindingRangeCount; ++catIdx)
                        {
                            auto category = fieldLayout->getCategoryByIndex(catIdx);
                            if (static_cast<SlangParameterCategory>(category) ==
                                SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT)
                            {
                                auto bindingIndex =
                                        fieldLayout->getOffset(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);

                                // Find matching binding
                                auto rangeBindingIndex = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
                                        relativeSetIndex,
                                        rangeIdx
                                );

                                if (static_cast<SlangInt>(bindingIndex) == rangeBindingIndex)
                                {
                                    ReflectionDefaultFromSlang(fieldLayout, output, 0);
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
                default: break;
            }
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

    std::vector<TargetDesc> targets = {{.format = SLANG_SPIRV, .profile = globalSession->findProfile("sm_6_0")}};

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
        auto findEntryResult = module->findEntryPointByName(entryName.c_str(), entryPoint.writeRef());
        MORTY_ASSERT(SLANG_SUCCEEDED(findEntryResult));
        components.push_back(entryPoint);
    }

    Slang::ComPtr<IComponentType> program;
    MORTY_ASSERT(SLANG_SUCCEEDED(
            session->createCompositeComponentType(components.data(), components.size(), program.writeRef())
    ));

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
        MORTY_ASSERT(SLANG_SUCCEEDED(
                linkedProgram->getEntryPointCode(entryIdx, targetIndex, kernelBlob.writeRef(), diagnostics.writeRef())
        ));

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

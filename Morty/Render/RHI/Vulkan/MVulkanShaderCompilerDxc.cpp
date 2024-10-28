#include "RHI/Vulkan/MVulkanShaderCompilerDxc.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Utility/MGlobal.h"
#include "Engine/MEngine.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include "Resource/MResource.h"
#include "Utility/MFileHelper.h"
#include "Utility/MLogger.h"
#include "Utility/MString.h"

#include <codecvt>
#include <dxcapi.h>
#include <locale>
#include <regex>
#include <string>
#include <windows.h>

using namespace morty;

struct MDxcIncludeHandler : public IDxcIncludeHandler {
    MDxcIncludeHandler(IDxcUtils* pUtils, IDxcIncludeHandler* pDefaultIncludeHandler)
        : m_utils(pUtils)
        , m_includeHandler(pDefaultIncludeHandler)
    {}

    virtual ~MDxcIncludeHandler() = default;

    void SetLocalPath(const std::wstring& strShaderDir) { m_strShaderDir = strShaderDir; }

    void SetSystemSearchPath(const std::vector<std::wstring>& paths) { m_searchPath = paths; }

    HRESULT STDMETHODCALLTYPE
    LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
    {
        IDxcBlobEncoding* pEncoding = nullptr;

        auto              pRelativeFileName = pFilename + m_strShaderDir.size();

        HRESULT           hr = MGlobal::M_INVALID_UINDEX;
        for (const auto& searchPath: m_searchPath)
        {
            auto fullPath = searchPath + pRelativeFileName;
            hr            = m_utils->LoadFile(fullPath.c_str(), nullptr, &pEncoding);
            if (SUCCEEDED(hr))
            {
                *ppIncludeSource = pEncoding;
                return hr;
            }
        }

        *ppIncludeSource = nullptr;
        return hr;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
    {
        return m_includeHandler->QueryInterface(riid, ppvObject);
    }

    ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }

    ULONG STDMETHODCALLTYPE Release(void) override { return 0; }

private:
    IDxcUtils*                m_utils;
    IDxcIncludeHandler*       m_includeHandler;
    std::vector<std::wstring> m_searchPath;
    std::wstring              m_strShaderDir;
};

MVulkanShaderCompilerDxc::MVulkanShaderCompilerDxc(MVulkanDevice* pDevice)
    : MVulkanShaderCompiler(pDevice)
{}

bool MVulkanShaderCompilerDxc::CompileShader(
        const MString&         _strShaderPath,
        const MEShaderType&    eShaderType,
        const MShaderMacro&    macro,
        std::vector<uint32_t>& vSpirv
)
{
    MString strShaderPath     = MFileHelper::FormatPath(_strShaderPath);
    auto    strShaderLocalDir = MStringUtil::ConvertToWString(MFileHelper::GetFileFolder(strShaderPath) + "/");


    GetDevice()->GetEngine()->GetLogger()->Log("compile file: \n{}", _strShaderPath.c_str());

    IDxcUtils* pUtils = nullptr;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));


    IDxcCompiler3* pCompiler = nullptr;
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

    // Create default include handler. (You can create your own...)
    //
    IDxcIncludeHandler* pDefaultIncludeHandler = nullptr;
    pUtils->CreateDefaultIncludeHandler(&pDefaultIncludeHandler);


    std::shared_ptr<MDxcIncludeHandler> pIncludeHandler =
            std::make_shared<MDxcIncludeHandler>(pUtils, pDefaultIncludeHandler);
    pIncludeHandler->SetSystemSearchPath(
            {strShaderLocalDir, MStringUtil::ConvertToWString(MORTY_RESOURCE_PATH) + L"/Shader/"}
    );
    pIncludeHandler->SetLocalPath(strShaderLocalDir);


    std::wstring              wstrShaderPath = MStringUtil::ConvertToWString(strShaderPath);

    std::vector<std::wstring> vCompArgs;
    vCompArgs.push_back(wstrShaderPath);

    //Macro
    MPreamble UserPreamble;
    ConvertMacro(macro, UserPreamble);
    if (UserPreamble.IsValid())
    {
        for (const auto& m: macro.s_vGlobalMacroParams)
        {
            if (m.second.empty())
                vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString()));
            else
                vCompArgs.push_back(
                        std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString() + "=" + m.second)
                );
        }

        for (const auto& m: macro.m_mortyMacroParams)
        {
            if (m.second.empty())
                vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString()));
            else
                vCompArgs.push_back(
                        std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString() + "=" + m.second)
                );
        }

        for (const auto& m: macro.m_macroParams)
        {
            if (m.second.empty())
                vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString()));
            else
                vCompArgs.push_back(
                        std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString() + "=" + m.second)
                );
        }
    }

    if (MEShaderType::EVertex == eShaderType)
    {
        vCompArgs.push_back(L"-E VS_MAIN");
        vCompArgs.push_back(L"-T vs_6_1");
    }
    else if (MEShaderType::EPixel == eShaderType)
    {
        vCompArgs.push_back(L"-E PS_MAIN");
        vCompArgs.push_back(L"-T ps_6_1");
    }
    else if (MEShaderType::ECompute == eShaderType)
    {
        vCompArgs.push_back(L"-E CS_MAIN");
        vCompArgs.push_back(L"-T cs_6_1");
    }
    else if (MEShaderType::EGeometry == eShaderType)
    {
        vCompArgs.push_back(L"-E GS_MAIN");
        vCompArgs.push_back(L"-T gs_6_1");
    }
    else { MORTY_ASSERT(false); }

    //vCompArgs.push_back(L"-enable-templates");

    vCompArgs.push_back(L"-spirv");
    vCompArgs.push_back(L"-fspv-target-env=vulkan1.2");
    vCompArgs.push_back(L"-fspv-extension=SPV_NV_ray_tracing");
    vCompArgs.push_back(L"-fspv-extension=SPV_KHR_multiview");
    vCompArgs.push_back(L"-fspv-extension=SPV_KHR_shader_draw_parameters");
    vCompArgs.push_back(L"-fspv-extension=SPV_EXT_descriptor_indexing");


    //	vCompArgs.push_back(L"-fspv-reflect");
    //	vCompArgs.push_back(L"-fspv-extension=SPV_GOOGLE_user_type");
    //	vCompArgs.push_back(L"-fspv-extension=SPV_GOOGLE_hlsl_functionality1");

#if MORTY_DEBUG
    vCompArgs.push_back(L"-Od");
    vCompArgs.push_back(L"-Zi");
    vCompArgs.push_back(L"-fspv-extension=SPV_KHR_non_semantic_info");
    vCompArgs.push_back(L"-fspv-debug=vulkan-with-source");
    vCompArgs.push_back(L"-fspv-debug=rich-with-source");
#else
    vCompArgs.push_back(L"-Zs");
    vCompArgs.push_back(L"-Oconfig="
                        "--ccp,"
                        "--cfg-cleanup,"
                        "--convert-local-access-chains,"
                        "--copy-propagate-arrays,"
                        "--eliminate-dead-branches,"
                        //		"--eliminate-dead-code-aggressive,"		//it will remove unused (binding,set)
                        "--eliminate-dead-functions,"
                        "--eliminate-local-multi-store,"
                        "--eliminate-local-single-block,"
                        "--eliminate-local-single-store,"
                        "--flatten-decorations,"
                        "--if-conversion,"
                        "--inline-entry-points-exhaustive,"
                        "--local-redundancy-elimination,");
#endif


    //
    // Open source file.
    //
    IDxcBlobEncoding* pSource = nullptr;
    pUtils->LoadFile(wstrShaderPath.c_str(), nullptr, &pSource);

    DxcBuffer Source;
    Source.Ptr      = pSource->GetBufferPointer();
    Source.Size     = pSource->GetBufferSize();
    Source.Encoding = DXC_CP_ACP;// Assume BOM says UTF8 or UTF16 or this is ANSI text.

    LPCWSTR* pszArgs = new LPCWSTR[vCompArgs.size()];

    for (size_t i = 0; i < vCompArgs.size(); ++i) { pszArgs[i] = vCompArgs[i].c_str(); };

    IDxcResult* pResults = nullptr;
    HRESULT     hrCompile =
            pCompiler->Compile(&Source, pszArgs, vCompArgs.size(), pIncludeHandler.get(), IID_PPV_ARGS(&pResults));

    delete[] pszArgs;

    IDxcBlobUtf8* pErrors = nullptr;
    pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
    if (pErrors != nullptr && pErrors->GetStringLength() != 0)
    {
        MORTY_ASSERT(pErrors);
        GetDevice()->GetEngine()->GetLogger()->Information("hlsl compile output: \n{}", pErrors->GetStringPointer());
    }

    HRESULT hrStatus;
    pResults->GetStatus(&hrStatus);
    if (FAILED(hrCompile))
    {
        GetDevice()->GetEngine()->GetLogger()->Error("Compilation Failed.");
        return false;
    }

    IDxcBlob*      pShader         = nullptr;
    IDxcBlobUtf16* pShaderNameTemp = nullptr;
    pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderNameTemp);
    std::shared_ptr<IDxcBlobUtf16> pShaderName(pShaderNameTemp);

    if (!pShader) return false;

    vSpirv.resize(pShader->GetBufferSize() / sizeof(uint32_t));
    memcpy(vSpirv.data(), pShader->GetBufferPointer(), pShader->GetBufferSize());


    return true;
}

#endif

#include "RHI/Vulkan/MVulkanPipelineManager.h"

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "MVulkanPhysicalDevice.h"
#include "RHI/MRenderPass.h"
#include "vulkan/vulkan_core.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Engine/MEngine.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include "Utility/MFunction.h"

#include "Material/MComputeDispatcher.h"
#include "Material/MMaterial.h"
#include "RHI/Vulkan/MTextureRHIVulkan.h"

using namespace morty;

MVulkanPipelineManager::MVulkanPipelineManager(MVulkanDevice* pDevice)
    : m_device(pDevice)
{}

MVulkanPipelineManager::~MVulkanPipelineManager() {}

void MVulkanPipelineManager::Release()
{
    for (auto& pr: m_pipelineTable)
    {
        if (pr.second) { DestroyPipeline(pr.second); }
    }

    m_pipelineTable.clear();

    for (auto& pr: m_defaultTexture)
    {
        if (pr.second) { pr.second->DestroyBuffer(m_device); }
    }

    m_defaultTexture.clear();
}

std::shared_ptr<MGraphicsPipeline>
MVulkanPipelineManager::FindOrCreateGraphicsPipeline(const MMaterialTemplate* pMaterial, const MRenderPass* pRenderPass)
{
    MPipelineKey key(pMaterial->GetShaderProgram(), pRenderPass);

    auto         findResult = m_pipelineTable.find(key);
    if (findResult != m_pipelineTable.end())
    {
        return std::dynamic_pointer_cast<MGraphicsPipeline>(findResult->second);
    }

    const std::shared_ptr<MShaderProgram>& pShaderProgram = pMaterial->GetShaderProgram();

    if (!pShaderProgram)
    {
        MORTY_ASSERT(pShaderProgram);
        return nullptr;
    }

    std::shared_ptr<MGraphicsPipeline> pPipeline = std::make_shared<MGraphicsPipeline>();
    m_pipelineTable[key]                         = pPipeline;

    if (pRenderPass->m_subpass.empty())
    {
        pPipeline->m_subpassPipeline.push_back(CreateGraphicsPipeline(pPipeline, pMaterial, pRenderPass, 0));
        return pPipeline;
    }

    for (size_t nSubPassIdx = 0; nSubPassIdx < pRenderPass->m_subpass.size(); ++nSubPassIdx)
    {
        pPipeline->m_subpassPipeline.push_back(
                CreateGraphicsPipeline(pPipeline, pMaterial, pRenderPass, static_cast<uint32_t>(nSubPassIdx))
        );
    }

    return pPipeline;
}

std::shared_ptr<MComputePipeline>
MVulkanPipelineManager::FindOrCreateComputePipeline(MComputeDispatcher* pComputeDispatcher)
{
    MPipelineKey key(pComputeDispatcher->GetShaderProgram(), nullptr);

    auto         findResult = m_pipelineTable.find(key);
    if (findResult != m_pipelineTable.end()) { return std::dynamic_pointer_cast<MComputePipeline>(findResult->second); }

    const std::shared_ptr<MShaderProgram>& pShaderProgram = pComputeDispatcher->GetShaderProgram();

    if (!pShaderProgram)
    {
        MORTY_ASSERT(pShaderProgram);
        return nullptr;
    }

    std::shared_ptr<MComputePipeline> pPipeline = std::make_shared<MComputePipeline>();
    m_pipelineTable[key]                        = pPipeline;

    pPipeline->m_vkPipeline = CreateComputePipeline(pPipeline, pComputeDispatcher);

    return pPipeline;
}

void MVulkanPipelineManager::DestroyRenderPass(MRenderPass* pRenderPass)
{
    for (auto iter = m_pipelineTable.begin(); iter != m_pipelineTable.end();)
    {
        if (iter->first == pRenderPass)
        {
            DestroyPipeline(iter->second);
            iter = m_pipelineTable.erase(iter);
        }
        else { ++iter; }
    }
}

void MVulkanPipelineManager::DestroyPipeline(const std::shared_ptr<MPipeline>& pipeline)
{
    if (!pipeline) { return; }

    if (std::shared_ptr<MGraphicsPipeline> graphicsPipeline = std::dynamic_pointer_cast<MGraphicsPipeline>(pipeline))
    {
        DestroyGraphicsPipeline(graphicsPipeline);
    }
    if (std::shared_ptr<MComputePipeline> graphicsPipeline = std::dynamic_pointer_cast<MComputePipeline>(pipeline))
    {
        DestroyComputePipeline(graphicsPipeline);
    }

    for (const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock: pipeline->m_shaderPropertyBlocks)
    {
        DestroyShaderPropertyBlockImpl(pPropertyBlock);
    }

    DestroyPipelineLayout(pipeline);

    pipeline->m_shaderPropertyBlocks.clear();
}

VkShaderStageFlags GetShaderStageFlags(MShaderProgram* program)
{
    VkShaderStageFlags                                                                           result = 0;

    static const std::array<VkShaderStageFlagBits, static_cast<size_t>(MEShaderType::TOTAL_NUM)> ShaderStageTable = {
            VK_SHADER_STAGE_VERTEX_BIT,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_SHADER_STAGE_COMPUTE_BIT,
            VK_SHADER_STAGE_GEOMETRY_BIT,
    };

    for (size_t nIdx = 0; nIdx < static_cast<size_t>(MEShaderType::TOTAL_NUM); ++nIdx)
    {
        if (program->GetShader(static_cast<MEShaderType>(nIdx))) { result |= ShaderStageTable[nIdx]; }
    }

    return result;
}

void MVulkanPipelineManager::GeneratePipelineLayout(
        const std::shared_ptr<MPipeline>&      pPipeline,
        const std::shared_ptr<MShaderProgram>& pShaderProgram
)
{
    //const VkShaderStageFlags vkShaderStageFlags = GetShaderStageFlags(pShaderProgram.get());
    VkShaderStageFlags vkShaderStageFlags =
            pShaderProgram->GetShader(MEShaderType::ECompute)
                    ? (VK_SHADER_STAGE_COMPUTE_BIT)
                    : (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT);

    std::vector<VkDescriptorSetLayout>        vSetLayouts;
    std::vector<VkDescriptorSetLayoutBinding> vParamBinding[MRenderGlobal::SHADER_PARAM_SET_NUM];

    for (uint32_t unSetIdx = 0; unSetIdx < MRenderGlobal::SHADER_PARAM_SET_NUM; ++unSetIdx)
    {
        std::shared_ptr<MShaderPropertyBlock> pPropertyBlock = pShaderProgram->GetShaderPropertyBlocks()[unSetIdx];

        for (const std::shared_ptr<MShaderConstantParam>& param: pPropertyBlock->m_params)
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding         = param->unBinding;
            uboLayoutBinding.descriptorType  = param->m_vkDescriptorType;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.stageFlags      = vkShaderStageFlags;

            uboLayoutBinding.pImmutableSamplers = nullptr;// Optional

            vParamBinding[unSetIdx].push_back(uboLayoutBinding);
        }

        for (const std::shared_ptr<MShaderTextureParam>& param: pPropertyBlock->m_textures)
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = param->unBinding;
            //uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            uboLayoutBinding.descriptorType  = param->m_vkDescriptorType;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.stageFlags      = vkShaderStageFlags;

            uboLayoutBinding.pImmutableSamplers = nullptr;// Optional

            vParamBinding[unSetIdx].push_back(uboLayoutBinding);
        }

        for (const std::shared_ptr<MShaderSampleParam>& param: pPropertyBlock->m_samples)
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding         = param->unBinding;
            uboLayoutBinding.descriptorType  = param->m_vkDescriptorType;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.stageFlags      = vkShaderStageFlags;

            if (param->eSamplerType == MESamplerType::ELinear)
                uboLayoutBinding.pImmutableSamplers = &m_device->m_vkLinearSampler;
            else
                uboLayoutBinding.pImmutableSamplers = &m_device->m_vkNearestSampler;

            vParamBinding[unSetIdx].push_back(uboLayoutBinding);
        }

        for (const std::shared_ptr<MShaderStorageParam>& param: pPropertyBlock->m_storages)
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding         = param->unBinding;
            uboLayoutBinding.descriptorType  = param->m_vkDescriptorType;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.stageFlags      = vkShaderStageFlags;

            vParamBinding[unSetIdx].push_back(uboLayoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        layoutInfo.bindingCount = static_cast<uint32_t>(vParamBinding[unSetIdx].size());
        layoutInfo.pBindings    = vParamBinding[unSetIdx].data();

        vSetLayouts.push_back(VkDescriptorSetLayout());

        if (vkCreateDescriptorSetLayout(m_device->m_vkDevice, &layoutInfo, nullptr, &vSetLayouts.back()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(vSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts    = vSetLayouts.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    MORTY_ASSERT(
            vkCreatePipelineLayout(m_device->m_vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS
    );

    pPipeline->m_pipelineLayout.vkPipelineLayout      = pipelineLayout;
    pPipeline->m_pipelineLayout.vDescriptorSetLayouts = std::move(vSetLayouts);
}

void MVulkanPipelineManager::DestroyPipelineLayout(const std::shared_ptr<MPipeline>& pPipeline)
{
    m_device->GetRecycleBin()->DestroyPipelineLayoutLater(pPipeline->m_pipelineLayout.vkPipelineLayout);

    for (VkDescriptorSetLayout& layout: pPipeline->m_pipelineLayout.vDescriptorSetLayouts)
    {
        m_device->GetRecycleBin()->DestroyDescriptorSetLayoutLater(layout);
    }

    pPipeline->m_pipelineLayout.vkPipelineLayout      = VK_NULL_HANDLE;
    pPipeline->m_pipelineLayout.vDescriptorSetLayouts = {};
}

void MVulkanPipelineManager::DestroyGraphicsPipeline(const std::shared_ptr<MGraphicsPipeline>& pGraphicsPipeline)
{
    if (!pGraphicsPipeline) { return; }

    for (auto& pipeline: pGraphicsPipeline->m_subpassPipeline)
    {
        m_device->GetRecycleBin()->DestroyPipelineLater(pipeline);
        pipeline = nullptr;
    }
}

void MVulkanPipelineManager::DestroyComputePipeline(const std::shared_ptr<MComputePipeline>& pComputePipeline)
{
    if (!pComputePipeline) return;

    m_device->GetRecycleBin()->DestroyPipelineLater(pComputePipeline->m_vkPipeline);
    pComputePipeline->m_vkPipeline = nullptr;
}

void GetBlendStage(
        const MMaterialTemplate*                          pMaterial,
        const MRenderPass*                                pRenderPass,
        std::vector<VkPipelineColorBlendAttachmentState>& vBlendAttach,
        VkPipelineColorBlendStateCreateInfo&              blendInfo
)
{
    blendInfo               = {};
    blendInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendInfo.logicOpEnable = VK_FALSE;
    blendInfo.logicOp       = VK_LOGIC_OP_COPY;

    MEMaterialType eType = pMaterial->GetMaterialType();

    if (MEMaterialType::EDepthPeel == eType)
    {
        if (pRenderPass->m_renderTarget.backTargets.size() < 4) return;

        vBlendAttach.resize(4);


        {//Front
            vBlendAttach[0].blendEnable    = VK_TRUE;
            vBlendAttach[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            vBlendAttach[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            vBlendAttach[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            vBlendAttach[0].colorBlendOp        = VK_BLEND_OP_ADD;
            vBlendAttach[0].alphaBlendOp        = VK_BLEND_OP_ADD;
        }
        {//Back
            vBlendAttach[1].blendEnable    = VK_TRUE;
            vBlendAttach[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            vBlendAttach[1].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            vBlendAttach[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            vBlendAttach[1].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            vBlendAttach[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[1].colorBlendOp        = VK_BLEND_OP_ADD;
            vBlendAttach[1].alphaBlendOp        = VK_BLEND_OP_ADD;
        }
        {//Front Depth
            vBlendAttach[2].blendEnable         = VK_TRUE;
            vBlendAttach[2].colorWriteMask      = VK_COLOR_COMPONENT_R_BIT;
            vBlendAttach[2].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[2].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[2].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[2].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[2].colorBlendOp        = VK_BLEND_OP_MIN;
            vBlendAttach[2].alphaBlendOp        = VK_BLEND_OP_MIN;
        }
        {//Back Depth
            vBlendAttach[3].blendEnable         = VK_TRUE;
            vBlendAttach[3].colorWriteMask      = VK_COLOR_COMPONENT_R_BIT;
            vBlendAttach[3].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[3].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[3].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[3].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            vBlendAttach[3].colorBlendOp        = VK_BLEND_OP_MAX;
            vBlendAttach[3].alphaBlendOp        = VK_BLEND_OP_MAX;
        }
    }
    else if (MEMaterialType::ETransparentBlend == eType)
    {
        for (uint32_t i = 0; i < pRenderPass->m_renderTarget.backTargets.size(); ++i)
        {
            vBlendAttach.push_back({});
            VkPipelineColorBlendAttachmentState& attachStage = vBlendAttach.back();
            attachStage.blendEnable                          = VK_TRUE;
            attachStage.colorWriteMask                       = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            attachStage.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            attachStage.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachStage.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            attachStage.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachStage.colorBlendOp        = VK_BLEND_OP_ADD;
            attachStage.alphaBlendOp        = VK_BLEND_OP_MAX;
        }
    }
    else if (MEMaterialType::EImGui == eType)
    {
        for (uint32_t i = 0; i < pRenderPass->m_renderTarget.backTargets.size(); ++i)
        {
            vBlendAttach.push_back({});
            VkPipelineColorBlendAttachmentState& attachStage = vBlendAttach.back();
            attachStage.blendEnable                          = VK_TRUE;
            attachStage.colorWriteMask                       = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            attachStage.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            attachStage.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            attachStage.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            attachStage.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            attachStage.colorBlendOp        = VK_BLEND_OP_ADD;
            attachStage.alphaBlendOp        = VK_BLEND_OP_ADD;
        }
    }
    else if (MEMaterialType::ECustom == eType)
    {
        for (uint32_t i = 0; i < pRenderPass->m_renderTarget.backTargets.size(); ++i)
        {
            vBlendAttach.push_back({});
            VkPipelineColorBlendAttachmentState& attachStage = vBlendAttach.back();
            attachStage.blendEnable                          = VK_TRUE;
            attachStage.colorWriteMask                       = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            attachStage.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            attachStage.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachStage.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            attachStage.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachStage.colorBlendOp        = VK_BLEND_OP_ADD;
            attachStage.alphaBlendOp        = VK_BLEND_OP_MAX;
        }
    }
    else
    {
        for (uint32_t i = 0; i < pRenderPass->m_renderTarget.backTargets.size(); ++i)
        {
            vBlendAttach.push_back({});
            VkPipelineColorBlendAttachmentState& attachStage = vBlendAttach.back();
            attachStage.blendEnable                          = VK_TRUE;
            attachStage.colorWriteMask                       = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            attachStage.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            attachStage.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            attachStage.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            attachStage.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachStage.colorBlendOp        = VK_BLEND_OP_ADD;
            attachStage.alphaBlendOp        = VK_BLEND_OP_ADD;
        }
    }

    blendInfo.attachmentCount   = static_cast<uint32_t>(vBlendAttach.size());
    blendInfo.pAttachments      = vBlendAttach.data();
    blendInfo.blendConstants[0] = 0.0f;
    blendInfo.blendConstants[1] = 0.0f;
    blendInfo.blendConstants[2] = 0.0f;
    blendInfo.blendConstants[3] = 0.0f;
}


VkCompareOp GetCompareOp(MDepthCompareType type)
{
    switch (type)
    {
        case MDepthCompareType::Never: return VkCompareOp::VK_COMPARE_OP_NEVER;
        case MDepthCompareType::Less: return VkCompareOp::VK_COMPARE_OP_LESS;
        case MDepthCompareType::Equal: return VkCompareOp::VK_COMPARE_OP_EQUAL;
        case MDepthCompareType::Less_Or_Equal: return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
        case MDepthCompareType::Greater: return VkCompareOp::VK_COMPARE_OP_GREATER;
        case MDepthCompareType::Not_Equal: return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
        case MDepthCompareType::Greater_Or_Equal: return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
        case MDepthCompareType::Always: return VkCompareOp::VK_COMPARE_OP_ALWAYS;
        default: MORTY_ASSERT(false);
    }

    return VkCompareOp::VK_COMPARE_OP_NEVER;
};

void GetDepthStencilStage(const MRenderPass* pRenderPass, VkPipelineDepthStencilStateCreateInfo& depthStencilInfo)
{
    depthStencilInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.pNext                 = NULL;
    depthStencilInfo.flags                 = 0;
    depthStencilInfo.minDepthBounds        = 0;
    depthStencilInfo.maxDepthBounds        = 0;
    depthStencilInfo.depthTestEnable       = pRenderPass->m_depthTestEnable;
    depthStencilInfo.depthWriteEnable      = pRenderPass->m_depthWriteEnable;
    depthStencilInfo.depthCompareOp        = GetCompareOp(pRenderPass->m_depthCompareOp);
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable     = pRenderPass->m_stencilTestEnable;
}

VkPipeline MVulkanPipelineManager::CreateGraphicsPipeline(
        const std::shared_ptr<MPipeline>& pPipeline,
        const MMaterialTemplate*          pMaterial,
        const MRenderPass*                pRenderPass,
        const uint32_t&                   nSubpassIdx
)
{
    if (!pMaterial)
    {
        MORTY_ASSERT(pMaterial);
        return VK_NULL_HANDLE;
    }

    if (pPipeline->m_pipelineLayout.vkPipelineLayout == VK_NULL_HANDLE)
    {
        GeneratePipelineLayout(pPipeline, pMaterial->GetShaderProgram());
    }

    VkPipelineLayout vkPipelineLayout = pPipeline->m_pipelineLayout.vkPipelineLayout;

    if (VK_NULL_HANDLE == vkPipelineLayout)
    {
        MORTY_ASSERT(vkPipelineLayout);
        return VK_NULL_HANDLE;
    }

    MShader* pVertexShader = pMaterial->GetShaderProgram()->GetShader(MEShaderType::EVertex);

    if (nullptr == pVertexShader)
    {
        MORTY_ASSERT(false);
        return VK_NULL_HANDLE;
    }

    std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH};

    //variable rate shading
    if (m_device->GetDeviceFeatureSupport(MEDeviceFeature::EVariableRateShading))
    {
        if (m_device->GetPhysicalDevice()->m_vkShadingRateImageFeatures.pipelineFragmentShadingRate)
        {
            dynamicStates.push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
        }
    }

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();


    VkRect2D scissor{};
    scissor.offset        = {0, 0};
    scissor.extent.width  = UINT_MAX;//default
    scissor.extent.height = UINT_MAX;

    VkViewport viewport{};
    viewport.x        = 0;
    viewport.y        = 0;
    viewport.width    = 512;
    viewport.height   = 512;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount                     = 1;
    viewportState.pViewports                        = &viewport;
    viewportState.scissorCount                      = 1;
    viewportState.pScissors                         = &scissor;


    std::vector<VkPipelineShaderStageCreateInfo> vShaderStageCreateInfos;
    for (size_t nIdx = 0; nIdx < size_t(MEShaderType::TOTAL_NUM); ++nIdx)
    {
        if (MShader* pShader = pMaterial->GetShaderProgram()->GetShader(MEShaderType(nIdx)))
        {
            if (nullptr == pShader->GetBuffer())
            {
                MORTY_ASSERT(false);
                return VK_NULL_HANDLE;
            }
            vShaderStageCreateInfos.push_back(pShader->GetBuffer()->m_vkShaderStageInfo);
        }
    }

    MVertexShaderBuffer* pVertexShaderBuffer = static_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer());

    VkPipelineVertexInputStateCreateInfo inputStateInfo = {};
    inputStateInfo                                      = VkPipelineVertexInputStateCreateInfo{};
    inputStateInfo.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputStateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(pVertexShaderBuffer->m_bindingDescs.size());
    inputStateInfo.pVertexBindingDescriptions    = pVertexShaderBuffer->m_bindingDescs.data();
    inputStateInfo.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(pVertexShaderBuffer->m_attributeDescs.size());
    inputStateInfo.pVertexAttributeDescriptions = pVertexShaderBuffer->m_attributeDescs.data();

    //Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable        = VK_FALSE;
    rasterizationState.lineWidth               = 1.0f;
    rasterizationState.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizationState.depthBiasEnable         = VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0.0f;// Optional
    rasterizationState.depthBiasClamp          = 0.0f;// Optional
    rasterizationState.depthBiasSlopeFactor    = 0.0f;// Optional

    VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterStateCI{};
    conservativeRasterStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
    conservativeRasterStateCI.conservativeRasterizationMode    = VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT;
    conservativeRasterStateCI.extraPrimitiveOverestimationSize = 0.0f;

    if (pMaterial->GetConservativeRasterizationEnable())
    {
        if (m_device->GetDeviceFeatureSupport(MEDeviceFeature::EConservativeRasterization))
        {
            const float fOverestSize =
                    m_device->GetPhysicalDevice()
                            ->m_vkConservativeRasterProps.extraPrimitiveOverestimationSizeGranularity;
            conservativeRasterStateCI.conservativeRasterizationMode =
                    VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
            conservativeRasterStateCI.extraPrimitiveOverestimationSize = fOverestSize;
            rasterizationState.pNext                                   = &conservativeRasterStateCI;
        }
    }

    switch (pMaterial->GetCullMode())
    {
        case MECullMode::ECullNone:
            rasterizationState.cullMode    = VK_CULL_MODE_NONE;
            rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
            break;

        case MECullMode::ECullBack:
            rasterizationState.cullMode    = VK_CULL_MODE_BACK_BIT;//vulkan inverse
            rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
            break;

        case MECullMode::ECullFront:
            rasterizationState.cullMode    = VK_CULL_MODE_FRONT_BIT;
            rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        case MECullMode::EWireframe:
            rasterizationState.cullMode    = VK_CULL_MODE_NONE;
            rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
            break;
        default: break;
    }


    VkPipelineColorBlendStateCreateInfo              blendInfo        = {};
    VkPipelineDepthStencilStateCreateInfo            depthStencilInfo = {};

    std::vector<VkPipelineColorBlendAttachmentState> vBlendAttach;
    GetBlendStage(pMaterial, pRenderPass, vBlendAttach, blendInfo);

    GetDepthStencilStage(pRenderPass, depthStencilInfo);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable                  = VK_FALSE;
    multisampleState.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.minSampleShading                     = 1.0f;    // Optional
    multisampleState.pSampleMask                          = nullptr; // Optional
    multisampleState.alphaToCoverageEnable                = VK_FALSE;// Optional
    multisampleState.alphaToOneEnable                     = VK_FALSE;// Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = static_cast<uint32_t>(vShaderStageCreateInfos.size());
    pipelineInfo.pStages             = vShaderStageCreateInfos.data();
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.pVertexInputState   = &inputStateInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState   = &multisampleState;
    pipelineInfo.pColorBlendState    = &blendInfo;
    pipelineInfo.pDepthStencilState  = &depthStencilInfo;
    pipelineInfo.layout              = vkPipelineLayout;
    pipelineInfo.renderPass          = pRenderPass->m_vkRenderPass;
    pipelineInfo.subpass             = nSubpassIdx;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkResult   result           = vkCreateGraphicsPipelines(
            m_device->m_vkDevice,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &graphicsPipeline
    );

    if (result != VK_SUCCESS)
    {
        MORTY_ASSERT(result == VK_SUCCESS);
        return VK_NULL_HANDLE;
    }

#if MORTY_DEBUG
    m_device->SetDebugName(
            reinterpret_cast<uint64_t>(graphicsPipeline),
            VkObjectType::VK_OBJECT_TYPE_PIPELINE,
            pMaterial->GetDebugName()
    );
#endif

    return graphicsPipeline;
}

VkPipeline MVulkanPipelineManager::CreateComputePipeline(
        const std::shared_ptr<MPipeline>& pPipeline,
        MComputeDispatcher*               pComputeDispatcher
)
{
    if (VK_NULL_HANDLE == pPipeline->m_pipelineLayout.vkPipelineLayout)
    {
        GeneratePipelineLayout(pPipeline, pComputeDispatcher->GetShaderProgram());
    }

    VkPipelineLayout vkPipelineLayout = pPipeline->m_pipelineLayout.vkPipelineLayout;
    if (VK_NULL_HANDLE == vkPipelineLayout)
    {
        MORTY_ASSERT(vkPipelineLayout);
        return VK_NULL_HANDLE;
    }


    MShader* pComputeShader = pComputeDispatcher->GetComputeShader();

    if (nullptr == pComputeShader) return VK_NULL_HANDLE;

    if (nullptr == pComputeShader->GetBuffer()) return VK_NULL_HANDLE;

    MShaderBuffer*              pComputeShaderBuffer = pComputeShader->GetBuffer();

    VkComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext              = VK_NULL_HANDLE;
    pipelineInfo.flags              = 0;
    pipelineInfo.stage              = pComputeShaderBuffer->m_vkShaderStageInfo;
    pipelineInfo.layout             = vkPipelineLayout;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = 0;

    // TODO
    // fill pipelineInfo.stage.pSpecializationInfo

    VkPipeline computePipeline;
    if (vkCreateComputePipelines(m_device->m_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) !=
        VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

#if MORTY_DEBUG
    m_device->SetDebugName(
            reinterpret_cast<uint64_t>(computePipeline),
            VkObjectType::VK_OBJECT_TYPE_PIPELINE,
            pComputeDispatcher->GetDebugName()
    );
#endif

    return computePipeline;
}

void MVulkanPipelineManager::AllocateShaderPropertyBlock(
        const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock,
        const std::shared_ptr<MPipeline>&            pPipeline
)
{
    std::shared_ptr<MShaderProgram> pShaderProgram = pPropertyBlock->GetShaderProgram();

    if (pPropertyBlock->m_vkDescriptorSet)
    {
        m_device->GetRecycleBin()->DestroyDescriptorSetLater(pPropertyBlock->m_vkDescriptorSet);
        pPropertyBlock->m_vkDescriptorSet = VK_NULL_HANDLE;
    }

    MORTY_ASSERT(VK_NULL_HANDLE == pPropertyBlock->m_vkDescriptorSet);
    MORTY_ASSERT(VK_NULL_HANDLE != pPipeline->m_pipelineLayout.vkPipelineLayout);

    //pPipeline->m_shaderPropertyBlocks.insert(pPropertyBlock);
    VkDescriptorSetLayout vkDescriptorSetLayout =
            pPipeline->m_pipelineLayout.vDescriptorSetLayouts[pPropertyBlock->m_unKey];
    MORTY_ASSERT(VK_NULL_HANDLE != vkDescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = m_device->m_vkDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &vkDescriptorSetLayout;

    VkDescriptorSet descriptorSet;
    if (vkAllocateDescriptorSets(m_device->m_vkDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
    {
        m_device->GetEngine()->GetLogger()->Error("MVulkanPipelineManager::AllocateShaderPropertyBlock error: "
                                                  "descriptor pool == 0");
        return;
    }

    pPropertyBlock->m_vkDescriptorSet = descriptorSet;
}

void MVulkanPipelineManager::DestroyShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock)
{
    std::shared_ptr<MShaderProgram> pShaderProgram = pPropertyBlock->GetShaderProgram();

    DestroyShaderPropertyBlockImpl(pPropertyBlock);
}

void MVulkanPipelineManager::DestroyShaderPropertyBlockImpl(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock
) const
{
    if (!pPropertyBlock) return;

    if (pPropertyBlock->m_vkDescriptorSet)
    {
        m_device->GetRecycleBin()->DestroyDescriptorSetLater(pPropertyBlock->m_vkDescriptorSet);
        pPropertyBlock->m_vkDescriptorSet = VK_NULL_HANDLE;
    }
}

void MVulkanPipelineManager::BindConstantParam(
        const std::shared_ptr<MShaderConstantParam> pParam,
        VkWriteDescriptorSet&                       descriptorWrite
)
{
    VkDescriptorBufferInfo& bufferInfo = pParam->m_vkBufferInfo;
    MORTY_ASSERT(VK_NULL_HANDLE != bufferInfo.buffer);

    descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstBinding      = pParam->unBinding;
    descriptorWrite.dstArrayElement = 0;

    descriptorWrite.descriptorType  = pParam->m_vkDescriptorType;
    descriptorWrite.descriptorCount = 1;

    descriptorWrite.pBufferInfo      = &bufferInfo;
    descriptorWrite.pImageInfo       = nullptr;// Optional
    descriptorWrite.pTexelBufferView = nullptr;// Optional
}

void MVulkanPipelineManager::BindTextureParam(
        const std::shared_ptr<MShaderTextureParam> pParam,
        VkWriteDescriptorSet&                      descriptorWrite
)
{
    std::shared_ptr<MTexture> pTexture = pParam->GetTexture();

    if (!pTexture || pTexture->GetTextureRHI<MTextureRHIVulkan>()->vkImageView == VK_NULL_HANDLE)
    {
        pTexture = GetDefaultTexture(pParam.get());
    }

    MORTY_ASSERT(pTexture);
    auto textureRHI = pTexture->GetTextureRHI<MTextureRHIVulkan>();
    MORTY_ASSERT(textureRHI);


    VkDescriptorImageInfo& imageInfo = pParam->m_vkImageInfo;
    imageInfo.imageView              = textureRHI->vkImageView;
    MORTY_ASSERT(textureRHI->vkImageLayout != VK_IMAGE_LAYOUT_UNDEFINED);

    //TODO: Do not set image layout from a constants value.
    if (pTexture->GetWriteUsage() & METextureWriteUsageBit::EStorageWrite)
    {
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
    else { imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; }
    imageInfo.sampler = textureRHI->vkSampler;

    if (VK_NULL_HANDLE == imageInfo.sampler) { imageInfo.sampler = m_device->m_vkNearestSampler; }

    descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstBinding      = pParam->unBinding;
    descriptorWrite.dstArrayElement = 0;

    descriptorWrite.descriptorType  = pParam->m_vkDescriptorType;
    descriptorWrite.descriptorCount = 1;

    descriptorWrite.pBufferInfo      = nullptr;
    descriptorWrite.pImageInfo       = &imageInfo;
    descriptorWrite.pTexelBufferView = nullptr;

    //A VkDescripotrSet can only be updated once on per render. .
}

void MVulkanPipelineManager::BindStorageParam(
        const std::shared_ptr<MShaderStorageParam> pParam,
        VkWriteDescriptorSet&                      descriptorWrite
)
{
    const MBuffer* pBuffer = pParam->pBuffer;

    if (!pBuffer || !pBuffer->m_vkBuffer)
    {
        MORTY_ASSERT(pBuffer && pBuffer->m_vkBuffer);
        return;
    }

    VkDescriptorBufferInfo& bufferInfo = pParam->m_vkBufferInfo;
    bufferInfo.buffer                  = pBuffer->m_vkBuffer;
    bufferInfo.offset                  = 0;
    bufferInfo.range                   = pBuffer->GetSize();

    descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstBinding      = pParam->unBinding;
    descriptorWrite.dstArrayElement = 0;

    descriptorWrite.descriptorType  = pParam->m_vkDescriptorType;
    descriptorWrite.descriptorCount = 1;

    descriptorWrite.pBufferInfo      = &bufferInfo;
    descriptorWrite.pImageInfo       = nullptr;// Optional
    descriptorWrite.pTexelBufferView = nullptr;// Optional
}

std::shared_ptr<MTexture> MVulkanPipelineManager::GetDefaultTexture(MShaderTextureParam* pParam)
{
    const auto findResult = m_defaultTexture.find({pParam->eFormat, pParam->eType});
    if (findResult != m_defaultTexture.end()) { return findResult->second; }

    static const std::unordered_map<MESamplerFormat, METextureFormat> TypeMapping = {
            {MESamplerFormat::EFloat, METextureFormat::UNorm_RGBA8},
            {MESamplerFormat::EInt, METextureFormat::UInt_R8},
    };

    MORTY_ASSERT(TypeMapping.find(pParam->eFormat) != TypeMapping.end());

    std::vector<std::vector<MByte>> cubeBytes{std::vector<MByte>(32)};
    memset(cubeBytes[0].data(), 255, sizeof(MByte) * 32);

    auto pTexture = MTexture::CreateTexture({
            .strName         = "Shader Default Texture",
            .n3Size          = Vector3i(1, 1, 1),
            .eTextureType    = pParam->eType,
            .eFormat         = TypeMapping.at(pParam->eFormat),
            .eMipmapDataType = MEMipmapDataType::Disable,
            .nReadUsage      = METextureReadUsageBit::EPixelSampler,
            .nWriteUsage     = METextureWriteUsageBit::EUnknow,
    });

    pTexture->GenerateBuffer(m_device, cubeBytes);
    MORTY_ASSERT(pTexture);

    m_defaultTexture[{pParam->eFormat, pParam->eType}] = pTexture;
    return pTexture;
}


#endif

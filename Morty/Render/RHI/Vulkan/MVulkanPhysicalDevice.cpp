#include "RHI/Vulkan/MVulkanPhysicalDevice.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#ifdef MORTY_WIN
#include <windows.h>

#include "vulkan/vulkan_win32.h"
#endif

#ifdef MORTY_ANDROID
#include "vulkan/vulkan_android.h"

#endif

#include "Engine/MEngine.h"
#include "Utility/MLogger.h"

using namespace morty;

const std::vector<const char*> ValidationLayers = {

        "VK_LAYER_KHRONOS_validation"

};
const std::vector<const char*> DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,
        VK_KHR_MULTIVIEW_EXTENSION_NAME,
#if defined(MORTY_WIN) && defined(MORTY_DEBUG)
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
#endif

#ifdef MORTY_MACOS
        "VK_KHR_portability_subset",
#endif

};

const std::map<MString, MEDeviceFeature> OptionalDeviceExtensions = {
        {VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, MEDeviceFeature::EConservativeRasterization},
        {VK_GOOGLE_HLSL_FUNCTIONALITY_1_EXTENSION_NAME, MEDeviceFeature::EHLSLFunctionality},
        {VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, MEDeviceFeature::EVariableRateShading},
};

const std::set<VkFormat> DepthOnlyTextureFormat = {
        VK_FORMAT_D32_SFLOAT,
};

const std::set<VkFormat> DepthStencilTextureFormat = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
};

std::vector<const char*> InstanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef MORTY_DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif

#ifdef MORTY_MACOS
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
        "VK_EXT_metal_surface",
        "VK_MVK_macos_surface",
#endif

#ifdef MORTY_IOS
        "VK_EXT_metal_surface",
        "VK_MVK_ios_surface",
        "VK_MVK_moltenvk",
#endif

#ifdef MORTY_WIN
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

#ifdef MORTY_ANDROID
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif

        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};


template<typename TYPE> void RegisterExtensionFeatures(void*& pExtensionFeatureList, TYPE& feature)
{
    feature.pNext         = pExtensionFeatureList;
    pExtensionFeatureList = &feature;
}


MVulkanPhysicalDevice::MVulkanPhysicalDevice(MEngine* pEngine)
    : m_engine(pEngine)
{}

MEngine* MVulkanPhysicalDevice::GetEngine() const { return m_engine; }

VkDevice MVulkanPhysicalDevice::CreateLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    const float                          graphics_priorities[] = {0.0f, 1.0f};//range 0~1
    const float                          compute_priorities[]  = {1.0f};      //range 0~1
    VkDeviceQueueCreateInfo              queueInfo{};
    queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext            = NULL;
    queueInfo.flags            = 0;
    queueInfo.queueFamilyIndex = m_graphicsFamilyIndex;
    queueInfo.queueCount       = 2;
    queueInfo.pQueuePriorities = graphics_priorities;
    queueCreateInfos.push_back(queueInfo);

    if (m_computeFamilyIndex != m_graphicsFamilyIndex)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = m_computeFamilyIndex;
        queueInfo.queueCount       = 1;
        queueInfo.pQueuePriorities = compute_priorities;
        queueCreateInfos.push_back(queueInfo);
    }

    vkGetPhysicalDeviceFeatures(m_vkPhysicalDevice, &m_vkPhysicalDeviceFeatures);

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.flags                   = 0;
    deviceInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    deviceInfo.pQueueCreateInfos       = queueCreateInfos.data();
    deviceInfo.enabledExtensionCount   = static_cast<uint32_t>(m_enableDeviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = m_enableDeviceExtensions.data();
    deviceInfo.pEnabledFeatures        = &m_vkPhysicalDeviceFeatures;
    deviceInfo.pNext                   = m_vkExtensionFeaturesList;

    VkDevice vkDevice = VK_NULL_HANDLE;
    VkResult result   = vkCreateDevice(m_vkPhysicalDevice, &deviceInfo, NULL, &vkDevice);
    if (result != VK_SUCCESS)
    {
        GetEngine()->GetLogger()->Error("Initialize Vulkan Error : vkCreateDevice error.");
        return VK_NULL_HANDLE;
    }

    return vkDevice;
}

bool MVulkanPhysicalDevice::Initialize()
{
    if (!InitVulkanInstance()) return false;

    if (!InitPhysicalDevice()) return false;

    //log
    GetEngine()->GetLogger()->Information("Vulkan Validation Extensions:  ");
    for (const char* svExtensionName: ValidationLayers)
    {
        GetEngine()->GetLogger()->Information("   {}", svExtensionName);
    }

    GetEngine()->GetLogger()->Information("\nVulkan Device Extensions:  ");
    for (const char* svExtensionName: DeviceExtensions)
    {
        GetEngine()->GetLogger()->Information("   {}", svExtensionName);
    }

    GetEngine()->GetLogger()->Information("\nVulkan Device Optional Extensions:  ");
    for (size_t nIdx = DeviceExtensions.size(); nIdx < m_enableDeviceExtensions.size(); ++nIdx)
    {
        GetEngine()->GetLogger()->Information("   {}", m_enableDeviceExtensions[nIdx]);
    }

    GetEngine()->GetLogger()->Information("\nVulkan Instance Extensions:  ");
    for (const char* svExtensionName: InstanceExtensions)
    {
        GetEngine()->GetLogger()->Information("    {}", svExtensionName);
    }

    InitDeviceFeature();

    if (!InitDepthFormat()) { return false; }

    return true;
}

void MVulkanPhysicalDevice::Release()
{
#if MORTY_DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT pvkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT
    ) vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
    pvkDestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugUtilsMessenger, nullptr);
#endif

    vkDestroyInstance(m_vkInstance, NULL);
}

int MVulkanPhysicalDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    static std::unordered_map<VkMemoryPropertyFlags, int> MemoryTypeTable;

    const auto                                            findResult = MemoryTypeTable.find(properties);

    if (findResult != MemoryTypeTable.end()) { return findResult->second; }

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            MemoryTypeTable[properties] = i;
            return static_cast<int>(i);
        }
    }

    return MGlobal::M_INVALID_INDEX;
}

VkFormat MVulkanPhysicalDevice::FindSupportedFormat(
        const std::set<VkFormat>& candidates,
        VkImageTiling             tiling,
        VkFormatFeatureFlags      features
) const
{
    for (VkFormat format: candidates)
    {
        const VkFormatProperties props = GetFormatProperties(format);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) { return format; }

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

VkFormatProperties MVulkanPhysicalDevice::GetFormatProperties(VkFormat vkFormat) const
{
    static std::unordered_map<VkFormat, VkFormatProperties> FormatTable;
    auto                                                    findResult = FormatTable.find(vkFormat);
    if (findResult != FormatTable.end()) { return findResult->second; }

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_vkPhysicalDevice, vkFormat, &formatProperties);

    //TODO (formatProperties.optimalTilingFeatures) check support.

    FormatTable[vkFormat] = formatProperties;
    return formatProperties;
}

VkImageLayout MVulkanPhysicalDevice::FindDepthImageLayout() const
{
    //VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL support after vulkan 1.2.0
    if (!CheckVersion(1, 2, 0)) { return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; }

    if (m_vkDepthAspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) { return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; }

    return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
}

VkBool32 MVulkanPhysicalDevice::FormatIsFilterable(VkFormat format, VkImageTiling tiling) const
{
    const VkFormatProperties formatProps = GetFormatProperties(format);

    if (tiling == VK_IMAGE_TILING_OPTIMAL)
        return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

    if (tiling == VK_IMAGE_TILING_LINEAR)
        return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

    return false;
}

bool MVulkanPhysicalDevice::CheckVersion(int major, int minor, int patch) const
{
    if (major == m_vulkanVersionMajor)
    {
        if (minor == m_vulkanVersionMinor) { return patch <= m_vulkanVersionPatch; }
        return minor < m_vulkanVersionMinor;
    }

    return major < m_vulkanVersionMajor;
}

bool MVulkanPhysicalDevice::InitDepthFormat()
{
    constexpr VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkFormat format = FindSupportedFormat(DepthStencilTextureFormat, VK_IMAGE_TILING_OPTIMAL, features);

    if (format != VK_FORMAT_UNDEFINED)
    {
        m_vkDepthTextureFormat = format;
        m_vkDepthAspectFlags   = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        m_vkDepthImageLayout   = FindDepthImageLayout();
        return true;
    }

    format = FindSupportedFormat(DepthOnlyTextureFormat, VK_IMAGE_TILING_OPTIMAL, features);
    if (format != VK_FORMAT_UNDEFINED)
    {
        m_vkDepthTextureFormat = format;
        m_vkDepthAspectFlags   = VK_IMAGE_ASPECT_DEPTH_BIT;
        m_vkDepthImageLayout   = FindDepthImageLayout();
        return true;
    }

    return false;
}

void MVulkanPhysicalDevice::InitDeviceFeature()
{
    if constexpr (true)
    {
        m_vkVulkan11Features.sType     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        m_vkVulkan11Features.multiview = VK_TRUE;
        RegisterExtensionFeatures(m_vkExtensionFeaturesList, m_vkVulkan11Features);
    }

    if (GetDeviceFeatureSupport(MEDeviceFeature::EConservativeRasterization))
    {
        VkPhysicalDeviceProperties2KHR deviceProps2{};
        deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        deviceProps2.pNext = &m_vkConservativeRasterProps;

        m_vkConservativeRasterProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
        vkGetPhysicalDeviceProperties2(m_vkPhysicalDevice, &deviceProps2);
    }

    if (GetDeviceFeatureSupport(MEDeviceFeature::EVariableRateShading))
    {
        VkPhysicalDeviceFeatures2KHR deviceFeats2{};
        deviceFeats2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
        deviceFeats2.pNext = &m_vkShadingRateImageFeatures;

        m_vkShadingRateImageFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;

        vkGetPhysicalDeviceFeatures2KHR(m_vkPhysicalDevice, &deviceFeats2);

        RegisterExtensionFeatures(m_vkExtensionFeaturesList, m_vkShadingRateImageFeatures);


        VkPhysicalDeviceProperties2KHR deviceProps2{};
        deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        deviceProps2.pNext = &m_vkFragmentShadingRateProperties;

        m_vkFragmentShadingRateProperties.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;

        vkGetPhysicalDeviceProperties2(m_vkPhysicalDevice, &deviceProps2);

        GetEngine()->GetLogger()->Information(
                "\nVulkan ShadingRate pipeline fragment enable: {}.",
                m_vkShadingRateImageFeatures.pipelineFragmentShadingRate
        );
        GetEngine()->GetLogger()->Information(
                "Vulkan ShadingRate primitive fragment enable: {}.",
                m_vkShadingRateImageFeatures.primitiveFragmentShadingRate
        );
        GetEngine()->GetLogger()->Information(
                "Vulkan ShadingRate attachment fragment enable: {}.",
                m_vkShadingRateImageFeatures.attachmentFragmentShadingRate
        );
    }
}

bool MVulkanPhysicalDevice::GetDeviceFeatureSupport(MEDeviceFeature feature) const
{
    return m_disableFeature.find(feature) == m_disableFeature.end();
}

VkBool32 VKAPI_PTR OutputDebugUtilsMessenger(
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void*                                       pUserData
)
{
    MORTY_UNUSED(messageType);

    if (VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT == messageSeverity)
    {
        MVulkanPhysicalDevice* pVulkanInstance = static_cast<MVulkanPhysicalDevice*>(pUserData);
        pVulkanInstance->GetEngine()->GetLogger()->Error(pCallbackData->pMessage);
        MORTY_ASSERT(false);
    }
    return VK_FALSE;
}

bool MVulkanPhysicalDevice::InitVulkanInstance()
{
    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext              = nullptr;
    appInfo.pApplicationName   = "Morty App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "Morty Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;
#ifdef MORTY_MACOS
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledLayerCount   = static_cast<uint32_t>(ValidationLayers.size());
    createInfo.ppEnabledLayerNames = ValidationLayers.data();


    createInfo.enabledExtensionCount   = static_cast<uint32_t>(InstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = InstanceExtensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);

    if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        GetEngine()->GetLogger()->Error("Cannot find a compatible Vulkan installable client "
                                        "driver (ICD). Please make sure your driver supports "
                                        "Vulkan before continuing. The call to vkCreateInstance failed.");
        return false;
    }
    else if (result != VK_SUCCESS)
    {
        GetEngine()->GetLogger()->Error("The call to vkCreateInstance failed. error code: {}", int(result));
        return false;
    }

    vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetInstanceProcAddr(m_vkInstance, "vkSetDebugUtilsObjectNameEXT")
    );
    vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
            vkGetInstanceProcAddr(m_vkInstance, "vkGetPhysicalDeviceProperties2KHR")
    );
    vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
            vkGetInstanceProcAddr(m_vkInstance, "vkGetPhysicalDeviceFeatures2KHR")
    );
    vkCmdSetFragmentShadingRateKHR = reinterpret_cast<PFN_vkCmdSetFragmentShadingRateKHR>(
            vkGetInstanceProcAddr(m_vkInstance, "vkCmdSetFragmentShadingRateKHR")
    );

#if MORTY_DEBUG
    // load kCreateDebugUtilsMessengerEXT
    PFN_vkCreateDebugUtilsMessengerEXT pvkCreateDebugUtilsMessengerEXT =
            (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugUtilsMessengerEXT");
    if (pvkCreateDebugUtilsMessengerEXT == NULL) return false;

    // create debug utils messenger
    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {};
    debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils_messenger_create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_utils_messenger_create_info.pfnUserCallback = OutputDebugUtilsMessenger;
    debug_utils_messenger_create_info.pUserData       = this;

    if (VK_SUCCESS != pvkCreateDebugUtilsMessengerEXT(
                              m_vkInstance,
                              &debug_utils_messenger_create_info,
                              NULL,
                              &m_vkDebugUtilsMessenger
                      ))
        return false;
#endif

    return true;
}

bool MVulkanPhysicalDevice::InitPhysicalDevice()
{
    uint32_t nDeviceCount = 0;
    VkResult result       = vkEnumeratePhysicalDevices(m_vkInstance, &nDeviceCount, NULL);

    if (result != VK_SUCCESS || nDeviceCount < 1)
    {
        GetEngine()->GetLogger()->Error("Initialize Vulkan Error : device count < 1");
        return false;
    }

    std::vector<VkPhysicalDevice> vPhysicalDevices(nDeviceCount);
    result = vkEnumeratePhysicalDevices(m_vkInstance, &nDeviceCount, vPhysicalDevices.data());
    if (result != VK_SUCCESS)
    {
        GetEngine()->GetLogger()->Error("Initialize Vulkan Error : vkEnumeratePhysicalDevices error.");
        return false;
    }

    size_t nMinNotSupportFeatureNum = INT_MAX;
    for (size_t i = 0; i < nDeviceCount; i++)
    {
        if (IsDeviceSuitable(vPhysicalDevices[i]))
        {
            auto tNotSupportFeature = GetDeviceOptionFeatureNotSupport(vPhysicalDevices[i]);
            if (nMinNotSupportFeatureNum > tNotSupportFeature.size())
            {
                nMinNotSupportFeatureNum = tNotSupportFeature.size();
                m_vkPhysicalDevice       = vPhysicalDevices[i];
            }
        }
    }

    m_vkPhysicalDeviceProperties = {};
    vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_vkPhysicalDeviceProperties);

    m_disableFeature.clear();
    const auto tNotSupportFeature = GetDeviceOptionFeatureNotSupport(m_vkPhysicalDevice);
    for (const auto& strNotSupportFeature: tNotSupportFeature)
    {
        m_disableFeature.insert(OptionalDeviceExtensions.at(strNotSupportFeature));
        GetEngine()->GetLogger()->Information("Not support optional extension: {}.", strNotSupportFeature.c_str());
    }

    m_enableDeviceExtensions = DeviceExtensions;
    for (const auto& [name, feature]: OptionalDeviceExtensions)
    {
        if (tNotSupportFeature.find(name) == tNotSupportFeature.end())
        {
            m_enableDeviceExtensions.push_back(name.c_str());
        }
    }

    if (m_vkPhysicalDevice == VK_NULL_HANDLE)
    {
        MORTY_ASSERT(m_vkPhysicalDevice);
        GetEngine()->GetLogger()->Error("Initialize Vulkan Error : m_physicalDeviceIndex == -1");
        return false;
    }

    m_vulkanVersionMajor = VK_VERSION_MAJOR(m_vkPhysicalDeviceProperties.apiVersion);
    m_vulkanVersionMinor = VK_VERSION_MINOR(m_vkPhysicalDeviceProperties.apiVersion);
    m_vulkanVersionPatch = VK_VERSION_PATCH(m_vkPhysicalDeviceProperties.apiVersion);

    GetEngine()->GetLogger()->Information(
            "Vulkan API Version:    {}.{}.{}\n",
            m_vulkanVersionMajor,
            m_vulkanVersionMinor,
            m_vulkanVersionPatch
    );

    m_graphicsFamilyIndex = FindQueueGraphicsFamilies(m_vkPhysicalDevice);
    m_computeFamilyIndex  = FindQueueComputeFamilies(m_vkPhysicalDevice);

    return true;
}

bool MVulkanPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device) const
{
    if (-1 == FindQueueGraphicsFamilies(device)) { return false; }
    if (-1 == FindQueueComputeFamilies(device)) { return false; }

    const std::set<MString> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());
    const auto              tNotSupportExtensions = GetNotSupportDeviceExtension(device, requiredExtensions);

    if (!tNotSupportExtensions.empty())
    {
        for (auto extensionName: tNotSupportExtensions)
        {
            GetEngine()->GetLogger()->Error("Not support extension: {}.", extensionName.c_str());
        }
        return false;
    }

    return true;
}

std::set<MString> MVulkanPhysicalDevice::GetDeviceOptionFeatureNotSupport(VkPhysicalDevice device) const
{
    std::set<MString> tOptionFeatures;
    for (const auto& [name, feature]: OptionalDeviceExtensions) { tOptionFeatures.insert(name); }
    return GetNotSupportDeviceExtension(device, tOptionFeatures);
}

int MVulkanPhysicalDevice::FindQueueGraphicsFamilies(VkPhysicalDevice device) const
{
    int      graphicsFamily = -1;

    uint32_t unQueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, vQueueProperties.data());

    for (size_t i = 0; i < unQueueFamilyCount; ++i)
    {
        if (vQueueProperties[i].queueCount > 0 && vQueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicsFamily = static_cast<uint32_t>(i);

        if (graphicsFamily >= 0) break;
    }

    return graphicsFamily;
}

int MVulkanPhysicalDevice::FindQueuePresentFamilies(VkSurfaceKHR surface) const
{

    int      nPresentFamily = -1;

    uint32_t unQueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &unQueueFamilyCount, NULL);

    std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &unQueueFamilyCount, vQueueProperties.data());

    VkBool32 bSupportsPresenting(VK_FALSE);
    for (uint32_t i = 0; i < unQueueFamilyCount; ++i)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, surface, &bSupportsPresenting);

        if (vQueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            if (bSupportsPresenting == VK_TRUE)
            {
                nPresentFamily = i;
                break;
            }
        }
    }

    return nPresentFamily;
}

int MVulkanPhysicalDevice::FindQueueComputeFamilies(VkPhysicalDevice device) const
{
    int      graphicsFamily = -1;

    uint32_t unQueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, vQueueProperties.data());

    for (size_t i = 0; i < unQueueFamilyCount; ++i)
    {
        if (vQueueProperties[i].queueCount > 0 && vQueueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            graphicsFamily = static_cast<uint32_t>(i);

        if (graphicsFamily >= 0) break;
    }

    return graphicsFamily;
}

bool MVulkanPhysicalDevice::MultiDrawIndirectSupport() const { return m_vkPhysicalDeviceFeatures.multiDrawIndirect; }

bool MVulkanPhysicalDevice::SparseTextureSupport() const { return m_vkPhysicalDeviceFeatures.sparseBinding; }

bool MVulkanPhysicalDevice::ASTCTextureSupport() const
{
    static const auto formatProperties = GetFormatProperties(VK_FORMAT_ASTC_4x4_UNORM_BLOCK);

    return (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
}

bool MVulkanPhysicalDevice::BCTextureSupport() const
{
    static const auto formatProperties = GetFormatProperties(VK_FORMAT_BC7_UNORM_BLOCK);

    return (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
}

std::set<MString> MVulkanPhysicalDevice::GetNotSupportDeviceExtension(
        VkPhysicalDevice         device,
        const std::set<MString>& tRequiredExtensions
) const
{
    std::set<MString> tNotSupportExtensions = tRequiredExtensions;

    uint32_t          extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    for (const auto& extension: availableExtensions)
    {
        //found
        tNotSupportExtensions.erase(extension.extensionName);
    }

    return tNotSupportExtensions;
}

#endif

#include "backend/platforms/VulkanPlatform.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "volk.h"
#include "vulkan/VulkanDriver.h"
#include "vulkan/utils/Helper.h"

namespace engine::backend {

namespace {

using ExtensionSet = VulkanPlatform::ExtensionSet;

inline bool setContains(ExtensionSet const& set, std::string const& extension) {
  return set.find(extension) != set.end();
};

const std::string_view DESIRED_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation",
};

std::vector<const char*> getEnabledLayers() {
  constexpr size_t kMaxEnabledLayersCount =
      sizeof(DESIRED_LAYERS) / sizeof(DESIRED_LAYERS[0]);

  std::vector<VkLayerProperties> const availableLayers =
      vkutils::enumerate(vkEnumerateInstanceLayerProperties);

  std::vector<const char*> enabledLayers;
  enabledLayers.reserve(kMaxEnabledLayersCount);
  for (auto const& desired : DESIRED_LAYERS) {
    for (VkLayerProperties const& layer : availableLayers) {
      std::string_view const availableLayer(layer.layerName);
      if (availableLayer == desired) {
        enabledLayers.push_back(desired.data());
        break;
      }
    }
  }
  return enabledLayers;
}

ExtensionSet getInstanceExtensions(
    ExtensionSet const& externallyRequiredExts = {}) {
  ExtensionSet const TARGET_EXTS = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
  };
  ExtensionSet exts;
  std::vector<VkExtensionProperties> const availableExts =
      vkutils::enumerate(vkEnumerateInstanceExtensionProperties,
                         static_cast<char const*>(nullptr));
  for (auto const& extension : availableExts) {
    std::string name{extension.extensionName};

    if (name.size() == 0) {
      continue;
    }

    if (setContains(TARGET_EXTS, name) ||
        setContains(externallyRequiredExts, name)) {
      exts.insert(name);
    }
  }
  return exts;
}

VkInstance createInstance(ExtensionSet const& requiredExts) {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pEngineName = "Engine";
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstance mInstance;
  VkInstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  bool validationFeaturesSupported = false;

  auto const enabledLayers = getEnabledLayers();
  if (!enabledLayers.empty()) {
    std::vector<VkExtensionProperties> const availableValidationExts =
        vkutils::enumerate(vkEnumerateInstanceExtensionProperties,
                           "VK_LAYER_KHRONOS_validation");
    for (auto const& extProps : availableValidationExts) {
      if (!strcmp(extProps.extensionName,
                  VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) {
        validationFeaturesSupported = true;
        break;
      }
    }
    instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
  } else {
    LOG(INFO)
        << "Validation layer not available; did you install the Vulkan SDK?\n"
        << "Please ensure that VK_LAYER_PATH is set correctly.";
  }

  constexpr uint32_t MAX_INSTANCE_EXTENSION_COUNT = 8;
  char const* ppEnabledExtensions[MAX_INSTANCE_EXTENSION_COUNT];
  uint32_t enabledExtensionCount = 0;

  if (validationFeaturesSupported) {
    ppEnabledExtensions[enabledExtensionCount++] =
        VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME;
  }
  for (auto const& requiredExt : requiredExts) {
    assert(enabledExtensionCount < MAX_INSTANCE_EXTENSION_COUNT);
    ppEnabledExtensions[enabledExtensionCount++] = requiredExt.data();
  }

  instanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
  instanceCreateInfo.ppEnabledExtensionNames = ppEnabledExtensions;

  VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance);
  CHECK(result == VK_SUCCESS) << "Unable to create Vulkan instance. error="
                              << static_cast<int32_t>(result);
  return mInstance;
}

}  // anonymous namespace

void VulkanPlatform::terminate() { vkDestroyInstance(mInstance, nullptr); }

Driver* VulkanPlatform::createDriver() noexcept {
  VkResult result = volkInitialize();
  CHECK(result == VK_SUCCESS) << "Unable to load Vulkan entry points.";

  VulkanContext context;
  ExtensionSet instExts;
  instExts = getInstanceExtensions();
  mInstance = createInstance(instExts);
  assert(mInstance != VK_NULL_HANDLE);

  volkLoadInstance(mInstance);

  context.mDebugUtilsSupported =
      setContains(instExts, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef NDEBUG
  CHECK(!context.mDebugUtilsSupported)
      << "Debug utils should not be enabled in release build.";
#endif

  mContext = context;

  return VulkanDriver::create(this, context);
}

VulkanPlatform::VulkanPlatform() : mInstance(VK_NULL_HANDLE), mContext({}) {}

VulkanPlatform::~VulkanPlatform() = default;

VkInstance VulkanPlatform::getInstance() const noexcept { return mInstance; }

}  // namespace engine::backend

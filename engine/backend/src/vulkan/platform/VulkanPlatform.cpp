#include "backend/platforms/VulkanPlatform.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "volk.h"
#include "vulkan/VulkanDriver.h"
#include "vulkan/utils/Helper.h"

namespace engine::backend {

namespace {

constexpr uint32_t INVALID_VK_INDEX = 0xFFFFFFFF;

using ExtensionSet = VulkanPlatform::ExtensionSet;

inline bool setContains(ExtensionSet const& set, std::string const& extension) {
  return set.find(extension) != set.end();
};

#ifndef NDEBUG
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
#endif

template <typename StructA, typename StructB>
StructA* chainStruct(StructA* structA, StructB* structB) {
  structB->pNext = const_cast<void*>(structA->pNext);
  structA->pNext = (void*)structB;
  return structA;
}

void printDeviceInfo(VkInstance instance, VkPhysicalDevice device) {
  if (vkGetPhysicalDeviceProperties2) {
    VkPhysicalDeviceDriverProperties driverProperties{};
    driverProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
    VkPhysicalDeviceProperties2 physicalDeviceProperties2{};
    physicalDeviceProperties2.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    chainStruct(&physicalDeviceProperties2, &driverProperties);
    vkGetPhysicalDeviceProperties2(device, &physicalDeviceProperties2);
    LOG(INFO) << "Vulkan device driver: " << driverProperties.driverName << " "
              << driverProperties.driverInfo;
  }

  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);

  uint32_t const driverVersion = deviceProperties.driverVersion;
  uint32_t const vendorID = deviceProperties.vendorID;
  uint32_t const deviceID = deviceProperties.deviceID;
  int const major = VK_VERSION_MAJOR(deviceProperties.apiVersion);
  int const minor = VK_VERSION_MINOR(deviceProperties.apiVersion);

  std::vector<VkPhysicalDevice> const physicalDevices =
      vkutils::enumerate(vkEnumeratePhysicalDevices, instance);

  LOG(INFO) << "Selected physical device '" << deviceProperties.deviceName
            << "' from " << physicalDevices.size() << " physical devices. "
            << "(vendor " << std::hex << vendorID << ", "
            << "device " << deviceID << ", "
            << "driver " << driverVersion << ", " << std::dec << "api " << major
            << "." << minor << ")";
}

ExtensionSet getInstanceExtensions(
    ExtensionSet const& externallyRequiredExts = {}) {
  ExtensionSet const TARGET_EXTS = {
#ifndef NDEBUG
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
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
  appInfo.apiVersion = VK_API_VERSION_1_1;

  VkInstance mInstance;
  VkInstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  bool validationFeaturesSupported = false;

#ifndef NDEBUG
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
#endif

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

std::vector<VkQueueFamilyProperties>
getPhysicalDeviceQueueFamilyPropertiesHelper(VkPhysicalDevice device) {
  uint32_t queueFamiliesCount;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount,
                                           nullptr);
  std::vector<VkQueueFamilyProperties> queueFamiliesProperties(
      queueFamiliesCount);
  if (queueFamiliesCount > 0) {
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount,
                                             queueFamiliesProperties.data());
  }
  return queueFamiliesProperties;
}

uint32_t identifyGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice,
                                          VkQueueFlags flags) {
  const std::vector<VkQueueFamilyProperties> queueFamiliesProperties =
      getPhysicalDeviceQueueFamilyPropertiesHelper(physicalDevice);
  uint32_t graphicsQueueFamilyIndex = INVALID_VK_INDEX;
  for (uint32_t j = 0; j < queueFamiliesProperties.size(); ++j) {
    VkQueueFamilyProperties props = queueFamiliesProperties[j];
    if (props.queueCount != 0 && props.queueFlags & flags) {
      graphicsQueueFamilyIndex = j;
      break;
    }
  }
  return graphicsQueueFamilyIndex;
}

inline int deviceTypeOrder(VkPhysicalDeviceType deviceType) {
  constexpr std::array<VkPhysicalDeviceType, 5> TYPES = {
      VK_PHYSICAL_DEVICE_TYPE_OTHER,
      VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
      VK_PHYSICAL_DEVICE_TYPE_CPU,
      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
  };
  if (auto itr = std::find(TYPES.begin(), TYPES.end(), deviceType);
      itr != TYPES.end()) {
    return std::distance(TYPES.begin(), itr);
  }
  return -1;
}

VkPhysicalDevice selectPhysicalDevice(VkInstance instance) {
  std::vector<VkPhysicalDevice> const physicalDevices =
      vkutils::enumerate(vkEnumeratePhysicalDevices, instance);
  struct DeviceInfo {
    VkPhysicalDevice device = VK_NULL_HANDLE;
    VkPhysicalDeviceType deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
    int8_t index = -1;
    std::string_view name;
  };
  std::vector<DeviceInfo> deviceList(physicalDevices.size());

  for (size_t deviceInd = 0; deviceInd < physicalDevices.size(); ++deviceInd) {
    auto const candidateDevice = physicalDevices[deviceInd];
    VkPhysicalDeviceProperties targetDeviceProperties;
    vkGetPhysicalDeviceProperties(candidateDevice, &targetDeviceProperties);

    int const major = VK_VERSION_MAJOR(targetDeviceProperties.apiVersion);
    int const minor = VK_VERSION_MINOR(targetDeviceProperties.apiVersion);

    if (major < 1) {
      continue;
    }
    if (major == 1 && minor < 1) {
      continue;
    }

    if (identifyGraphicsQueueFamilyIndex(
            candidateDevice, VK_QUEUE_GRAPHICS_BIT) == INVALID_VK_INDEX) {
      continue;
    }

    deviceList[deviceInd].device = candidateDevice;
    deviceList[deviceInd].deviceType = targetDeviceProperties.deviceType;
    deviceList[deviceInd].index = (int8_t)deviceInd;
    deviceList[deviceInd].name = targetDeviceProperties.deviceName;
  }

  std::sort(deviceList.begin(), deviceList.end(),
            [](DeviceInfo const& a, DeviceInfo const& b) {
              if (b.device == VK_NULL_HANDLE) {
                return false;
              }
              if (a.device == VK_NULL_HANDLE) {
                return true;
              }
              return deviceTypeOrder(a.deviceType) <
                     deviceTypeOrder(b.deviceType);
            });
  auto device = deviceList.back().device;
  CHECK(device != VK_NULL_HANDLE) << "Unable to find suitable device.";
  return device;
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

  mPhysicalDevice = selectPhysicalDevice(mInstance);
  assert(mPhysicalDevice != VK_NULL_HANDLE);

  printDeviceInfo(mInstance, mPhysicalDevice);

  context.mDebugUtilsSupported =
      setContains(instExts, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#ifdef NDEBUG
  CHECK(!context.mDebugUtilsSupported)
      << "Debug utils should not be enabled in release build.";
#endif

  mContext = context;

  return VulkanDriver::create(this, context);
}

VulkanPlatform::VulkanPlatform()
    : mInstance(VK_NULL_HANDLE),
      mPhysicalDevice(VK_NULL_HANDLE),
      mContext({}) {}

VulkanPlatform::~VulkanPlatform() = default;

VkInstance VulkanPlatform::getInstance() const noexcept { return mInstance; }

VkPhysicalDevice VulkanPlatform::getPhysicalDevice() const noexcept {
  return mPhysicalDevice;
}

}  // namespace engine::backend

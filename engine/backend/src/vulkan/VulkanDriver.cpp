#include "VulkanDriver.h"

#include <backend/platforms/VulkanPlatform.h>

#include "absl/log/check.h"
#include "absl/log/log.h"

namespace engine::backend {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT* cbdata, void* pUserData) {
  if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    LOG(ERROR) << "VULKAN ERROR: (" << cbdata->pMessageIdName << ") "
               << cbdata->pMessage;
  } else {
    LOG(WARNING) << "VULKAN WARNING: (" << cbdata->pMessageIdName << ") "
                 << cbdata->pMessage;
  }
  return VK_FALSE;
}

}  // anonymous namespace

using DebugUtils = VulkanDriver::DebugUtils;
DebugUtils* DebugUtils::mSingleton = nullptr;

DebugUtils::DebugUtils(VkInstance instance, VkDevice device,
                       VulkanContext const* context)
    : mInstance(instance),
      mDevice(device),
      mEnabled(context->isDebugUtilsSupported()),
      mDebugMessenger(VK_NULL_HANDLE) {
  if (mEnabled) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
    createInfo.pfnUserCallback = debugUtilsCallback;

    VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &createInfo,
                                                     nullptr, &mDebugMessenger);
    CHECK(result == VK_SUCCESS)
        << "Unable to create Vulkan debug messenger. error="
        << static_cast<int32_t>(result);
  }
}

DebugUtils* DebugUtils::get() {
  assert(DebugUtils::mSingleton);
  return DebugUtils::mSingleton;
}

DebugUtils::~DebugUtils() {
  if (mDebugMessenger) {
    vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
  }
}

void DebugUtils::setName(VkObjectType type, uint64_t handle, char const* name) {
  auto impl = DebugUtils::get();
  if (!impl->mEnabled) {
    return;
  }
  VkDebugUtilsObjectNameInfoEXT info{};
  info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
  info.objectType = type;
  info.objectHandle = handle;
  info.pObjectName = name;
  vkSetDebugUtilsObjectNameEXT(impl->mDevice, &info);
}

inline VulkanDriver::VulkanDriver(VulkanPlatform* mPlatform,
                                  VulkanContext const& context) noexcept
    : mPlatform(mPlatform), mContext(context) {
  DebugUtils::mSingleton =
      new DebugUtils(mPlatform->getInstance(), VK_NULL_HANDLE, &context);
}

VulkanDriver::~VulkanDriver() noexcept = default;

Driver* engine::backend::VulkanDriver::create(
    VulkanPlatform* mPlatform, VulkanContext const& context) noexcept {
  return new VulkanDriver(mPlatform, context);
}

void VulkanDriver::terminate() {
  assert(DebugUtils::mSingleton);
  delete DebugUtils::mSingleton;

  mPlatform->terminate();
}

}  // namespace engine::backend

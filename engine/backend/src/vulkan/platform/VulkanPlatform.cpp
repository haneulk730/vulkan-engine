#include "backend/platforms/VulkanPlatform.h"

#include <vulkan/vulkan.h>

#include "absl/log/check.h"
#include "vulkan/VulkanDriver.h"

namespace engine::backend {

namespace {

VkInstance createInstance() {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pEngineName = "Engine";
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstance mInstance;
  VkInstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;

  VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance);
  CHECK(result == VK_SUCCESS) << "Unable to create Vulkan instance. error="
                              << static_cast<int32_t>(result);
  return mInstance;
}

}  // anonymous namespace

VulkanPlatform::VulkanPlatform() : mInstance(VK_NULL_HANDLE) {}

VulkanPlatform::~VulkanPlatform() = default;

Driver* VulkanPlatform::createDriver() noexcept {
  mInstance = createInstance();
  assert(mInstance != VK_NULL_HANDLE);

  return VulkanDriver::create(this);
}

}  // namespace engine::backend

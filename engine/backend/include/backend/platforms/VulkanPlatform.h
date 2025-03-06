#pragma once

#include <backend/Platform.h>

#include <string>
#include <unordered_set>

#include "volk.h"
#include "vulkan/VulkanContext.h"

namespace engine::backend {

class VulkanPlatform : public Platform {
 public:
  using ExtensionSet = std::unordered_set<std::string>;

  VulkanPlatform();

  ~VulkanPlatform();

  Driver* createDriver() noexcept override;

  virtual void terminate();

  VkInstance getInstance() const noexcept;

  VkDevice getDevice() const noexcept;

  VkPhysicalDevice getPhysicalDevice() const noexcept;

  uint32_t getGraphicsQueueFamilyIndex() const noexcept;

  uint32_t getGraphicsQueueIndex() const noexcept;

  VkQueue getGraphicsQueue() const noexcept;

 private:
  static VkSurfaceKHR createVkSurfaceKHR(void* nativeWindow,
                                         VkInstance instance) noexcept;

  VkInstance mInstance;
  VkPhysicalDevice mPhysicalDevice;
  VkDevice mDevice;
  uint32_t mGraphicsQueueFamilyIndex;
  uint32_t mGraphicsQueueIndex;
  VkQueue mGraphicsQueue;
  VulkanContext mContext;
};

}  // namespace engine::backend

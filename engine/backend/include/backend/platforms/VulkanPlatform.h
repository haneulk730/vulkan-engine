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

  VkPhysicalDevice getPhysicalDevice() const noexcept;

 private:
  VkInstance mInstance;
  VkPhysicalDevice mPhysicalDevice;
  VulkanContext mContext;
};

}  // namespace engine::backend

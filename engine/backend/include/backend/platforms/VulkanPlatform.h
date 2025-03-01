#pragma once

#include <backend/Platform.h>
#include <vulkan/vulkan.h>

namespace engine::backend {

class VulkanPlatform : public Platform {
 public:
  VulkanPlatform();

  ~VulkanPlatform();

  Driver* createDriver() noexcept override;

 private:
  VkInstance mInstance;
};

}  // namespace engine::backend

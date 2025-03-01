#pragma once

#include "DriverBase.h"
#include "private/backend/Driver.h"

namespace engine::backend {

class VulkanPlatform;

class VulkanDriver final : public DriverBase {
 public:
  static Driver* create(VulkanPlatform* mPlatform) noexcept;

  inline VulkanDriver(VulkanPlatform* mPlatform) noexcept;

  ~VulkanDriver() noexcept override;

 private:
  VulkanPlatform* mPlatform;
};

}  // namespace engine::backend

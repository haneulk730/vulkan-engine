#include "VulkanDriver.h"

namespace engine::backend {

Driver* engine::backend::VulkanDriver::create(
    VulkanPlatform* mPlatform) noexcept {
  return new VulkanDriver(mPlatform);
}

inline VulkanDriver::VulkanDriver(VulkanPlatform* mPlatform) noexcept
    : mPlatform(mPlatform) {}

VulkanDriver::~VulkanDriver() noexcept = default;

}  // namespace engine::backend

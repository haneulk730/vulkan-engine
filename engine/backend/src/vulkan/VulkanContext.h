#pragma once

#include "volk.h"

namespace engine::backend {

struct VulkanContext {
 public:
  inline bool isDebugUtilsSupported() const noexcept {
    return mDebugUtilsSupported;
  }

 private:
  bool mDebugUtilsSupported = false;

  friend class VulkanPlatform;
};

}  // namespace engine::backend

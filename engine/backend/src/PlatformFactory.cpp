#include <private/backend/PlatformFactory.h>

#include "backend/platforms/VulkanPlatform.h"

namespace engine::backend {

Platform* PlatformFactory::create() noexcept { return new VulkanPlatform(); }

void PlatformFactory::destroy(Platform** mPlatform) noexcept {
  delete *mPlatform;
  *mPlatform = nullptr;
}

}  // namespace engine::backend

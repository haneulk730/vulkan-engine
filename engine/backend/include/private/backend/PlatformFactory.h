#pragma once

namespace engine::backend {

class Platform;

class PlatformFactory {
 public:
  static Platform* create() noexcept;

  static void destroy(Platform** mPlatform) noexcept;
};

}  // namespace engine::backend

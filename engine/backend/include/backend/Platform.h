#pragma once

namespace engine::backend {

class Driver;

class Platform {
 public:
  Platform() noexcept;

  virtual ~Platform() noexcept;

  virtual Driver* createDriver() noexcept = 0;
};

}  // namespace engine::backend

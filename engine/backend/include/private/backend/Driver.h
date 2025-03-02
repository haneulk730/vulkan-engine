#pragma once

namespace engine::backend {

class Driver {
 public:
  virtual ~Driver() noexcept;

  virtual void terminate() = 0;
};

}  // namespace engine::backend

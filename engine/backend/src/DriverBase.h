#pragma once

#include "private/backend/Driver.h"

namespace engine::backend {

class DriverBase : public Driver {
 public:
  DriverBase() noexcept;
  ~DriverBase() noexcept override;
};

}  // namespace engine::backend

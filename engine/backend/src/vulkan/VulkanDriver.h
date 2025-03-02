#pragma once

#include "DriverBase.h"
#include "VulkanContext.h"
#include "private/backend/Driver.h"

namespace engine::backend {

class VulkanPlatform;

class VulkanDriver final : public DriverBase {
 public:
  static Driver* create(VulkanPlatform* mPlatform,
                        VulkanContext const& context) noexcept;

  class DebugUtils {
   public:
    static void setName(VkObjectType type, uint64_t handle, char const* name);

   private:
    static DebugUtils* get();

    DebugUtils(VkInstance instance, VkDevice device,
               VulkanContext const* context);
    ~DebugUtils();

    VkInstance const mInstance;
    VkDevice const mDevice;
    bool const mEnabled;
    VkDebugUtilsMessengerEXT mDebugMessenger;

    static DebugUtils* mSingleton;

    friend class VulkanDriver;
  };

  inline VulkanDriver(VulkanPlatform* mPlatform,
                      VulkanContext const& context) noexcept;

  ~VulkanDriver() noexcept override;

  void terminate() override;

  VulkanDriver(VulkanDriver const&) = delete;
  VulkanDriver& operator=(VulkanDriver const&) = delete;

 private:
  VulkanPlatform* mPlatform;

  VulkanContext mContext;
};

}  // namespace engine::backend

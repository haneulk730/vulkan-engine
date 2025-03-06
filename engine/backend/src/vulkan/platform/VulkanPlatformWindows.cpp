#include <backend/platforms/VulkanPlatform.h>
#include <stddef.h>
#include <stdint.h>
#include <volk.h>

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#endif

#include <tuple>

#include "absl/log/check.h"

namespace engine::backend {

VkSurfaceKHR VulkanPlatform::createVkSurfaceKHR(void* nativeWindow,
                                                VkInstance instance) noexcept {
  VkSurfaceKHR surface;

#if defined(WIN32)
  SetThreadDpiAwarenessContext(
      GetWindowDpiAwarenessContext((HWND)nativeWindow));

  VkWin32SurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hinstance = GetModuleHandle(nullptr);
  createInfo.hwnd = (HWND)nativeWindow;
  VkResult const result = vkCreateWin32SurfaceKHR(
      instance, &createInfo, nullptr, (VkSurfaceKHR*)&surface);
  CHECK(result == VK_SUCCESS) << "vkCreateWin32SurfaceKHR failed."
                              << " error=" << static_cast<int32_t>(result);
#endif
  return surface;
}

}  // namespace engine::backend

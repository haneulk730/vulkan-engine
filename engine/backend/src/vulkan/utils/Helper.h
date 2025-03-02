#pragma once

#include <vector>

#include "absl/log/check.h"
#include "volk.h"

namespace engine::backend::vkutils {

#define EXPAND_ENUM(...)                                 \
  uint32_t size = 0;                                     \
  VkResult result = func(__VA_ARGS__, nullptr);          \
  CHECK(result == VK_SUCCESS) << "enumerate size error"; \
  std::vector<OutType> ret(size);                        \
  result = func(__VA_ARGS__, ret.data());                \
  CHECK(result == VK_SUCCESS) << "enumerate error";      \
  return std::move(ret);

#define EXPAND_ENUM_NO_ARGS() EXPAND_ENUM(&size)
#define EXPAND_ENUM_ARGS(...) EXPAND_ENUM(__VA_ARGS__, &size)

template <typename OutType>
std::vector<OutType> enumerate(VKAPI_ATTR VkResult (*func)(uint32_t*,
                                                           OutType*)) {
  EXPAND_ENUM_NO_ARGS();
}

template <typename InType, typename OutType>
std::vector<OutType> enumerate(VKAPI_ATTR VkResult (*func)(InType, uint32_t*,
                                                           OutType*),
                               InType inData) {
  EXPAND_ENUM_ARGS(inData);
}

template <typename InTypeA, typename InTypeB, typename OutType>
std::vector<OutType> enumerate(VKAPI_ATTR VkResult (*func)(InTypeA, InTypeB,
                                                           uint32_t*, OutType*),
                               InTypeA inDataA, InTypeB inDataB) {
  EXPAND_ENUM_ARGS(inDataA, inDataB);
}

#undef EXPAND_ENUM
#undef EXPAND_ENUM_NO_ARGS
#undef EXPAND_ENUM_ARGS

}  // namespace engine::backend::vkutils

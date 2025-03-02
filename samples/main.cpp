#include <GLFW/glfw3.h>
#include <backend/Platform.h>
#include <private/backend/Driver.h>
#include <private/backend/PlatformFactory.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

using namespace engine::backend;

class VulkanApp {
 public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

 private:
  GLFWwindow* mWindow;

  Driver* mDriver;

  Platform* mPlatform;

  void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  }

  void initVulkan() {
    mPlatform = PlatformFactory::create();
    mDriver = mPlatform->createDriver();
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(mWindow)) {
      glfwPollEvents();
    }
  }

  void cleanup() {
    mDriver->terminate();

    PlatformFactory::destroy(&mPlatform);

    glfwDestroyWindow(mWindow);

    glfwTerminate();
  }
};

int main() {
  VulkanApp app;

  app.run();

  return 0;
}

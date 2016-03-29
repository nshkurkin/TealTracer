///
///
///
///
///

#include "glfw_base.h"

using namespace glfw;

util::Vec2i glfw::getWindowSize(GLFWwindow * window) {
    util::Vec2i size;
    glfwGetWindowSize(window, &size.x(), &size.y());
    return size;
}

util::Vec2i glfw::getFramebufferSize(GLFWwindow * window) {
    util::Vec2i size;
    glfwGetFramebufferSize(window, &size.x(), &size.y());
    return size;
}

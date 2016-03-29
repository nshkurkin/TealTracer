///
/// glfw_base.h
/// -----------
/// Nikolai Shkurkin
/// GLFW Extension Library
///

#ifndef ____glfw_base__
#define ____glfw_base__

#include <src/nsgl/gl_interface/gl.h>
#include <src/util/util.h>
//#include "../nsgl/gl_interface/gl.h"
//#include "../util/util.h"

namespace glfw {
    DEFINE_EIGEN_TYPES_EXT(int, i)
    DEFINE_EIGEN_TYPES_EXT(float, f)
    
    /// Gets the current size of `window`.
    util::Vec2i getWindowSize(GLFWwindow * window);
    util::Vec2i getFramebufferSize(GLFWwindow * window);
}

#endif // ____glfw_base__

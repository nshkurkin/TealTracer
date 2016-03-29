//
//  gl_include.h
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef gl_include_h
#define gl_include_h

//// Represents the target opengl major version.
#define OPENGL_MAJOR_VERSION 4
/// Represents the target opengl minor verions.
#define OPENGL_MINOR_VERSION 0
////

#if (OPENGL_MAJOR_VERSION >= 3 && OPENGL_MINOR_VERSION >= 2) \
    || (OPENGL_MAJOR_VERSION >= 4)
/// MODERN_OPENGL is a shorthand for if OpenGL >=3.2 is used
#define MODERN_OPENGL
#endif

#ifdef MODERN_OPENGL
/// Define this to get 3.2+ OpenGL contexts in GLFW
#ifndef GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLCOREARB
#endif
#endif

#ifndef MODERN_OPENGL
#ifdef __APPLE__
/// For some reason GLFW will include latest headers even if not using 3.2+ OGL
/// Makes sure to include Apple's OpenGl 2.0 headers (not gl3.h)
#include <OpenGL/gl.h>
#endif
#endif

#ifdef __WINDOWS__
/// GLEW_STATIC is need for when linkign to GLEW's static libraries on Windows.
//#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>
#endif

/// Include GL and the Windowing API
/// However, here you can set the API that you use, such as glut or straight gl.h
#include <GLFW/glfw3.h>

#ifdef OLD_OPENGL
#ifdef __APPLE__
// Mac OpenGL < 3.2 has special functions for some reason
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glGenVertexArray glGenVertexArrayAPPLE
#define glBindVertexArrays glBindVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#define glDeleteVertexArray glDeleteVertexArrayAPPLE
#endif
#endif

#endif /* gl_include_h */

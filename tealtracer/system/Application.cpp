///
/// Application.cpp
/// ---------------
///


#include "Application.h"
#include <src/nsgl/nsgl.h>
#include <src/glfw/glfw.h>

using namespace game;

Application::Application(std::string title, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    /// Tell GLFW what kind of OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
#ifdef MODERN_OPENGL
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    mainWindow = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (mainWindow == NULL) {
        glfwTerminate();
        return;
    }
    
    glfwSetWindowUserPointer(mainWindow, this);
    setCallbacksForWindow(mainWindow);
    glfwMakeContextCurrent(mainWindow);
    
#ifdef __WINDOWS__
    NSGL_ERRORS(); // Make sure no errors to this point
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        return;
    glGetError(); // Ignore INVALID_ENUM thrown by glewInit
#endif

    cursorShouldBeCaptured = false;
}

Application::~Application() {
    
}

void Application::toggleCursorCaptured() {
    cursorShouldBeCaptured = !cursorShouldBeCaptured;
    setCursorCaptured(cursorShouldBeCaptured);
}

bool Application::windowOpen() {
    return !glfwWindowShouldClose(mainWindow);
}

void Application::swapBuffers() {
    glfwSwapBuffers(mainWindow);
}

void Application::initGL() {
    nsgl::assertMinimumGLVersion(OPENGL_MAJOR_VERSION, OPENGL_MINOR_VERSION);
    
    // Set background color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    /// Check for OpenGL errors
    NSGL_ERRORS();
}

void Application::setCursorCaptured(bool captured) {
    if (captured)
        glfwSetInputMode(getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Application::quit() {
    glfwSetWindowShouldClose(mainWindow, GL_TRUE);
}

GLFWwindow * Application::getWindow() {
    return mainWindow;
}

void Application::setEventListener(EventListener * listener) {
    this->listener = listener;
}

void Application::keyDown(int key, int scancode, int mods) {
    // NSGL_LOG("Key: " + glfw::getKeyName(key) + " was pressed.");
    if (listener != NULL)
        listener->keyDown(key, scancode, mods);
}
void Application::keyUp(int key, int scancode, int mods) {
    if (listener != NULL)
        listener->keyUp(key, scancode, mods);
}
void Application::mouseUp(int button, int mods) {
    if (listener != NULL)
        listener->mouseUp(button, mods);
}
void Application::mouseDown(int button, int mods) {
    if (listener != NULL)
        listener->mouseDown(button, mods);
}
void Application::mouseMoved(double x, double y) {
    if (listener != NULL)
        listener->mouseMoved(x, y);
}
void Application::mouseScroll(double dx, double dy) {
    if (listener != NULL)
        listener->mouseScroll(dx, dy);
}

void Application::windowResize(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    
    if (listener != NULL)
        listener->windowResize(w, h);
}
void Application::framebufferResize(int w, int h) {
    if (listener != NULL)
        listener->framebufferResize(w, h);
}

int Application::getCurrentKeyModifierFlags() {
    int modifierBits = 0;
    if (glfwGetKey(getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
     || glfwGetKey(getWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        modifierBits |= GLFW_MOD_SHIFT;
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS
     || glfwGetKey(getWindow(), GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        modifierBits |= GLFW_MOD_CONTROL;
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS
     || glfwGetKey(getWindow(), GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
        modifierBits |= GLFW_MOD_ALT;
    }
    if (glfwGetKey(getWindow(), GLFW_KEY_LEFT_SUPER) == GLFW_PRESS
     || glfwGetKey(getWindow(), GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS) {
        modifierBits |= GLFW_MOD_SUPER;
    }
    return modifierBits;
}

void Application::broadcastKeyState(int key) {
    int state = glfwGetKey(getWindow(), key);
    if (state == GLFW_PRESS)
        this->keyDown(key, 0, getCurrentKeyModifierFlags());
    else if (state == GLFW_RELEASE)
        this->keyUp(key, 0, getCurrentKeyModifierFlags());
}


void Application::broadcastWindowAndFramebufferResize() {
    util::Vec2i windowSize = glfw::getWindowSize(getWindow());
    util::Vec2i framebufferSize = glfw::getFramebufferSize(getWindow());
    
    windowResize(windowSize.x(), windowSize.y());
    framebufferResize(framebufferSize.x(), framebufferSize.y());
}

///
///
/// Static functions and variables handled by the application class itself
void Application::setCallbacksForWindow(GLFWwindow * window) {
    glfwSetKeyCallback(window, keyEventFunc);
    glfwSetCursorPosCallback(window, mouseMotionEventFunc);
    glfwSetMouseButtonCallback(window, mouseButtonEventFunc);
    glfwSetScrollCallback(window, mouseScrollEventFunc);
    
    glfwSetWindowSizeCallback(window, windowResizeFunc);
    glfwSetFramebufferSizeCallback(window, framebufferResizeFunc);
}

void Application::keyEventFunc(GLFWwindow * window, int key, int scancode,
 int action, int mods) {
    Application *application = (Application*)glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS)
        application->keyDown(key, scancode, mods);
    else if (action == GLFW_RELEASE)
        application->keyUp(key, scancode, mods);
}

void Application::mouseMotionEventFunc(GLFWwindow * window, double xpos,
 double ypos) {
    Application *application = (Application*)glfwGetWindowUserPointer(window);
    application->mouseMoved(xpos, ypos);
}

void Application::mouseButtonEventFunc(GLFWwindow * window, int button,
 int action, int mods) {
    Application *application = (Application*)glfwGetWindowUserPointer(window);
    if (action == GLFW_PRESS)
        application->mouseDown(button, mods);
    else if (action == GLFW_RELEASE)
        application->mouseUp(button, mods);
}

void Application::mouseScrollEventFunc(GLFWwindow * window, double xoffset,
 double yoffset) {
    Application *application = (Application*)glfwGetWindowUserPointer(window);
    application->mouseScroll(xoffset, yoffset);
}

/// Window events
void Application::windowResizeFunc(GLFWwindow * window, int w, int h) {
    Application *application = (Application*)glfwGetWindowUserPointer(window);
    application->windowResize(w, h);
}

void Application::framebufferResizeFunc(GLFWwindow * window, int w, int h) {
    Application *application = (Application*)glfwGetWindowUserPointer(window);
    application->framebufferResize(w, h);
}

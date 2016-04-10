//
//  Window.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef Window_hpp
#define Window_hpp

#include "TSWindow.hpp"

#include "gl_include.h"
#include <string>

class Window : public TSWindow {
public:
    ///
    Window() {
        cachedWidth_ = -1;
        cachedHeight_ = -1;
        window_ = nullptr;
        listener_ = TSNullUserEventListener::createManaged(new TSNullUserEventListener());
    }

    ///
    virtual ~Window() {
        if (window_) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
    }
    
    ///
    virtual void setup(int width, int height, const std::string & title) {
        /// Tell GLFW what kind of OpenGL context
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
#ifdef MODERN_OPENGL
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
        setGLFWWindowPointer(glfwCreateWindow(width, height, title.c_str(), NULL, NULL));
        setTitle(title);
        if (window_ == NULL) {
            glfwTerminate();
            return;
        }
    }
    
    ///
    virtual int width() const {
        return cachedWidth_;
    }
    
    ///
    virtual int height() const {
        return cachedHeight_;
    }
    
    ///
    virtual void setWidth(int width) {
        cachedWidth_ = width;
        setWindowDims_();
    }
    
    ///
    virtual void setHeight(int height) {
        cachedHeight_ = height;
        setWindowDims_();
    }
    
    ///
    virtual std::string title() const {
        return cachedTitle_;
    }
    
    ///
    virtual void setTitle(const std::string & title) {
        cachedTitle_ = title;
        glfwSetWindowTitle(window_, title.c_str());
    }
    
    ///
    virtual int frameBufferWidth() const {
        return cachedFBWidth_;
    }
    
    ///
    virtual int frameBufferHeight() const {
        return cachedFBHeight_;
    }
    
    ///
    virtual int posX() const {
        return cachedX_;
    }
    
    ///
    virtual int posY() const {
        return cachedY_;
    }
    
    ///
    virtual void setPosX(int x) {
        cachedX_ = x;
        setWindowPos_();
    }
    
    ///
    virtual void setPosY(int y) {
        cachedY_ = y;
        setWindowPos_();
    }
    
    ///
    virtual void setEventListener(std::shared_ptr<TSUserEventListener> listener) {
        listener_ = listener;
    }
    
    ///
    virtual std::shared_ptr<TSUserEventListener> getEventListener() const {
        return listener_;
    }
    
    ///
    virtual void makeContextCurrent() {
        glfwMakeContextCurrent(window_);
    }
    
    ///
    virtual void swapBuffers() {
        glfwSwapBuffers(window_);
    }
    
    ///
    virtual bool opened() {
        return !glfwWindowShouldClose(window_);
    }
    
    ///
    virtual void close() {
        glfwSetWindowShouldClose(window_, GL_TRUE);
    }
    
    ///
    virtual bool keyDown(int key) const {
        return glfwGetKey(window_, key) == GLFW_PRESS;
    }

private:
    
    ///
    void setGLFWWindowPointer(GLFWwindow * window) {
        window_ = window;
        if (window_ != nullptr) {
            updateWindowDims_();
            updateWindowPos_();
            glfwSetWindowUserPointer(window_, this);
            glfwSetKeyCallback(window_, keyEventFunc_);
            glfwSetCursorPosCallback(window_, mouseMotionEventFunc_);
            glfwSetMouseButtonCallback(window_, mouseButtonEventFunc_);
            glfwSetScrollCallback(window_, mouseScrollEventFunc_);
            glfwSetWindowSizeCallback(window_, windowResizeFunc_);
            glfwSetFramebufferSizeCallback(window_, framebufferResizeFunc_);
            glfwSetWindowCloseCallback(window_, windowCloseFunc_);
            glfwSetWindowPosCallback(window_, windowPosFunc_);
        }
    }

    ///
    virtual void prepareForDrawing() {
        makeContextCurrent();
    }
    
    ///
    virtual void finishDrawing() {
        swapBuffers();
    }

private:
    GLFWwindow * window_;
    
    ///
    int cachedWidth_;
    ///
    int cachedHeight_;
    
    ///
    int cachedFBWidth_;
    ///
    int cachedFBHeight_;
    
    ///
    int cachedX_;
    ///
    int cachedY_;
    
    ///
    std::string cachedTitle_;
    
    ///
    std::shared_ptr<TSUserEventListener> listener_;
    
    ///
    void setWindowDims_() const {
        glfwSetWindowSize(window_, cachedWidth_, cachedHeight_);
    }
    
    ///
    void updateWindowDims_() {
        glfwGetWindowSize(window_, &cachedWidth_, &cachedHeight_);
        glfwGetFramebufferSize(window_, &cachedFBWidth_, &cachedFBHeight_);
    }
    
    ///
    void setWindowPos_() const {
        glfwSetWindowPos(window_, cachedX_, cachedY_);
    }
    
    ///
    void updateWindowPos_() {
        glfwGetWindowPos(window_, &cachedX_, &cachedY_);
    }
    
    ///
    static void keyEventFunc_(GLFWwindow * window, int key, int scancode,
     int action, int mods) {
        auto win = (Window*)glfwGetWindowUserPointer(window);
        if (action == GLFW_PRESS) {
            win->listener_->keyDown(win, key, scancode, mods);
        }
        else if (action == GLFW_RELEASE)
            win->listener_->keyUp(win, key, scancode, mods);
    }

    ///
    static void mouseMotionEventFunc_(GLFWwindow * window, double xpos,
     double ypos) {
        auto win = (Window*)glfwGetWindowUserPointer(window);
        win->listener_->mouseMoved(win, xpos, ypos);
    }

    ///
    static void mouseButtonEventFunc_(GLFWwindow * window, int button,
     int action, int mods) {
        auto win = (Window*)glfwGetWindowUserPointer(window);
        if (action == GLFW_PRESS) {
            win->listener_->mouseDown(win, button, mods);
        }
        else if (action == GLFW_RELEASE) {
            win->listener_->mouseUp(win, button, mods);
        }
    }

    ///
    static void mouseScrollEventFunc_(GLFWwindow * window, double xoffset,
     double yoffset) {
        auto win = (Window*)glfwGetWindowUserPointer(window);
        win->listener_->mouseScroll(win, xoffset, yoffset);
    }

    ///
    static void windowResizeFunc_(GLFWwindow * window, int w, int h) {
        auto win = (Window*)glfwGetWindowUserPointer(window);
        win->updateWindowDims_();
        win->drawingDelegate()->windowResize(win, w, h);
    }

    ///
    static void framebufferResizeFunc_(GLFWwindow * window, int w, int h) {
        auto win = (Window*)glfwGetWindowUserPointer(window);
        win->updateWindowDims_();
        win->drawingDelegate()->framebufferResize(win, w, h);
    }
    
    ///
    static void windowCloseFunc_(GLFWwindow * window) {
        auto win = (Window*)glfwGetWindowUserPointer(window);
        win->drawingDelegate()->windowClose(win);
    }
    
    ///
    static void windowPosFunc_(GLFWwindow * window, int x, int y) {
        auto win = (Window*)glfwGetWindowUserPointer(window);
        win->updateWindowPos_();
    }
};

#endif /* Window_hpp */

//
//  TSWindow.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSWindow_hpp
#define TSWindow_hpp

#include <memory>

#include "TSManagedObject.hpp"

class TSWindow;

class TSUserEventListener : public TSManagedObject {
public:        
    virtual void keyDown(TSWindow * window, int key, int scancode, int mods) = 0;
    virtual void keyUp(TSWindow * window, int key, int scancode, int mods) = 0;
    virtual void mouseUp(TSWindow * window, int button, int mods) = 0;
    virtual void mouseDown(TSWindow * window, int button, int mods) = 0;
    virtual void mouseMoved(TSWindow * window, double x, double y) = 0;
    virtual void mouseScroll(TSWindow * window, double dx, double dy) = 0;
};

class TSNullUserEventListener : public TSUserEventListener {
public:
    virtual ~TSNullUserEventListener();
        
    virtual void keyDown(TSWindow * window, int key, int scancode, int mods);
    virtual void keyUp(TSWindow * window, int key, int scancode, int mods);
    virtual void mouseUp(TSWindow * window, int button, int mods);
    virtual void mouseDown(TSWindow * window, int button, int mods);
    virtual void mouseMoved(TSWindow * window, double x, double y);
    virtual void mouseScroll(TSWindow * window, double dx, double dy);
};

class TSWindowDrawingDelegate {
public:
    virtual ~TSWindowDrawingDelegate() {};
    
    virtual void setupDrawingInWindow(TSWindow * window) = 0;
    virtual void drawInWindow(TSWindow * window) = 0;
    
    virtual void windowResize(TSWindow * window, int w, int h) = 0;
    virtual void framebufferResize(TSWindow * window, int w, int h) = 0;
    virtual void windowClose(TSWindow * window) = 0;
};

class TSNullWindowDrawingDelegate : public TSWindowDrawingDelegate {
public:
    virtual ~TSNullWindowDrawingDelegate() {}
    
    virtual void setupDrawingInWindow(TSWindow * window) {}
    virtual void drawInWindow(TSWindow * window) {}
    
    virtual void windowResize(TSWindow * window, int w, int h) {}
    virtual void framebufferResize(TSWindow * window, int w, int h) {}
    virtual void windowClose(TSWindow * window) {}
};

#include <string>

class TSWindow : public TSManagedObject {
public:
    TSWindow();
    virtual ~TSWindow();

    virtual void setup(int width, int height, const std::string & title) = 0;
    
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual std::string title() const = 0;
    virtual void setWidth(int width) = 0;
    virtual void setHeight(int height) = 0;
    virtual void setTitle(const std::string & title) = 0;
    
    virtual int frameBufferWidth() const = 0;
    virtual int frameBufferHeight() const = 0;
    
    virtual int posX() const = 0;
    virtual int posY() const = 0;
    virtual void setPosX(int x) = 0;
    virtual void setPosY(int y) = 0;
    
    virtual void makeContextCurrent() = 0;
    virtual void swapBuffers() = 0;
    virtual bool opened() = 0;
    virtual void close() = 0;
    
    virtual bool keyDown(int key) const = 0;
    
    void setDrawingDelegate(std::shared_ptr<TSWindowDrawingDelegate> delegate);
    std::shared_ptr<TSWindowDrawingDelegate> drawingDelegate() const;
    
    /// Call this function to have the `drawingDelegate` call `drawInWindow` at
    /// the appropriate time.
    void draw();
    
    virtual void setEventListener(std::shared_ptr<TSUserEventListener> listener) = 0;
    virtual std::shared_ptr<TSUserEventListener> getEventListener() const = 0;
    
protected:
    virtual void prepareForDrawing() = 0;
    virtual void finishDrawing() = 0;

private:

    std::shared_ptr<TSWindowDrawingDelegate> drawingDelegate_;

};

#endif /* TSWindow_hpp */

//
//  TSWindow.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TSWindow.hpp"

TSNullUserEventListener::~TSNullUserEventListener() {}
    
void TSNullUserEventListener::keyDown(TSWindow * window, int key, int scancode, int mods) {}
void TSNullUserEventListener::keyUp(TSWindow * window, int key, int scancode, int mods) {}
void TSNullUserEventListener::mouseUp(TSWindow * window, int button, int mods) {}
void TSNullUserEventListener::mouseDown(TSWindow * window, int button, int mods) {}
void TSNullUserEventListener::mouseMoved(TSWindow * window, double x, double y) {}
void TSNullUserEventListener::mouseScroll(TSWindow * window, double dx, double dy) {}
void TSNullUserEventListener::windowResize(TSWindow * window, int w, int h) {}
void TSNullUserEventListener::framebufferResize(TSWindow * window, int w, int h) {}

void TSNullUserEventListener::windowClose(TSWindow * window) {}

///
TSWindow::TSWindow() {
    drawingDelegate_ = std::shared_ptr<TSWindowDrawingDelegate>(new TSNullWindowDrawingDelegate());
}

///
TSWindow::~TSWindow() {}

///
void
TSWindow::setDrawingDelegate(std::shared_ptr<TSWindowDrawingDelegate> delegate) {
    drawingDelegate_ = delegate;
    makeContextCurrent();
    drawingDelegate_->setupDrawingInWindow(this);
}

///
std::shared_ptr<TSWindowDrawingDelegate>
TSWindow::drawingDelegate() const {
    return drawingDelegate_;
}

///
void
TSWindow::draw() {
    prepareForDrawing();
    drawingDelegate_->drawInWindow(this);
    finishDrawing();
}

///
/// Application.h
/// --------------
///


#ifndef ____game_Application__
#define ____game_Application__

#include <src/glfw/glfw.h>
//#include "../../glfw/glfw.h"
#include "EventListener.h"

namespace game {
    
    class Application : public EventListener {
    public:
        
        /// Sets up and executes the application
        ///   In particular, it sets up |instance| and calls initGL and initScene
        ///   as well as creating and launching the application window. After this
        ///   is complete, the main event loop begins until the window is closed.
        void setCallbacksForWindow(GLFWwindow * window);
        
        bool windowOpen();
        GLFWwindow * getWindow();
        void swapBuffers();
        
        Application(std::string title, int width, int height);
        ~Application();
        
        //// Called to setup opengl states.
        void initGL();
        void quit();
        
        void setEventListener(EventListener * listener);
        
        int getHeight() { return windowHeight; }
        int getWidth() { return windowWidth; }
        
        void setCursorCaptured(bool captured);
        void toggleCursorCaptured();
        
    private:
        /// Storage of window size
        int windowHeight, windowWidth;
        bool cursorShouldBeCaptured;
        /// Handle on the main window
        GLFWwindow * mainWindow;
        
        /// The event listener that Application will forward events to.
        EventListener * listener;
        
    public:
        int getCurrentKeyModifierFlags();
    
        void keyDown(int key, int scancode, int mods);
        void keyUp(int key, int scancode, int mods);
        void mouseUp(int button, int mods);
        void mouseDown(int button, int mods);
        void mouseMoved(double x, double y);
        void mouseScroll(double dx, double dy);
        void windowResize(int w, int h);
        void framebufferResize(int w, int h);
        
        void broadcastKeyState(int key);
        void broadcastWindowAndFramebufferResize();
   private:
        /// Events in the Event loop
        static void keyEventFunc(GLFWwindow * window, int key, int scancode,
         int action, int mods);
        static void mouseMotionEventFunc(GLFWwindow * window, double xpos,
         double ypos);
        static void mouseButtonEventFunc(GLFWwindow * window, int button, int action,
         int mods);
        static void mouseScrollEventFunc(GLFWwindow * window, double xoffset,
         double yoffset);
        /// Window events
        static void windowResizeFunc(GLFWwindow * window, int w, int h);
        static void framebufferResizeFunc(GLFWwindow * window, int w, int h);
    };
    
}

#endif // defined(____Application__)

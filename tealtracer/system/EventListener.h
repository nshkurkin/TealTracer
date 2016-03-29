///
///
///
///
///
///

#ifndef ____game_EventListener__
#define ____game_EventListener__

namespace game {

    class EventListener {
    public:
        EventListener() {}
        virtual ~EventListener() {};
        
        virtual void keyDown(int key, int scancode, int mods) {};
        virtual void keyUp(int key, int scancode, int mods) {};
        virtual void mouseUp(int button, int mods) {};
        virtual void mouseDown(int button, int mods) {};
        virtual void mouseMoved(double x, double y) {};
        virtual void mouseScroll(double dx, double dy) {};
        virtual void windowResize(int w, int h) {};
        virtual void framebufferResize(int w, int h) {};
    };

}

#endif

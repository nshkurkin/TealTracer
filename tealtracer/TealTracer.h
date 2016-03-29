///
///
///
///
///
///

#ifndef ____GameEngine__
#define ____GameEngine__

#include <src/game/app/app.h>

///
class GameEngine {
public:
    GameEngine();
    ~GameEngine();
    
    void run();
private:
    game::Application application;
    game::StateManager stateManager;
    game::ResourceManager resourceManager;
    
    int FPSCount;
    float FPSTicks;
    
    // variable time step
    float curFrameTick, prevFrameTick;
};


#endif

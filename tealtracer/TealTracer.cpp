///
///
///
///
///
///

#include "GameEngine.h"

#include <src/glfw/glfw.h>
#include <src/states/overworld/OverworldGameState.h>

using namespace game;

GameEngine::GameEngine() : application("VitalK", 800 * 1.2, 500 * 1.2) {

}

GameEngine::~GameEngine() {

}

void GameEngine::run() {
    float dt;

    /// Intialize first scene
    application.initGL();

    stateManager.init(&application, &resourceManager);
    stateManager.pushState(new OverworldGameState());

    prevFrameTick = 0;
    curFrameTick = (float) glfwGetTime();
   
    FPSCount = 0;
    FPSTicks = (float) glfwGetTime();
    
    /// Enter main event loop
    while (application.windowOpen()) {
        glfwPollEvents();

        prevFrameTick = curFrameTick;
        curFrameTick = (float) glfwGetTime();
        dt = curFrameTick - prevFrameTick;
        
        resourceManager.updateAsyncLoads();
        stateManager.update(dt);
        stateManager.render();

        FPSCount++;
        if (((float) glfwGetTime()) - FPSTicks > 1.0f) {
            FPSTicks = FPSTicks + 1.0f;
            stateManager.top()->setFPS(FPSCount);
            FPSCount = 0;
        }
    }
    
    stateManager.popAll();
}

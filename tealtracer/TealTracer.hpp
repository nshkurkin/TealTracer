//
//  TealTracer.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TealTracer_hpp
#define TealTracer_hpp

#include "TSApplication.hpp"
#include "PovrayScene.hpp"

#include <cassert>

#include "gl_include.h"
#include "Window.hpp"
#include "GPURayTracer.hpp"
#include "CPURayTracer.hpp"

class TealTracer : public TSApplication {
public:

    ///
    std::shared_ptr<TSWindow> gpuWindow();
    ///
    std::shared_ptr<TSWindow> cpuWindow();
    
protected:

    ///
    virtual int run(const std::vector<std::string> & args);
    ///
    virtual std::shared_ptr<TSWindow> newWindow();
    ///
    virtual void quit();
    
private:

    std::shared_ptr<GPURayTracer> gpuRayTracer_;
    std::shared_ptr<CPURayTracer> cpuRayTracer_;
    std::shared_ptr<PovrayScene> scene_;

};

#endif /* TealTracer_hpp */

//
//  main.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TealTracer.hpp"
#include "opengl_errors.hpp"

///
int main(int argc, const char * argv[]) {
    std::cout << ns_requestOpenGLAPIErrorInfoForFunction("glBindBuffer").description() << std::endl;

    return TSApplication::main(new TealTracer(), argc, argv);
}


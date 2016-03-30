//
//  main.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/29/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "TealTracer.hpp"
#include "PovrayParser.hpp"

///
int main(int argc, const char * argv[]) {

    PovrayParser::loadScene("/Users/Bo/Documents/Programming/csc490/tealtracer/tealtracer/lab1_simple.pov");

    return TSApplication::main(new TealTracer(), argc, argv);
}


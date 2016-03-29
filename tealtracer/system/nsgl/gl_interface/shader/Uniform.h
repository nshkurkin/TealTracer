///
/// Uniform.h
/// ---------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_Uniform__
#define ____nsgl_Uniform__

#include <string>
#include <src/nsgl/nsgl_base.h>
//#include "../../nsgl_base.h"

namespace nsgl {
    
    /// Represents a uniform from a shader program.
    struct Uniform {
        std::string name;
        GLint location;
        
        Uniform() : name(""), location(-1) {}
        Uniform(std::string name, GLint location) : name(name), location(location) {}
        
        bool isValid() {return location >= 0;}
    };
    
}

#endif // ____nsgl_Uniform__

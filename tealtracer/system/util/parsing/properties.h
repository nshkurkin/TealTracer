///
/// properties.h
/// ------------
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_properties__
#define ____util_properties__

#include <map>
#include <vector>

#include <src/util/util_base.h>
#include <src/util/util_types.h>
#include <src/util/data/Opt.h>

#include "fileIO.h"

namespace util {
    
    /// Represents a particular `property` in a file. It is equivalent to a
    /// string of the form "type:name=value".
    struct Property {
        /// The name of this property.
        std::string name;
        /// A string the represents the vlaue of thie property.
        std::string value;
        /// The type of this property as a string.
        std::string type;
        
        /// Creates an empty property.
        Property();
        Property(std::string value);
        /// Designated initializer.
        Property(std::string name, std::string value, std::string type);
        
        bool isDataType(std::string type);
        bool isBool();
        bool isDouble();
        bool isString();
        bool isVec3();
        bool isMat4();
        bool isArray();
        
        Opt<bool> cachedBool;
        Opt<double> cachedDouble;
        Opt<Vec3d> cachedVec3d;
        Opt<Mat4d> cachedMat4d;
        Opt< std::vector<double> > cachedDoubleArray;
        void clearCaches();
        
        bool        asBool();
        float       asFloat(float def = 0.0f);
        double      asDouble(double def = 0.0);
        std::string asString();
        Vec3d       asVec3d(Vec3d def = Vec3d(0, 0, 0));
        Vec3f       asVec3f(Vec3f def = Vec3f(0, 0, 0));
        Mat4d       asMat4d(Mat4d def = Mat4d::Identity());
        Mat4f       asMat4f(Mat4f def = Mat4f::Identity());
        std::vector<double> asArray();
    };
    
    /// Represents a collection of Property objects read from a file.
    struct Properties {
        std::string name;
        std::map< std::string, Property > propertyMap;
        
        bool hasProperty(std::string key);
        Property & getProperty(std::string key);
    };
    
    /// Reads a property file named `fname` and outputs a collection of
    /// Properties to be used in the program. File is of the form:
    /// 1] type:key=value
    /// 2] # This is a comment
    /// 3] type:key=value
    /// ...
    Properties readPropertiesFile(std::string fname);
}


#endif // ____util_properties__

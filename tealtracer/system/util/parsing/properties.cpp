///
/// properties.cpp
/// --------------
/// Nikolai Shkurkin
/// Utility Library
///

#include "properties.h"

using namespace util;

/////
Property::Property() {}
Property::Property(std::string value) : name("(no name)"), value(value),
 type("string") {}
Property::Property(std::string name, std::string value, std::string type)
: name(name), value(value), type(type) {}

bool Property::isDataType(std::string type) {
    return this->type == type;
}

bool Property::isBool() {return isDataType("bool");}
bool Property::isDouble() {return isDataType("double") || isDataType("float");}
bool Property::isString() {return isDataType("string");}
bool Property::isVec3() {
    return isDataType("Vec3d") || isDataType("Vec3f") || isDataType("Vec3");
}
bool Property::isMat4() {
    return isDataType("matrix")
     || isDataType("mat4")
     || isDataType("mat4f")
     || isDataType("mat4f");
}
bool Property::isArray() {
    return isDataType("array")
     || isDataType("double[]")
     || isDataType("float[]");
}

bool Property::asBool() {
    if (!cachedBool.isSet())
        cachedBool.set(value == "yes" || value == "true");
    return cachedBool.get();
}
float Property::asFloat(float def) {return (float) asDouble(def);}
double Property::asDouble(double def) {
    if (!cachedDouble.isSet()) {
        StringParser parser(value);
        double v = def;
        parser >> v;
        cachedDouble.set(v);
    }
    return cachedDouble.get();
}
std::string Property::asString() {return value;}
Vec3d Property::asVec3d(Vec3d def) {
    if (!cachedVec3d.isSet()) {
        StringParser parser(value);
        Vec3d v = def;
        double d;
        
        for (int i = 0; i < 3 && parser >> d; i++)
            v[i] = d;
        cachedVec3d.set(v);
    }
    return cachedVec3d.get();
}
Vec3f Property::asVec3f(Vec3f def) {
    return asVec3d(def.cast<double>()).cast<float>();
}
Mat4d Property::asMat4d(Mat4d def) {
    if (!cachedMat4d.isSet()) {
        StringParser parser(value); Mat4d m = def; double d;
        for (int i = 0; i < 16 && parser >> d; i++)
            m.block<1, 1>(i%4, i/4) << d;
        cachedMat4d.set(m);
    }
    return cachedMat4d.get();
}
Mat4f Property::asMat4f(Mat4f def) {
    return asMat4d(def.cast<double>()).cast<float>();
}
std::vector<double> Property::asArray() {
    if (!cachedDoubleArray.isSet()) {
        std::vector<double> array;
        double parsedDouble; StringParser parser(value);
        while (parser >> parsedDouble)
            array.push_back(parsedDouble);
        cachedDoubleArray.set(array);
    }
    return cachedDoubleArray.get();
}

void Property::clearCaches() {
    cachedBool.clearSet();
    cachedDouble.clearSet();
    cachedVec3d.clearSet();
    cachedMat4d.clearSet();
    cachedDoubleArray.clearSet();
}

///
///
///
bool Properties::hasProperty(std::string key) {
    return propertyMap.count(key) != 0;
}

Property & Properties::getProperty(std::string key) {
    UTIL_ASSERT(hasProperty(key));
    return propertyMap[key];
}


///////
Properties util::readPropertiesFile(std::string fname) {
    FileReader reader(fname);
    Properties props;
    
    props.name = fname;
    while (reader.hasNextLine()) {
        std::string line(reader.readln());
        if (line.length() > 0 && line[0] != '#') {
            std::string typeAndKey = trimRight(line, "=");
            std::string type = trimRight(typeAndKey, ":");
            std::string key = trimLeft(typeAndKey, ":");
            std::string value = trimLeft(line, "=");
            //LOG("Read property: " + key + " " + value + " " + type);
            props.propertyMap[key] = Property(key, value, type);
        }
    }
    return props;
}

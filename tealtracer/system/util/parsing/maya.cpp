///
/// maya.cpp
/// --------
/// Nikolai Shkurkin
/// Utility Library
///

#include "maya.h"

using namespace util;

//////
maya::Attribute::Attribute() {
    rawParseableLine = rawParseableLine = name = "";
    dataType = maya::Attribute::Type::Unknown;
    arraySize = -1;
}

void maya::Attribute::handleParse(bool parseResult, const std::string & msg) {
    if (!parseResult)
        LOG("Error: failed to parse " + msg + " for " + name);
}

bool maya::Attribute::isDataType(maya::Attribute::Type::Enum type) {
    return type == dataType;
}
bool maya::Attribute::isUnknownType() {
    return isDataType(maya::Attribute::Type::Unknown);
}
bool maya::Attribute::isBool() {return isDataType(maya::Attribute::Type::Bool);}
bool maya::Attribute::asBool() {
    return rawData.find("yes") != std::string::npos;
}

bool maya::Attribute::isDouble() {return isDataType(maya::Attribute::Type::Double);}
double maya::Attribute::asDouble(double def) {
    double toRet = def;
    handleParse(!(util::StringParser(rawData) >> toRet).fail(), "double");
    return toRet;
}
bool maya::Attribute::isFloat() {return isDouble();}
float maya::Attribute::asFloat(float def) {
    return float(asDouble(double(def)));
}

bool maya::Attribute::isString() {return isDataType(maya::Attribute::Type::String);}
std::string maya::Attribute::asString() {return rawData;}

bool maya::Attribute::isVec3d() {return isDataType(maya::Attribute::Type::Vec3d);}
maya::Vec3d maya::Attribute::asVec3d(maya::Vec3d def) {
    util::StringParser parser(rawData);
    int count = 0;
    double nextValue;
    Vec3d toRet = def;
    while (count < 3 && parser >> nextValue)
        toRet[count++] = nextValue;
    if (count != 3)
        LOG("Error: failed to parse Vec3d for " + name + ":\n" + rawParseableLine);
    return toRet;
}
bool maya::Attribute::isVec3f() {return isVec3d();}
maya::Vec3f maya::Attribute::asVec3f(maya::Vec3f def) {
    return asVec3d(def.cast<double>()).cast<float>();
}

bool maya::Attribute::isMat4d() {return isDataType(maya::Attribute::Type::Mat4d);}
maya::Mat4d maya::Attribute::asMat4d(maya::Mat4d def) {
    util::StringParser parser(rawData);
    int count = 0;
    double nextValue;
    maya::Mat4d toRet = def;
    while (count < 16 && parser >> nextValue) {
        toRet.block<1, 1>(count % 4, count / 4) << nextValue;
        count++;
    }
    if (count != 16)
        LOG("Error: failed to parse Mat4d for " + name + ":\n" + rawParseableLine);
    return toRet;
}
bool maya::Attribute::isMat4f() {return isMat4d();}
maya::Mat4f maya::Attribute::asMat4f(maya::Mat4f def) {
    return asMat4d(def.cast<double>()).cast<float>();
}

bool maya::Attribute::isArray() {return isDataType(maya::Attribute::Type::Array);}
std::vector<double> maya::Attribute::asArray() {
    std::vector<double> array;
    int count = 0;
    double parsedDouble;
    util::StringParser parser(rawData);
    while (parser >> parsedDouble) {
        count++;
        array.push_back(parsedDouble);
    }
    if (count != arraySize) {
        LOG("Could not parse " + arraySize + " elements from "
            + rawData + ", instead parsed " + count);
    }
    return array;
}

std::string maya::Attribute::typeAsString() {
    if (dataType == maya::Attribute::Type::String)
        return "string";
    else if (dataType == maya::Attribute::Type::Bool)
        return "bool";
    else if (dataType == maya::Attribute::Type::Double)
        return "double";
    else if (dataType == maya::Attribute::Type::Vec3d)
        return "Vec3d";
    else if (dataType == maya::Attribute::Type::Mat4d)
        return "Mat4d";
    else if (dataType == maya::Attribute::Type::Array)
        return "Array";
    else
        return "unknown";
}

///////
maya::Node::Node(std::string type) : nodeType(type) {}

bool maya::Node::isNodeType(std::string type) {return type == nodeType;}
bool maya::Node::hasAttribute(std::string name) {
    int which = -1;
    while (++which < (int) attributeList.size()
           && attributeList[which].name != name);
    bool found = which < (int) attributeList.size();
    if (found)
        cachedAttribute = std::make_pair(std::make_pair(name, which),
                                         attributeList[which]);
    return found;
}
maya::Attribute maya::Node::getAttribute(const std::string & name) {
    if (hasAttribute(name))
        return cachedAttribute.second;
    else
        return Attribute();
}


///////
bool lineEndsWithSemicolon(std::string line) {
    return line.length() > 0 && line[line.length() - 1] == ';';
}

///////
maya::FileContentWrapper maya::parseFile(std::string fname) {
    using namespace maya;
    
    FileContentWrapper contents;
    FileReader reader(fname);
    std::vector< std::string > lines;
    
    contents.fileName = fname;
    
    // Read in each line completely (until ending semicolon)
    lines.push_back("");
    while (reader.hasNextLine()) {
        std::string line(reader.readln());
        //UTIL_LOG("Read line: " + line);
        if (!lineEndsWithSemicolon(lines[lines.size() - 1]))
            lines[lines.size() - 1] = lines[lines.size() - 1] + line;
        else
            lines.push_back(line);
    }
    
    //UTIL_WHAT_IS(lines.size());
    
    // Now we read in each whole command, now we parse out the nodes, attributes,
    // and connections.
    int which = -1;
    Node node;
    while (++which < (int) lines.size()) {
        std::string line = lines[which];
        // Create a node
        if (line.find("createNode") != std::string::npos) {
            node = Node();
            node.rawParseableLine = line;
            
            line = trimLeft(line, "createNode");
            // Read in the type of the node
            {
                StringParser parser(line);
                parser >> node.nodeType;
            }
            if (line.find("-name") != std::string::npos) {
                line = trimLeft(line, "-name");
                line = trimLeft(line, "\"");
                line = trimRight(line, "\"");
                
                StringParser parser(line);
                parser >> node.name;
            }
            else {
                node.name = node.nodeType + "-" + which;
            }
            
            //UTIL_LOG("Created new node: " + node.nodeType + " " + node.name);
            contents.nodes[node.name] = node;
        }
        // Set an attribute of the last created node
        else if (line.find("setAttr") != std::string::npos) {
            Node & lastNode = contents.nodes[node.name];
            Attribute attr = Attribute();
            
            attr.rawParseableLine = line;
            
            // Get the size (if applicable)
            if (line.find("-size") != std::string::npos) {
                std::string str = trimLeft(line, "-size");
                StringParser parser(str);
                parser >> attr.arraySize;
            }
            
            // Get the attribute name
            line = trimLeft(line, ".");
            attr.name = trimRight(line, "\"");
            line = trimLeft(line, "\"");
            
            // Get the type that is defined
            if (line.find("-type") != std::string::npos) {
                line = trimLeft(line, "-type");
                if (line.find("\"string\"") != std::string::npos)
                    attr.dataType = maya::Attribute::Type::String;
                else if (line.find("\"double\"") != std::string::npos)
                    attr.dataType = maya::Attribute::Type::Double;
                else if (line.find("\"double3\"") != std::string::npos)
                    attr.dataType = maya::Attribute::Type::Vec3d;
                else if (line.find("\"matrix\"") != std::string::npos)
                    attr.dataType = maya::Attribute::Type::Mat4d;
                else
                    attr.dataType = maya::Attribute::Type::Unknown;
                
                line = trimLeft(line, "-type");
                // Get rid of type name
                line = trimLeft(line, "\"");
                line = trimLeft(line, "\"");
                // Removing ending semicolon
                line = trimRight(line, ";");
                if (attr.isString())
                    line = trimRight(line, "\"");
                else {
                    while (line.find("\"") != std::string::npos)
                        line = trimLeft(line, "\"");
                }
                attr.rawData = line;
            }
            // Type not specified
            else {
                // Trim off excess specifiers
                while (line.find("\"") != std::string::npos)
                    line = trimLeft(line, "\"");
                line = trimRight(line, ";");
                attr.rawData = line;
                
                // If we can parse a double, its double
                StringParser parser(line);
                float tmpf;
                if (parser >> tmpf)
                    attr.dataType = maya::Attribute::Type::Double;
                // If we can parse a bool, it bool
                else if (line.find("yes") != std::string::npos
                         || line.find("no") != std::string::npos
                         || line.length() == 0)
                    attr.dataType = maya::Attribute::Type::Bool;
                else
                    attr.dataType = maya::Attribute::Type::Unknown;
            }
            
            // All array types have a non-zero "size"
            if (attr.arraySize != -1)
                attr.dataType = maya::Attribute::Type::Array;
            
            //LOG("Read attribute: " + attr.name + " (type) "
            // + attr.typeAsString() + " (rawData) \n" + attr.rawData);
            
            lastNode.attributeList.push_back(attr);
        }
        else if (line.find("connectAttr") != std::string::npos) {
            std::string left = line, right = line;
            
            left = trimLeft(left, "\"");
            left = trimRight(left, "\"");
            
            right = trimLeft(right, "\"");
            right = trimLeft(right, "\"");
            right = trimLeft(right, "\"");
            right = trimRight(right, "\"");
            
            contents.connections.push_back(std::make_pair(left, right));
            
            //LOG("Added connection: " + left + " <-> " + right);
        }
        // Ignore attribute adds, and other lines
    }
    
    return contents;
}

///////
bool maya::FileContentWrapper::hasNode(std::string nodeName) {
    return nodes.count(nodeName) != 0;
}

maya::Node & maya::FileContentWrapper::getNode(std::string nodeName) {
    return nodes[nodeName];
}



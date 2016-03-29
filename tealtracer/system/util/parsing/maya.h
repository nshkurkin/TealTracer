///
/// maya.h
/// ------
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_maya__
#define ____util_maya__

#include <map>
#include <vector>

#include <src/util/util_base.h>
#include <src/util/util_types.h>
//#include "../util_base.h"
//#include "../util_types.h"
#include "fileIO.h"

namespace util {
    namespace maya {
        DEFINE_EIGEN_TYPES_EXT(float, f)
        DEFINE_EIGEN_TYPES_EXT(double, d)
        
        /// Defines an attribute of a `node` in a maya ASCII file.
        struct Attribute {
            /// Holds all of the "types" that Maya can represent.
            struct Type {
                /// All of the specific parseable types.
                enum Enum {Unknown = 0, String, Bool, Double, Vec3d, Mat4d, Array};
            };
            
            /// The entire line that define this attribute.
            std::string rawParseableLine;
            /// The data component of `rawParesableLine`.
            std::string rawData;
            /// The name of this attribute.
            std::string name;
            /// The type of `rawData`
            Type::Enum dataType;
            /// If this attribute is an array, this is the size of that array.
            int arraySize;
            
            /// Creates an empty attribute.
            Attribute();
            
            ///
            void handleParse(bool parseResult, const std::string & msg);
            
            /// Returns the type of this attribute as a human-readable string.
            std::string typeAsString();
            /// Whether or not this attribute is of type `type`.
            bool isDataType(Type::Enum type);
            
            bool isUnknownType();
            bool isBool();
            bool isFloat();
            bool isDouble();
            bool isString();
            bool isVec3d();
            bool isVec3f();
            bool isMat4d();
            bool isMat4f();
            bool isArray();
            
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
    }
}

namespace util {
    namespace maya {
        
        typedef std::vector< std::pair<std::string, std::string> > StringPairList;
        typedef std::vector< Attribute > AttributeList;
        
        /// Represents a collections of attributes in Maya.
        struct Node {
            /// The type of this, e.g. "skinCluster".
            std::string nodeType;
            /// The name given by the user in Maya.
            std::string name;
            /// The raw string that defines this node.
            std::string rawParseableLine;
            /// All of the parsed attributes of this node.
            AttributeList attributeList;
            /// Caches attributes for quick retrieval.
            std::pair< std::pair< std::string, int >, Attribute > cachedAttribute;
            
            /// Creates an empty node.
            Node() : nodeType("Unknown") {};
            /// Create a node with the given `type`.
            Node(std::string type);
            
            bool isNodeType(std::string type);
            /// Checks whether `name` was found. `name`, if it is found, will
            /// be cached allowing for O(1) access when using a subsequent
            /// `getAttribute` call.
            bool hasAttribute(std::string name);
            /// Gets an attributes with `name`. If it can't be found, the
            /// attribute will be empty. Use `hasAttribute` to check whether an
            /// attribute is found.
            Attribute getAttribute(const std::string & name);
        };
        
        typedef std::map< std::string, Node > NodeMap;
        /// Represents a wrapper over an entire maya file. It includes all of
        /// the nodes, connections, and the filename of the maya file.
        struct FileContentWrapper {
            NodeMap nodes;
            StringPairList connections;
            std::string fileName;
            
            bool hasNode(std::string nodeName);
            Node & getNode(std::string nodeName);
        };
        
        /// Parses a maya ascii file name `fname`. Its contents will be fully
        /// expanded in the returned FileContentWrapper.
        FileContentWrapper parseFile(std::string fname);
    }
}


#endif // ____util_maya__

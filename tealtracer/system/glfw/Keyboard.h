///
/// Keyboard.h
/// ----------
/// Nikolai Shkurkin
/// GLFW Extension Library
///

#ifndef ____glfw_Keyboard__
#define ____glfw_Keyboard__

#include <map>
#include <string>

#include "glfw_base.h"

namespace glfw {
    
    /// A static interface for accessing the names for various keycodes.
    struct Keyboard {
    public:
        /// Gets the key code for the given `keyName`. Can either be the long
        /// name or short name for the key.
        static int getKeyCode(std::string keyName);
        /// Gets short debug name for `keycode`.
        static std::string getKeyName(int keyCode);
        /// Get a human-readable key name for `keyCode`.
        static std::string getKeyLongName(int keyCode);
        
    private:
        static bool keyMapIsSetup;
        static std::map< std::string, int > keyToConstMap;
        static std::map< std::string, int > prettyKeyToConstMap;
        static std::map< int, std::string > constToKeyMap;
        static std::map< int, std::string > constToPrettyKeyMap;
        static void setupKeyMap();
    };
    
    /// Gets the key code for the given `keyName`. Can either be the long
    /// name or short name for the key.
    inline int getKeyCode(std::string keyName) {
        return Keyboard::getKeyCode(keyName);
    }
    /// Gets short debug name for `keycode`.
    inline std::string getKeyName(int keyCode) {
        return Keyboard::getKeyName(keyCode);
    }
    /// Get a human-readable key name for `keyCode`.
    inline std::string getKeyLongName(int keyCode) {
        return Keyboard::getKeyLongName(keyCode);
    }
    
}

#endif // ____glfw_Keyboard__

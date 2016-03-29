///
/// Keyboard.cpp
/// ------------
/// Nikolai Shkurkin
/// GLFW Extension Library
///

#include "Keyboard.h"

using namespace glfw;

/// Can either be the long name or short name for the key
int Keyboard::getKeyCode(std::string keyName) {
    setupKeyMap();
    if (keyToConstMap.count(keyName) != 0)
        return keyToConstMap[keyName];
    else if (prettyKeyToConstMap.count(keyName) != 0)
        return prettyKeyToConstMap[keyName];
    else
        return prettyKeyToConstMap["Unknown"];
}
std::string Keyboard::getKeyName(int keyCode) {
    setupKeyMap();
    if (constToKeyMap.count(keyCode) != 0)
        return constToKeyMap[keyCode];
    else
        return constToKeyMap[GLFW_KEY_UNKNOWN];
}
std::string Keyboard::getKeyLongName(int keyCode) {
    setupKeyMap();
    if (constToPrettyKeyMap.count(keyCode) != 0)
        return constToPrettyKeyMap[keyCode];
    else
        return constToPrettyKeyMap[GLFW_KEY_UNKNOWN];
}

bool Keyboard::keyMapIsSetup = false;
std::map< std::string, int > Keyboard::keyToConstMap;
std::map< std::string, int > Keyboard::prettyKeyToConstMap;
std::map< int, std::string > Keyboard::constToKeyMap;
std::map< int, std::string > Keyboard::constToPrettyKeyMap;

#define GLFW_MAP_KEY_TO_STRING(glfwName, short, long) \
glfw::Keyboard::keyToConstMap[ short ] = GLFW_KEY_ ## glfwName ; \
glfw::Keyboard::constToKeyMap[ GLFW_KEY_ ## glfwName ] = short; \
glfw::Keyboard::prettyKeyToConstMap[ long ] = GLFW_KEY_ ## glfwName ; \
glfw::Keyboard::constToPrettyKeyMap[ GLFW_KEY_ ## glfwName ] = long

void Keyboard::setupKeyMap() {
    if (!keyMapIsSetup) {
        GLFW_MAP_KEY_TO_STRING(UNKNOWN,     "(?)", "Unknown");
        GLFW_MAP_KEY_TO_STRING(SPACE,       " ", "Space");
        GLFW_MAP_KEY_TO_STRING(APOSTROPHE,  "'", "Apostrophe");
        GLFW_MAP_KEY_TO_STRING(COMMA,       ",", "Comma");
        GLFW_MAP_KEY_TO_STRING(MINUS,       "-", "Minus");
        GLFW_MAP_KEY_TO_STRING(PERIOD,      ".", "Period");
        GLFW_MAP_KEY_TO_STRING(SLASH,       "/", "FSlash");
        
        GLFW_MAP_KEY_TO_STRING(0,           "0", "0");
        GLFW_MAP_KEY_TO_STRING(1,           "1", "1");
        GLFW_MAP_KEY_TO_STRING(2,           "2", "2");
        GLFW_MAP_KEY_TO_STRING(3,           "3", "3");
        GLFW_MAP_KEY_TO_STRING(4,           "4", "4");
        GLFW_MAP_KEY_TO_STRING(5,           "5", "5");
        GLFW_MAP_KEY_TO_STRING(6,           "6", "6");
        GLFW_MAP_KEY_TO_STRING(7,           "7", "7");
        GLFW_MAP_KEY_TO_STRING(8,           "8", "8");
        GLFW_MAP_KEY_TO_STRING(9,           "9", "9");
        
        GLFW_MAP_KEY_TO_STRING(SEMICOLON,   ";", "Semicolon");
        GLFW_MAP_KEY_TO_STRING(EQUAL,       "=", "Equals");
        
        GLFW_MAP_KEY_TO_STRING(A,           "A", "A");
        GLFW_MAP_KEY_TO_STRING(B,           "B", "B");
        GLFW_MAP_KEY_TO_STRING(C,           "C", "C");
        GLFW_MAP_KEY_TO_STRING(D,           "D", "D");
        GLFW_MAP_KEY_TO_STRING(E,           "E", "E");
        GLFW_MAP_KEY_TO_STRING(F,           "F", "F");
        GLFW_MAP_KEY_TO_STRING(G,           "G", "G");
        GLFW_MAP_KEY_TO_STRING(H,           "H", "H");
        GLFW_MAP_KEY_TO_STRING(I,           "I", "I");
        GLFW_MAP_KEY_TO_STRING(J,           "J", "J");
        GLFW_MAP_KEY_TO_STRING(K,           "K", "K");
        GLFW_MAP_KEY_TO_STRING(L,           "L", "L");
        GLFW_MAP_KEY_TO_STRING(M,           "M", "M");
        GLFW_MAP_KEY_TO_STRING(N,           "N", "N");
        GLFW_MAP_KEY_TO_STRING(O,           "O", "O");
        GLFW_MAP_KEY_TO_STRING(P,           "P", "P");
        GLFW_MAP_KEY_TO_STRING(Q,           "Q", "Q");
        GLFW_MAP_KEY_TO_STRING(R,           "R", "R");
        GLFW_MAP_KEY_TO_STRING(S,           "S", "S");
        GLFW_MAP_KEY_TO_STRING(T,           "T", "T");
        GLFW_MAP_KEY_TO_STRING(U,           "U", "U");
        GLFW_MAP_KEY_TO_STRING(V,           "V", "V");
        GLFW_MAP_KEY_TO_STRING(W,           "W", "W");
        GLFW_MAP_KEY_TO_STRING(X,           "X", "X");
        GLFW_MAP_KEY_TO_STRING(Y,           "Y", "Y");
        GLFW_MAP_KEY_TO_STRING(Z,           "Z", "Z");
        
        GLFW_MAP_KEY_TO_STRING(LEFT_BRACKET, "[", "LBracket");
        GLFW_MAP_KEY_TO_STRING(BACKSLASH,   "\\", "BSlash");
        GLFW_MAP_KEY_TO_STRING(RIGHT_BRACKET, "]", "RBracket");
        GLFW_MAP_KEY_TO_STRING(GRAVE_ACCENT, "`", "Accent");
        
        GLFW_MAP_KEY_TO_STRING(ESCAPE,      "(esc)", "Escape");
        GLFW_MAP_KEY_TO_STRING(ENTER,       "(\\n)", "Enter");
        GLFW_MAP_KEY_TO_STRING(TAB,         "(\\t)", "Tab");
        GLFW_MAP_KEY_TO_STRING(BACKSPACE,   "(\\b)", "Backspace");
        GLFW_MAP_KEY_TO_STRING(INSERT,      "(ins)", "Insert");
        GLFW_MAP_KEY_TO_STRING(DELETE,      "(del)", "Delete");
        GLFW_MAP_KEY_TO_STRING(RIGHT,       "(right)", "RightArrow");
        GLFW_MAP_KEY_TO_STRING(LEFT,        "(left)", "LeftArrow");
        GLFW_MAP_KEY_TO_STRING(DOWN,        "(down)", "DownArrow");
        GLFW_MAP_KEY_TO_STRING(UP,          "(up)", "UpArrow");
        GLFW_MAP_KEY_TO_STRING(PAGE_UP,     "(pg-up)", "PageUp");
        GLFW_MAP_KEY_TO_STRING(PAGE_DOWN,   "(pg-down)", "PageDown");
        GLFW_MAP_KEY_TO_STRING(HOME,        "(home)", "Home");
        GLFW_MAP_KEY_TO_STRING(END,         "(end)", "End");
        GLFW_MAP_KEY_TO_STRING(CAPS_LOCK,   "(caps)", "CapsLock");
        
        GLFW_MAP_KEY_TO_STRING(LEFT_SHIFT,  "(lshift)", "LShift");
        GLFW_MAP_KEY_TO_STRING(RIGHT_SHIFT, "(rshift)", "RShift");
        
        GLFW_MAP_KEY_TO_STRING(LEFT_CONTROL, "(lctrl)", "LCtrl");
        GLFW_MAP_KEY_TO_STRING(RIGHT_CONTROL, "(rctrl)", "RCtrl");
        
        GLFW_MAP_KEY_TO_STRING(LEFT_ALT,    "(lalt)", "LAlt");
        GLFW_MAP_KEY_TO_STRING(RIGHT_ALT,   "(ralt)", "RAlt");
        
        GLFW_MAP_KEY_TO_STRING(LEFT_SUPER,  "(lsuper)", "LSuper");
        GLFW_MAP_KEY_TO_STRING(RIGHT_SUPER, "(rsuper)", "RSuper");
        keyMapIsSetup = true;
    }
}

#undef GLFW_MAP_KEY_TO_STRING



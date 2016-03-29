///
/// util_base.cpp
/// --------------
/// Nikolai Shkurkin
/// Utility Library
///

#include "util_base.h"

IMPL_STR_OP(+, double)

void util::log(std::string toLog, const std::string file, int line) {
    std::cout << "{" << file << ":" << line << "} " << toLog << "\n";
}

std::string util::trimLeft(std::string toTrim, const std::string & t) {
    if (toTrim.find(t) != std::string::npos)
        return toTrim.substr(toTrim.find(t) + t.length());
    else
        return toTrim;
}

std::string util::trimRight(std::string toTrim, const std::string & t) {
    if (toTrim.find(t) != std::string::npos)
        return toTrim.substr(0, toTrim.find(t));
    else
        return toTrim;
}


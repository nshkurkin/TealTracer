//
//  TSLogger.hpp
//  VitalK
//
//  Created by Nikolai Shkurkin on 3/22/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef TSLogger_h
#define TSLogger_h

#include <iostream>
#include <string>

#define TSLoggerLog(out, ...) TSLogger::log(out, "{", TSLogger::ltrim(__FILE__, "VitalK/"), ":", __LINE__, " (", __FUNCTION__, ")} ", __VA_ARGS__)

class TSLogger {
public:
    template <typename OutStream, typename FirstParam>
    static void log(OutStream & ostream, const FirstParam & first) {
        ostream << first << std::endl;
    }

    template <typename OutStream, typename FirstParam, typename... Args>
    static void log(OutStream & ostream, const FirstParam & first, Args... args) {
        ostream << first;
        log(ostream, args...);
    }
    
    /// Returns the string after the first occurence of `t` in `toTrim`
    static std::string ltrim(std::string toTrim, const std::string & t) {
        if (toTrim.find(t) != std::string::npos)
            return toTrim.substr(toTrim.find(t) + t.length());
        else
            return toTrim;
    }
    /// Returns the string before the first occurence of `t` in `toTrim`
    static std::string rtrim(std::string toTrim, const std::string & t) {
        if (toTrim.find(t) != std::string::npos)
            return toTrim.substr(0, toTrim.find(t));
        else
            return toTrim;
    }
};

#endif /* TSLogger_h */

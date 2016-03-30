//
//  PovraySceneElement.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PovraySceneElement.hpp"

#include "TSLogger.hpp"
#include <fstream>

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

///
PovraySceneElement::~PovraySceneElement() {}

///
void
PovraySceneElement::parseBody(std::string & body, const std::map<std::string, std::pair<ValueType, void *>> & contentMapping) {
    trim(body);
    while (body.length() > 0) {
//            TSLoggerLog(std::cout, "body=(", body, ")");
        /// Find the first word we recognize instead?
        std::string earliestWord = "";
        auto earliestLoc = std::string::npos;
        for (auto itr = contentMapping.begin(); itr != contentMapping.end(); itr++) {
            auto word = itr->first;
            auto loc = body.find(word);
            if (earliestLoc == std::string::npos || (earliestLoc != std::string::npos && loc < earliestLoc)) {
                earliestLoc = loc;
                earliestWord = word;
            }
        }
        
        if (earliestLoc != std::string::npos) {
            auto word = earliestWord;
            body = body.substr(earliestLoc + earliestWord.length(), body.length() - earliestLoc - earliestWord.length());
            trim(body);
            if (contentMapping.count(word) != 0) {
//                    TSLoggerLog(std::cout, "recognized word=", word);
                auto valueType = contentMapping.at(word).first;
                auto dataPtr = contentMapping.at(word).second;
                switch (valueType) {
                case Float:
                    parseFloat(body, *(float *)dataPtr);
                    break;
                case Vec3:
                    parseVector3(body, *(Eigen::Vector3f *)dataPtr);
                    break;
                case Vec4:
                    parseVector4(body, *(Eigen::Vector4f *)dataPtr);
                    break;
                case Pigment:
                    parsePigment(body, *(PovrayPigment *)dataPtr);
                    break;
                case Finish:
                    parseFinish(body, *(PovrayFinish *)dataPtr);
                    break;
                default:
                    assert(false);
                    break;
                }
                trim(body);
            }
            else {
                TSLoggerLog(std::cout, "unrecognized word=(", word,")");
            }
        }
        else {
//                TSLoggerLog(std::cout, "nothing useful left in body=(", body, ")");
            body = "";
        }
    }
}

/// Body is of the form "( )<#, #, # >( ...)"
bool
PovraySceneElement::parseVector3(std::string & body, Eigen::Vector3f & vec) {
    auto lessThanLoc = body.find("<");
    auto greaterThanLoc = body.find(">");
    
//        TSLoggerLog(std::cout, "parsing vec3 from body=(", body, ")");
    
    if (lessThanLoc != std::string::npos && greaterThanLoc != std::string::npos) {
        auto valuesOnly = body.substr(lessThanLoc + 1, greaterThanLoc - lessThanLoc - 1);
        while (valuesOnly.find(",") != std::string::npos) {
            valuesOnly[valuesOnly.find(",")] = ' ';
        }
        std::stringstream stream(valuesOnly);
//            TSLoggerLog(std::cout, "vector str =", valuesOnly);
        stream >> vec[0];
        stream >> vec[1];
        stream >> vec[2];
//            TSLoggerLog(std::cout, "vector=", vec);
        body = body.substr(greaterThanLoc + 1, body.length() - greaterThanLoc -1);
//            TSLoggerLog(std::cout, "new body=(", body, ")");
        return true;
    }
    else {
        assert(false);
        return false;
    }
}

///
bool
PovraySceneElement::parseVector4(std::string & body, Eigen::Vector4f & vec) {
    auto lessThanLoc = body.find("<");
    auto greaterThanLoc = body.find(">");
    
//        TSLoggerLog(std::cout, "parsing vec3 from body=(", body, ")");
    
    if (lessThanLoc != std::string::npos && greaterThanLoc != std::string::npos) {
        auto valuesOnly = body.substr(lessThanLoc + 1, greaterThanLoc - lessThanLoc - 1);
        while (valuesOnly.find(",") != std::string::npos) {
            valuesOnly[valuesOnly.find(",")] = ' ';
        }
        std::stringstream stream(valuesOnly);
//            TSLoggerLog(std::cout, "vector str =", valuesOnly);
        stream >> vec[0];
        stream >> vec[1];
        stream >> vec[2];
        stream >> vec[3];
//            TSLoggerLog(std::cout, "vector=", vec);
        body = body.substr(greaterThanLoc + 1, body.length() - greaterThanLoc -1);
//            TSLoggerLog(std::cout, "new body=(", body, ")");
        return true;
    }
    else {
        assert(false);
        return false;
    }
}

/// Body is of the form "( )#( ...)"
bool
PovraySceneElement::parseFloat(std::string & body, float & val) {
    std::stringstream stream(body);
    stream >> val;
    auto loc = stream.tellg();
    if (loc != std::string::npos) {
        body = body.substr(stream.tellg(), body.length() - stream.tellg());
    }
    else {
        body = "";
    }
    return true;
}

///
bool
PovraySceneElement::parsePigment(std::string & body, PovrayPigment & pigment) {
//        TSLoggerLog(std::cout, "parsing pigment");
    pigment.color << 0, 0, 0, 1;
    
    std::map<std::string, std::pair<ValueType, void *>> content;
    content["color rgb"] = std::make_pair(Vec3, &pigment.color);
    content["color rgbf"] = std::make_pair(Vec4, &pigment.color);
    auto firstCurlyLoc = body.find("{");
    auto secondCurlyLoc = body.find("}");
    auto pigmentBody = body.substr(firstCurlyLoc + 1, secondCurlyLoc - firstCurlyLoc - 1);
    
    parseBody(pigmentBody, content);
    
    secondCurlyLoc = body.find("}");
    body = body.substr(secondCurlyLoc + 1, body.length() - secondCurlyLoc - 1);
    return true;
}

///
bool
PovraySceneElement::parseFinish(std::string & body, PovrayFinish & finish) {
//        TSLoggerLog(std::cout, "parsing finish");
    std::map<std::string, std::pair<ValueType, void *>> content;
    content["ambient"] = std::make_pair(Float, &finish.ambient);
    content["diffuse"] = std::make_pair(Float, &finish.diffuse);
    auto firstCurlyLoc = body.find("{");
    auto secondCurlyLoc = body.find("}");
    auto finishBody = body.substr(firstCurlyLoc + 1, secondCurlyLoc - firstCurlyLoc - 1);
    
    parseBody(finishBody, content);
    
    secondCurlyLoc = body.find("}");
    body = body.substr(secondCurlyLoc + 1, body.length() - secondCurlyLoc - 1);
    return true;
}

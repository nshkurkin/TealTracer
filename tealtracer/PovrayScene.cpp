//
//  PovrayScene.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "PovrayScene.hpp"
#include "PovraySceneElements.hpp"

#include <fstream>

///
void
PovrayScene::addElement(std::shared_ptr<PovraySceneElement> element) {
    element->id_ = (uint16_t) elements_.size();
    elements_.push_back(element);
}

///
static inline std::string
trimRight(const std::string & toTrim, const std::string & t) {
    auto result = toTrim.find(t);
    if (result != std::string::npos) {
        return toTrim.substr(0, result);
    }
    else {
        return toTrim;
    }
}

///
static void
getObjectBodyAndNewContent(std::string & content, std::string & body) {
    /// content is currently "{ ...... } ...."
    /// first we find the matching curly brace's location
    bool done = false;
    int leftCurlyCount = 0;
    int matchingCurlyIndex = (int) std::string::npos;
    for (int loc = 0; loc < (int) content.length() && !done; loc++) {
        if (content[loc] == '{') {
            leftCurlyCount++;
        }
        else if (content[loc] == '}') {
            leftCurlyCount--;
        }
        
        if (leftCurlyCount == 0) {
            matchingCurlyIndex = loc;
            done = true;
        }
    }
    
    /// Then we make body the content in between the curly braces
    auto leftCurlyLoc = content.find("{");
    body = content.substr(leftCurlyLoc + 1, matchingCurlyIndex - 1 - leftCurlyLoc);
    content = content.substr(matchingCurlyIndex + 1, content.length() - matchingCurlyIndex - 1);
}

///
std::shared_ptr<PovrayScene>
PovrayScene::loadScene(const std::string & file) {
    std::ifstream source;
    source.open(file.c_str(), std::ios_base::in);
    if (!source) {
        TSLoggerLog(std::cout, "could not find file=", file);
        assert(false);
        return nullptr;
    }
    
    std::string content;
    std::string line;
    while (!std::getline(source, line).eof()) {
        /// Remove all comments
        content.append(trimRight(line, "//") + " ");
    }
    
//        TSLoggerLog(std::cout, "content=(", content, ")");
    
    auto scene = std::shared_ptr<PovrayScene>(new PovrayScene());
    std::vector<std::pair<std::string, std::shared_ptr<PovraySceneElement>>> recognizedElements;
    
    recognizedElements.push_back(std::make_pair("camera",std::shared_ptr<PovraySceneElement>(new PovrayCamera())));
    recognizedElements.push_back(std::make_pair("light_source",std::shared_ptr<PovraySceneElement>(new PovrayLightSource())));
    recognizedElements.push_back(std::make_pair("sphere",std::shared_ptr<PovraySceneElement>(new PovraySphere())));
    recognizedElements.push_back(std::make_pair("plane",std::shared_ptr<PovraySceneElement>(new PovrayPlane())));
    
    while (content.length() > 0) {
        auto loc = content.find("{");
        if (loc != std::string::npos) {
//                TSLoggerLog(std::cout, "found '{' at ", loc);
            auto sub = content.substr(0, loc);
//                TSLoggerLog(std::cout, "substring = ", sub);
            bool recognizedElement = false;
            auto itr = recognizedElements.begin();
            while (!recognizedElement && itr != recognizedElements.end()) {
                auto keyword = itr->first;
                auto subloc = sub.find(keyword);
                recognizedElement = subloc != std::string::npos;
                if (recognizedElement) {
//                        TSLoggerLog(std::cout, "matches ", keyword);
                    std::string body = "";
                    content = content.substr(loc, content.length() - loc);
                    getObjectBodyAndNewContent(content, body);
//                        TSLoggerLog(std::cout, "body=(", body, "), content=(", content, ")");
                    auto element = itr->second;
                    element->parse(body);
                    scene->addElement(element->copy());
                }
                else {
//                        TSLoggerLog(std::cout, "is not ", keyword);
                    itr++;
                }
            }
            if (!recognizedElement) {
                /// Skip over the body of the object so we can move on.
//                    TSLoggerLog(std::cout, sub, " was not recognized");
                std::string body = "";
                content = content.substr(loc, content.length() - loc);
                getObjectBodyAndNewContent(content, body);
//                    TSLoggerLog(std::cout, "body=(", body, "), content=(", content, ")");
            }
        }
        else {
            /// No more content to process
//                TSLoggerLog(std::cout, "content=(", content, ") had no more to show off");
            content = "";
        }
    }
    
//        TSLoggerLog(std::cout, "done");
    return scene;
}

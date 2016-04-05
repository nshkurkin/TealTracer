//
//  opengl_errors.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "opengl_errors.hpp"

/// The error message to append to any given error.
const std::string OpenGLEnumInfo::kGeneralErrorDescription = "The offending command is ignored and has no other side effect than to set the error flag.";


/// The generic meaning to GL_INVALID_ENUM
const std::string OpenGLEnumInfo::kInvalidEnumDescription = "(GL_INVALID_ENUM) An unacceptable value is specified for an enumerated argument.";

/// The generic meaning to GL_INVALID_VALUE
const std::string OpenGLEnumInfo::kInvalidValueDescription = "(GL_INVALID_VALUE) A numeric argument is out of range.";

///
const std::string OpenGLEnumInfo::kInvalidOperationDescription = "(GL_INVALID_OPERATION) The specified operation is not allowed in the current state.";

/// The generic meaning to GL_INVALID_FRAMEBUFFER_OPERATION
const std::string OpenGLEnumInfo::kInvalidFramebufferOperationDescription = "(GL_INVALID_FRAMEBUFFER_OPERATION) The command is trying to render to or read from the framebuffer while the currently bound framebuffer is not framebuffer complete (i.e. the return value from glCheckFramebufferStatus is not GL_FRAMEBUFFER_COMPLETE).";

/// The generic meaning to GL_OUT_OF_MEMORY
const std::string OpenGLEnumInfo::kOutOfMemoryDescription = "(GL_OUT_OF_MEMORY) There is not enough memory left to execute the command. The state of the GL is undefined.";

/// The meaning of GL_NO_ERROR
const std::string OpenGLEnumInfo::kNoErrorDescription = "(GL_NO_ERROR) No error has been recorded.";

/// Constructs an OpenGLEnumInfo with an enum value.
OpenGLEnumInfo::OpenGLEnumInfo(GLenum enumValue) : enumValue(enumValue) {}

/// Returns whether `enumValue` contains `value` in its bits.
bool
OpenGLEnumInfo::enumMatches(GLenum value) const {
    return (enumValue & value) == value;
}

/// Whether this error contains GL_INVALID_ENUM
bool
OpenGLEnumInfo::invalidEnum() const {
    return enumMatches(GLenum(GL_INVALID_ENUM));
}

/// Whether this error contains GL_INVALID_VALUE
bool
OpenGLEnumInfo::invalidValue() const {
    return enumMatches(GLenum(GL_INVALID_VALUE));
}

/// Whether this error contains GL_INVALID_OPERATION
bool
OpenGLEnumInfo::invalidOperation() const {
    return enumMatches(GLenum(GL_INVALID_OPERATION));
}

/// Whether this error contains GL_INVALID_FRAMEBUFFER_OPERATION
bool
OpenGLEnumInfo::invalidFramebufferOperation() const {
    return enumMatches(GLenum(GL_INVALID_FRAMEBUFFER_OPERATION));
}

/// Whether this error contains GL_OUT_OF_MEMORY
bool
OpenGLEnumInfo::outOfMemory() const {
    return enumMatches(GLenum(GL_OUT_OF_MEMORY));
}

/// Whether this error is GL_NO_ERROR
bool
OpenGLEnumInfo::noError() const {
    return enumValue == GLenum(GL_NO_ERROR);
}

/// The concatenated result of each error and its generic description.
std::string
OpenGLEnumInfo::generalErrorDescription() const {
    std::string result = "";
    if (invalidEnum()) {
        result += kInvalidEnumDescription + " ";
    }
    if (invalidValue()) {
        result += kInvalidValueDescription + " ";
    }
    if (invalidOperation()) {
        result += kInvalidOperationDescription + " ";
    }
    if (invalidFramebufferOperation()) {
        result += kInvalidFramebufferOperationDescription + " ";
    }
    if (outOfMemory()) {
        result += kOutOfMemoryDescription + " ";
    }
    
    if (noError()) {
        result += kNoErrorDescription;
    }
    else {
        result += kGeneralErrorDescription;
    }
    
    return result;
}


/// The errors for GL_INVALID_ENUM
const std::vector<std::string> & OpenGLAdditionalErrorInfo::invalidEnum() const {
    return errorDescriptions.at(GLenum(GL_INVALID_ENUM));
}
/// The errors for GL_INVALID_VALUE
const std::vector<std::string> & OpenGLAdditionalErrorInfo::invalidValue() const {
    return errorDescriptions.at(GLenum(GL_INVALID_VALUE));
}
/// The errors for GL_INVALID_OPERATION
const std::vector<std::string> & OpenGLAdditionalErrorInfo::invalidOperation() const {
    return errorDescriptions.at(GLenum(GL_INVALID_OPERATION));
}
/// The errors for GL_INVALID_FRAMEBUFFER_OPERATION
const std::vector<std::string> & OpenGLAdditionalErrorInfo::invalidFramebufferOperation() const {
    return errorDescriptions.at(GLenum(GL_INVALID_FRAMEBUFFER_OPERATION));
}
/// The errors for GL_OUT_OF_MEMORY
const std::vector<std::string> & OpenGLAdditionalErrorInfo::outOfMemory() const {
    return errorDescriptions.at(GLenum(GL_OUT_OF_MEMORY));
}

///
static inline std::string joinStringsWithSeperator(const std::vector<std::string> & strings, const std::string & separator) {
    std::string result = "";
    for (auto itr = strings.begin(); itr != strings.end(); itr++) {
        result += *itr + separator;
    }
    return result;
}

/// The concatenated result of all the strings in `invalidEnum`
std::string OpenGLAdditionalErrorInfo::invalidEnumDescription() const {
    return joinStringsWithSeperator(invalidEnum(), " ");
}
/// The concatenated result of all the strings in `invalidValue`
std::string OpenGLAdditionalErrorInfo::invalidValueDescription() const {
    return joinStringsWithSeperator(invalidValue(), " ");
}
/// The concatenated result of all the strings in `invalidOperation`
std::string OpenGLAdditionalErrorInfo::invalidOperationDescription() const {
    return joinStringsWithSeperator(invalidOperation(), " ");
}
/// The concatenated result of all the strings in `invalidFramebufferOperation`
std::string OpenGLAdditionalErrorInfo::invalidFramebufferOperationDescription() const {
    return joinStringsWithSeperator(invalidFramebufferOperation(), " ");
}
/// The concatenated result of all the strings in `outOfMemory`
std::string OpenGLAdditionalErrorInfo::outOfMemoryDescription() const {
    return joinStringsWithSeperator(outOfMemory(), " ");
}


/// Constructs this type with a function and all of its associated error descriptions.
OpenGLAdditionalErrorInfo::OpenGLAdditionalErrorInfo(
    const std::string & functionName,
    const std::string & url,
    const std::vector<std::string> & invalidEnum,
    const std::vector<std::string> & invalidValue,
    const std::vector<std::string> & invalidOperation,
    const std::vector<std::string> & invalidFramebufferOperation,
    const std::vector<std::string> & outOfMemory,
    const std::vector<std::string> & errorNotes) {
    
    this->functionName = functionName;
    this->url = url;
    this->errorNotes = errorNotes;
    errorDescriptions[GLenum(GL_INVALID_ENUM)] = invalidEnum;
    errorDescriptions[GLenum(GL_INVALID_VALUE)] = invalidValue;
    errorDescriptions[GLenum(GL_INVALID_OPERATION)] = invalidOperation;
    errorDescriptions[GLenum(GL_INVALID_FRAMEBUFFER_OPERATION)] = invalidFramebufferOperation;
    errorDescriptions[GLenum(GL_OUT_OF_MEMORY)] = outOfMemory;
}

/// Returns the the list of errors correlate to the enums found in `info`
std::vector<std::string> OpenGLAdditionalErrorInfo::errorsForEnumInfo(const OpenGLEnumInfo & info) const {
    std::vector<std::string> result;
    if (info.invalidEnum()) {
        result.insert(result.end(), invalidEnum().begin(), invalidEnum().end());
    }
    if (info.invalidValue()) {
        result.insert(result.end(), invalidValue().begin(), invalidValue().end());
    }
    if (info.invalidOperation()) {
        result.insert(result.end(), invalidOperation().begin(), invalidOperation().end());
    }
    if (info.invalidFramebufferOperation()) {
        result.insert(result.end(), invalidFramebufferOperation().begin(), invalidFramebufferOperation().end());
    }
    if (info.outOfMemory()) {
        result.insert(result.end(), outOfMemory().begin(), outOfMemory().end());
    }
    
    return result;
}

/// Returns all of error messages congealed together into a single string.
std::string OpenGLAdditionalErrorInfo::description() const {
    return invalidEnumDescription()
     + " " + invalidValueDescription()
     + " " + invalidOperationDescription()
     + " " + invalidFramebufferOperationDescription()
     + " " + outOfMemoryDescription();
}


///
OpenGLStateErrorInfo ns_glGetStateErrors() {
    std::string toRet = "";
    auto error = glGetError();
    std::vector<OpenGLEnumInfo> rawErrors;
    bool foundError = false;
    
    if (error != GLenum(GL_NO_ERROR)) {
        foundError = true;
        toRet += "GL State Error(s): ";
    }
    
    while (error != GLenum(GL_NO_ERROR)) {
        auto info = OpenGLEnumInfo(error);
        toRet += info.generalErrorDescription();
        rawErrors.push_back(info);
        
        error = glGetError();
    }
    
    OpenGLStateErrorInfo toReturn;
    
    toReturn.encounteredError = foundError;
    toReturn.description = toRet;
    toReturn.rawErrors = rawErrors;
    
    return toReturn;
}

#include "url_fopen.h"
/// NOTE: to get this include working, you need to add -I/usr/local/include/libxml2
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <iostream>
#include <cassert>

/// Adapted from "node.cc" in the C++ libxml wrapper found here:
///     http://ftp.gnome.org/pub/GNOME/sources/libxml++/3.0/
static std::vector<xmlNodePtr> findXMLNodeChildrenForXPath(
    const std::string & xpath,
    xmlNode * node) {
    
    std::vector<xmlNodePtr> nodes;
    auto ctxt = xmlXPathNewContext(node->doc);
    assert(ctxt != nullptr);
    ctxt->node = node;
    
    auto result = xmlXPathEval((const xmlChar*)xpath.c_str(), ctxt);
    assert(result != nullptr); // Bad path given
    assert(result->type == XPATH_NODESET); // Only nodeset result types are supported.
    
    auto nodeset = result->nodesetval;
    if (nodeset && !xmlXPathNodeSetIsEmpty(nodeset)) {
        const int count = xmlXPathNodeSetGetLength(nodeset);
        nodes.reserve(count);
        for (int i = 0; i != count; ++i) {
            auto cnode = xmlXPathNodeSetItem(nodeset, i);
            
            if (!cnode) {
                std::cerr << "{" << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")} " << "The xmlNode found was null???" << std::endl;
                continue;
            }
            
            if (cnode->type == XML_NAMESPACE_DECL) {
                continue;
            }
            nodes.push_back(cnode);
        }
    }
    
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(ctxt);
    return nodes;
}

///
std::string unannotedXMLNodeContent(xmlNode * node) {
    std::string result = "";
    if (node->content != nullptr) {
        result += std::string((char *)node->content);
    }
    auto child = node->children;
    while (child != nullptr) {
        result += unannotedXMLNodeContent(child);
        child = child->next;
    }
    
    return result;
}

///
OpenGLAdditionalErrorInfo ns_requestOpenGLAPIErrorInfoForFunction(const std::string & function) {
    std::vector<std::string> invalidEnum, invalidValue, invalidOperation, invalidFramebufferOperation, outOfMemory, errorNotes;
    
    URL_FILE * urlDataStream = nullptr;
    int subStringLen = (int) function.length();
    std::string functionGroupName;
    std::string url;
    
    while (urlDataStream == nullptr && subStringLen > 0) {
        functionGroupName = function.substr(0, subStringLen);
        url = "https://www.opengl.org/sdk/docs/man/html/" + functionGroupName + ".xhtml";
        
        urlDataStream = url_fopen(url.c_str(), "r");
        if (urlDataStream == nullptr) {
            subStringLen--;
        }
    }
    
    if (urlDataStream != nullptr) {
        std::string urlData = "";
//        TSLoggerLog(std::cout, "url=", url);
//        TSLoggerLog(std::cout, "found a man page for function ", function);
        char buffer[256] = {'\0'};
        while (url_fread(buffer, sizeof(buffer), sizeof(char), urlDataStream) > 0) {
            urlData += std::string(buffer);
        }
//        TSLoggerLog(std::cout, "Read data from url: ", urlData);
        auto doc = htmlReadDoc((xmlChar*)urlData.c_str(), NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
        auto root = xmlDocGetRootElement(doc);
        auto errorSearch = findXMLNodeChildrenForXPath("//div[@id='errors']/p", root);
        for (auto itr = errorSearch.begin(); itr != errorSearch.end(); itr++) {
            auto content = unannotedXMLNodeContent(*itr);
//            TSLoggerLog(std::cout, "found content: ", content);
            if (content.find("GL_INVALID_ENUM") != std::string::npos) {
                invalidEnum.push_back(content);
            }
            else if (content.find("GL_INVALID_VALUE") != std::string::npos) {
                invalidValue.push_back(content);
            }
            else if (content.find("GL_INVALID_OPERATION") != std::string::npos) {
                invalidOperation.push_back(content);
            }
            else if (content.find("GL_INVALID_FRAMEBUFFER_OPERATION") != std::string::npos) {
                invalidFramebufferOperation.push_back(content);
            }
            else if (content.find("GL_OUT_OF_MEMORY") != std::string::npos) {
                outOfMemory.push_back(content);
            }
            else {
                errorNotes.push_back(content);
            }
        }
        
        xmlFreeDoc(doc);
    }
    else {
        std::cerr << "{" << __FILE__ << ":" << __LINE__ << " (" << __FUNCTION__ << ")} " << "Could not find documentation for funciton " << function << std::endl;
    }
    
    return OpenGLAdditionalErrorInfo(function, url, invalidEnum, invalidValue, invalidOperation, invalidFramebufferOperation, outOfMemory, errorNotes);
}

/// Checks to see if any OpenGL errors exists, and if so the program will crash and an error will
/// be printed out including the location of where this function was called. If `oglFuncName` is
/// supplied, then OpenGLAPIDocRequester will be used to try and find the documentation for that
/// function and print out the specific descriptions for the errors. Otherwise, the generic errors
/// are printed instead. Ideally this function is called before and after every OpenGL API call
/// in order to make debugging a lot easier.
void ns_assertNoOpenGLErrors(const std::string & message, const std::string & functionName, const std::string & fileName, int lineNumber, const std::string & oglFuncName) {

    auto stateErrors = ns_glGetStateErrors();
    if (stateErrors.encounteredError) {
        
        std::cerr << "{" << fileName << ":" << lineNumber << " (" << functionName << ")} opengl assertion failure: " << message << std::endl;
        
        if (oglFuncName.length() > 0) {
            std::string causeMessages = "";
            auto additionalInfo = ns_requestOpenGLAPIErrorInfoForFunction(oglFuncName);
            for (auto itr = stateErrors.rawErrors.begin(); itr != stateErrors.rawErrors.end(); itr++) {
                auto errorStrings = additionalInfo.errorsForEnumInfo(*itr);
                for (auto strItr = errorStrings.begin(); strItr != errorStrings.end(); strItr++) {
                    causeMessages += *strItr;
                }
            }
            
            if (causeMessages.length() > 0) {
                /// Specific possible causes
                std::cerr << "{" << fileName << ":" << lineNumber << " (" << functionName << ")} possible causes: " << causeMessages << " \n\nSource: " << additionalInfo.url << std::endl;
            }
            else {
                /// General causes
                std::cerr << "{" << fileName << ":" << lineNumber << " (" << functionName << ")} general causes: " << stateErrors.description << std::endl;
            }
            
        }

        assert(false);
    }
}

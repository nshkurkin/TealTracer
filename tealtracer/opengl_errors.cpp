//
//  opengl_errors.cpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#include "opengl_errors.hpp"

#include <stdio.h>
#include <string.h>
#ifndef WIN32
#  include <sys/time.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include <curl/curl.h>

enum fcurl_type_e {
    CFTYPE_NONE=0,
    CFTYPE_FILE=1,
    CFTYPE_CURL=2
};

struct fcurl_data
{
    enum fcurl_type_e type;     /* type of handle */
    union {
        CURL *curl;
        FILE *file;
    } handle;                   /* handle */
    
    char *buffer;               /* buffer to store cached data*/
    size_t buffer_len;          /* currently allocated buffers length */
    size_t buffer_pos;          /* end of data in buffer*/
    int still_running;          /* Is background url fetch still in progress */
};

typedef struct fcurl_data URL_FILE;

/* exported functions */
URL_FILE *url_fopen(const char *url, const char *operation);
int url_fclose(URL_FILE *file);
int url_feof(URL_FILE *file);
size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file);
char * url_fgets(char *ptr, size_t size, URL_FILE *file);
void url_rewind(URL_FILE *file);

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

/* we use a global one for convenience */
static CURLM *multi_handle;

/* curl calls this routine to get more data */
static size_t write_callback(char *buffer,
                             size_t size,
                             size_t nitems,
                             void *userp)
{
    char *newbuff;
    size_t rembuff;
    
    URL_FILE *url = (URL_FILE *)userp;
    size *= nitems;
    
    rembuff=url->buffer_len - url->buffer_pos; /* remaining space in buffer */
    
    if(size > rembuff) {
        /* not enough space in buffer */
        newbuff = (char *) realloc(url->buffer, url->buffer_len + (size - rembuff));
        if(newbuff==NULL) {
            fprintf(stderr, "callback buffer grow failed\n");
            size=rembuff;
        }
        else {
            /* realloc succeeded increase buffer size*/
            url->buffer_len+=size - rembuff;
            url->buffer=newbuff;
        }
    }
    
    memcpy(&url->buffer[url->buffer_pos], buffer, size);
    url->buffer_pos += size;
    
    return size;
}

/* use to attempt to fill the read buffer up to requested number of bytes */
static int fill_buffer(URL_FILE *file, size_t want)
{
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    struct timeval timeout;
    int rc;
    CURLMcode mc; /* curl_multi_fdset() return code */
    
    /* only attempt to fill buffer if transactions still running and buffer
     * doesn't exceed required size already
     */
    if((!file->still_running) || (file->buffer_pos > want))
        return 0;
    
    /* attempt to fill buffer */
    do {
        int maxfd = -1;
        long curl_timeo = -1;
        
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);
        
        /* set a suitable timeout to fail on */
        timeout.tv_sec = 60; /* 1 minute */
        timeout.tv_usec = 0;
        
        curl_multi_timeout(multi_handle, &curl_timeo);
        if(curl_timeo >= 0) {
            timeout.tv_sec = curl_timeo / 1000;
            if(timeout.tv_sec > 1)
                timeout.tv_sec = 1;
            else
                timeout.tv_usec = (curl_timeo % 1000) * 1000;
        }
        
        /* get file descriptors from the transfers */
        mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
        
        if(mc != CURLM_OK) {
            fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
            break;
        }
        
        /* On success the value of maxfd is guaranteed to be >= -1. We call
         select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
         no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
         to sleep 100ms, which is the minimum suggested value in the
         curl_multi_fdset() doc. */
        
        if(maxfd == -1) {
#ifdef _WIN32
            Sleep(100);
            rc = 0;
#else
            /* Portable sleep for platforms other than Windows. */
            struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
            rc = select(0, NULL, NULL, NULL, &wait);
#endif
        }
        else {
            /* Note that on some platforms 'timeout' may be modified by select().
             If you need access to the original value save a copy beforehand. */
            rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
        }
        
        switch(rc) {
            case -1:
                /* select error */
                break;
                
            case 0:
            default:
                /* timeout or readable/writable sockets */
                curl_multi_perform(multi_handle, &file->still_running);
                break;
        }
    } while(file->still_running && (file->buffer_pos < want));
    return 1;
}

/* use to remove want bytes from the front of a files buffer */
static int use_buffer(URL_FILE *file, size_t want)
{
    /* sort out buffer */
    if((file->buffer_pos - want) <=0) {
        /* ditch buffer - write will recreate */
        free(file->buffer);
        file->buffer=NULL;
        file->buffer_pos=0;
        file->buffer_len=0;
    }
    else {
        /* move rest down make it available for later */
        memmove(file->buffer,
                &file->buffer[want],
                (file->buffer_pos - want));
        
        file->buffer_pos -= want;
    }
    return 0;
}

URL_FILE *url_fopen(const char *url, const char *operation)
{
    /* this code could check for URLs or types in the 'url' and
     basically use the real fopen() for standard files */
    
    URL_FILE *file;
    (void)operation;
    
    file = (URL_FILE *) malloc(sizeof(URL_FILE));
    if(!file)
        return NULL;
    
    memset(file, 0, sizeof(URL_FILE));
    
    if((file->handle.file=fopen(url, operation)))
        file->type = CFTYPE_FILE; /* marked as URL */
    
    else {
        file->type = CFTYPE_CURL; /* marked as URL */
        file->handle.curl = curl_easy_init();
        
        curl_easy_setopt(file->handle.curl, CURLOPT_URL, url);
        curl_easy_setopt(file->handle.curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(file->handle.curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(file->handle.curl, CURLOPT_WRITEFUNCTION, write_callback);
        
        if(!multi_handle)
            multi_handle = curl_multi_init();
        
        curl_multi_add_handle(multi_handle, file->handle.curl);
        
        /* lets start the fetch */
        curl_multi_perform(multi_handle, &file->still_running);
        
        if((file->buffer_pos == 0) && (!file->still_running)) {
            /* if still_running is 0 now, we should return NULL */
            
            /* make sure the easy handle is not in the multi handle anymore */
            curl_multi_remove_handle(multi_handle, file->handle.curl);
            
            /* cleanup */
            curl_easy_cleanup(file->handle.curl);
            
            free(file);
            
            file = NULL;
        }
    }
    return file;
}

int url_fclose(URL_FILE *file)
{
    int ret=0;/* default is good return */
    
    switch(file->type) {
        case CFTYPE_FILE:
            ret=fclose(file->handle.file); /* passthrough */
            break;
            
        case CFTYPE_CURL:
            /* make sure the easy handle is not in the multi handle anymore */
            curl_multi_remove_handle(multi_handle, file->handle.curl);
            
            /* cleanup */
            curl_easy_cleanup(file->handle.curl);
            break;
            
        default: /* unknown or supported type - oh dear */
            ret=EOF;
            errno=EBADF;
            break;
    }
    
    free(file->buffer);/* free any allocated buffer space */
    free(file);
    
    return ret;
}

int url_feof(URL_FILE *file)
{
    int ret=0;
    
    switch(file->type) {
        case CFTYPE_FILE:
            ret=feof(file->handle.file);
            break;
            
        case CFTYPE_CURL:
            if((file->buffer_pos == 0) && (!file->still_running))
                ret = 1;
            break;
            
        default: /* unknown or supported type - oh dear */
            ret=-1;
            errno=EBADF;
            break;
    }
    return ret;
}

size_t url_fread(void *ptr, size_t size, size_t nmemb, URL_FILE *file)
{
    size_t want;
    
    switch(file->type) {
        case CFTYPE_FILE:
            want=fread(ptr, size, nmemb, file->handle.file);
            break;
            
        case CFTYPE_CURL:
            want = nmemb * size;
            
            fill_buffer(file, want);
            
            /* check if theres data in the buffer - if not fill_buffer()
             * either errored or EOF */
            if(!file->buffer_pos)
                return 0;
            
            /* ensure only available data is considered */
            if(file->buffer_pos < want)
                want = file->buffer_pos;
            
            /* xfer data to caller */
            memcpy(ptr, file->buffer, want);
            
            use_buffer(file, want);
            
            want = want / size;     /* number of items */
            break;
            
        default: /* unknown or supported type - oh dear */
            want=0;
            errno=EBADF;
            break;
            
    }
    return want;
}

char *url_fgets(char *ptr, size_t size, URL_FILE *file)
{
    size_t want = size - 1;/* always need to leave room for zero termination */
    size_t loop;
    
    switch(file->type) {
        case CFTYPE_FILE:
            ptr = fgets(ptr, (int)size, file->handle.file);
            break;
            
        case CFTYPE_CURL:
            fill_buffer(file, want);
            
            /* check if theres data in the buffer - if not fill either errored or
             * EOF */
            if(!file->buffer_pos)
                return NULL;
            
            /* ensure only available data is considered */
            if(file->buffer_pos < want)
                want = file->buffer_pos;
            
            /*buffer contains data */
            /* look for newline or eof */
            for(loop=0;loop < want;loop++) {
                if(file->buffer[loop] == '\n') {
                    want=loop+1;/* include newline */
                    break;
                }
            }
            
            /* xfer data to caller */
            memcpy(ptr, file->buffer, want);
            ptr[want]=0;/* allways null terminate */
            
            use_buffer(file, want);
            
            break;
            
        default: /* unknown or supported type - oh dear */
            ptr=NULL;
            errno=EBADF;
            break;
    }
    
    return ptr;/*success */
}

void url_rewind(URL_FILE *file)
{
    switch(file->type) {
        case CFTYPE_FILE:
            rewind(file->handle.file); /* passthrough */
            break;
            
        case CFTYPE_CURL:
            /* halt transaction */
            curl_multi_remove_handle(multi_handle, file->handle.curl);
            
            /* restart */
            curl_multi_add_handle(multi_handle, file->handle.curl);
            
            /* ditch buffer - write will recreate - resets stream pos*/
            free(file->buffer);
            file->buffer=NULL;
            file->buffer_pos=0;
            file->buffer_len=0;
            
            break;
            
        default: /* unknown or supported type - oh dear */
            break;
    }
}

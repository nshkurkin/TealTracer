//
//  opengl_errors.hpp
//  tealtracer
//
//  Created by Nikolai Shkurkin on 3/30/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

/// Define `NS_OVERWRITE_GL_FUNCTIONS` before including this file to enable
/// automatic OpenGL function error checking.
///
/// When an OpenGL call causes a state error, the following will occur:
///
///     1) Collect all of the error flags sets by the state machine
///
///     2) Try to find the online documentation (if on a network) for those errors
///
///     3) Show (a) where the function was called and (b) why the error might
///         have happened.
///
///     4) Crash the program just after calling the offending OpenGL function.
///

#ifndef opengl_errors_hpp
#define opengl_errors_hpp

/// NOTE: Include "gl.h" here, or your equivalent
#include "gl_include.h"
#if __cplusplus < 201103L
    #define nullptr NULL
#endif

/// NOTE: You may need to add an include path for libxml to your build. For example:
///     -I/usr/local/include/libxml2
///
/// NOTE: You may need to add the following options to your linker:
///     -L/usr/local/lib -lcurl -lxml

#include <string>
#include <vector>
#include <map>

/// Call this to stop program execution if an OpenGL error has occurred. If an
/// error has occurred, `message` will be displayed with the additionally
/// formatted information: `functionName` (the function that called this assert),
/// `fileName` (the file of the source where `functionName` was written),
/// and `lineNumber` (the line file `fileName` in `functionName` that this assert
/// call was made).
///
/// Optionally, the caller can specify `oglFuncName` to state which function they
/// expected to be the source of the OpenGL error if one occurs. If an error does
/// occur, a network query will be performed and will attempt to look up the error
/// description on the OpenGL website.
void ns_assertNoOpenGLErrors(const std::string & message, const std::string & functionName, const std::string & fileName, int lineNumber, const std::string & oglFuncName = "");

/// Calls `oglfun` with the arguments `...` in between two
/// `ns_assertNoOpenGLErrors` calls that are intended catch errors that occur
/// at or near the call of `oglfun`. This version of the macro is used for
/// OpenGL functions with no return value.
#define ns_safeGLCall(oglfun, ...) ({\
    ns_assertNoOpenGLErrors("some function before calling" #oglfun " threw an error", __FUNCTION__, __FILE__, __LINE__, #oglfun); \
    oglfun ( __VA_ARGS__ ); \
    ns_assertNoOpenGLErrors( #oglfun " threw an error", __FUNCTION__, __FILE__, __LINE__, #oglfun); \
})

/// Calls `oglfun` with the arguments `...` in between two
/// `ns_assertNoOpenGLErrors` calls that are intended catch errors that occur
/// at or near the call of `oglfun`. This version of the macro is used for
/// OpenGL functions with a return value.
#define ns_safeGLCallWithReturn(oglfun, ...) ({\
    ns_assertNoOpenGLErrors("some function before calling" #oglfun " threw an error", __FUNCTION__, __FILE__, __LINE__, #oglfun); \
    auto __result = oglfun ( __VA_ARGS__ ); \
    ns_assertNoOpenGLErrors( #oglfun " threw an error", __FUNCTION__, __FILE__, __LINE__, #oglfun); \
    __result; \
})

/// Check if the programmer wants to overwrite and insert "safe" and immediate
/// error checking.
#define NS_OVERWRITE_GL_FUNCTIONS
#ifdef NS_OVERWRITE_GL_FUNCTIONS

#define glGetIntegerv(...) ns_safeGLCall(glGetIntegerv, ##__VA_ARGS__)
#define glGetBooleanv(...) ns_safeGLCall(glGetBooleanv, ##__VA_ARGS__)
#define glGetString(...) ns_safeGLCallWithReturn(glGetString, ##__VA_ARGS__)

#define glGenBuffers(...) ns_safeGLCall(glGenBuffers, ##__VA_ARGS__)
#define glBindBuffer(...) ns_safeGLCall(glBindBuffer, ##__VA_ARGS__)
#define glDeleteBuffers(...) ns_safeGLCall(glDeleteBuffers, ##__VA_ARGS__)
#define glBufferData(...) ns_safeGLCall(glBufferData, ##__VA_ARGS__)

#define glGenVertexArrays(...) ns_safeGLCall(glGenVertexArrays, ##__VA_ARGS__)
#define glBindVertexArray(...) ns_safeGLCall(glBindVertexArray, ##__VA_ARGS__)
#define glDeleteVertexArrays(...) ns_safeGLCall(glDeleteVertexArrays, ##__VA_ARGS__)

#define glGenTextures(...) ns_safeGLCall(glGenTextures, ##__VA_ARGS__)
#define glDeleteTextures(...) ns_safeGLCall(glDeleteTextures, ##__VA_ARGS__)
#define glTexImage1D(...) ns_safeGLCall(glTexImage1D, ##__VA_ARGS__)
#define glTexImage2D(...) ns_safeGLCall(glTexImage2D, ##__VA_ARGS__)
#define glTexImage3D(...) ns_safeGLCall(glTexImage3D, ##__VA_ARGS__)
#define glActiveTexture(...) ns_safeGLCall(glActiveTexture, ##__VA_ARGS__)
#define glBindTexture(...) ns_safeGLCall(glBindTexture, ##__VA_ARGS__)
#define glGenerateMipmap(...) ns_safeGLCall(glGenerateMipmap, ##__VA_ARGS__)
#define glTexParameterf(...) ns_safeGLCall(glTexParameterf, ##__VA_ARGS__)
#define glTexParameterfv(...) ns_safeGLCall(glTexParameterfv, ##__VA_ARGS__)

#define glCreateShader(...) ns_safeGLCallWithReturn(glCreateShader, ##__VA_ARGS__)
#define glShaderSource(...) ns_safeGLCall(glShaderSource, ##__VA_ARGS__)
#define glGetShaderiv(...) ns_safeGLCall(glGetShaderiv, ##__VA_ARGS__)
#define glGetShaderInfoLog(...) ns_safeGLCall(glGetShaderInfoLog, ##__VA_ARGS__)
#define glCompileShader(...) ns_safeGLCall(glCompileShader, ##__VA_ARGS__)
#define glDeleteShader(...) ns_safeGLCall(glDeleteShader, ##__VA_ARGS__)

#define glCreateProgram(...) ns_safeGLCallWithReturn(glCreateProgram, ##__VA_ARGS__)
#define glAttachShader(...) ns_safeGLCall(glAttachShader, ##__VA_ARGS__)
#define glLinkProgram(...) ns_safeGLCall(glLinkProgram, ##__VA_ARGS__)
#define glGetProgramiv(...) ns_safeGLCall(glGetProgramiv, ##__VA_ARGS__)
#define glDeleteProgram(...) ns_safeGLCall(glDeleteProgram, ##__VA_ARGS__)
#define glGetProgramInfoLog(...) ns_safeGLCall(glGetProgramInfoLog, ##__VA_ARGS__)
#define glValidateProgram(...) ns_safeGLCall(glValidateProgram, ##__VA_ARGS__)
#define glUseProgram(...) ns_safeGLCall(glUseProgram, ##__VA_ARGS__)

#define glEnableVertexAttribArray(...) ns_safeGLCall(glEnableVertexAttribArray, ##__VA_ARGS__)
#define glDisableVertexAttribArray(...) ns_safeGLCall(glDisableVertexAttribArray, ##__VA_ARGS__)
#define glVertexAttribPointer(...) ns_safeGLCall(glVertexAttribPointer, ##__VA_ARGS__)
#define glGetActiveAttrib(...) ns_safeGLCall(glGetActiveAttrib, ##__VA_ARGS__)
#define glGetAttribLocation(...) ns_safeGLCallWithReturn(glGetAttribLocation, ##__VA_ARGS__)

#define glGetActiveUniformsiv(...) ns_safeGLCall(glGetActiveUniformsiv, ##__VA_ARGS__)
#define glGetActiveUniform(...) ns_safeGLCall(glGetActiveUniform, ##__VA_ARGS__)
#define glGetUniformLocation(...) ns_safeGLCallWithReturn(glGetUniformLocation, ##__VA_ARGS__)

#define glUniform1f(...) ns_safeGLCall(glUniform1f, ##__VA_ARGS__)
#define glUniform2f(...) ns_safeGLCall(glUniform2f, ##__VA_ARGS__)
#define glUniform3f(...) ns_safeGLCall(glUniform3f, ##__VA_ARGS__)
#define glUniform4f(...) ns_safeGLCall(glUniform4f, ##__VA_ARGS__)

#define glUniform1fv(...) ns_safeGLCall(glUniform1fv, ##__VA_ARGS__)
#define glUniform2fv(...) ns_safeGLCall(glUniform2fv, ##__VA_ARGS__)
#define glUniform3fv(...) ns_safeGLCall(glUniform3fv, ##__VA_ARGS__)
#define glUniform4fv(...) ns_safeGLCall(glUniform4fv, ##__VA_ARGS__)

#define glUniform1i(...) ns_safeGLCall(glUniform1i, ##__VA_ARGS__)
#define glUniform2i(...) ns_safeGLCall(glUniform2i, ##__VA_ARGS__)
#define glUniform3i(...) ns_safeGLCall(glUniform3i, ##__VA_ARGS__)
#define glUniform4i(...) ns_safeGLCall(glUniform4i, ##__VA_ARGS__)

#define glUniform1iv(...) ns_safeGLCall(glUniform1iv, ##__VA_ARGS__)
#define glUniform2iv(...) ns_safeGLCall(glUniform2iv, ##__VA_ARGS__)
#define glUniform3iv(...) ns_safeGLCall(glUniform3iv, ##__VA_ARGS__)
#define glUniform4iv(...) ns_safeGLCall(glUniform4iv, ##__VA_ARGS__)

#define glUniform1ui(...) ns_safeGLCall(glUniform1ui, ##__VA_ARGS__)
#define glUniform2ui(...) ns_safeGLCall(glUniform2ui, ##__VA_ARGS__)
#define glUniform3ui(...) ns_safeGLCall(glUniform3ui, ##__VA_ARGS__)
#define glUniform4ui(...) ns_safeGLCall(glUniform4ui, ##__VA_ARGS__)

#define glUniform1uiv(...) ns_safeGLCall(glUniform1uiv, ##__VA_ARGS__)
#define glUniform2uiv(...) ns_safeGLCall(glUniform2uiv, ##__VA_ARGS__)
#define glUniform3uiv(...) ns_safeGLCall(glUniform3uiv, ##__VA_ARGS__)
#define glUniform4uiv(...) ns_safeGLCall(glUniform4uiv, ##__VA_ARGS__)

#define glUniformMatrix2fv(...) ns_safeGLCall(glUniformMatrix2fv, ##__VA_ARGS__)
#define glUniformMatrix3fv(...) ns_safeGLCall(glUniformMatrix3fv, ##__VA_ARGS__)
#define glUniformMatrix4fv(...) ns_safeGLCall(glUniformMatrix4fv, ##__VA_ARGS__)

#define glUniformMatrix2x3fv(...) ns_safeGLCall(glUniformMatrix2x3fv, ##__VA_ARGS__)
#define glUniformMatrix3x2fv(...) ns_safeGLCall(glUniformMatrix3x2fv, ##__VA_ARGS__)
#define glUniformMatrix2x4fv(...) ns_safeGLCall(glUniformMatrix2x4fv, ##__VA_ARGS__)
#define glUniformMatrix4x2fv(...) ns_safeGLCall(glUniformMatrix4x2fv, ##__VA_ARGS__)
#define glUniformMatrix3x4fv(...) ns_safeGLCall(glUniformMatrix3x4fv, ##__VA_ARGS__)
#define glUniformMatrix4x3fv(...) ns_safeGLCall(glUniformMatrix4x3fv, ##__VA_ARGS__)

#define glDrawArrays(...) ns_safeGLCall(glDrawArrays, ##__VA_ARGS__)
#define glDrawElements(...) ns_safeGLCall(glDrawElements, ##__VA_ARGS__)

#define glViewport(...) ns_safeGLCall(glViewport, ##__VA_ARGS__)

#endif

/// Takes the enum value given by |glGetError| and turns it into a parsed
/// string based on the descriptions for each error available here:
/// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetError.xml
struct OpenGLEnumInfo {
    
    /// The OpenGL error enumeration we seek to get information about.
    GLenum enumValue;
    
    /// Constructs an OpenGLEnumInfo with an enum value.
    OpenGLEnumInfo(GLenum enumValue);
    
    /// Returns whether `enumValue` contains `value` in its bits.
    bool enumMatches(GLenum value) const;
    
    
    /// The error message to append to any given error.
    static const std::string kGeneralErrorDescription;
    /// The generic meaning to GL_INVALID_ENUM
    static const std::string kInvalidEnumDescription;
    /// The generic meaning to GL_INVALID_VALUE
    static const std::string kInvalidValueDescription;
    /// The generic meaning to GL_INVALID_OPERATION
    static const std::string kInvalidOperationDescription;
    /// The generic meaning to GL_INVALID_FRAMEBUFFER_OPERATION
    static const std::string kInvalidFramebufferOperationDescription;
    /// The generic meaning to GL_OUT_OF_MEMORY
    static const std::string kOutOfMemoryDescription;
    /// The meaning of GL_NO_ERROR
    static const std::string kNoErrorDescription;
    
    /// Whether this error contains GL_INVALID_ENUM
    bool invalidEnum() const;
    /// Whether this error contains GL_INVALID_VALUE
    bool invalidValue() const;
    /// Whether this error contains GL_INVALID_OPERATION
    bool invalidOperation() const;
    /// Whether this error contains GL_INVALID_FRAMEBUFFER_OPERATION
    bool invalidFramebufferOperation() const;
    /// Whether this error contains GL_OUT_OF_MEMORY
    bool outOfMemory() const;
    /// Whether this error is GL_NO_ERROR
    bool noError() const;
    
    /// The concatenated result of each error and its generic description.
    std::string generalErrorDescription() const;
};

struct OpenGLStateErrorInfo {
    bool encounteredError;
    std::string description;
    std::vector<OpenGLEnumInfo> rawErrors;
};

/// A wrapper struct used to collect specific information about a particular OpenGL function.
/// In particular, the list of errors for each type is given and then can be used in other areas.
/// The intention of this class is to be used with OpenGLAPIDocRequester to encapsulate, in a 
/// generic way, the data present in the OpenGL documentation.
struct OpenGLAdditionalErrorInfo {

    /// The name of the function that each error property refers to.
    std::string functionName;
    ///
    std::string url;
    ///
    std::vector<std::string> errorNotes;

    /// A map of various OpenGL errors and a list of error descriptions that accompany each.
    std::map< GLenum, std::vector<std::string> > errorDescriptions;
    
    /// The errors for GL_INVALID_ENUM
    const std::vector<std::string> & invalidEnum() const;
    /// The errors for GL_INVALID_VALUE
    const std::vector<std::string> & invalidValue() const;
    /// The errors for GL_INVALID_OPERATION
    const std::vector<std::string> & invalidOperation() const;
    /// The errors for GL_INVALID_FRAMEBUFFER_OPERATION
    const std::vector<std::string> & invalidFramebufferOperation() const;
    /// The errors for GL_OUT_OF_MEMORY
    const std::vector<std::string> & outOfMemory() const;
    
    /// The concatenated result of all the strings in `invalidEnum`
    std::string invalidEnumDescription() const;
    /// The concatenated result of all the strings in `invalidValue`
    std::string invalidValueDescription() const;
    /// The concatenated result of all the strings in `invalidOperation`
    std::string invalidOperationDescription() const;
    /// The concatenated result of all the strings in `invalidFramebufferOperation`
    std::string invalidFramebufferOperationDescription() const;
    /// The concatenated result of all the strings in `outOfMemory`
    std::string outOfMemoryDescription() const;
    
    /// Constructs this type with a function and all of its associated error descriptions.
    OpenGLAdditionalErrorInfo(
        const std::string & functionName,
        const std::string & url,
        const std::vector<std::string> & invalidEnum,
        const std::vector<std::string> & invalidValue,
        const std::vector<std::string> & invalidOperation,
        const std::vector<std::string> & invalidFramebufferOperation,
        const std::vector<std::string> & outOfMemory,
        const std::vector<std::string> & errorNotes);
    
    /// Returns the the list of errors correlate to the enums found in `info`
    std::vector<std::string> errorsForEnumInfo(const OpenGLEnumInfo & info) const;
    
    /// Returns all of error messages congealed together into a single string.
    std::string description() const;
};

/// Returns whether an OpenGL error has occurred, and a general error description
/// for all of the errors found, as well as all of the error enums in an array.
/// After this function is called the caller is guaranteed that
/// glGetError() == GL_NO_ERROR (basically this function clears the error state
/// for OpenGL)
OpenGLStateErrorInfo ns_glGetStateErrors();
/// Tries to query the OpenGL website https://www.opengl.org for the
/// documentation of `function`.
OpenGLAdditionalErrorInfo ns_requestOpenGLAPIErrorInfoForFunction(const std::string & function);

#endif /* opengl_errors_hpp */

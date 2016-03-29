///
/// nsgl_base.h
/// -----------
/// Nikolai Shkurkin
/// NSGL Library
///
/// Synopsis:
///   Represents my own version of "GLSL"/"GLSLHelper" with my own methods for
///   interfacing with and getting data back from OpenGL. It also includes some
///   utilitiy methods for logging.
///
///   Programmers that wish to further extend operations on top of OpenGL should
///   consider placing functions and objects under the namespace "nsgl".
///
/// Note: "nsgl" is a sort of pun meaning "NextStep GL" and "Nikolai Shkurkin GL"
///   simultaneously (a lot of ObjC classes are prefixed by NS, which happen to
///   be my initials).
///

#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

#include <src/nsgl/gl_interface/gl.h>
#include <src/util/util_base.h>
#include <src/util/util_types.h>
#include <src/util/data/Image.h>

#ifndef __nsgl__base__
#define __nsgl__base__

/// Namespace helper methods
namespace nsgl {

	typedef util::RGBAImage RGBAImage;
    
    /// Parses the string given by `which`. May be empty.
    std::string getString(GLenum which);
    
    /// Takes the enum value given by `glGetError` and turns it into a parsed
    /// string based on the descriptions for each error available here:
    /// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetError.xml
    std::string parseErrorEnum(GLenum errors);
    
    /// Returns a string detailing any errors that may have occured due to bad
    /// open gl calls.
    std::string getStateErrors();
    
}

namespace nsgl {
    /// Returns a string detailing the Renderer, the GL Version, and any Extensions.
    std::string getGLInfo();
    
    /// Represents the particular OpenGl version a computer may be running.
    struct GLVersionInfo {
        int major_version, minor_version;
        
        std::string description() {
            std::ostringstream strFormatter;
            strFormatter << major_version << "." << minor_version;
            return strFormatter.str();
        }
        
        GLVersionInfo(int major, int minor) {
            major_version = major;
            minor_version = minor;
        }
    };
    
    /// Returns the current OpenGL version being run in the current context.
    nsgl::GLVersionInfo getGLVersionInfo();
    /// Fails if the current opengl version is not at least major.minor.
    void assertMinimumGLVersion(int major, int minor);
    /// Fails (crashes the program) if the current opengl version (of the form
    /// major.minor) is not at least `version`.
    void assertMinimumGLVersion(float version);
    
    /// Logs `toLog` IFF `toLog` != "".
    void logNonEmpty(std::string toLog,
                     const std::string file = __FILE__, int line = __LINE__);
    /// Logs `toLog` with `file` and `line` to specify the caller.
    void log(std::string toLog,
             const std::string file = __FILE__, int line = __LINE__);
    
    /// Prints the result of `getStateErrors` to stdout, if any exist.
    void printStateErrors(const std::string file = __FILE__, int line = __LINE__);
    /// Fails (crashes the program) if opengl has reported any errors.
    void assertNoStateErrors(std::string file = __FILE__,
                             int line = __LINE__);

	const int UNSET_TEX = 12345;
	RGBAImage screencap(int width, int height);
}

/// Use this to log messages with the added bonus of the "+" operator.
#define NSGL_LOG(statement) nsgl::log(std::string("") + statement, __FILE__, __LINE__)
/// Use this to quickly tell if/where opengl errors approximately occur. This
/// will crash the program is an OpenGL error occurs.
#define NSGL_ERRORS() nsgl::assertNoStateErrors(__FILE__, __LINE__)
/// Use this to quickly tell yourself where you are in the code.
#define NSGL_HERE() nsgl::log("here", __FILE__, __LINE__)

#endif // __nsgl__base__


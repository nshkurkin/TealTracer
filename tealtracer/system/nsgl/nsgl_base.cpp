///
/// nsgl_base.cpp
/// -------------
/// Nikolai Shkurkin
/// NSGL Library
///

#include "nsgl_base.h"

/// Parses the string given by |which|. May be empty.
std::string nsgl::getString(GLenum which) {
    const GLubyte * str = glGetString(which);
    return std::string((char *) str);
}

/// Takes the enum value given by |glGetError| and turns it into a parsed
/// string based on the descriptions for each error available here:
/// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetError.xml
std::string nsgl::parseErrorEnum(GLenum errors) {
    std::string toRet = "";
    struct anon {
        static bool matches(GLenum errors, int errorType) {
            return (errors == GLenum(errorType));
        }
    };
    bool (*enumMatches) (GLenum, int) = &anon::matches;
    
    if (enumMatches(errors, GL_INVALID_ENUM)) {
        toRet += "(GL_INVALID_ENUM) An unacceptable value is specified for an enumerated argument. ";
    }
    if (enumMatches(errors, GL_INVALID_VALUE)) {
        toRet += "(GL_INVALID_VALUE) A numeric argument is out of range. ";
    }
    if (enumMatches(errors, GL_INVALID_OPERATION)) {
        toRet += "(GL_INVALID_OPERATION) The specified operation is not allowed in the current state. ";
    }
    if (enumMatches(errors, GL_INVALID_FRAMEBUFFER_OPERATION)) {
        toRet += "(GL_INVALID_FRAMEBUFFER_OPERATION) The command is trying to render to or read from the framebuffer while the currently bound framebuffer is not framebuffer complete (i.e. the return value from glCheckFramebufferStatus is not GL_FRAMEBUFFER_COMPLETE). ";
    }
    if (enumMatches(errors, GL_OUT_OF_MEMORY)) {
        toRet += "(GL_OUT_OF_MEMORY) There is not enough memory left to execute the command. The state of the GL is undefined. ";
    }
    
    if (errors == GLenum(GL_NO_ERROR)) {
        toRet += "(GL_NO_ERROR) No error has been recorded. ";
    }
    else {
        toRet += "The offending command is ignored and has no other side effect than to set the error flag. ";
    }
    
    return toRet;
}

/// Returns a string detailing any errors that may have occured due to bad
/// open gl calls.
std::string nsgl::getStateErrors() {
    std::string toRet = "";
    GLenum error;
    
    error = glGetError();
    
    if (error != GLenum(GL_NO_ERROR)) {
        toRet += "GL State Error(s): ";
    }
    
    while (error != GLenum(GL_NO_ERROR)) {
        toRet += nsgl::parseErrorEnum(error);
        error = glGetError();
    }
    
    return toRet;
}

/// Returns a string detailing the Renderer, the GL Version, and any Extensions.
std::string nsgl::getGLInfo() {
    std::string toRet = "(OpenGL Info) {";
    std::string tmp;
    struct anon {
        static std::string parse(std::string base) {
            if (base.empty())
                return "(None)";
            else
                return base;
        }
    };
    std::string (*parseString) (std::string) = &anon::parse;
    
    toRet += "\n  Renderer: " + parseString(nsgl::getString(GL_RENDERER));
    toRet += "\n  Version: " + parseString(nsgl::getString(GL_VERSION));
    
    return toRet + "\n}";
}

nsgl::GLVersionInfo nsgl::getGLVersionInfo() {
    nsgl::GLVersionInfo info(0, 0);
    const char *verstr = (const char *) glGetString(GL_VERSION);
    
    if (verstr == NULL || sscanf(verstr, "%d.%d", &info.major_version,
                                 &info.minor_version) != 2) {
        std::cout << "Invalid GL_VERSION format: " << info << "\n";
    }
    
    return info;
}

void nsgl::assertMinimumGLVersion(int major, int minor) {
    nsgl::GLVersionInfo wanted(major, minor);
    nsgl::GLVersionInfo info = nsgl::getGLVersionInfo();
    if (info.major_version < major) {
        nsgl::log("error: GLVersion " + info + " is lower than " + wanted);
        exit(1);
    }
    else if (info.major_version == major && info.minor_version < minor) {
        nsgl::log("error: GLVersion " + info + " is lower than " + wanted);
        exit(1);
    }
}

void nsgl::assertMinimumGLVersion(float version) {
    nsgl::assertMinimumGLVersion((int) version, (int) (std::fmod(version, 1.0f) * 10.0f));
}

void nsgl::logNonEmpty(std::string toLog, const std::string file, int line) {
    if (!toLog.empty()) {
        std::cout << "{" << file << ":" << line << "} " << toLog << "\n";
    }
}

void nsgl::log(std::string toLog, const std::string file, int line) {
    std::cout << "{" << file << ":" << line << "} " << toLog << "\n";
}

/// Prints the result of |getStateErrors| to stdout, if any exist.
void nsgl::printStateErrors(const std::string file, int line) {
    nsgl::logNonEmpty(nsgl::getStateErrors(), file, line);
}

void nsgl::assertNoStateErrors(const std::string file, int line) {
    std::string errors = nsgl::getStateErrors();
    if (!errors.empty()) {
        nsgl::log(errors, file, line);
        nsgl::log("Assertion failed: Open GL state errors occurred.", file, line);
        exit(1);
    }
}

nsgl::RGBAImage nsgl::screencap(int width, int height) {
	unsigned char *pixels = new unsigned char[3 * width * height];
	std::vector<util::Vec4ub> pixVec(width * height);

	NSGL_ERRORS();
	GLint boundBuffer;
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &boundBuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, boundBuffer);
	NSGL_ERRORS();
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int idx = (height - 1 - j) * width + i;

			pixVec[j * width + i] = util::Vec4ub(pixels[3 * idx + 0], pixels[3 * idx + 1], pixels[3 * idx + 2], 255);
		}
	}

	delete pixels;

	return nsgl::RGBAImage(width, height, pixVec);
}
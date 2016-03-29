///
/// nsgl_types.h
/// ------------
/// Nikolai Shkurkin
/// NSGL Library
///

#include "nsgl_types.h"
#include <stb/stb.h>

///
nsgl::RGBImage nsgl::rgbImageFromFile(const std::string fname) {
    int w, h, ncomps;
    unsigned char *data = stbi_load(fname.c_str(), &w, &h, &ncomps, 0);
    RGBImage::CollectionType parsedData;
    
    if (data == NULL || (ncomps != 3 && ncomps != 4)) {
        if (data == NULL)
            NSGL_LOG(fname + " not found.");
        UTIL_WHAT_IS(ncomps);
        exit(1);
    }
    
    if (ncomps == 3) {
        int which = -3;
        while ((which += 3) < (w * h * 3)) {
            parsedData.push_back(RGBImage::DataType(data[which], data[which + 1],
                                                    data[which + 2]));
        }
    }
    else {
        int which = -4;
        while ((which += 4) < (w * h * 4)) {
            parsedData.push_back(RGBImage::DataType(data[which], data[which + 1],
                                                    data[which + 2]));
        }
    }
    
    RGBImage img(w, h, parsedData);
    stbi_image_free(data);
    return img;
}

nsgl::RGBAImage nsgl::rgbaImageFromFile(const std::string fname) {
    int w, h, ncomps;
    unsigned char *data = stbi_load(fname.c_str(), &w, &h, &ncomps, 0);
    RGBAImage::CollectionType parsedData;
    
    if (data == NULL || (ncomps != 4 && ncomps != 3)) {
        if (data == NULL)
            NSGL_LOG(fname + " not found.");
        UTIL_WHAT_IS(ncomps);
        exit(1);
    }
    
    if (ncomps == 3) {
        int which = -3;
        while ((which += 3) < (w * h * 3)) {
            parsedData.push_back(RGBAImage::DataType(data[which], data[which + 1],
             data[which + 2], 255));
        }
    }
    else {
        int which = -4;
        while ((which += 4) < (w * h * 4)) {
            parsedData.push_back(RGBAImage::DataType(data[which], data[which + 1],
            data[which + 2], data[which + 3]));
        }
    }
    
    RGBAImage img(w, h, parsedData);
    stbi_image_free(data);
    return img;
}

///
///
///
nsgl::RGBTexture nsgl::rgbTexture(RGBImage & img, GLenum textureUnit) {
    nsgl::RGBTexture buffer(img, GL_RGB, GLenum(GL_UNSIGNED_BYTE), textureUnit);
    return buffer;
}

nsgl::RGBTexture nsgl::rgbTexture(const RGBImage & img, GLenum textureUnit) {
    nsgl::RGBTexture buffer(img, GL_RGB, GLenum(GL_UNSIGNED_BYTE),
     textureUnit);
    return buffer;
}

nsgl::RGBTexture nsgl::rgbTexture() {
    RGBImage img;
    return nsgl::rgbTexture(img, GL_TEXTURE0);
}

nsgl::RGBTexture nsgl::rgbTextureFromFile(const std::string fname,
                                              GLenum texUnit) {
    RGBImage img;
    img = nsgl::rgbImageFromFile(fname);
    return nsgl::rgbTexture(img, texUnit);
}



///
///
///
nsgl::RGBATexture nsgl::rgbaTexture(const RGBAImage & img, GLenum textureUnit) {
    nsgl::RGBATexture buffer(img, GL_RGBA, GLenum(GL_UNSIGNED_BYTE),
     textureUnit);
    return buffer;
}

nsgl::RGBATexture nsgl::rgbaTexture(RGBAImage & img, GLenum textureUnit) {
    nsgl::RGBATexture buffer(img, GL_RGBA, GLenum(GL_UNSIGNED_BYTE),
     textureUnit);
    return buffer;
}

nsgl::RGBATexture nsgl::rgbaTexture() {
    RGBAImage img;
    return nsgl::rgbaTexture(img, GL_TEXTURE0);
}

nsgl::RGBATexture nsgl::rgbaTextureFromFile(const std::string fname,
                                                GLenum texUnit) {
    RGBAImage img;
    img = nsgl::rgbaImageFromFile(fname);
    return nsgl::rgbaTexture(img, texUnit);
}

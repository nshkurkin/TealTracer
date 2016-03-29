///
/// Image.h
/// -------
/// Nikolai Shkurkin
/// Utility Library
///

#ifndef ____util_Image__
#define ____util_Image__

#include <vector>
#include <string>
#include <Eigen/Dense>

#include <src/util/util_base.h>
#include <src/util/util_types.h>

namespace util {
    
    /// Represents a 2D image of a given `PixelType`. It allows for filling and
    /// access of data.
    template< typename PixelType >
    struct Image {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        /// The container type for the pixel data
        typedef std::vector< PixelType > CollectionType;
        typedef PixelType DataType;
        
        int width, height;
        CollectionType pixels;
        /// Whether or not the images has been changed
        bool changed;
        
        /// Creates a 1-pixel image with the default pixel (black).
        Image() {init_fill(0, 0, PixelType());}
        
        /// Creates a `width` by `height` image using an optional `filler`.
        Image(int width, int height, PixelType filler = PixelType()) {
            init_fill(width, height, filler);
        }
        
        /// Used as an in-place initialization of this image.
        void init_fill(int width, int height, PixelType filler) {
            this->width = width;
            this->height = height;
            
            fill(filler);
            setAsChanged();
        }
        
        /// Creates an image using `data` with respect to `width` and `height`.
        Image(int width, int height, CollectionType & data) {
            init_data(width, height, data);
        }
        
        /// Used to initialize this image with a given set of pixel `data`.
        void init_data(int width, int height, CollectionType data) {
            this->width = width;
            this->height = height;
            
            pixels.clear();
            int which = -1;
            while (++which < (int) data.size())
                pixels.push_back(data[which]);
            setAsChanged();
        }
        
        /// Fills all the pixels in this image with `filler`.
        void fill(PixelType filler) {
            pixels.resize(width * height, filler);
            setAsChanged();
        }
        
        /// Used to indicate that this image has changed. This is useful for
        /// buffers that may need to update their contents as this image changes.
        void setAsChanged() {changed = true;}
        /// Used to set `changed` to false to indicate that this image has not
        /// been changed. This is useful for signifying that a buffer that may
        /// use this image that this image does not be need to be re-loaded.
        void clearChanged() {changed = false;}
        
        /// Access a pixel at `x` and `y` (width and height respectively) in the
        /// image.
        PixelType & operator() (int x, int y) {
            return get(x, y);
        }
        
        /// Used to set the pixel (`x`, `y`) to `value`.
        void set(int x, int y, PixelType value) {
            get(x, y) = value;
            setAsChanged();
        }
        
        /// Returns a reference to the pixel (`x`, `y`).
        PixelType & get(int x, int y) {
            return pixels[x + width * y];
        }
        
        /// Used to vertically flip all of the pixels in-place.
        void flipVertically() {
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height / 2; y++) {
                    int yp = height - y - 1;
                    PixelType tmp = PixelType(get(x, y));
                    get(x, y) = get(x, yp);
                    get(x, yp) = tmp;
                }
            }
        }
        
        /// Used to vertically flip all of the pixels in-place.
        void flipHorizontally() {
            for (int x = 0; x < width / 2; x++) {
                for (int y = 0; y < height; y++) {
                    int xp = width - x - 1;
                    PixelType tmp = get(xp, y);
                    get(xp, y) = get(x, y);
                    get(x,  y) = tmp;
                }
            }
        }
        
    };
    
    /// An image represented using greyscale.
    typedef Image< uint8_t > BitmapImage;
    
    typedef Image< Vec3ub > Image3ub;
    typedef Image3ub RGBImage;
    
    typedef Image< Vec4ub > Image4ub;
    typedef Image4ub RGBAImage;
}


#endif // ____util_Image__

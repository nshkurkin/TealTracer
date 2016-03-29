///
/// DataBufferObject.h
/// ------------------
/// Nikolai Shkurkin
/// NSGL Library
///

#ifndef ____nsgl_DataBufferObject__
#define ____nsgl_DataBufferObject__

#include <src/nsgl/gl_interface/gl.h>
#include <src/nsgl/nsgl_base.h>

#include <vector>
#include <Eigen/Dense>

namespace nsgl {
    
    /// Represents a wrapper around data buffer objects in opengl. These are
    /// used send data to the GPU for use in your shaders. It can take any type,
    /// including Eigen types. All that is requires is that the objects have a
    /// default constructor. Data buffer objects come in two flavors:
    /// GL_ARRAY_BUFFER and GL_ELEMENT_ARRAY_BUFFER. You are recommended to use
    /// the arrayBuffer(...) and elementArrayBuffer(..) static methods to create
    /// these two kinds of data buffer types.
    template< typename DataType >
    struct DataBufferObject {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        
        typedef std::vector< DataType > CollectionType;
        
        /// The data of this data buffer object.
        CollectionType data;
        /// The type buffer, either GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER.
        GLenum bufferType;
        /// The opengl handle for this data buffer object.
        GLuint handle;
        
        /// Whether or not this buffer has been allocated within opengl.
        bool allocated;
        /// Whther or not the data of this buffer has been sent.
        bool dataIsSent;
        
        /// This constructor copies data of an STL vector
        DataBufferObject(GLenum bufferType = 0,
                         CollectionType data = CollectionType()) {
            init(bufferType, data);
        }
        
        /// Takes a copy the contents of another buffer
        DataBufferObject(const DataBufferObject<DataType> & buffer) {
            init(buffer.bufferType, buffer.data);
        }
        
        /// In-place initializer for this data buffer obejct.
        void init(GLenum bufferType, CollectionType data) {
            this->bufferType = bufferType;
            this->data = data;
            
            handle = -1;
            allocated = false;
            dataIsSent = false;
        }
        
        /// Creates a GL_ARRAY_BUFFER data buffer object with the given
        /// (optional) `data`.
        static DataBufferObject<DataType> arrayBuffer(CollectionType data
         = CollectionType()) {
            return DataBufferObject<DataType>(GLenum(GL_ARRAY_BUFFER), data);
        }
        
        /// Creates a GL_ELEMENT_ARRAY_BUFFER data buffer object with the given
        /// (optional) `data`.
        static DataBufferObject<DataType> elementArrayBuffer(CollectionType data
         = CollectionType()) {
            return DataBufferObject<DataType>(GLenum(GL_ELEMENT_ARRAY_BUFFER), data);
        }
        
        /// Allocates this buffer within opengl.
        void glAlloc() {
            if (!allocated) {
                glGenBuffers(1, &handle);
                allocated = true;
            }
        }
        
        /// Clears the internal data of this buffer
        void clearData() {
            data.clear();
            setNeedsUpdate();
        }
        
        /// Free the associated opengl data with this buffer.
        void glFree() {
            if (allocated) {
                glDeleteBuffers(1, &handle);
                data.clear();
                handle = -1;
                allocated = false;
                setNeedsUpdate();
            }
        }
        
        /// Makes this the current buffer in opengl.
        void glBind() {
            glAlloc();
            glBindBuffer(bufferType, handle);
        }
        
        /// Makes buffer 0 the current buffer.
        void glUnbind() {
            glBindBuffer(bufferType, 0);
        }
        
        /// Send the data of this buffer if needed. `usageType` is one of
        /// either GL_DYNAMIC_DRAW or GL_STATIC_DRAW. If you plan to update this
        /// buffer a lot, use GL_DYNAMIC_DRAW. Otherwise use GL_STATIC_DRAW.
        void glSendData(GLenum usageType) {
            if (!dataIsSent) {
                glBind();
                
                if (data.size() > 0) {
                    glBufferData(bufferType, data.size() * sizeof(DataType),
                     &data[0], usageType);
                }
                
                dataIsSent = true;
                glUnbind();
            }
        }
        
        /// Append data onto the current buffer. Will automatically set its
        /// state to send the new data.
        void add(CollectionType & moreData) {
            int which = -1;
            while (++which < (int) moreData.size())
                data.push_back(moreData[which]);
            setNeedsUpdate();
        }
        
        /// Append data onto the current buffer. Will automatically set its
        /// state to send the new data.
        void add(const CollectionType & moreData) {
            int which = -1;
            while (++which < moreData.size())
                data.push_back(moreData[which]);
            setNeedsUpdate();
        }
        
        /// Appends a single data item to the current buffer.
        void add(DataType & newItem) {
            data.push_back(newItem);
            setNeedsUpdate();
        }
        
        /// Appends a const reference data item.
        void add(const DataType & newItem) {
            data.push_back(newItem);
            setNeedsUpdate();
        }
        
        /// Call this to notify this buffer if the data needs to be re-sent to OpenGL
        void setNeedsUpdate() {dataIsSent = false;}
        /// Returns whether this buffer object is valid within opengl.
        bool isValid() {return handle >= 0;}
    };
    
}

#endif // ____nsgl_DataBufferObject__

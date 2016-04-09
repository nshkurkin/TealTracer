//
//
//
//
//

__constant float3 Up = (float3){0.0, 1.0, 0.0};
__constant float3 Forward = (float3){0.0, 0.0, -1.0};
__constant float3 Right = (float3){1.0, 0.0, 0.0};

///
struct PovrayCameraData {
    float3 location;
    float3 up;
    float3 right;
    float3 lookAt;
};

///
struct PovrayLightSourceData {
    float3 position;
    float4 color;
};

///
struct PovrayPigment {
    float4 color;
};

///
struct PovrayFinish {
    float ambient;
    float diffuse;
};

///
struct PovraySphereData {
    float3 position;
    float radius;
    
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
};

///
struct PovrayPlaneData {
    float3 normal;
    float distance;
    
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
};

////////////////////////////////////////////////////////////////////////////

///
struct mat4x4 {
    float4 col[4];
};

///
struct ubyte4 {
    unsigned char bytes[4];
};

///
struct ubyte4 ubyte4_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

///
void mat4x4_set(struct mat4x4 * mat, int row, int col, float val);
///
float mat4x4_get(struct mat4x4 * mat, int row, int col);
///
float4 mat4x4_getRow(struct mat4x4 * mat, int row);

///
float4 mat4x4_mult4x1(struct mat4x4 * mat, float4 vec);
///
void mat4x4_loadIdentity(struct mat4x4 * mat);
///
void mat4x4_loadLookAt(struct mat4x4 * result, float3 eye, float3 center, float3 up);

///
float3 normalized(float3 vec);
///
float3 crossed(float3 vecA, float3 vecB);

////////////////////////////////////////////////////////////////////////////

///
struct ubyte4 ubyte4_make(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    struct ubyte4 result;
    result.bytes[0] = r;
    result.bytes[1] = g;
    result.bytes[2] = b;
    result.bytes[3] = a;
    return result;
}

///
void mat4x4_set(struct mat4x4 * mat, int row, int col, float val) {
    ((float *) &(mat->col[col]))[row] = val;
}

///
float mat4x4_get(struct mat4x4 * mat, int row, int col) {
    float4 column = mat->col[col];
    return ((float *) &column)[row];
}

///
float4 mat4x4_getRow(struct mat4x4 * mat, int row) {
    float4 result;
    
    result.x = mat4x4_get(mat, row, 0);
    result.y = mat4x4_get(mat, row, 1);
    result.z = mat4x4_get(mat, row, 2);
    result.w = mat4x4_get(mat, row, 3);
    
    return result;
}

///
float4 mat4x4_mult4x1(struct mat4x4 * mat, float4 vec) {
    float4 result;
    
    result.x = dot(vec, mat4x4_getRow(mat, 0));
    result.y = dot(vec, mat4x4_getRow(mat, 1));
    result.z = dot(vec, mat4x4_getRow(mat, 2));
    result.w = dot(vec, mat4x4_getRow(mat, 3));
    
    return result;
}

///
void mat4x4_loadIdentity(struct mat4x4 * mat) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                mat4x4_set(mat,i,j, 1.0);
            }
            else {
                mat4x4_set(mat,i,j, 0.0);
            }
        }
    }
}

float3 normalized(float3 vec) {
    float len = length(vec);
    if (len < 0.001) {
        return (float3) {0,0,1};
    }
    else {
        return vec / len;
    }
}

float3 crossed(float3 vecA, float3 vecB) {
    return (float3) {
        vecA.y * vecB.z - vecA.z * vecB.y,
        vecA.z * vecB.x - vecA.x * vecB.z,
        vecA.x * vecB.y - vecA.y * vecB.x
    };
}

///
void mat4x4_loadLookAt(struct mat4x4 * result, float3 eye, float3 center, float3 up) {
//    float3 f = normalized(center - eye);
//    float3 s = normalized(crossed(f, up));
//    float3 u = cross(s, f);
    
//    mat4x4_loadIdentity(result);
    
//    mat4x4_set(result,0,0, s.x);
//    mat4x4_set(result,1,0, s.y);
//    mat4x4_set(result,2,0, s.z);
//    mat4x4_set(result,0,1, u.x);
//    mat4x4_set(result,1,1, u.y);
//    mat4x4_set(result,2,1, u.z);
//    mat4x4_set(result,0,2, -f.x);
//    mat4x4_set(result,1,2, -f.y);
//    mat4x4_set(result,2,2, -f.z);
    
//    mat4x4_set(result,3,0, -dot(s, eye));
//    mat4x4_set(result,3,1, -dot(u, eye));
//    mat4x4_set(result,3,2, dot(f, eye));
}

////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
///
/// KERNEL: raytrace_one_ray
///
/// SYNOPSIS: Used to cast a single ray into the scene. It calculates the ray
///     based on the global thread ID. Rays are then intersected with elements
///     in the scene and the closest found object will be used as the color
///     sample source. The resulting color is placed in the output texture buffer.
///

__kernel void raytrace_one_ray(
    /// input
    
    const struct PovrayCameraData camera,
    
    __global struct PovraySphereData * spheres,
    const unsigned int numSpheres,
    
    __global struct PovrayPlaneData * planes,
    const unsigned int numPlanes,
    
    /// output
    /// image data is organized into 4 byte pixels in a height*width pixels
    __global unsigned char * imageOutput,
    const unsigned int imageWidth,
    const unsigned int imageHeight
    ) {
    
    /// Create all of the rays
    struct mat4x4 viewTransform;

    float3 lookAt = camera.lookAt;
    float3 location = camera.location;
    float3 diff = lookAt - location;
    float3 f = diff / length(diff);
    float3 up = camera.up;
    float3 crossedFUP = crossed(f, up);
    float lenCrossedFUP = length(crossedFUP);
    
//    float3 s = crossedFUP / length(crossedFUP);
//    float3 s = normalized(crossed(f, up));
//    float3 f = normalized(camera.lookAt - camera.location);
//    float3 s = normalize(cross(f, camera.up));
//    float3 u = cross(s, f);

//    mat4x4_loadLookAt(&viewTransform, camera.location, camera.lookAt, camera.up);
//
//    float3 forward = mat4x4_mult4x1(&viewTransform, (float4){Forward.x, Forward.y, Forward.z, 0.0}).xyz;
//    float3 up = mat4x4_mult4x1(&viewTransform, (float4){Up.x, Up.y, Up.z, 0.0}).xyz * length(camera.up);
//    float3 right = mat4x4_mult4x1(&viewTransform, (float4){Right.x, Right.y, Right.z, 0.0}).xyz * length(camera.right);
    
    unsigned int threadId = (unsigned int) get_global_id(0);
    
    int px = threadId % imageWidth;
    int py = threadId / imageWidth;
    struct ubyte4 color;
    
    color = ubyte4_make(200,px%255,py%255,255);
    
//    float3 rayPos
//        = camera.location + forward - 0.5f*up - 0.5f*right
//        + right * ((0.5f+(float)px)/(float)imageWidth)
//        + up * ((0.5f+(float)py)/(float)imageHeight);
//    float3 rayDirection = normalize(rayPos - camera.location);
    
    /// Update the output
    for (int i = 0; i < 4; i++) {
        imageOutput[4 * (px + imageWidth * py) + i] = color.bytes[i];
    }
}


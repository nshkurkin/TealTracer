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

__constant const unsigned int kPovrayPigmentStride = 4;

///
struct PovrayFinish {
    float ambient;
    float diffuse;
};

__constant const unsigned int kPovrayFinishStride = 2;

///
struct PovraySphereData {
    float3 position;
    float radius;
    
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
};

__constant unsigned int kPovraySphereStride = 3 + 1 + kPovrayPigmentStride + kPovrayFinishStride;

///
struct PovrayPlaneData {
    float3 normal;
    float distance;
    
    struct PovrayPigment pigment;
    struct PovrayFinish finish;
};

__constant unsigned int kPovrayPlaneStride = 3 + 1 + kPovrayPigmentStride + kPovrayFinishStride;

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

/////
//float3 normalized(float3 vec);
/////
//float3 crossed(float3 vecA, float3 vecB);

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

///
void mat4x4_loadLookAt(struct mat4x4 * result, float3 eye, float3 center, float3 up) {
    float3 f = normalize(center - eye);
    float3 s = normalize(cross(f, up));
    float3 u = cross(s, f);
    
    mat4x4_loadIdentity(result);
    
    mat4x4_set(result,0,0, s.x);
    mat4x4_set(result,1,0, s.y);
    mat4x4_set(result,2,0, s.z);
    mat4x4_set(result,0,1, u.x);
    mat4x4_set(result,1,1, u.y);
    mat4x4_set(result,2,1, u.z);
    mat4x4_set(result,0,2, -f.x);
    mat4x4_set(result,1,2, -f.y);
    mat4x4_set(result,2,2, -f.z);
    
    mat4x4_set(result,3,0, -dot(s, eye));
    mat4x4_set(result,3,1, -dot(u, eye));
    mat4x4_set(result,3,2, dot(f, eye));
}

////////////////////////////////////////////////////////////////////////////

///
enum ObjectType {
    InvalidObjectTyoe = 0,
    SphereObjectType = 1,
    PlaneObjectType = 2
};

///
struct RayIntersectionResult {
    bool intersected;
    float timeOfIntersection;
    
    enum ObjectType type;
    __global float * dataPtr;
};

///
struct PovraySphereData PovraySphereData_fromData(__global float * data);
///
struct RayIntersectionResult closest_sphere_intersection(__global float * data, unsigned int numDataElements, float3 rayOrigin, float3 rayDirection);
///
void sphere_intersect(struct PovraySphereData data, float3 rayOrigin, float3 rayDirection, struct RayIntersectionResult * result);


///
struct PovraySphereData PovraySphereData_fromData(__global float * data) {
    struct PovraySphereData result;
    
    result.position = (float3) { data[0], data[1], data[2] };
    result.radius = data[3];
    result.pigment.color = (float4) { data[4], data[5], data[6], data[7] };
    result.finish.ambient = data[8];
    result.finish.diffuse = data[9];
    
    return result;
}

///
struct RayIntersectionResult closest_sphere_intersection(__global float * data, unsigned int numDataElements, float3 rayOrigin, float3 rayDirection) {

    struct RayIntersectionResult result;
    result.intersected = false;
    result.timeOfIntersection = INFINITY;
    
    for (unsigned int i = 0; i < numDataElements; i++) {
        struct RayIntersectionResult currentResult;
        currentResult.intersected = false;
        currentResult.timeOfIntersection = INFINITY;
    
        __global float * dataStart = &data[i*kPovraySphereStride];
        struct PovraySphereData data = PovraySphereData_fromData(dataStart);
        currentResult.dataPtr = dataStart;
        
        sphere_intersect(data, rayOrigin, rayDirection, &currentResult);
        
        if (currentResult.intersected && currentResult.timeOfIntersection < result.timeOfIntersection) {
            result = currentResult;
        }
    }
    
    result.type = SphereObjectType;
    return result;

}

///
void sphere_intersect(struct PovraySphereData data, float3 rayOrigin, float3 rayDirection, struct RayIntersectionResult * result) {
    
    float A = dot(rayDirection, rayDirection);
    float B = 2.0 * dot(rayOrigin - data.position, rayDirection);
    float C = dot(rayOrigin - data.position, rayOrigin - data.position) - data.radius * data.radius;
    
    float radical = B*B - 4.0*A*C;
    if (radical >= 0) {
        float sqrRadical = sqrt(radical);
        float t0 = (-B + sqrRadical)/(2.0 * A);
        float t1 = (-B - sqrRadical)/(2.0 * A);
        result->intersected = t0 >= 0 || t1 >= 0;
        if (t0 >= 0 && t1 >= 0) {
            result->timeOfIntersection = min(t0, t1);
        }
        else if (t0 >= 0) {
            result->timeOfIntersection = t0;
        }
        else if (t1 >= 0) {
            result->timeOfIntersection = t1;
        }
    }
}

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
    const float3 camera_location,
    const float3 camera_up,
    const float3 camera_right,
    const float3 camera_lookAt,
    
    __global float * sphereData,
    const unsigned int numSpheres,

    __global float * planeData,
    const unsigned int numPlanes,
    
    /// output
    /// image data is organized into 4 byte pixels in a height*width pixels
    __global unsigned char * imageOutput,
    const unsigned int imageWidth,
    const unsigned int imageHeight
    ) {
    
    /// Create the ray for this given pixel
    struct mat4x4 viewTransform;

    mat4x4_loadLookAt(&viewTransform, camera_location, camera_lookAt, camera_up);

    float3 forward = mat4x4_mult4x1(&viewTransform, (float4){Forward.x, Forward.y, Forward.z, 0.0}).xyz;
    float3 up = mat4x4_mult4x1(&viewTransform, (float4){Up.x, Up.y, Up.z, 0.0}).xyz * length(camera_up);
    float3 right = mat4x4_mult4x1(&viewTransform, (float4){Right.x, Right.y, Right.z, 0.0}).xyz * length(camera_right);
    
    unsigned int threadId = (unsigned int) get_global_id(0);
    
    int px = threadId % imageWidth;
    int py = threadId / imageWidth;
    struct ubyte4 color;
    
    color = ubyte4_make(0,px%255,py%255,255);
    
    float3 rayOrigin
        = camera_location + forward - 0.5f*up - 0.5f*right
        + right * ((0.5f+(float)px)/(float)imageWidth)
        + up * ((0.5f+(float)py)/(float)imageHeight);
    float3 rayDirection = normalize(rayOrigin - camera_location);
    
    /// Now we need to perform intersection tests
    struct RayIntersectionResult sphereIntersection = closest_sphere_intersection(sphereData, numSpheres, rayOrigin, rayDirection);
    
    if (sphereIntersection.intersected) {
        struct PovraySphereData sphere = PovraySphereData_fromData(sphereIntersection.dataPtr);
        color = ubyte4_make(sphere.pigment.color.x * 255, sphere.pigment.color.y * 255, sphere.pigment.color.z * 255, sphere.pigment.color.w * 255);
    }
    
    /// Update the output
    for (int i = 0; i < 4; i++) {
        imageOutput[4 * (px + imageWidth * py) + i] = color.bytes[i];
    }
}


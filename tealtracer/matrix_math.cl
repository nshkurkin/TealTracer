//
//  matrix_math.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/15/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef matrix_math_h
#define matrix_math_h

////////////////////////////////////////////////////////////////////////////

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif // define(M_PI)

///
struct mat4x4 {
    float4 col[4];
};

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

////////////////////////////////////////////////////////////////////////////

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
                mat4x4_set(mat,i,j, 1.0f);
            }
            else {
                mat4x4_set(mat,i,j, 0.0f);
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

#endif /* matrix_math_h */

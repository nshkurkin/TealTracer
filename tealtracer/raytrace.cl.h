//
//  raytrace.cl.h
//  tealtracer
//
//  Created by Nikolai Shkurkin on 4/16/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef raytrace_cl_h
#define raytrace_cl_h

#ifdef DEVICE_INCLUDE

typedef int cl_int;
typedef unsigned int cl_uint;
typedef float cl_float;
typedef float3 cl_float3;

#else // !defined(DEVICE_INCLUDE)

#include <OpenCL/opencl.h>
#include "compute_types.hpp"

typedef float3 cl_float3;

#endif // defined(DEVICE_INCLUDE)

#ifndef packed_struct
#define packed_struct struct __attribute__((__packed__))
#endif


#endif /* raytrace_cl_h */

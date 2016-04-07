//
// https://developer.apple.com/library/mac/samplecode/OpenCL_Hello_World_Example/Listings/hello_c.html
//

__kernel void square(
    __global float * input,
    __global float * output,
    const unsigned int count) {
    
    unsigned int id = (unsigned int) get_global_id(0);
    if (id < count) {
        output[id] = input[id] * input[id];
    }
}

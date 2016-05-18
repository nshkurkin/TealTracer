//
//  photons.cl
//  tealtracer
//
//  Created by Nikolai Shkurkin on 5/17/16.
//  Copyright © 2016 Teal Sunset Studios. All rights reserved.
//

#ifndef photons_h
#define photons_h

typedef float3 RGBf;

///
struct JensenPhoton {

    ///
    float3 position;
    float3 incomingDirection;
    
    ///
    RGBf energy;
};

struct JensenPhoton JensenPhoton_fromData(
    global const float * photon_data,
    int whichPhoton);

void JensenPhoton_setData(
    struct JensenPhoton * photon,
    // output
    global float * photon_data,
    int whichPhoton);

///
struct JensenPhoton JensenPhoton_fromData(
    global const float * photon_data,
    int whichPhoton) {
 
    struct JensenPhoton photon;
    
    global const float * photon_floats_start = &(photon_data[whichPhoton * 9]);
    
    photon.position = (float3) {
        photon_floats_start[0],
        photon_floats_start[1],
        photon_floats_start[2]
    };
    
    photon.incomingDirection = (float3) {
        photon_floats_start[3],
        photon_floats_start[4],
        photon_floats_start[5]
    };
    
    photon.energy = (float3) {
        photon_floats_start[6],
        photon_floats_start[7],
        photon_floats_start[8]
    };
    
    return photon;
}

///
void JensenPhoton_setData(
    struct JensenPhoton * photon,
    // output
    global float * photon_data,
    int whichPhoton) {

    global float * photon_floats_start = &(photon_data[whichPhoton * 9]);
    
    photon_floats_start[0] = photon->position.x;
    photon_floats_start[1] = photon->position.y;
    photon_floats_start[2] = photon->position.z;
    
    photon_floats_start[3 + 0] = photon->incomingDirection.x;
    photon_floats_start[3 + 1] = photon->incomingDirection.y;
    photon_floats_start[3 + 2] = photon->incomingDirection.z;
    
    photon_floats_start[6 + 0] = photon->energy.x;
    photon_floats_start[6 + 1] = photon->energy.y;
    photon_floats_start[6 + 2] = photon->energy.z;
}

#endif /* photons_h */

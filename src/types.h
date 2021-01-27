#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

// 3D vector
struct __v3 {
    float x;
    float y;
    float z;
};

#define Vector3 struct __v3

// 2D vector
struct __v2 {
    float x;
    float y;
};


#define Vector2 struct __v2

// 32-bit color
struct __color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

#define Color struct __color

// Mesh
struct __mesh {
    
    // Vertice count - NOTE: Max vertice count is 65 535 for a single model
    uint16_t vertcount;
    
    // Face count
    uint16_t facecount;

    // Vertice array
    Vector3 *verts;

    // Face definitions, array of integers grouped in 3 values. Value corresponds to the vertice in the verts array
    uint16_t *vertindex;
    
    // Normals
    Vector3 *normals;

    // Normal indices
    uint16_t *normalindex;

    // Material indices
    uint16_t materialindex[32];

};

#define Mesh struct __mesh


#endif
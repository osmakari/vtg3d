#ifndef _ENTITY_H
#define _ENTITY_H

#include "types.h"

// Entity structure
struct __ent {
    Vector3 position;
    Vector3 rotation;

    Mesh *mesh;
};

#define Entity struct __ent

// Creates an entity - returns created entity or NULL
Entity *vtg_createEntity ();

// Destroys an entity
uint8_t vtg_destroyEntity (Entity *e);


#endif
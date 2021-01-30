#ifndef _OBJIMPORT_H
#define _OBJIMPORT_H

#include "../types.h"

#define OBJIMPORT_MAX_MESHES 256

extern Mesh *loaded_meshes[OBJIMPORT_MAX_MESHES];

uint8_t objLoad (const char *path, Mesh *mesh);
uint8_t objUnload (Mesh *mesh);


#endif
#include "entity.h"

#include <stdio.h>
#include <stdlib.h>

extern int __vtg_entity_array_size;
extern Entity **_vtg_entities;
extern int _vtg_entity_count;



Entity *vtg_createEntity () {
    int p = -1;
    for(int a = 0; a < __vtg_entity_array_size; a++) {
        if(_vtg_entities[a] == NULL) {
            p = a;
            break;
        }
    }
    if(p == -1) {
        return NULL;
    }
    Entity *e = malloc(sizeof(Entity));
    e->position = (Vector3) {
        .x = 0, .y = 0, .z = 1
    };
    e->rotation = (Vector3) {
        .x = 0, .y = 0, .z = 0
    };

    e->mesh = NULL;

    _vtg_entities[p] = e;

    _vtg_entity_count++; 

    return e;
}

uint8_t vtg_destroyEntity (Entity *e) {
    if(e == NULL)
        return 1;
    
    for(int a = 0; a < __vtg_entity_array_size; a++) {
        if(_vtg_entities[a] != NULL && _vtg_entities[a] == e) {
            _vtg_entities[a] = NULL;
            _vtg_entity_count--;
            free(e);
            return 0;
        }
    }
    return 2;
}
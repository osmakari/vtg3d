// Object importer
#include "objimport.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define OBJIMPORT_MAX_VERTICES      2048 // 1 vert = 12 bytes
#define OBJIMPORT_MAX_FACES         2048 // 1 face = 3 * 2 bytes = 6 bytes
#define OBJIMPORT_MAX_MATERIALS     32

Mesh *loaded_meshes[OBJIMPORT_MAX_MESHES];

struct __tmp_obj {
    // Vertex position information
    Vector3 *vertices;
    int vertexcount;

    // Vertex indices
    uint16_t *vertexindex;
    int vertexindexcount;

    // Normal vector information
    Vector3 *normals;
    int normalcount;

    // Normal indices
    uint16_t *normalindex;
    int normalindexcount;

    // UV vertices
    Vector2 *uvvs;
    int uvvcount;

    // UV indices
    uint16_t *uvindex;
    int uvindexcount;

    // Material offsets
    uint16_t materialoffsets[OBJIMPORT_MAX_MATERIALS]; // max materials
    int materialoffsetcount;

    // Materials buffer
    Material materials[32];
    int materialcount;

    // Material file name
    char mtlfile[64];

};


uint8_t is_whitespace (char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void normalizeVector (Vector3 *v) {
    float d = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    v->x /= d;
    v->y /= d;
    v->z /= d;
}

uint8_t obj_parse_line (char *line, struct __tmp_obj *obj) {
    char strbuf[256] = { 0 };
    int strix = 0;
    size_t slen = strlen(line);

    Vector3 tmp_vec = {
        0, 0, 0
    };

    int facevs[16] = { 0 };
    int uvvs[16] = { 0 };
    int normalvs[16] = { 0 };
    int fcnt = 0;

    if(strlen(line) <= 0 || line[0] == '#') {
        // Comment or empty row
        return 1;
    }

    int state = 0;
    for(int i = 0; i < slen; i++) {
        char c = line[i];
        if(state == 0) {
            if(!is_whitespace(c)) {
                strbuf[strix++] = c;
            }
            else {
                strbuf[strix] = 0;
                strix = 0;
                if(strcmp(strbuf, "v") == 0) {
                    state = 1; // read vertice positions
                    continue;
                }
                else if(strcmp(strbuf, "f") == 0) {
                    state = 2; // read faces
                    continue;
                }
                else if(strcmp(strbuf, "vn") == 0) {
                    state = 3; // read normals
                    continue;
                }
                else if(strcmp(strbuf, "vt") == 0) {
                    state = 4; // read UV
                    continue;
                }
                else if(strcmp(strbuf, "usemtl") == 0) {
                    state = 5; // change material
                    continue;
                }
                else if(strcmp(strbuf, "mtllib") == 0) {
                    state = 6; // Get mtl file name
                    continue;
                }
                else {
                    // Unknown command
                    return 2;
                }
            }
        }
        else if(state == 1) {
            // Whitespace already skipped, read vertice positions
            for(int x = 0; x < 3; x++) {
                do {
                    c = line[i];
                    strbuf[strix++] = c;
                    i++;
                } while (!is_whitespace(c) && i < slen);
                strbuf[strix] = 0;
                switch(x) {
                    case 0:
                    {
                        tmp_vec.x = atof(strbuf);
                        break;
                    }
                    case 1:
                    {
                        tmp_vec.y = atof(strbuf);
                        break;
                    }
                    case 2:
                    {
                        tmp_vec.z = atof(strbuf);
                        break;
                    }
                }
                strix = 0;
            }
            
            obj->vertices[obj->vertexcount] = tmp_vec;
            obj->vertexcount++;
            break;
        }
        else if(state == 2) {
            // Whitespace already skipped, read faces, uvs and normals
            for(int x = 0; x < 3; x++) {
                do {
                    c = line[i];
                    strbuf[strix++] = c;
                    i++;
                } while (!is_whitespace(c) && i < slen);
                strbuf[strix] = 0;

                // Face offset is always 0

                // UV and normal come after /
                int uv_offset = 0;
                int normal_offset = 0;

                int ccount = 0;
                // Get face vertex
                int cc = strbuf[ccount++];
                while(cc != '/' && ccount < strix && !is_whitespace(cc)) {

                    cc = strbuf[ccount++];
                }
                strbuf[ccount - 1] = 0;
                facevs[x] = atoi(strbuf);

                // Get face UV
                uv_offset = ccount;
                cc = strbuf[ccount++];
                while(cc != '/' && ccount < strix && !is_whitespace(cc)) {

                    cc = strbuf[ccount++];
                }
                strbuf[ccount - 1] = 0;
                uvvs[x] = atoi(strbuf + uv_offset);
                
                // Get face Normal
                normal_offset = ccount;
                cc = strbuf[ccount++];
                while(cc != '/' && ccount < strix && !is_whitespace(cc)) {

                    cc = strbuf[ccount++];
                }
                strbuf[ccount - 1] = 0;
                normalvs[x] = atoi(strbuf + normal_offset);


                strix = 0;
            }
            obj->vertexindex[obj->vertexindexcount++] = facevs[0] - 1;
            obj->vertexindex[obj->vertexindexcount++] = facevs[1] - 1;
            obj->vertexindex[obj->vertexindexcount++] = facevs[2] - 1;

            obj->uvindex[obj->uvindexcount++] = uvvs[0] - 1;
            obj->uvindex[obj->uvindexcount++] = uvvs[1] - 1;
            obj->uvindex[obj->uvindexcount++] = uvvs[2] - 1;

            obj->normalindex[obj->normalindexcount++] = normalvs[0] - 1;
            // Don't need extra indices, only 1 normal for a face
            //obj->normalindex[obj->normalindexcount++] = normalvs[1] - 1;
            //obj->normalindex[obj->normalindexcount++] = normalvs[2] - 1;

            break;
        }
        else if(state == 3) {
            // Whitespace already skipped, read normals
            for(int x = 0; x < 3; x++) {
                do {
                    c = line[i];
                    strbuf[strix++] = c;
                    i++;
                } while (!is_whitespace(c) && i < slen);
                strbuf[strix] = 0;
                switch(x) {
                    case 0:
                    {
                        tmp_vec.x = atof(strbuf);
                        break;
                    }
                    case 1:
                    {
                        tmp_vec.y = atof(strbuf);
                        break;
                    }
                    case 2:
                    {
                        tmp_vec.z = atof(strbuf);
                        break;
                    }
                }
                strix = 0;
            }
            normalizeVector(&tmp_vec);
            
            obj->normals[obj->normalcount] = tmp_vec;
            obj->normalcount++;
            
            break;
        }
        else if(state == 4) {
            // Whitespace already skipped, read uv positions
            for(int x = 0; x < 2; x++) {
                do {
                    c = line[i];
                    strbuf[strix++] = c;
                    i++;
                } while (!is_whitespace(c) && i < slen);
                strbuf[strix] = 0;
                switch(x) {
                    case 0:
                    {
                        tmp_vec.x = atof(strbuf);
                        break;
                    }
                    case 1:
                    {
                        tmp_vec.y = atof(strbuf);
                        break;
                    }
                }
                strix = 0;
            }
            
            obj->uvvs[obj->uvvcount].x = tmp_vec.x;
            obj->uvvs[obj->uvvcount].y = tmp_vec.y;
            obj->uvvcount++;
            break;
        }
        else if(state == 5) {
            // TODO TODO TODO
            obj->materialoffsets[obj->materialoffsetcount++] = obj->vertexindexcount/3;

            // SKIP REST TODO: read material name and assign it
            return 1;

            break;
        }
        else if(state == 6) {
            if(!is_whitespace(c) && i < slen - 1) {
                strbuf[strix++] = c;
            }
            else {
                if(i < slen) {
                    strbuf[strix++] = c;
                }
                strbuf[strix] = 0;
                strcpy(obj->mtlfile, strbuf);
#if DEBUGLEVEL > 1
                printf("ADDED MTL FILE: [%s]\n", obj->mtlfile);
#endif
                return 0;
            }
        }
    }

    return 0;
}

uint8_t mtl_parse_line (char *line, struct __tmp_obj *obj) {
    char strbuf[256] = { 0 };
    int strix = 0;
    size_t slen = strlen(line);

    Color tmp_color = {
        0, 0, 0
    };

    if(strlen(line) <= 0 || line[0] == '#') {
        // Comment or empty row
        return 1;
    }

    int state = 0;
    for(int i = 0; i < slen; i++) {
        char c = line[i];
        if(state == 0) {
            if(!is_whitespace(c)) {
                strbuf[strix++] = c;
            }
            else {
                strbuf[strix] = 0;
                strix = 0;
                if(strcmp(strbuf, "newmtl") == 0) {
                    state = 1; // Create new material
                    
                    continue;
                }
                else if(strcmp(strbuf, "Kd") == 0) {
                    state = 2; // Get diffuse color
                    continue;
                }
                else {
                    // Unknown command
                    return 2;
                }
            }
        }
        else if(state == 1) {
            // TODO maybe: Can a material's name have a space???
            if(!is_whitespace(c) && i < slen - 1) {
                strbuf[strix++] = c;
            } 
            else {
                if(i < slen) {
                    strbuf[strix++] = c;
                }
                strbuf[strix] = 0;
                obj->materialcount++;
                
                strcpy(obj->materials[obj->materialcount].name, strbuf);
#if DEBUGLEVEL > 1
                printf("Added a material: %s %i\n", obj->materials[obj->materialcount].name, obj->materialcount);
#endif

            }
        }
        else if(state == 2) {
            
            for(int x = 0; x < 3; x++) {
                do {
                    c = line[i];
                    strbuf[strix++] = c;
                    i++;
                } while (!is_whitespace(c) && i < slen);
                strbuf[strix] = 0;
                switch(x) {
                    case 0:
                    {
                        tmp_color.r = (uint8_t)(atof(strbuf) * 255);
                        break;
                    }
                    case 1:
                    {
                        tmp_color.g = (uint8_t)(atof(strbuf) * 255);
                        break;
                    }
                    case 2:
                    {
                        tmp_color.b = (uint8_t)(atof(strbuf) * 255);
                        break;
                    }
                }
                strix = 0;
            }
            
            obj->materials[obj->materialcount].color = tmp_color;
            break;
        }
    }

    return 0;
}

uint8_t objLoad (const char *path, Mesh *mesh) {
    FILE *f = fopen(path, "r");
    if(f == NULL) {
        // Failed to open file
        return 1;
    }

    int c = 0;
    char strbuf[256] = { 0 };
    char strln = 0;

    struct __tmp_obj temp_object;

    // Initialize temporary object
    temp_object.vertices = malloc(sizeof(Vector3) * OBJIMPORT_MAX_VERTICES);
    temp_object.vertexindex = malloc(sizeof(uint16_t) * OBJIMPORT_MAX_FACES * 3);
    temp_object.vertexcount = 0;
    temp_object.vertexindexcount = 0;

    temp_object.normals = malloc(sizeof(Vector3) * OBJIMPORT_MAX_VERTICES);
    temp_object.normalindex = malloc(sizeof(uint16_t) * OBJIMPORT_MAX_FACES * 3);
    temp_object.normalcount = 0;
    temp_object.normalindexcount = 0;

    temp_object.uvvs = malloc(sizeof(Vector2) * OBJIMPORT_MAX_VERTICES);
    temp_object.uvindex = malloc(sizeof(uint16_t) * OBJIMPORT_MAX_FACES * 3);
    temp_object.uvvcount = 0;
    temp_object.uvindexcount = 0;

    temp_object.materialoffsetcount = 0;
    temp_object.materialcount = -1;
    memset(temp_object.materials, 0, sizeof(Material) * 32);
    temp_object.mtlfile[0] = 0; // terminate string at 0

    while((c = fgetc(f)) != EOF) {
        if(c != '\n') {
            strbuf[strln++] = c;
        }
        else {
            strbuf[strln] = 0;
            obj_parse_line(strbuf, &temp_object); 
            strln = 0;
        }
    }
#if DEBUGLEVEL > 0
    printf("Found %i vertices and %i faces, %i normals and %i normal indices, %i uvs and %i uv indices\n", 
        temp_object.vertexcount, 
        temp_object.vertexindexcount/3, 
        temp_object.normalcount, 
        temp_object.normalindexcount,
        temp_object.uvvcount,
        temp_object.uvindexcount
    );
#endif
#if DEBUGLEVEL > 1

    for(int i = 0; i < temp_object.vertexcount; i++) {
        printf("\tV: %f %f %f\n", temp_object.vertices[i].x, temp_object.vertices[i].y, temp_object.vertices[i].z);
    } 

    for(int i = 0; i < temp_object.vertexindexcount; i += 3) {
        printf("\tF: %i %i %i\n", temp_object.vertexindex[i], temp_object.vertexindex[i + 1], temp_object.vertexindex[i + 2]);
    }

    for(int i = 0; i < temp_object.normalcount; i++) {
        printf("\tN: %f %f %f\n", temp_object.normals[i].x, temp_object.normals[i].y, temp_object.normals[i].z);
    } 

    for(int i = 0; i < temp_object.normalindexcount; i++) {
        printf("\tNi: %i\n", temp_object.normalindex[i]);
    } 
    
    for(int i = 0; i < temp_object.uvvcount; i++) {
        printf("\tU: %f %f\n", temp_object.uvvs[i].x, temp_object.uvvs[i].x);
    } 

    for(int i = 0; i < temp_object.uvindexcount; i++) {
        printf("\tUi: %i\n", temp_object.uvindex[i]);
    } 

    for(int i = 0; i < temp_object.materialoffsetcount; i++) {
        printf("\tMo: %i\n", temp_object.materialoffsets[i]);
    }
    
#endif
    

    fclose(f);

    if(temp_object.mtlfile[0] != 0) {
        // File found, parse path from .obj path...
        char mtlpath[64];
        strcpy(mtlpath, path);
        int i = 0;
        for(i = strlen(mtlpath) - 1; i > 0; i--) {
            if(mtlpath[i] == '/') {
                break;
            }
        }
        strcpy(mtlpath + i + 1, temp_object.mtlfile);

        f = fopen(mtlpath, "r");
        if(f) {

            while((c = fgetc(f)) != EOF) {
                if(c != '\n') {
                    strbuf[strln++] = c;
                }
                else {
                    strbuf[strln] = 0;
                    mtl_parse_line(strbuf, &temp_object); 
                    strln = 0;
                }
            }

            fclose(f);
        }
    }


    if(mesh != NULL) {
        mesh->verts = malloc(sizeof(Vector3) * temp_object.vertexcount);
        memcpy(mesh->verts, temp_object.vertices, sizeof(Vector3) * temp_object.vertexcount);
        mesh->vertcount = temp_object.vertexcount;
        
        mesh->vertindex = malloc(sizeof(uint16_t) * temp_object.vertexindexcount);
        memcpy(mesh->vertindex, temp_object.vertexindex, sizeof(uint16_t) * temp_object.vertexindexcount);
        mesh->facecount = temp_object.vertexindexcount/3;

        mesh->normals = malloc(sizeof(Vector3) * temp_object.normalcount);
        memcpy(mesh->normals, temp_object.normals, sizeof(Vector3) * temp_object.normalcount);

        mesh->normalindex = malloc(sizeof(uint16_t) * temp_object.normalindexcount);
        memcpy(mesh->normalindex, temp_object.normalindex, sizeof(uint16_t) * temp_object.normalindexcount);

        memcpy(mesh->materialindex, temp_object.materialoffsets, sizeof(uint16_t) * temp_object.materialoffsetcount);
        mesh->materialindex[temp_object.materialoffsetcount] = 0xFFFF;

        memcpy(mesh->materials, temp_object.materials, sizeof(Material) * (temp_object.materialcount + 1));

        for(int i = 0; i < 256; i++) {
            if(loaded_meshes[i] == NULL) {
                loaded_meshes[i] = mesh;
                break;
            }

        }
    }

    // Free temporary object
    free(temp_object.vertices);
    free(temp_object.vertexindex);

    free(temp_object.normals);
    free(temp_object.normalindex);

    free(temp_object.uvvs);
    free(temp_object.uvindex);

    

    return 0;
}

uint8_t objUnload (Mesh *mesh) {
    if(mesh == NULL)
        return 1;

    if(mesh->normalindex != NULL) {
        free(mesh->normalindex);
    } 
    if(mesh->normals != NULL) {
        free(mesh->normals);
    }
    if(mesh->vertindex != NULL) {
        free(mesh->vertindex);
    }
    if(mesh->verts != NULL) {
        free(mesh->verts);
    }

    return 0;
}
#include "vtg3d.h"
#include <math.h>
#include "vtgutil.h"
#include <stdlib.h>
#include <stdio.h>

uint16_t VTG_SCREEN_WIDTH = 1280;
uint16_t VTG_SCREEN_HEIGHT = 720;

uint32_t __vtg_pixel_array_size = 1280 * 720 * 4;

int __vtg_entity_array_size = 64;

int _vtg_entity_count = 0;


Uint32 ltick = 0;

// Main window
struct SDL_Window *_vtg_window = NULL;

// Main renderer
struct SDL_Renderer *_vtg_renderer = NULL;

// Main texture
struct SDL_Texture *_vtg_texture = NULL;

// Pixel array - must be initialized via vtg_initWindow(const char *title)
uint8_t *_vtg_pixels;

// Z buffer
uint16_t *_vtg_zbuffer;

// Default background color
Color _vtg_background_color;

// Entity array
Entity **_vtg_entities;


Color __vtg_draw_color = {
    255, 255, 255, 255
};

uint8_t vtg_initWindow (const char *title) {

    // TODO: check for errors!!
    SDL_Init(0);
    // Destroy renderer, texture and window if necessary
    if(_vtg_renderer != NULL) {
        SDL_DestroyRenderer(_vtg_renderer);

    }
    if(_vtg_window != NULL) {
        SDL_DestroyWindow(_vtg_window);
    }
    if(_vtg_texture != NULL) {
        SDL_DestroyTexture(_vtg_texture);
    }

    if(_vtg_entities != NULL) {
        free(_vtg_entities);
    }
    _vtg_entities = malloc(sizeof(Entity) * __vtg_entity_array_size);
    memset(_vtg_entities, 0, sizeof(Entity) * __vtg_entity_array_size);

    // Create windows
    _vtg_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, VTG_SCREEN_WIDTH, VTG_SCREEN_HEIGHT, 0);
    _vtg_renderer = SDL_CreateRenderer(_vtg_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    _vtg_texture = SDL_CreateTexture(_vtg_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VTG_SCREEN_WIDTH, VTG_SCREEN_HEIGHT);

    // Initialize pixel array
    if(_vtg_pixels != NULL) {
        free(_vtg_pixels);
    }
    if(_vtg_zbuffer != NULL) {
        free(_vtg_zbuffer);
    }
    
    __vtg_pixel_array_size = VTG_SCREEN_WIDTH * VTG_SCREEN_HEIGHT * 4;
    _vtg_pixels = malloc(__vtg_pixel_array_size);
    _vtg_zbuffer = malloc(VTG_SCREEN_WIDTH * VTG_SCREEN_HEIGHT * sizeof(uint16_t));

    _vtg_background_color.a = 0xFF;
    _vtg_background_color.r = 0;
    _vtg_background_color.g = 0;
    _vtg_background_color.b = 0;


    SDL_memset4(_vtg_pixels, *(Uint32*)&_vtg_background_color, __vtg_pixel_array_size/4);


    return 0;

}

// function protos for loop
void __vtg_drawEntityMesh (uint8_t mode, Entity *entity);
void __vtg_draw_line (int x0, int y0, int x1, int y1, float depth);

void __vtg_set_pixel_zb (uint16_t x, uint16_t y, float depth);


uint8_t vtg_startLoop (void (*callback)(SDL_Event *ev, int eventcount)) {
    SDL_Event ev[256];
    int ecount = 0;
    uint8_t exit = 0;
    
    int fcount = 0;
    float fps = 0;
    while(1) {
        ecount = 0;
        // Poll all events and forward to the update callback
        while(SDL_PollEvent(&ev[ecount++])) {
            if(ev[ecount -1].type == SDL_QUIT) {
                exit = 1;
                break;
            }
        }
        callback(ev, ecount);

        if(exit) {
            break;
        }
        // Clear window
        SDL_memset4(_vtg_pixels, *(Uint32*)&_vtg_background_color, __vtg_pixel_array_size/4);

        memset(_vtg_zbuffer, 0xFF, VTG_SCREEN_WIDTH * VTG_SCREEN_HEIGHT * sizeof(uint16_t));

        for(int i = 0; i < __vtg_entity_array_size; i++) {
            Entity *et = _vtg_entities[i];
            if(et != NULL) {
                // TODO: BETTER way to set rotation between -PI and PI
                et->rotation.x = fmod(et->rotation.x, PI * 2);
                et->rotation.y = fmod(et->rotation.y, PI * 2);
                et->rotation.z = fmod(et->rotation.z, PI * 2); 

                // Draw mesh
                __vtg_drawEntityMesh(0b01, et);
            }
                
        }

        // shitty fps counter
        uint32_t ticks = SDL_GetTicks() - ltick;
        
        if(fcount > 50) {

            fps = ((float)1 / (float)((float)ticks/1000));
            

            fcount = 0;
        }

        char cbuf[32];
        SDL_itoa(fps, cbuf, 10);

        __vtg_draw_color = (Color) {
            .r = 255, .g = 255, .b = 255
        };

        for(int g = 0; g < strlen(cbuf); g++) {
            int off = (cbuf[g] - '0') * 8;

            for(int y = 0; y < 8; y++) {
                for(int x = 0; x < 8; x++) {
                    if((fonts[off + y] >> (8 - x)) & 1) {
                        __vtg_set_pixel_zb(10 + x + g * 10, y + 10, 0);
                    }
                }
            }
        }
        fcount++;

        // Draw window
        SDL_UpdateTexture(_vtg_texture, NULL, _vtg_pixels, VTG_SCREEN_WIDTH * 4);

        SDL_RenderCopy(_vtg_renderer, _vtg_texture, NULL, NULL);

        SDL_RenderPresent(_vtg_renderer);

        ltick = SDL_GetTicks();
    }

    // CLEANUP

    for(int a = 0; a < __vtg_entity_array_size; a++) {
        vtg_destroyEntity(_vtg_entities[a]);
    }

    SDL_DestroyTexture(_vtg_texture);
    SDL_DestroyRenderer(_vtg_renderer);
    SDL_DestroyWindow(_vtg_window);


    SDL_Quit();
    return 0;
}


// RENDERING AND MISC FUNCTIONS (PRIVATE)

// Sets the color used for rendering
void __vtg_set_render_color (uint8_t r, uint8_t g, uint8_t b) {
    __vtg_draw_color.r = r;
    __vtg_draw_color.g = g;
    __vtg_draw_color.b = b;
}

// Sets a pixel and z buffer
void __vtg_set_pixel_zb (uint16_t x, uint16_t y, float depth) {
    if(x >= VTG_SCREEN_WIDTH || y >= VTG_SCREEN_HEIGHT)
        return;

    int SW_BYTES = VTG_SCREEN_WIDTH * 4;
    int x4 = x * 4;
    if(*(uint16_t*)&_vtg_zbuffer[y * VTG_SCREEN_WIDTH + x] >= (int)(depth/100 * 0xFFFF)) {
        *(uint16_t*)&_vtg_zbuffer[y * VTG_SCREEN_WIDTH + x] = (int)(depth/100 * 0xFFFF);
        _vtg_pixels[y * SW_BYTES + x4] = __vtg_draw_color.b;
        _vtg_pixels[y * SW_BYTES + x4 + 1] = __vtg_draw_color.g;
        _vtg_pixels[y * SW_BYTES + x4 + 2] = __vtg_draw_color.r;
        _vtg_pixels[y * SW_BYTES + x4 + 3] = 255;
    }
    
}

void __vtg_clear_pixel (uint16_t x, uint16_t y) {
    if(x >= VTG_SCREEN_WIDTH || y >= VTG_SCREEN_HEIGHT)
        return;

    int SW_BYTES = VTG_SCREEN_WIDTH * 4;
    int x4 = x * 4;
    _vtg_pixels[y * SW_BYTES + x4 + 3] = 0;
}

// Transforms world coordinates to screen coordinates -- TODO: add interpolation for z-buffer
int __vtg_worldToScreen (Vector3 *position, int *screenx, int *screeny) {
    int xp = 0;
    int yp = 0;

    const int swh = VTG_SCREEN_WIDTH/2;
    const int shh = VTG_SCREEN_HEIGHT/2;

    xp = position->x / position->z * shh + swh;
    yp = position->y / position->z * shh + shh;

    *screenx = xp;
    *screeny = yp;

    if(xp < 0 || xp >= VTG_SCREEN_WIDTH || yp < 0 || yp >= VTG_SCREEN_HEIGHT)
        return 1;

    return 0;
}

// Fills a bottom flat triangle
void __vtg_fill_tri_bottom (uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float depth) {


    float invslope1 = (float)(x1 - x0) / (float)(y1 - y0);
    float invslope2 = (float)(x2 - x0) / (float)(y2 - y0);

    float curx1 = x0;
    float curx2 = x0;

    for (int scanlineY = y0; scanlineY <= y1; scanlineY++) {
        __vtg_draw_line ((int)curx1, scanlineY, (int)curx2, scanlineY, depth);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

// Fills a top flat triangle
void __vtg_fill_tri_top (uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float depth) {

    float invslope1 = (float)(x2 - x0) / (float)(y2 - y0);
    float invslope2 = (float)(x2 - x1) / (float)(y2 - y1);

    float curx1 = x2;
    float curx2 = x2;

    for (int scanlineY = y2; scanlineY > y0; scanlineY--)
    {
        __vtg_draw_line ((int)curx1, scanlineY, (int)curx2, scanlineY, depth);
        
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

// Fills a triangle
void __vtg_fill_triangle (uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, float depth) {
    int rx0 = x0, ry0 = y0, rx1 = x1, ry1 = y1, rx2 = x2, ry2 = y2;

    if(ry0 > ry1) {
        // swap 0 and 1
        int t = ry0;
        ry0 = ry1; ry1 = t;
        t = rx0;
        rx0 = rx1; rx1 = t;
    }
    if(ry0 > ry2) {
        // swap 0 and 2
        int t = ry0;
        ry0 = ry2; ry2 = t;
        t = rx0;
        rx0 = rx2; rx2 = t;
    }
    if(ry1 > ry2) {
        // swap 1 and 2
        int t = ry1;
        ry1 = ry2; ry2 = t;
        t = rx1;
        rx1 = rx2; rx2 = t;
    }

    if (ry1 == ry2) {
        __vtg_fill_tri_bottom (rx0, ry0, rx1, ry1, rx2, ry2, depth);
    }
    else if (ry0 == ry1) {
        __vtg_fill_tri_top (rx0, ry0, rx1, ry1, rx2, ry2, depth);
    }
    else {
        int rx3 = (int)(rx0 + ((float)(ry1 - ry0) / (float)(ry2 - ry0)) * (rx2 - rx0));
        int ry3 = ry1;
        __vtg_fill_tri_bottom (rx0, ry0, rx1, ry1, rx3, ry3, depth);

        __vtg_fill_tri_top (rx1, ry1, rx3, ry3, rx2, ry2, depth);
    }
}

// Draws a line, only used by triangle filling!!
void __vtg_draw_line (int x0, int y0, int x1, int y1, float depth) {
    if(y0 == y1) {
        // HZ line
        int d = x1 - x0;
        if(d < 0) {
            for(int i = 0; i > d - 1; i--) {
                __vtg_set_pixel_zb(x0 + i, y0, depth);
                __vtg_set_pixel_zb(x0 + i, y0 + 1, depth);
            }
        }
        else {
            for(int i = 0; i < d + 1; i++) {
                __vtg_set_pixel_zb(x0 + i, y0, depth);
                __vtg_set_pixel_zb(x0 + i, y0 + 1, depth);
            }
        }
            
    }
    else if(x0 == x1) {
        // VT line
        int d = y1 - y0;
        if(d < 0) {
            for(int i = 0; i > d; i--) {
                __vtg_set_pixel_zb(x0, y0 + i, depth);
            }
        }
        else {
            for(int i = 0; i < d; i++) {
                __vtg_set_pixel_zb(x0, y0 + i, depth);
            }
        }
    }
    else {
        int dx = x1 - x0;
        int dy = y1 - y0;

        int adx = abs(dx), ady = abs(dy);
        if(adx == 0 && ady == 0)
            return;
        
        int steps = adx > ady ? adx : ady;

        float xInc = (float)dx / (float)steps;
        float yInc = (float)dy / (float)steps;

        float x = x0;
        float y = y0;
        for(int i= 0; i < steps; i++) {
            x += xInc;
            y += yInc;
            __vtg_set_pixel_zb(x, y, depth);
        }

    }
}

// Rotating vectors
void __vtg_rotatePoints (Vector3 *points, int count, Vector3 rotation) {
    float cosa = cos(rotation.z);
    float sina = sin(rotation.z);

    float cosb = cos(rotation.y);
    float sinb = sin(rotation.y);

    float cosc = cos(rotation.x);
    float sinc = sin(rotation.x);

    float Axx = cosa*cosb;
    float Axy = cosa*sinb*sinc - sina*cosc;
    float Axz = cosa*sinb*cosc + sina*sinc;

    float Ayx = sina*cosb;
    float Ayy = sina*sinb*sinc + cosa*cosc;
    float Ayz = sina*sinb*cosc - cosa*sinc;

    float Azx = -sinb;
    float Azy = cosb*sinc;
    float Azz = cosb*cosc;

    for (int i = 0; i < count; i++) {
        float px = points[i].x;
        float py = points[i].y;
        float pz = points[i].z;

        points[i].x = Axx*px + Axy*py + Axz*pz;
        points[i].y = Ayx*px + Ayy*py + Ayz*pz;
        points[i].z = Azx*px + Azy*py + Azz*pz;
    }
}

// Translate points by translation
void __vtg_translatePoints (Vector3 *points, int count, Vector3 translation) {


    for (int i = 0; i < count; i++) {
        float px = points[i].x;
        float py = points[i].y;
        float pz = points[i].z;

        points[i].x += translation.x;
        points[i].y += translation.y;
        points[i].z += translation.z;
    }
}


// Draws a mesh
void __vtg_drawEntityMesh (uint8_t mode, Entity *entity) {
    uint8_t shade = mode & 1;
    uint8_t wireframe = (mode >> 1) & 1;

    if(entity->mesh == NULL)
        return;

    Vector3 *points = malloc(sizeof(Vector3) * entity->mesh->vertcount);
    memcpy(points, entity->mesh->verts, entity->mesh->vertcount * sizeof(Vector3));

    __vtg_rotatePoints(points, entity->mesh->vertcount, entity->rotation);

    __vtg_translatePoints(points, entity->mesh->vertcount, entity->position);

    int material_index = -1;

    for(int i = 0; i < entity->mesh->facecount; i++) {
        Vector3 v0 = points[entity->mesh->vertindex[i * 3]];
        Vector3 v1 = points[entity->mesh->vertindex[i * 3 + 1]];
        Vector3 v2 = points[entity->mesh->vertindex[i * 3 + 2]];

        Vector3 vc = (Vector3) {
            .x = (v0.x + v1.x + v2.x)/3, (v0.y + v1.y + v2.y)/3, (v0.z + v1.z + v2.z)/3
        };
        float davg = sqrt(vc.x*vc.x + vc.y*vc.y + vc.z*vc.z);

        int x0, y0, x1, y1, x2, y2;

        __vtg_worldToScreen(&v0, &x0, &y0);
        __vtg_worldToScreen(&v1, &x1, &y1);
        __vtg_worldToScreen(&v2, &x2, &y2);

        y0 = VTG_SCREEN_HEIGHT - y0;
        y1 = VTG_SCREEN_HEIGHT - y1;
        y2 = VTG_SCREEN_HEIGHT - y2;

        if(i == entity->mesh->materialindex[material_index + 1]) {
            material_index++;
        }


        if(shade) {
            Vector3 nrm = entity->mesh->normals[entity->mesh->normalindex[i]];

            // Add rotation to the normals
            __vtg_rotatePoints(&nrm, 1, entity->rotation);
            
            // Calculate angle between light (-0.7071, -0.7071, 0) and normal
            float angle = acosf(nrm.x * -0.7071 + nrm.y * -0.7071 + nrm.z * 0);

            // angle between 0 and 1
            float api = angle/PI;


            __vtg_draw_color = (Color) {
                .r = api * entity->mesh->materials[material_index].color.r,
                .g = api * entity->mesh->materials[material_index].color.g,
                .b = api * entity->mesh->materials[material_index].color.b
            };
            // TODO: set actual materials, this is only for tests
            /*
            if(material_index <= 0) {
                __vtg_draw_color = (Color) {
                    .r = api * 255, .g = api * 255, .b = api * 255
                };
            }
            else {
                __vtg_draw_color = (Color) {
                    .r = api * 255, .g = api * 100, .b = api * 100
                };
            }
            */
            
            // Fill triangle
            __vtg_fill_triangle(x0, y0, x1, y1, x2, y2, davg);
            __vtg_draw_color = (Color) {
                .r = 255, .g = 255, .b = 255
            };
            
#ifdef SHOW_NORMALS
            Vector3 mid = (Vector3) {
                .x = (v0.x + v1.x + v2.x)/3, .y = (v0.y + v1.y + v2.y)/3, .z = (v0.z + v1.z + v2.z)/3
            };
            Vector3 ep;

            ep.x = mid.x + nrm.x/10;
            ep.y = mid.y + nrm.y/10;
            ep.z = mid.z + nrm.z/10;

            

            int __nrmx0, __nrmy0, __nrmx1, __nrmy1;
            __vtg_worldToScreen(&mid, &__nrmx0, &__nrmy0);
            __vtg_worldToScreen(&ep, &__nrmx1, &__nrmy1);
            __nrmy0 = VTG_SCREEN_HEIGHT - __nrmy0;
            __nrmy1 = VTG_SCREEN_HEIGHT - __nrmy1;
            __vtg_draw_color = (Color) {
                .r = 100, .g = 100, .b = 255
            };
            __vtg_draw_line(__nrmx0, __nrmy0, __nrmx1, __nrmy1, davg - 0.05);
            __vtg_draw_color = (Color) {
                .r = 255, .g = 255, .b = 255
            };
#endif

        }


        if(wireframe) {
            __vtg_draw_color = (Color) {
                .r = 0, .g = 255, .b = 0
            };

            
            __vtg_draw_line(x0, y0, x1, y1, 0);
            __vtg_draw_line(x1, y1, x2, y2, 0);
            __vtg_draw_line(x2, y2, x0, y0, 0);

            __vtg_draw_color = (Color) {
                .r = 255, .g = 255, .b = 255
            };
        }
        
    }
    // Free points
    free(points);

}
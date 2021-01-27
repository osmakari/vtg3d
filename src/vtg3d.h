#ifndef _VTG3D_H
#define _VTG3D_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include "types.h"
#include "entity.h"

#define PI 3.14159265359

// Screen width and height
extern uint16_t VTG_SCREEN_WIDTH;
extern uint16_t VTG_SCREEN_HEIGHT;

// Main window
extern struct SDL_Window *_vtg_window;

// Main renderer
extern struct SDL_Renderer *_vtg_renderer;

// Main texture
extern struct SDL_Texture *_vtg_texture;

// Pixel array - must be initialized via vtg_initWindow(const char *title)
extern uint8_t *_vtg_pixels;

// Default background color
extern Color _vtg_background_color;

// Entity array
extern Entity **_vtg_entities;

// Entity count
extern int _vtg_entity_count;

// Entity array size
extern int __vtg_entity_array_size;


// FUNCTIONS

// vtg_initWindow
// Initializes a window
// returns 0 on success
uint8_t vtg_initWindow (const char *title);


// vtg_startLoop
// Starts the main loop
// blocking function, so callback must be set
uint8_t vtg_startLoop (void (*callback)(SDL_Event *ev, int eventcount));

#endif
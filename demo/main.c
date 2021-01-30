// Main file for demo

#include "../src/vtg3d.h"
#include "../src/utilities/objimport.h"

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

void loop (SDL_Event *ev, int eventcount);

Entity *et;
Entity *et2;

Entity *ruukku;

Mesh testmodel;
Mesh housemodel;
Mesh ruukkumodel;

int main () {
    objLoad("models/tob.obj", &testmodel);
    objLoad("models/house.obj", &housemodel);
    objLoad("models/ruukku.obj", &ruukkumodel);

    vtg_initWindow("VTG Demo");

    ruukku = vtg_createEntity();
    ruukku->position.z = 0;
    ruukku->position.y = 0;

    et = vtg_createEntity();
    et->position.z = 0;
    et->position.x = -4;

    et2 = vtg_createEntity();
    et2->position.z = 0;
    et2->position.x = 4;

    et2->mesh = &housemodel;
    et->mesh = &testmodel;

    ruukku->mesh = &ruukkumodel;

    vtg_startLoop(loop);
    return 0;
}

void loop (SDL_Event *ev, int eventcount) {
    //et->rotation.y += 0.01;
    //et->rotation.x += 0.001;

    //et2->rotation.y -= 0.01;

    //ruukku->rotation.y += 0.01;

    //vtg_cameraPosition.z += 0.01;
    /*
    for(int i = 0; i < eventcount; i++) {
        SDL_Event *evv = &ev[i];
        if(evv->type == SDL_KEYDOWN && evv->key.keysym.scancode == SDL_SCANCODE_A) {
            vtg_cameraRotation.y -= 0.01;
        }
        else if(evv->type == SDL_KEYDOWN && evv->key.keysym.scancode == SDL_SCANCODE_D) {
            vtg_cameraRotation.y += 0.01;
        }
    }
    */
    float ts = (float)SDL_GetTicks()/1000;
    vtg_cameraPosition.x = 6 * cos(ts);
    vtg_cameraPosition.z = 6 * sin(ts);

    vtg_cameraRotation.y = -ts - PI/2;
    

}
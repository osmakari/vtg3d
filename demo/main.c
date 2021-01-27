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
    ruukku->position.z = 5;
    ruukku->position.y = -3;

    et = vtg_createEntity();
    et->position.z = 3;
    et->position.x = -2;

    et2 = vtg_createEntity();
    et2->position.z = 4;
    et2->position.x = 2;

    et2->mesh = &housemodel;
    et->mesh = &testmodel;

    ruukku->mesh = &ruukkumodel;
    
    vtg_startLoop(loop);
    return 0;
}

void loop (SDL_Event *ev, int eventcount) {
    et->rotation.y += 0.01;
    //et->rotation.x += 0.001;

    et2->rotation.y -= 0.01;

    ruukku->rotation.y += 0.01;
}
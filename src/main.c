//#include "commons.h"
#include <stdio.h>

/* ================================================================
   MAIN.C — Punto de entrada y game loop
   Este archivo arranca el programa y mantiene el loop corriendo.
   No contiene lógica de juego. Su único trabajo es inicializar
   todo, llamar a los módulos en el orden correcto, y cerrar limpio.
   ================================================================ */

int main(void) {

    /* ------------------------------------------------------------
       1. INICIALIZACIÓN DE ALLEGRO
       ------------------------------------------------------------ */


    /* ------------------------------------------------------------
       2. CREAR LA VENTANA Y EL DISPLAY
       ------------------------------------------------------------ */


    /* ------------------------------------------------------------
       3. CREAR EL EVENT QUEUE Y EL TIMER
       ------------------------------------------------------------ */


    /* ------------------------------------------------------------
       4. INICIALIZAR EL ESTADO DEL JUEGO Y LOS ASSETS
       ------------------------------------------------------------ */
    /*GameState game_state;
    InputState input_state;

    game_init(&game_state);
    assets_load();*/


    /* ------------------------------------------------------------
       5. GAME LOOP
       ------------------------------------------------------------ */
    //while (game_state.running) {


        /* -- INPUT -------------------------------------------- */
        //input_update(&input_state);

        /* -- UPDATE ------------------------------------------- */
        //update(&game_state, &input_state);

        /* -- RENDER ------------------------------------------- */
        //render_gameview(&game_state);
        //render_ui(&game_state);
    //}


    /* ------------------------------------------------------------
       6. CIERRE Y LIBERACIÓN DE RECURSOS
       ------------------------------------------------------------ */
printf("Hello World");

    return 0;
}

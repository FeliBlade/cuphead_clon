//#include "commons.h"

/* ================================================================
   MAIN.C — Punto de entrada y game loop
   Este archivo arranca el programa y mantiene el loop corriendo.
   No contiene lógica de juego. Su único trabajo es inicializar
   todo, llamar a los módulos en el orden correcto, y cerrar limpio.
   ================================================================ */



    /* ------------------------------------------------------------
       1. INICIALIZACIÓN DE ALLEGRO
       ------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

void must_init(bool test, const char *description)
{
    if(test) return;

    printf("couldn't initialize %s\n", description);
    exit(1);
}

int main()
{
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

    ALLEGRO_DISPLAY* disp = al_create_display(640, 480);
    must_init(disp, "display");

    ALLEGRO_FONT* font = al_create_builtin_font();
    must_init(font, "font");

    must_init(al_init_primitives_addon(), "primitives");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    float x, y;
    x = 100;
    y = 100;

    ALLEGRO_KEYBOARD_STATE ks;

    al_start_timer(timer);

    #define KEY_SEEN     1
    #define KEY_RELEASED 2

    unsigned char key[ALLEGRO_KEY_MAX];
    memset(key, 0, sizeof(key));

    while(1)
    {
        al_wait_for_event(queue, &event);

        switch(event.type)
        {
           case ALLEGRO_EVENT_TIMER:
              if(key[ALLEGRO_KEY_UP])
                 y=y-5;
              if(key[ALLEGRO_KEY_DOWN])
                 y=y+5;
              if(key[ALLEGRO_KEY_LEFT])
                 x=x-5;
              if(key[ALLEGRO_KEY_RIGHT])
                 x=x+5;

              if(key[ALLEGRO_KEY_ESCAPE])
                 done = true;

              for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
                 key[i] &= KEY_SEEN;

              redraw = true;
              break;

           case ALLEGRO_EVENT_KEY_DOWN:
              key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
              break;

           case ALLEGRO_EVENT_KEY_UP:
              key[event.keyboard.keycode] &= KEY_RELEASED;
              break;

           case ALLEGRO_EVENT_DISPLAY_CLOSE: // Caso de que se cierre la ventana
              done = true;
              break;
        }

        if(done)
            break;

        if(redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "X: %.1f Y: %.1f", x, y);
            al_draw_filled_rectangle(x, y, x + 10, y + 10, al_map_rgb(255, 0, 0));

            al_flip_display();

            redraw = false;
        }
    }

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}

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

  //  return 0;
//}



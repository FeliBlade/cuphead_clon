#include "commons.h"

/* ================================================================
   MAIN.C — Punto de entrada y game loop
   Este archivo arranca el programa y mantiene el loop corriendo.
   No contiene lógica de juego. Su único trabajo es inicializar
   todo, llamar a los módulos en el orden correcto, y cerrar limpio.
   ================================================================ */



    /* ------------------------------------------------------------
       1. INICIALIZACIÓN DE ALLEGRO
       ------------------------------------------------------------ */

#include <allegro5/color.h>
#include <allegro5/timer.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h> // Imprimir matriz de mapa por archivo - Semana: Cargar mapa por matriz en otro archivo, interaccion con otros elementos

#define LARGO 40 // Largo de un bloque
#define ANCHO 40 // Ancho de un bloque

#define LARGO_MAPA 32
#define ANCHO_MAPA 18

#define SPEED_FACTOR 1
#define TERMINAL_VELOCITY 40 // 40 px/seg.

void must_init(bool test, const char *description)
{
    if(test) return;

    printf("couldn't initialize %s\n", description);
    exit(1);
}

bool collide(ALLEGRO_FONT *font, float posX, float posY, float *sueloX, float *sueloY)
{
   int chequeoColision=0;

   if(posX+LARGO>*sueloX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 20, 0, "DEBUG: condicion 1: true");
      chequeoColision++;
   }
   if(*sueloX+LARGO>posX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 30, 0, "DEBUG: condicion 2: true");
      chequeoColision++;
   }
   if(posY+ANCHO>*sueloY)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 40, 0, "DEBUG: condicion 3: true");
      chequeoColision++;
   }
   if(*sueloY+ANCHO>posY)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 50, 0, "DEBUG: condicion 4: true");
      chequeoColision++;
   }

   if(chequeoColision==4)
   {
      return true;
   }

   return false;
}

bool collideSuelo(ALLEGRO_FONT *font, float posX, float posY, const float *sueloX, const float *sueloY)
{
   int chequeoColision=0;

   if(posX+LARGO>*sueloX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 20, 0, "DEBUG: condicion 1: true");
      chequeoColision++;
   }
   if(*sueloX+LARGO>posX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 30, 0, "DEBUG: condicion 2: true");
      chequeoColision++;
   }
   if(posY+ANCHO+1>*sueloY) // **** Se puede reutilizar en la funcion "collide"
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 40, 0, "DEBUG: condicion piso: true");
      chequeoColision++;
   }
   if(*sueloY+ANCHO>posY)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 50, 0, "DEBUG: condicion 4: true");
      chequeoColision++;
   }   

   if(chequeoColision==4)
   {
      return true;
   }

   return false;
}

float anularMovimientoX(ALLEGRO_FONT* font, bool direccionIzquierda, bool direccionDerecha, float posX) // Deshace el movimiento en el eje X al chocar
{
   float ajusteX;
   ajusteX = posX;
   if(direccionIzquierda == true)
   {
      ajusteX=ajusteX+SPEED_FACTOR;
   }
   if(direccionDerecha == true)
   {
      ajusteX=ajusteX-SPEED_FACTOR;
   }

   return ajusteX;
}

float anularMovimientoY(ALLEGRO_FONT* font, bool direccionArriba, bool direccionAbajo, float posY) // Deshace el movimiento en el eje Y al chocar
{
   float ajusteY;
   ajusteY = posY;

   if(direccionArriba == true)
   {
      ajusteY=ajusteY+SPEED_FACTOR;
   }
   if(direccionAbajo == true)
   {
      ajusteY=ajusteY-SPEED_FACTOR;
   }

   return ajusteY;
}

int main()
{
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / TARGET_FPS);
    must_init(timer, "timer");

    ALLEGRO_TIMER* tempGravedad = al_create_timer(1.0 / TARGET_FPS); // Temporizador de gravedad.
    must_init(tempGravedad, "tempGravedad");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST); //
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST); // **** ANTIALIASING ****
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR); //

    ALLEGRO_DISPLAY* disp = al_create_display(LARGO_PANTALLA, ANCHO_PANTALLA);
    must_init(disp, "display");

    ALLEGRO_FONT* font = al_create_builtin_font();
    must_init(font, "font");

    must_init(al_init_primitives_addon(), "primitives");

    /*if(!al_init_image_addon())
    {
       printf("couldn't initialize image addon\n");
       return 1;
    }*/

    /*ALLEGRO_BITMAP* mysha = al_load_bitmap("mysha.png");
    must_init(mysha, "mysha");*/

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    float posX, posY; // Coordenadas del cuadrado rojo
    posX = 80; // Punto de inicio
    posY = 80;

    // Coordenadas de rectangulos suelo:

    float x1 = 80; // Esquina superior izquierda del cuadrado.
    float y1 = 360; // Las otras esquinas se obtienen sumandole el largo y/o el ancho. y1=340

    float x2 = 240;
    float y2 = 240;

    float x3 = 440;
    float y3 = 120;

    bool direccionArriba = true; // Variables que registran la direccion de colision con paredes.
    bool direccionAbajo = true;
    bool direccionIzquierda = true;
    bool direccionDerecha = true;

    int valorTimerGravedad;

    int i,j; // Contadores generales reutilizables

    FILE *contenidoMapa; // Variables de obtencion de datos de "mapa1.txt"
    int valorObtenido;

    int mapa[ANCHO_MAPA][LARGO_MAPA]={0};

    for(i = 0; i < ANCHO_MAPA; i++)
    {
       for(j = 0; j < LARGO_MAPA; j++)
       {
          printf("%d  ", mapa[i][j]);
       }
       printf("\n");
    }
    printf("\n");
    i = 0;
    j = 0;

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
              {
                 //al_set_timer_count(tempGravedad, -15);
                 //al_start_timer(tempGravedad);
                 posY=posY-25;
                 direccionArriba = true;
              }
              if(key[ALLEGRO_KEY_DOWN])
              {
                 //posY=posY+SPEED_FACTOR;
                 direccionAbajo = true;
              }
              if(key[ALLEGRO_KEY_LEFT])
              {
                 posX=posX-SPEED_FACTOR;
                 direccionIzquierda = true;
              }
              if(key[ALLEGRO_KEY_RIGHT])
              {
                 posX=posX+SPEED_FACTOR;
                 direccionDerecha = true;
              }

              if(collide(font, posX, posY, &x1, &y1)==true) // Deshace el movimento antes de que cargue el frame moviendo al personaje en la direccion opuesta.
              {
                 posX = anularMovimientoX(font, direccionIzquierda, direccionDerecha, posX);
                 posY = anularMovimientoY(font, direccionArriba, direccionAbajo, posY);
              }

              if(collide(font, posX, posY, &x2, &y2)==true)
              {
                 posX = anularMovimientoX(font, direccionIzquierda, direccionDerecha, posX);
                 posY = anularMovimientoY(font, direccionArriba, direccionAbajo, posY);
              }

              if(collide(font, posX, posY, &x3, &y3)==true)
              {
                 posX = anularMovimientoX(font, direccionIzquierda, direccionDerecha, posX);
                 posY = anularMovimientoY(font, direccionArriba, direccionAbajo, posY);
              }

              if(key[ALLEGRO_KEY_ESCAPE])
                 done = true;

              for(i = 0; i < ALLEGRO_KEY_MAX; i++)
              {
                 key[i] &= KEY_SEEN;
              }
              i = 0;

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

            // 1: Realizar los ajustes necesarios en el cuadrado personaje.

               valorTimerGravedad = al_get_timer_count(tempGravedad);

            if(collideSuelo(font, posX, posY, &x1, &y1)==0)
            {
               al_start_timer(tempGravedad); // (1. revisar: inicia el timer cada que esta en el aire?) (2. mejor editar directamente el valor del timer con al_set_timer_count o al_add_timer_count)
               if(valorTimerGravedad % 2 == 0)
               {
                  valorTimerGravedad++;
               }
               if(valorTimerGravedad>TERMINAL_VELOCITY)
               {
                  valorTimerGravedad--;
               }
               posY = posY + valorTimerGravedad/5.0;
               if(collide(font, posX, posY, &x1, &y1)==1)
               {
                  posY = y1 - ANCHO;
               }
            }
            else
            {
               al_stop_timer(tempGravedad);
               al_set_timer_count(tempGravedad, 0);
            }

            if(collideSuelo(font, posX, posY, &x1, &y1)==0)
            {
               al_start_timer(tempGravedad); // (1. revisar: inicia el timer cada que esta en el aire?) (2. mejor editar directamente el valor del timer con al_set_timer_count o al_add_timer_count)
               if(valorTimerGravedad % 2 == 0)
               {
                  valorTimerGravedad++;
               }
               if(valorTimerGravedad>TERMINAL_VELOCITY)
               {
                  valorTimerGravedad--;
               }
               posY = posY + valorTimerGravedad/5.0;
               if(collide(font, posX, posY, &x1, &y1)==1)
               {
                  posY = y1 - ANCHO;
               }
            }

            else
            {
               al_stop_timer(tempGravedad);
            }
            if(collideSuelo(font, posX, posY, &x1, &y1)==0)
            {
               al_start_timer(tempGravedad); // (1. revisar: inicia el timer cada que esta en el aire?) (2. mejor editar directamente el valor del timer con al_set_timer_count o al_add_timer_count)
               if(valorTimerGravedad % 2 == 0)
               {
                  valorTimerGravedad++;
               }
               if(valorTimerGravedad>TERMINAL_VELOCITY)
               {
                  valorTimerGravedad--;
               }
               posY = posY + valorTimerGravedad/5.0;
               if(collide(font, posX, posY, &x1, &y1)==1)
               {
                  posY = y1 - ANCHO;
               }
            }
            else
            {
               al_stop_timer(tempGravedad);
            }

            if(posY > ANCHO_PANTALLA + ANCHO) // Si el jugador se "cae" (Se sale de la pantalla por abajo)
            {
               posX = 80;
               posY = 80;
            }

            // 2: Dibujar el siguiente frame.

            for(i = 0; i < ANCHO_MAPA; i++)
            {
               for(j = 0; j < LARGO_MAPA; j++)
               {
                  if(mapa[i][j] != 0)
                  {
                     al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  }
                  else
                  {
                     al_draw_textf(font, al_map_rgb(100, 100, 100), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  }
               }
            }

            al_draw_filled_rectangle(posX, posY, posX + LARGO, posY + ANCHO, al_map_rgb(255, 0, 0)); // Rectangulo de personaje.
            //al_draw_bitmap(mysha, 100, 100, 0);
            /*al_draw_rectangle(100, 100, 400, 120, al_map_rgba_f(0, 0, 0.5, 0.5), 1); // Inferior
            al_draw_rectangle(100, 100, 120, 300, al_map_rgba_f(0, 0, 0.5, 0.5), 1); // Izquierdo
            al_draw_rectangle(100, 280, 400, 300, al_map_rgba_f(0, 0, 0.5, 0.5), 1); // Superior
            al_draw_rectangle(380, 100, 400, 300, al_map_rgba_f(0, 0, 0.5, 0.5), 1); // Derecho*/
            al_draw_rectangle(x1, y1, x1 + LARGO, y1 + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.5), 1); // Cuadrados suelo.
            al_draw_rectangle(x2, y2, x2 + LARGO, y2 + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.5), 1);
            al_draw_rectangle(x3, y3, x3 + LARGO, y3 + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.5), 1);
            /*al_draw_filled_rectangle(x1, y1, x1 + LARGO, y1 + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.5)); // Relleno de cuadrados suelo.
            al_draw_filled_rectangle(x2, y2, x2 + LARGO, y2 + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.5));
            al_draw_filled_rectangle(x3, y3, x3 + LARGO, y3 + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.5));*/

            // 3. Dibujar informacion de debug.

            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "X: %.1f Y: %.1f", posX, posY);          
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 10, 0, "DEBUG: collide = %d", collide(font, posX, posY, &x1, &y1));
            al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 10, 0, "DEBUG: collideSuelo = %d", collideSuelo(font, posX, posY, &x1, &y1)); 
            al_draw_textf(font, al_map_rgb(255, 255, 255), 250, 0, 0, "Temporizador de gravedad: %d", valorTimerGravedad);

            if(direccionArriba == true) // Imprime las direcciones ingresadas.
            {
               al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 70, 0, "^");
            }
            if(direccionAbajo == true)
            {
               al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 90, 0, "v");
            }
            if(direccionIzquierda == true)
            {
               al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 80, 0, "<");
            }
            if(direccionDerecha == true)
            {
               al_draw_textf(font, al_map_rgb(255, 255, 255), 20, 80, 0, ">");
            }

            direccionArriba = false; // Reinicia la direccion (esta se obtiene cada frame).
            direccionAbajo = false;
            direccionIzquierda = false;
            direccionDerecha = false;            

            al_flip_display();

            redraw = false;
        }
    } 

    //al_destroy_bitmap(mysha);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_timer(tempGravedad);
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

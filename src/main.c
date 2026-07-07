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
#include <allegro5/allegro_primitives.h> // Sigue: Interaccion con otros elementos (semiplataformas, enemigos, balas, corazones extra), scrolling, pasar personajes y enemigos a una estructura, implementar joystick

#define SPAWNPOINT_X 240 // Punto de inicio
#define SPAWNPOINT_Y 160

#define LARGO 40 // Largo de un bloque
#define ANCHO 40 // Ancho de un bloque

#define LARGO_MAPA 32
#define ANCHO_MAPA 18

#define SPEED_FACTOR 7
#define TERMINAL_VELOCITY 40 // 40 px/seg.

typedef struct
{
   float posX;
   float posY;
   int vida;
}
entidad;

void must_init(bool test, const char *description)
{
   if(test) return;

   printf("couldn't initialize %s\n", description);
   exit(1);
}

bool collide(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY)
{
   int chequeoColision=0;

   if(entidad.posX+LARGO>*sueloX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 20, 0, "DEBUG: condicion 1: true");
      chequeoColision++;
   }
   if(*sueloX+LARGO>entidad.posX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 30, 0, "DEBUG: condicion 2: true");
      chequeoColision++;
   }
   if(entidad.posY+ANCHO>*sueloY)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 40, 0, "DEBUG: condicion 3: true");
      chequeoColision++;
   }
   if(*sueloY+ANCHO>entidad.posY)
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

bool collideSuelo(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY)
{
   int chequeoColision = 0;

   if(entidad.posX+LARGO-2>*sueloX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 20, 0, "DEBUG: condicion 1: true");
      chequeoColision++;
   }
   if(*sueloX+LARGO-2>entidad.posX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 30, 0, "DEBUG: condicion 2: true");
      chequeoColision++;
   }
   if(entidad.posY + ANCHO >= *sueloY) // **** Se puede reutilizar en la funcion "collide" (posY+ANCHO+1>*sueloY)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 40, 0, "DEBUG: condicion piso: true");
      chequeoColision++;
   }
   if(*sueloY+ANCHO>entidad.posY)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 50, 0, "DEBUG: condicion 4: true");
      chequeoColision++;
   }   

   if(chequeoColision == 4)
   {
      return true;
   }

   return false;
}

float anularMovimientoX(ALLEGRO_FONT* font, bool direccionIzquierda, bool direccionDerecha, entidad entidad, float *sueloX) // Deshace el movimiento en el eje X al chocar
{
   float ajusteX;
   ajusteX = entidad.posX;

   if(direccionIzquierda == true)
   {
      ajusteX = *sueloX + LARGO;
   }
   if(direccionDerecha == true)
   {
      ajusteX = *sueloX - LARGO;
   }

   return ajusteX;
}

float anularMovimientoY(ALLEGRO_FONT* font, bool direccionArriba, bool direccionAbajo, entidad entidad, float *sueloY) // Deshace el movimiento en el eje Y al chocar
{
   float ajusteY;
   ajusteY = entidad.posY;

   if(entidad.posY+ANCHO>*sueloY)
   {
      ajusteY = *sueloY + ANCHO;
   }
   if(*sueloY+ANCHO>entidad.posY)
   {
      ajusteY = *sueloY - ANCHO;
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

   entidad jugador;
   jugador.posX = SPAWNPOINT_X; // Punto de inicio
   jugador.posY = SPAWNPOINT_Y;
   jugador.vida = 9;
   int iFrames = 0;

   bool direccionArriba = false; // Variables que registran la direccion de colision con paredes.
   bool direccionAbajo = false;
   bool direccionIzquierda = false;
   bool direccionDerecha = false;

   int valorTimerGravedad = 0; // Obtiene el valor del temporizador de gravedad.
   bool jugadorEnAire = true; // Revisa si esta cayendo el cuadrado personaje.
   bool teclaPresionada = 0;
   int i,j; // Contadores generales reutilizables.
   float puntoX, puntoY; // Reciben los valores de i y j para traspasarlos a variables flotantes que puedan ser traspasadas a las funciones de colision.

   FILE *contenidoMapa1; // Variables de obtencion de datos de "mapa1.txt".
   int valorRecibido;

   bool flag = 0;

   contenidoMapa1 = fopen("mapa1.txt", "r");
   must_init(contenidoMapa1, "mapa1");

   int mapa[ANCHO_MAPA][LARGO_MAPA];

   for(i = 0; i < ANCHO_MAPA; i++)
   {
      for(j = 0; j < LARGO_MAPA; j++)
      {
         if(fscanf(contenidoMapa1, "%d", &valorRecibido) != EOF)
         {
            mapa[i][j] = valorRecibido;
         }
         else
         {
            break;
         }
      }
      printf("\n");
   }
   printf("\n");

   for(i = 0; i < ANCHO_MAPA; i++)
   {
      for(j = 0; j < LARGO_MAPA; j++)
      {
         printf("%d  ", mapa[i][j]);
      }
      printf("\n");
   }

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
               jugador.posY = jugador.posY - 20;
               direccionArriba = true;
            }
            if(key[ALLEGRO_KEY_DOWN])
            {
               //posY=posY+SPEED_FACTOR;
               direccionAbajo = true;
            }
            if(key[ALLEGRO_KEY_LEFT])
            {
               jugador.posX = jugador.posX-SPEED_FACTOR;
               direccionIzquierda = true;
            }
            if(key[ALLEGRO_KEY_RIGHT])
            {
               jugador.posX = jugador.posX+SPEED_FACTOR;
               direccionDerecha = true;
            }

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

         // 1: Realizar los ajustes necesarios en el cuadrado personaje.

         if(jugadorEnAire == true)
         {
         for(i = 0; i < ANCHO_MAPA; i++) // Define el valor de jugadorEnAire.
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               puntoX = j*LARGO;
               puntoY = i*ANCHO;
               if(mapa[i][j] == 1)
               {
                  if(collideSuelo(font, jugador, &puntoX, &puntoY) == true)
                  {
                     jugadorEnAire = false;
                     al_set_timer_count(tempGravedad, 0);
                     al_stop_timer(tempGravedad);
                     break;
                  }
               }
            }
         }
         }
         if(jugadorEnAire == true) // Logica de gravedad.
         {
            al_start_timer(tempGravedad);
            if(valorTimerGravedad > TERMINAL_VELOCITY)
            {
               al_stop_timer(tempGravedad);
               al_set_timer_count(tempGravedad, TERMINAL_VELOCITY);
            }
            valorTimerGravedad = al_get_timer_count(tempGravedad);
            jugador.posY = jugador.posY + valorTimerGravedad;
         }

         for(i = 0; i < ANCHO_MAPA; i++) // Revisa las colisiones en cada bloque.
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               puntoX = j*LARGO;
               puntoY = i*ANCHO;
               if(mapa[i][j] == 1) // Colision personaje-suelo/pared:
               {
                  if(collide(font, jugador, &puntoX, &puntoY) == true)
                  {
                     // Deshace el movimiento del personaje respecto a la colision.
                     al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 10, 0, "DEBUG: collide = %d", collide(font, jugador, &puntoX, &puntoY));
                     if(jugadorEnAire == true)
                     {
                        jugador.posY = anularMovimientoY(font, direccionArriba, direccionAbajo, jugador, &puntoY);
                     }
                     else
                     {
                        jugador.posX = anularMovimientoX(font, direccionIzquierda, direccionDerecha, jugador, &puntoX);
                     }
                  }
               }
               if(mapa[i][j] == 2) // Colision personaje-pincho:
               {
                  if(collide(font, jugador, &puntoX, &puntoY) == true && iFrames == 0)
                  {
                     jugador.vida--; // Resta 1 punto de vida.
                     iFrames = 120; // Otorga 120 frames de invencibilidad.
                  }
               }
            }
         }

         if(jugador.vida <= 0) // Logica de game over.
         {
            al_draw_textf(font, al_map_rgb(255, 13, 69), 600,360, 0, "GAME OVER");
            if(direccionIzquierda == true)
            {
               jugador.posX = jugador.posX + SPEED_FACTOR;
            }
            if(direccionDerecha == true)
            {
               jugador.posX = jugador.posX - SPEED_FACTOR;
            }

         }

         if(jugador.posY > ANCHO_PANTALLA + ANCHO) // Si el jugador se "cae" (Se sale de la pantalla por abajo)
         {
            jugador.posX = SPAWNPOINT_X;
            jugador.posY = SPAWNPOINT_Y;
         }

         // 2: Dibujar el siguiente frame.

         for(i = 0; i < ANCHO_MAPA; i++)
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               if(mapa[i][j] == 1)
               {
                  al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]); // Dibuja la posicion en la matriz del cuadrado suelo en pantalla.
                  al_draw_filled_rectangle(j*LARGO, i*ANCHO, j*LARGO + LARGO, i*ANCHO + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.5)); // Dibuja el cuadrado suelo.
               }
               if(mapa[i][j] == 2)
               {
                  al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  al_draw_filled_rectangle(j*LARGO, i*ANCHO, j*LARGO + LARGO, i*ANCHO + ANCHO, al_map_rgba_f(0.5, 0, 0, 0.5));
               }
               if(mapa[i][j] == 0)
               {
                  al_draw_textf(font, al_map_rgb(100, 100, 100), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
               }
            }
         }

         al_draw_filled_rectangle(jugador.posX, jugador.posY, jugador.posX + LARGO, jugador.posY + ANCHO, al_map_rgb(255, 0, 0)); // Rectangulo de personaje.
         //al_draw_bitmap(mysha, 100, 100, 0);

         // Dibujar informacion de debug.

         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "X: %.1f Y: %.1f", jugador.posX, jugador.posY);

         valorTimerGravedad = al_get_timer_count(tempGravedad);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 250, 0, 0, "Temporizador de gravedad: %d", valorTimerGravedad);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 100, 200, 0, "teclaPresionada: %d", teclaPresionada);

         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 700, 0, "vida: %d", jugador.vida);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 710, 0, "iFrames: %d", iFrames);

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

         for(i = 0; i < ANCHO_MAPA; i++)
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               puntoX = j*LARGO;
               puntoY = i*ANCHO;
               if(mapa[i][j] == 1)
               {
                  if(collideSuelo(font, jugador, &puntoX, &puntoY) == true)
                  {          
                     al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 10, 0, "DEBUG: collideSuelo = %d", collideSuelo(font, jugador, &puntoX, &puntoY));
                     flag = 1;        
                     break;
                  }
               }
            }
         }
         if(flag == 0)
         {
            al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 10, 0, "DEBUG: collideSuelo = %d", collideSuelo(font, jugador, &puntoX, &puntoY));
         }
         al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 100, 0, "DEBUG: jugadorEnAire = %d", jugadorEnAire);

         direccionArriba = false; // Reinicia la direccion (esta se obtiene cada frame).
         direccionAbajo = false;
         direccionIzquierda = false;
         direccionDerecha = false;

         jugadorEnAire = true;

         flag = 0;

         if(iFrames > 0)
         {
            iFrames--;
         }

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

   fclose(contenidoMapa1);

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
/*         if(chequeoGravedad == 0)
         {
            if(cayendo == false)
            {
               al_start_timer(tempGravedad);
               cayendo = true;
            }
            valorTimerGravedad = al_get_timer_count(tempGravedad);
            if(valorTimerGravedad % 2 == 0)
            {
               valorTimerGravedad++;
            }
            posY = posY + valorTimerGravedad/50.0;
            if(collide(font, posX, posY, &bloqueColisionX, &bloqueColisionY) == true)
            {
               posY = posY - ANCHO;
            }
            break;
         }
         else
         {
            al_stop_timer(tempGravedad);
            cayendo = false;
         }*/
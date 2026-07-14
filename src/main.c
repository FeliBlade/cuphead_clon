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

#include <allegro5/bitmap.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro5.h> // PROBLEMAS:
#include <allegro5/color.h> // El salto permite que cuando el jugador se caiga al fondo de la pantalla, pueda saltar de nuevo cuando valorTimerGravedad es 0.
#include <allegro5/timer.h> // El parry no tiene cooldown.
#include <allegro5/allegro_audio.h> // La funcion anularMovimientoY lleva al personaje al tope del bloque.
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h> // Sigue: Interaccion con otros elementos (semiplataformas, enemigos, balas, corazones extra), pasar enemigos a un arreglo de estructura, implementar joystick

#define KEY_SEEN     1
#define KEY_RELEASED 2

#define LARGO 40 // Largo de un bloque
#define ANCHO 40 // Ancho de un bloque

#define LARGO_MAPA 160 // Largo de un subnivel. Normalmente caben 32 bloques en la pantalla
#define ANCHO_MAPA 18

#define SPEED_FACTOR 8
#define TERMINAL_VELOCITY 40 // 40 px/seg.
#define INVINCIBILITY_FRAMES 120
#define PARRY_FRAMES 30

#define MAX_ENEMIGOS 100

#define VIDA_ENEMIGO_A 20
#define VELOCIDAD_ENEMIGO_A 4

#define OUT_OF_BOUNDS -100

#define PI 3.14159

typedef struct
{
   float posX;
   float posY;
   float posParryX;
   float posParryY;
   int vida;
   int direccionMovimientoA;
   bool colisionEnemigoA;
   float puntoColisionA;
}
entidad;

typedef struct
{
   bool Arriba;
   bool Abajo;
   bool Izquierda;
   bool Derecha;
}
_direccion;

// crear estructura balas o municion
// definir una cantidad de balas para el jugador, de tal manera de ir recargando las balas
void must_init(bool test, const char *description);

bool collide(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
bool collideParry(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
bool collideSuelo(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
float anularMovimientoX(ALLEGRO_FONT* font, entidad entidad, _direccion direccion, float *sueloX);
float anularMovimientoY(ALLEGRO_FONT* font, entidad entidad, float *sueloY);

void cargarMapa(int mapa[ANCHO_MAPA][LARGO_MAPA], int nivel, entidad *jugador, entidad enemigos[MAX_ENEMIGOS], int *cantidadEnemigosA);

float angulo(float x1, float x2, float y1, float y2);
float distancia(float x1, float x2, float y1, float y2);

// escribir funcion direccionMovimiento, de tipo struct direccion, que retorna la direccion de movimiento del personaje
// en main grabaria posY en una variable y al inicio del bucle, la funcion compararia sus valores para determinar el movimiento

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
   jugador.vida = 99;

   entidad enemigosA[MAX_ENEMIGOS];
   int cantidadEnemigosA = 0;

   int iFrames = 0;
   int parryFrames = 0;
   int healCD = 0;

   _direccion direccion; // Estructura que registra la direccion de colision con paredes.
   direccion.Arriba = false;
   direccion.Abajo = false;
   direccion.Izquierda = false;
   direccion.Derecha = false;

   int x = 480;
   int y = 480;
   //float theta;

   int valorTimerGravedad; // Obtiene el valor del temporizador de gravedad.
   bool jugadorEnAire = true; // Revisa si esta cayendo el cuadrado personaje.
   bool cayendo = true; // Bandera que permite o denega el choque con semiplataformas.
   bool primeraVez = false; // explicar luego

   bool teclaSoltada = false; // Se activa cuando la tecla es soltada en el aire.
   bool puedeHacerParry = true; // Forma parte de las condiciones para hacer parry.

   int i, j, contEnemigos; // Contadores generales reutilizables.
   float puntoX, puntoY; // Reciben los valores de i y j para traspasarlos a variables flotantes que puedan ser traspasadas a las funciones de colision.

   FILE *contenidoMapa1; // Variables de obtencion de datos de "mapa1.txt".
   int valorRecibido; 
   int nivel = 1; // Numero de nivel.

   float camaraX = 0; // Variables de camara
   float camaraY = 0;
   float drawX, drawY, drawEnemigosX, drawEnemigosY; // Determinan la posicion en la pantalla para dibujar los objetos y enemigos.

   bool flag = 0; // BANDERA DE PRUEBA

   // Inicializacion general

   int mapa[ANCHO_MAPA][LARGO_MAPA];
   cargarMapa(mapa, nivel, &jugador, enemigosA, &cantidadEnemigosA);

   for(i = 0; i < cantidadEnemigosA; i++)
   {
      printf("X_%d = %f\n", i, enemigosA[i].posX);
      printf("Y_%d = %f\n", i, enemigosA[i].posY);
   }

   /*for(i = 0; i < ANCHO_MAPA; i++)
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
   printf("\n");*/

   ALLEGRO_KEYBOARD_STATE ks;

   al_start_timer(timer);

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
               if(valorTimerGravedad == 0 && primeraVez == false)
               {
                  al_set_timer_count(tempGravedad, -20);
                  primeraVez = true;
               }
               if(teclaSoltada == true && puedeHacerParry == true)
               {
                  parryFrames = PARRY_FRAMES; // Otorga 30 frames de parry.
               }
               jugadorEnAire = true;
               teclaSoltada = false;
               direccion.Arriba = true;
            }
            else if(jugadorEnAire == true)
            {
               teclaSoltada = true;
            }
            if(key[ALLEGRO_KEY_DOWN])
            {
               //posY=posY+SPEED_FACTOR;
               direccion.Abajo = true;
            }
            if(key[ALLEGRO_KEY_LEFT])
            {
               jugador.posX = jugador.posX - SPEED_FACTOR;
               direccion.Izquierda = true;
            }
            if(key[ALLEGRO_KEY_RIGHT])
            {
               jugador.posX = jugador.posX + SPEED_FACTOR;
               direccion.Derecha = true;
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

         case ALLEGRO_EVENT_DISPLAY_CLOSE: // Caso de que se cierre la ventana.
            done = true;
            break;
      }

      if(done)
         break;

      if(redraw && al_is_event_queue_empty(queue))
      {
         al_clear_to_color(al_map_rgb(0, 0, 0));

         // 1: Realizar los ajustes necesarios en el cuadrado personaje.

         jugador.posParryX = jugador.posX - 20;
         jugador.posParryY = jugador.posY - 20;
         valorTimerGravedad = al_get_timer_count(tempGravedad);
         if(valorTimerGravedad > 0)
         {
            cayendo = true;
         }
         else
         {
            cayendo = false;
         }
         if(parryFrames > 0)
         {
            puedeHacerParry = false;
         }
         else
         {
            puedeHacerParry = true;
         }

         if(jugadorEnAire == true) // Logica de gravedad.
         {
            al_start_timer(tempGravedad); // PASAR A FUNCION DESPUES
            if(valorTimerGravedad > TERMINAL_VELOCITY)
            {
               al_stop_timer(tempGravedad);
               al_set_timer_count(tempGravedad, TERMINAL_VELOCITY);
            }
            jugador.posY = jugador.posY + valorTimerGravedad;
            jugador.posParryY = jugador.posY - 20;
         }

         for(i = 0; i < ANCHO_MAPA; i++) // Define el valor de jugadorEnAire.
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               puntoX = j*LARGO;
               puntoY = i*ANCHO;
               if(fabs(puntoX - jugador.posX) <= LARGO * 3 && fabs(puntoY - jugador.posY) <= ANCHO * 3 && (mapa[i][j] == 1 || (mapa[i][j] == 2 && jugador.posY <= puntoY && cayendo == true)))
               {
                  if(jugadorEnAire == true && collideSuelo(font, jugador, &puntoX, &puntoY) == true)
                  {
                     jugadorEnAire = false;
                     al_set_timer_count(tempGravedad, 0);
                     al_stop_timer(tempGravedad);
                     primeraVez = false;
                     parryFrames = 0;
                     teclaSoltada = false;
                     puedeHacerParry = true;
                     break;
                  }
               }
            }
         }

         if(parryFrames > 0) // Dibuja el cuadrado parry.
         {
            al_draw_filled_rectangle(jugador.posParryX - camaraX, jugador.posParryY - camaraY, jugador.posParryX - camaraX + LARGO * 2, jugador.posParryY - camaraY + ANCHO * 2, al_map_rgb(255, 0, 255));
         }
         else
         {
            al_draw_filled_rectangle(jugador.posParryX - camaraX, jugador.posParryY - camaraY, jugador.posParryX - camaraX + LARGO * 2, jugador.posParryY - camaraY + ANCHO * 2, al_map_rgb(100, 0, 100));
         }

         for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++) // Movimiento de los enemigos A.
         {
            if(enemigosA[contEnemigos].colisionEnemigoA == true) // Si chocan:
            {
               if(enemigosA[contEnemigos].direccionMovimientoA == VELOCIDAD_ENEMIGO_A) // Anula el movimiento del enemigo segun el valor de direccionMovimiento.
               {
                  enemigosA[contEnemigos].posX = enemigosA[contEnemigos].puntoColisionA - LARGO;
               }
               else if(enemigosA[contEnemigos].direccionMovimientoA == -VELOCIDAD_ENEMIGO_A)
               {
                  enemigosA[contEnemigos].posX = enemigosA[contEnemigos].puntoColisionA + LARGO;
               }
               enemigosA[contEnemigos].direccionMovimientoA *= -1; // Invierte la direccion de movimiento.
               enemigosA[contEnemigos].colisionEnemigoA = false; // Anula la bandera de colision para que no se de vuelta de nuevo.
            }
            enemigosA[contEnemigos].posX += enemigosA[contEnemigos].direccionMovimientoA; // Desplaza al enemigo correspondiente.
         }
         
         for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++)  // Revisa las colisiones en los enemigos A.
         {
            int fila = enemigosA[contEnemigos].posY / ANCHO;
            for(j = 0; j < LARGO_MAPA; j++)
            {
               if(mapa[fila][j] == 1)
               {
                  puntoX = j*LARGO;
                  if(enemigosA[contEnemigos].posX + LARGO > puntoX && puntoX + LARGO > enemigosA[contEnemigos].posX)
                  {
                     enemigosA[contEnemigos].colisionEnemigoA = true;
                     enemigosA[contEnemigos].puntoColisionA = puntoX;
                     break;
                  }
               }
            }
         }
         /*for(i = 0; i < ANCHO_MAPA; i++) // (dejar comentado por si falla el reemplazo)
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               if(mapa[i][j] == 1)
               {
                  puntoX = j*LARGO;
                  puntoY = i*ANCHO;
                  for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++) // Enemigos A
                  {
                     if(enemigosA[contEnemigos].posY == puntoY && enemigosA[contEnemigos].posX + LARGO > puntoX && puntoX + LARGO > enemigosA[contEnemigos].posX) // Si chocan:
                     {
                        enemigosA[contEnemigos].colisionEnemigo = true;
                        enemigosA[contEnemigos].puntoColision = puntoX;
                        break;
                     }
                  }
               }
            }
         }*/

         for(i = 0; i < ANCHO_MAPA; i++) // Revisa las colisiones en cada bloque. (REVISAR LAS COLISIONES CON LOS ENEMIGOS DINAMICOS RESTANTES)
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               puntoX = j*LARGO;
               puntoY = i*ANCHO;
               if(fabs(puntoX - jugador.posX) <= LARGO * 3 && fabs(puntoY - jugador.posY) <= ANCHO * 3) // Revisa solo si la distancia entre el bloque dado y el personaje es menor o igual a cierto rango.
               {
                  if(mapa[i][j] == 1) // Colision personaje-suelo/pared:
                  {
                     if(collide(font, jugador, &puntoX, &puntoY) == true)
                     {
                        // Deshace el movimiento del personaje respecto a la colision.
                        al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 10, 0, "DEBUG: collide = %d", collide(font, jugador, &puntoX, &puntoY));
                        if(jugadorEnAire == false || (direccion.Izquierda == false && direccion.Derecha == false))
                        {
                           jugador.posY = anularMovimientoY(font, jugador, &puntoY);
                           al_set_timer_count(tempGravedad, 0);
                        }
                        if(direccion.Izquierda == true || direccion.Derecha == true)
                        {
                           //jugador.posX = anularMovimientoX(font, jugador, direccion, &puntoX);
                        }
                     }
                  }
                  if(mapa[i][j] == 2) // Colision personaje-semiplataforma:
                  {
                     if(collide(font, jugador, &puntoX, &puntoY) == true && jugador.posY <= puntoY && cayendo == true) // Deben chocar, el personaje debe estar mas alto que la plataforma, y el personaje debe estar cayendo estrictamente para abajo.
                     {
                        jugador.posY = anularMovimientoY(font, jugador, &puntoY);
                     }
                  }
                  for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++) // Colision personaje-enemigo A:
                  {
                     if(fabs(enemigosA[contEnemigos].posX - jugador.posX) <= LARGO * 3 && fabs(enemigosA[contEnemigos].posY - jugador.posY) <= ANCHO * 3)
                     {
                        if(collide(font, jugador, &enemigosA[contEnemigos].posX, &enemigosA[contEnemigos].posY) == true && iFrames == 0)
                        {
                           jugador.vida--; // Resta 1 punto de vida.
                           iFrames = INVINCIBILITY_FRAMES; // Otorga 120 frames de invencibilidad.
                        }
                     }
                  }
                  /*if(mapa[i][j] == 3) // Colision personaje-enemigo:
                  {
                     if(collide(font, jugador, &puntoX, &puntoY) == true && iFrames == 0)
                     {
                        jugador.vida--; // Resta 1 punto de vida.
                        iFrames = INVINCIBILITY_FRAMES; // Otorga 120 frames de invencibilidad.
                     }
                  }*/
                  if(mapa[i][j] == 4 && direccion.Arriba == true) // Colision parry propio-objeto parriable:
                  {
                     if(collideParry(font, jugador, &puntoX, &puntoY) == true && parryFrames > 0) // Si se efectua un parry correctamente:
                     {
                        al_rest(0.1);
                        al_set_timer_count(tempGravedad, -20);
                        teclaSoltada = true;
                        parryFrames = 0;
                     }
                  }
                  if(mapa[i][j] == 5) // Colision personaje-corazon:
                  {
                     if(collide(font, jugador, &puntoX, &puntoY) == true && healCD == 0)
                     {
                        jugador.vida++; // Otorga 1 punto de vida.
                        healCD = INVINCIBILITY_FRAMES;
                     }
                  }
                  if(mapa[i][j] == 6) // Colision personaje-pincho:
                  {
                     if(collide(font, jugador, &puntoX, &puntoY) == true)
                     {
                        al_set_timer_count(tempGravedad, -30);
                        if(iFrames == 0)
                        {
                           jugador.vida--; // Resta 1 punto de vida.
                           iFrames = INVINCIBILITY_FRAMES; // Otorga 120 frames de invencibilidad.
                        }
                     }
                  }
                  if(mapa[i][j] == 8) // Colision personaje-portal
                  {
                     if(collideParry(font, jugador, &puntoX, &puntoY) == true && direccion.Arriba == true) // Utiliza el cuadrado parry para facilitar la entrada al portal.
                     {
                        al_draw_textf(font, al_map_rgb(255, 255, 255), jugador.posX, jugador.posY + 50, 0, "Transicionando...");
                        nivel++;
                        al_rest(1);
                        cargarMapa(mapa, nivel, &jugador, enemigosA, &cantidadEnemigosA);
                     }
                  }
               }
            }
         }

         if(jugador.vida <= 0) // Logica de game over.
         {
            al_draw_textf(font, al_map_rgb(255, 13, 69), 600,360, 0, "GAME OVER");
            /*if(direccion.Izquierda == true)
            {
               jugador.posX = jugador.posX + SPEED_FACTOR;
            }
            if(direccion.Derecha == true)
            {
               jugador.posX = jugador.posX - SPEED_FACTOR;
            }*/

         }

         if(jugador.posY > ANCHO_PANTALLA + ANCHO) // Si el jugador se "cae" (Se sale de la pantalla por abajo)
         {
            al_set_timer_count(tempGravedad, -30);
            if(iFrames == 0)
            {
               jugador.vida--; // Resta 1 punto de vida.
               iFrames = INVINCIBILITY_FRAMES; // Otorga 120 frames de invencibilidad.
            }
         }

         if(fabs(x - jugador.posX) > 0) // TEST
         {
            if(x > jugador. posX)
            {
               x--;
            }
            if(x < jugador. posX)
            {
               x++;
            }
         }
         if(fabs(y - jugador.posY) > 0)
         {
            if(y > jugador.posY)
            {
               y--;
            }
            if(y < jugador.posY)
            {
               y++;
            }
         }

         camaraX = jugador.posX - LARGO_PANTALLA / 2.0; // Centra la cámara en el jugador.
         camaraY = jugador.posY - ANCHO_PANTALLA / 2.0;

         if(camaraX < 0) // Ajusta los valores de la camara para no mostrar fuera del mapa
         {
            camaraX = 0;
         }
         /*if(camaraY < 0)
         {
            camaraY = 0;
         }*/
         if(camaraX > LARGO_MAPA * LARGO - LARGO_PANTALLA)
         {
            camaraX = LARGO_MAPA * LARGO - LARGO_PANTALLA;
         }
         if(camaraY > ANCHO_MAPA * ANCHO - ANCHO_PANTALLA)
         {
            camaraY = ANCHO_MAPA * ANCHO - ANCHO_PANTALLA;
         }

         // 2: Dibujar el siguiente frame.

         for(i = 0; i < ANCHO_MAPA; i++) // Dibuja el respectivo mapa (pasar a funcion despues)
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               drawX = j*LARGO - camaraX;
               drawY = i*ANCHO - camaraY;
               if(mapa[i][j] == 1) // Suelo
               {
                  //al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]); // Dibuja la posicion en la matriz del cuadrado suelo en pantalla.
                  al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.2)); // Dibuja el cuadrado suelo.
               }
               if(mapa[i][j] == 2) // Plataforma atravesable
               {
                  //al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0, 0.8, 0.8, 0.2));
               }
               for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++)
               {
                  drawEnemigosX = enemigosA[contEnemigos].posX - camaraX;
                  drawEnemigosY = enemigosA[contEnemigos].posY - camaraY;
                  al_draw_filled_rectangle(drawEnemigosX, drawEnemigosY, drawEnemigosX + LARGO, drawEnemigosY + ANCHO, al_map_rgba_f(0.8, 0, 0, 0.2));
               }
               /*if(mapa[i][j] == 3) // Enemigo (obstaculo)
               {
                  //al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0.8, 0, 0, 0.2));
               }*/
               if(mapa[i][j] == 4) // Parry enemigo
               {
                  //al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 0.5, 0.6, 0.2));
               }
               if(mapa[i][j] == 5) // Corazon
               {
                  //al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0, 0.6, 0, 0.2));
               }
               if(mapa[i][j] == 6) // Pincho
               {
                  //al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0.5, 0, 0, 0.2));
               }
               if(mapa[i][j] == 8) // Portal
               {
                  //al_draw_textf(font, al_map_rgb(255, 255, 255), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
                  al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 0.5, 0, 0.2));
               }
               if(mapa[i][j] == 9) // Punto de inicio
               {
                  al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 1, 1, 0.2));
                  //al_draw_textf(font, al_map_rgb(0, 0, 0), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
               }
               if(mapa[i][j] == 0) // Vacio
               {
                  //al_draw_textf(font, al_map_rgb(50, 50, 50), j*LARGO, i*ANCHO, 0, "%d", mapa[i][j]);
               }
            }
         }

         al_draw_filled_rectangle(jugador.posX - camaraX, jugador.posY - camaraY, jugador.posX - camaraX + LARGO, jugador.posY - camaraY + ANCHO, al_map_rgb(255, 255, 0)); // Rectangulo de personaje.
         al_draw_filled_rectangle(x - camaraX, y - camaraY, x - camaraX + LARGO, y - camaraY + ANCHO, al_map_rgb(207, 255, 163));
         //al_draw_bitmap(mysha, 100, 100, 0);

         // 3. Dibujar informacion de debug.

         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "X: %.1f Y: %.1f", jugador.posX, jugador.posY);

         valorTimerGravedad = al_get_timer_count(tempGravedad);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 250, 0, 0, "Temporizador de gravedad: %d", valorTimerGravedad);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 110, 0, "cayendo: %d", cayendo);

         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 700, 0, "vida: %d", jugador.vida);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 710, 0, "iFrames: %d", iFrames);

         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 500, 0, "camaraX: %f", camaraX);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 510, 0, "camaraY: %f", camaraY);

         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 530, 0, "enemigosA[0].direccionMovimiento: %d", enemigosA[0].direccionMovimientoA);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 540, 0, "enemigosA[0].colisionEnemigo: %d", enemigosA[0].colisionEnemigoA);

         al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 150, 0, "enemigosA[0].posX = %f", enemigosA[0].posX);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 160, 0, "enemigosA[0].posY = %f", enemigosA[0].posY);

         if(direccion.Arriba == true) // Imprime las direcciones ingresadas.
         {
            al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 70, 0, "^");
         }
         if(direccion.Abajo == true)
         {
            al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 90, 0, "v");
         }
         if(direccion.Izquierda == true)
         {
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 80, 0, "<");
         }
         if(direccion.Derecha == true)
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
         al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 100, 0, "DEBUG: jugadorEnAire = %d", jugadorEnAire);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 110, 0, "DEBUG: teclaSoltada = %d", teclaSoltada);

         al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 120, 0, "DEBUG: puedeHacerParry = %d", puedeHacerParry);
         al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 130, 0, "DEBUG: parryFrames = %d", parryFrames);

         // 4. Ajustar ciertas variables al final de un frame.

         direccion.Arriba = false; // Reinicia la direccion (esta se obtiene cada frame).
         direccion.Abajo = false;
         direccion.Izquierda = false;
         direccion.Derecha = false;

         jugadorEnAire = true;

         flag = 0;

         if(iFrames > 0)
         {
            iFrames--;
         }
         if(healCD > 0)
         {
            healCD--;
         }
         if(parryFrames > 0)
         {
            parryFrames--;
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

   return 0;
}

void must_init(bool test, const char *description)
{
   if(test) return;

   printf("couldn't initialize %s\n", description);
   exit(1);
}

bool collide(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY)
{
   int chequeoColision = 0;

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

bool collideParry(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY)
{
   int chequeoColision = 0;

   if(entidad.posParryX + LARGO * 2 > *sueloX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 120, 0, "DEBUG: condicion 1: true");
      chequeoColision++;
   }
   if(*sueloX + LARGO > entidad.posParryX)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 130, 0, "DEBUG: condicion 2: true");
      chequeoColision++;
   }
   if(entidad.posParryY + ANCHO * 2 > *sueloY)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 140, 0, "DEBUG: condicion 3: true");
      chequeoColision++;
   }
   if(*sueloY + ANCHO > entidad.posParryY)
   {
      al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 150, 0, "DEBUG: condicion 4: true");
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

float anularMovimientoX(ALLEGRO_FONT* font, entidad entidad, _direccion direccion, float *sueloX) // Deshace el movimiento en el eje X al chocar
{
   float ajusteX;
   ajusteX = entidad.posX;

   if(direccion.Izquierda == true)
   {
      ajusteX = *sueloX + LARGO;
   }
   if(direccion.Derecha == true)
   {
      ajusteX = *sueloX - LARGO;
   }

   return ajusteX;
}

float anularMovimientoY(ALLEGRO_FONT* font, entidad entidad, float *sueloY) // Deshace el movimiento en el eje Y al chocar
{
   float ajusteY;
   ajusteY = entidad.posY;

   if(ajusteY+ANCHO>*sueloY)
   {
      ajusteY = *sueloY - ANCHO;
   }
   if(ajusteY<*sueloY+ANCHO)
   {
      ajusteY = *sueloY - ANCHO;
   }

   return ajusteY;
}

void cargarMapa(int mapa[ANCHO_MAPA][LARGO_MAPA], int nivel, entidad *jugador, entidad enemigosA[5], int *cantidadEnemigosA)
//void cargarMapa(char *ruta_archivo, entidad jugador)
{
   FILE *contenidoMapa;
   int i, j, valorRecibido;

   /*sacar la lectura del archivo y carga de mapa a una funcion, psar como parametros mapa y jugador*/

   // switch o if donde pasas el nivle y dices si es 1 carga mapa 1, si es 2 carga 2

   switch(nivel)
   {
      case 1:
      contenidoMapa = fopen("mapa1.txt", "r");
      must_init(contenidoMapa, "mapa1");
      break;

      case 2:
      contenidoMapa = fopen("mapa2.txt", "r");
      must_init(contenidoMapa, "mapa2");
      break;

      default:
      printf("no existe el nivel\n");
      exit(1);
   }

// ¨Poner verificacion si contenidoMapa! tirar error y cerrar

   for(i = 0; i < ANCHO_MAPA; i++)
   {
      for(j = 0; j < LARGO_MAPA; j++)
      {
         if(fscanf(contenidoMapa, "%d", &valorRecibido) != EOF)
         {
            mapa[i][j] = valorRecibido;
            if(valorRecibido == 3) // Define los puntos de inicio de los enemigos
            {
               if(*cantidadEnemigosA < MAX_ENEMIGOS)
               {
                  // revisar esto, por ejemplo: si coloco mas enemigos que el maximo, se cae
                  // la solucion: llegado a esta parte, recorrer el arreglo de enemigos y buscar un espacio para el nuevo enemigo
                  enemigosA[*cantidadEnemigosA].posX = j*LARGO;
                  enemigosA[*cantidadEnemigosA].posY = i*ANCHO;
                  enemigosA[*cantidadEnemigosA].vida = VIDA_ENEMIGO_A;
                  enemigosA[*cantidadEnemigosA].direccionMovimientoA = -VELOCIDAD_ENEMIGO_A;
                  enemigosA[*cantidadEnemigosA].colisionEnemigoA = false;
                  (*cantidadEnemigosA)++;
               }
               else
               {
                  printf("Advertencia: Hay mas enemigos que los que soporta el arreglo\n");
               }
            }
            if(valorRecibido == 9) // Define el punto de inicio del jugador en el caso del cargado de una casilla 9
            {
               jugador->posX = j*LARGO;
               jugador->posY = i*ANCHO;
            }
         }
         else
         {
            break;
         }
      }
   }
/*
   for(i = 0; i < ANCHO_MAPA; i++)
   {
      for(j = 0; j < LARGO_MAPA; j++)
      {
         printf("%d  ", mapa[i][j]);
      }
      printf("\n");
   }
   printf("dos\n");
*/
   fclose(contenidoMapa);

   return;
}

float angulo(float x1, float x2, float y1, float y2)
{
   float anguloRad, anguloDeg, pendiente; 

   pendiente = (y2-y1)/(x2-x1);
   anguloRad = atan(pendiente);
   anguloDeg = anguloRad * (180/PI);

   return anguloDeg;
}

float distancia(float x1, float x2, float y1, float y2)
{
   float distancia, radicando;

   radicando = powf((x2-x1), 2) + powf((y2-y1), 2);
   distancia = sqrtf(radicando);

   return distancia;
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
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

#include <allegro5/altime.h> // hacer arreglo de portales 'o', de componentes 0 y 1 por si han sido usados.
#include <allegro5/bitmap.h>
#include <allegro5/bitmap_draw.h>
#include <allegro5/events.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro5.h> // PROBLEMAS:
#include <allegro5/color.h> // El salto permite que cuando el jugador se caiga al fondo de la pantalla, pueda saltar de nuevo cuando valorTimerGravedad es 0.
#include <allegro5/timer.h> // El parry no tiene cooldown despues de fallarlo.
#include <allegro5/allegro_audio.h> // La funcion anularMovimientoY lleva al personaje al tope del bloque.
#include <allegro5/allegro_acodec.h> // Se descoloca el cuadrado parry al moverse (puede ser por el scrolling con la camara o por la mal optimizada revision de colision de enemigos)
#include <allegro5/allegro_font.h> // No funcionan las colsiones con el jugador y los enemigos E.
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h> // Sigue: Interaccion con otros elementos (balas, llaves, corazones extra), pasar enemigos a un arreglo de estructura, implementar joystick

#define KEY_SEEN     1
#define KEY_RELEASED 2

#define LARGO 40 // Largo de un bloque
#define ANCHO 40 // Ancho de un bloque

#define LARGO_MAPA 160 // Largo de un subnivel. Normalmente caben 32 bloques en la pantalla
#define ANCHO_MAPA 18

#define SPEED_FACTOR 7
#define TERMINAL_VELOCITY 40 // 40 px/seg.
#define INVINCIBILITY_FRAMES 120
#define DASH_FRAMES 30
#define DASH_SPEED 15
#define PARRY_FRAMES 30

#define MAX_ENEMIGOS 100
#define MAX_BALAS 50
#define VELOCIDAD_BALA 4
#define LARGO_BALA 10
#define ANCHO_BALA 6
#define COOLDOWN_DISPARO 15 // frames entre disparos

#define VIDA_ENEMIGO_A 20
#define VELOCIDAD_ENEMIGO_A 4

#define HORIZONTAL_OSCILLATION_RANGE_C 100
#define VERTICAL_OSCILLATION_RANGE_C 25
#define OSCILLATION_SPEED_C 25
#define FALLING_SPEED_C 2

#define OSCILLATION_RANGE_E 150
#define OSCILLATION_SPEED_E 25

#define OUT_OF_BOUNDS -100

#define LARGO_BLOQUE 40
#define ANCHO_BLOQUE 40
#define LARGO_SEMIPLATAFORMA 40
#define ANCHO_SEMIPLATAFORMA 20
#define LARGO_ENEMIGO_A 40
#define ANCHO_ENEMIGO_A 48
#define FRAMES_ENEMIGO_A 7
#define FRAME_RATE_ENEMIGO_A 5
#define OFFSET_ENEMIGO_A_Y 8
#define LARGO_ENEMIGO_C 64
#define ANCHO_ENEMIGO_C 88
#define OFFSET_ENEMIGO_C_X -10
#define OFFSET_ENEMIGO_C_Y 10
#define LARGO_ENEMIGO_E 48
#define ANCHO_ENEMIGO_E 48
#define SPIN_RATE_ENEMIGO_E 10
#define OFFSET_ENEMIGO_E_X 4
#define OFFSET_ENEMIGO_E_Y 4
#define FRAMES_QUIETO 2
#define FRAME_RATE_QUIETO 20
#define OFFSET_QUIETO_Y 40
#define FRAMES_CORRIENDO 5
#define FRAME_RATE_CORRIENDO 5
#define OFFSET_CORRIENDO_Y 48
#define PI 3.14159

#define VARIABLES_CARGARMAPA char mapa[ANCHO_MAPA][LARGO_MAPA], int nivel, entidad *jugador, entidad enemigosA[MAX_ENEMIGOS], int *cantidadEnemigosA, entidad enemigosC[MAX_ENEMIGOS], int *cantidadEnemigosC, entidad enemigosE[MAX_ENEMIGOS], int *cantidadEnemigosE
#define CARGADO_DE_MAPA mapa, nivel, &jugador, enemigosA, &cantidadEnemigosA, enemigosC, &cantidadEnemigosC, enemigosE, &cantidadEnemigosE

typedef struct
{
   float posX;
   float posY;
   int direccion;
   bool activa;
}
bala;

typedef struct
{
   float posX;
   float posY;
   float posParryX;
   float posParryY;
   int vida;
   bala balas[MAX_BALAS];
   int disparoCD;
   int orientacion;
   int frameQuieto;
   int frameCorriendo;
   int direccionMovimientoA;
   bool colisionEnemigoA;
   float puntoColisionA;
   int frameA;
   int cicladoFramesA;
   int contadorSaltitoA;
   ALLEGRO_TIMER* tempEnemigosC;
   float valorTimerEnemigosC;
   float nodoCX;
   float nodoCY;
   float nodoE;
}
entidad;

typedef struct
{
   bool Arriba;
   bool Abajo;
   bool Izquierda;
   bool Derecha;
   bool X;
}
_direccion;

typedef struct
{
   ALLEGRO_BITMAP* _sheet;

   ALLEGRO_BITMAP* tierra;
   ALLEGRO_BITMAP* pasto;
   ALLEGRO_BITMAP* semiplataforma;
   ALLEGRO_BITMAP* enredadera;
   ALLEGRO_BITMAP* flor;
   ALLEGRO_BITMAP* puerta;

   ALLEGRO_BITMAP* enemigoA[FRAMES_ENEMIGO_A];
   ALLEGRO_BITMAP* enemigoC;
   ALLEGRO_BITMAP* enemigoE;

   ALLEGRO_BITMAP* jugador_quieto[FRAMES_QUIETO];
   ALLEGRO_BITMAP* jugador_corriendo[FRAMES_CORRIENDO];
   ALLEGRO_BITMAP* jugador_salto;
   ALLEGRO_BITMAP* jugador_parry;
}
_sprites;
_sprites sprites;

// crear estructura balas o municion
// definir una cantidad de balas para el jugador, de tal manera de ir recargando las balas
void must_init(bool test, const char *description);

bool generalCollide(float x1, float y1, float largo1, float ancho1, float x2, float y2, float largo2, float ancho2);
bool collide(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
bool collideParry(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
bool collideSuelo(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
float anularMovimientoX(ALLEGRO_FONT* font, entidad *entidad, float posXAnterior, float *sueloX);
float anularMovimientoY(ALLEGRO_FONT* font, entidad *entidad, float posYAnterior, float *sueloY, ALLEGRO_TIMER* tempGravedad);

void cargarMapa(VARIABLES_CARGARMAPA);
ALLEGRO_BITMAP* sprite_grab(int x, int y, int largo, int ancho);
void sprites_init();
void sprites_deinit();

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
   ALLEGRO_TIMER* tempEnemigosE = al_create_timer(1.0 / TARGET_FPS); // Temporizador de posicion de enemigos E.
   must_init(tempEnemigosE, "tempEnemigosE");

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

   must_init(al_init_image_addon(), "image addon");
   sprites_init();

    /*ALLEGRO_BITMAP* mysha = al_load_bitmap("mysha.png");
    must_init(mysha, "mysha");*/
   al_register_event_source(queue, al_get_keyboard_event_source());
   al_register_event_source(queue, al_get_display_event_source(disp));
   al_register_event_source(queue, al_get_timer_event_source(timer));

   bool MOVIMIENTO = true; // Variables interruptor para activar y desactivar ciertas propiedades del juego.
   bool GRAVEDAD = true;
   bool HITBOXES = false;
   bool SPRITES = true;
   bool DEBUG = true;

   bool done = false;
   bool redraw = true;
   ALLEGRO_EVENT event;

   entidad jugador;
   jugador.orientacion = 1;
   jugador.vida = 99;
   jugador.frameQuieto = 0;
   jugador.frameCorriendo = 0;

   entidad enemigosA[MAX_ENEMIGOS]; // Se declaran los arreglos de enemigos con sus cantidades.
   int cantidadEnemigosA = 0;
   entidad enemigosC[MAX_ENEMIGOS];
   int cantidadEnemigosC = 0;
   float valorTimerEnemigosC; // Obtiene el valor del temporizador de posicion de enemigos C.
   entidad enemigosE[MAX_ENEMIGOS];
   int cantidadEnemigosE = 0;
   float valorTimerEnemigosE; // Obtiene el valor del temporizador de posicion de enemigos E.

   int iFrames = 0;
   int dashFrames = 0;
   int parryFrames = 0;
   int healCD = 0;

   _direccion direccion; // Estructura que registra la direccion de colision con paredes.
   direccion.Arriba = false;
   direccion.Abajo = false;
   direccion.Izquierda = false;
   direccion.Derecha = false;
   direccion.X = false;

   int x = 480;
   int y = 480;
   //float theta;

   float valorTimer; // Obtiene el valor del timer de frames.
   int valorTimerGravedad; // Obtiene el valor del temporizador de gravedad.
   bool jugadorEnAire = true; // Revisa si esta cayendo el cuadrado personaje.
   bool cayendo = true; // Bandera que permite o denega el choque con semiplataformas.
   bool primeraVezSalto = false; // explicar luego

   bool primeraVezDash = false;

   bool teclaSoltada = false; // Se activa cuando la tecla es soltada en el aire.
   bool puedeHacerParry = true; // Forma parte de las condiciones para hacer parry.

   int i, j, cont, contEnemigos; // Contadores generales reutilizables.
   float puntoX, puntoY; // Reciben los valores de i y j para traspasarlos a variables flotantes que puedan ser traspasadas a las funciones de colision.

   int nivel = 1; // Numero de nivel.

   float camaraX = 0; // Variables de camara.
   float camaraY = 0;
   float drawX, drawY, drawEnemigosX, drawEnemigosY; // Con camara(x, y), determinan la posicion en la pantalla para dibujar las entidades.

   bool flag = 0; // BANDERA DE PRUEBA

   // Inicializacion general

   char mapa[ANCHO_MAPA][LARGO_MAPA];
   cargarMapa(CARGADO_DE_MAPA);

   for(i = 0; i < cantidadEnemigosA; i++)
   {
      printf("X_%d = %f\n", i, enemigosA[i].posX);
      printf("Y_%d = %f\n", i, enemigosA[i].posY);
   }
   for(int i = 0; i < MAX_BALAS; i++)
   {
      jugador.balas[i].activa = false;
   }

   ALLEGRO_KEYBOARD_STATE ks;

   al_start_timer(timer);
   al_start_timer(tempEnemigosE);

   unsigned char key[ALLEGRO_KEY_MAX];
   memset(key, 0, sizeof(key));

   while(1)
   {
      al_wait_for_event(queue, &event);

      switch(event.type)
      {
         case ALLEGRO_EVENT_TIMER:
         if(MOVIMIENTO == true)
         {
            if(key[ALLEGRO_KEY_UP])
            {
               if(valorTimerGravedad == 0 && primeraVezSalto == false)
               {
                  al_set_timer_count(tempGravedad, -20);
                  primeraVezSalto = true;
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
               direccion.Abajo = true;
            }
            if(key[ALLEGRO_KEY_LEFT])
            {
               jugador.posX = jugador.posX - SPEED_FACTOR;
               jugador.orientacion = -1;
               direccion.Izquierda = true;
            }
            if(key[ALLEGRO_KEY_RIGHT])
            {
               jugador.posX = jugador.posX + SPEED_FACTOR;
               jugador.orientacion = 1;
               direccion.Derecha = true;
            }
         }
         if(key[ALLEGRO_KEY_Z])
         {
            if(jugador.disparoCD == 0)
            {
               for(i = 0; i < MAX_BALAS; i++) // Busca el primer slot libre del arreglo.
               {
                  if(jugador.balas[i].activa == false)
                  {
                     jugador.balas[i].posX = jugador.posX;
                     jugador.balas[i].posY = jugador.posY;
                     jugador.balas[i].direccion = jugador.orientacion;
                     jugador.balas[i].activa = true;
                     jugador.disparoCD = COOLDOWN_DISPARO;
                     break;
                  }
               }
            }
         }
         if(key[ALLEGRO_KEY_X])
         {
            if(primeraVezDash == false && direccion.X == false)
            {
               dashFrames = DASH_FRAMES;
               primeraVezDash = true;
            }
            direccion.X = true;
         }
         if(key[ALLEGRO_KEY_ESCAPE])
         {
            done = true;
         }
         for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
         {
            key[i] &= KEY_SEEN;
         }
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
         al_clear_to_color(al_map_rgb(122, 122, 122));

         // 1: Realizar los ajustes necesarios en los objetos dinamicos.
         // 1.1: Personaje:

         jugador.posParryX = jugador.posX - 20;
         jugador.posParryY = jugador.posY - 20;
         for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++)
         {
            enemigosC[contEnemigos].valorTimerEnemigosC = al_get_timer_count(enemigosC[contEnemigos].tempEnemigosC);
         }
         valorTimer = al_get_timer_count(timer);
         valorTimerEnemigosE = al_get_timer_count(tempEnemigosE);
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

         if(GRAVEDAD == true)
         {
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
         }

         for(i = 0; i < ANCHO_MAPA; i++) // Define el valor de jugadorEnAire.
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               puntoX = j*LARGO;
               puntoY = i*ANCHO;
               if(fabs(puntoX - jugador.posX) <= LARGO * 3 && fabs(puntoY - jugador.posY) <= ANCHO * 3 && (mapa[i][j] == '#' || (mapa[i][j] == '=' && jugador.posY <= puntoY && cayendo == true)))
               {
                  if(jugadorEnAire == true && collideSuelo(font, jugador, &puntoX, &puntoY) == true)
                  {
                     jugadorEnAire = false;
                     al_set_timer_count(tempGravedad, 0);
                     al_stop_timer(tempGravedad);
                     primeraVezSalto = false;
                     parryFrames = 0;
                     teclaSoltada = false;
                     primeraVezDash = false;
                     puedeHacerParry = true;
                     break;
                  }
               }
            }
         }

         if(dashFrames > 0) // Logica de dash.
         {
            MOVIMIENTO = false;
            GRAVEDAD = false;
            jugador.posX += DASH_SPEED * jugador.orientacion;
            dashFrames--;
         }
         if(dashFrames == 1)
         {
            MOVIMIENTO = true;
            GRAVEDAD = true;
            al_set_timer_count(tempGravedad, 0);
         }

         if(parryFrames > 0) // Dibuja el cuadrado parry.
         {
            al_draw_filled_rectangle(jugador.posParryX - camaraX, jugador.posParryY - camaraY, jugador.posParryX - camaraX + LARGO * 2, jugador.posParryY - camaraY + ANCHO * 2, al_map_rgb(255, 0, 255));
         }
         else
         {
            al_draw_filled_rectangle(jugador.posParryX - camaraX, jugador.posParryY - camaraY, jugador.posParryX - camaraX + LARGO * 2, jugador.posParryY - camaraY + ANCHO * 2, al_map_rgb(100, 0, 100));
         }

         for(i = 0; i < MAX_BALAS; i++)
         {
            if(jugador.balas[i].activa == true) // Despawnea la bala si esta fuera de la camara.
            {
               jugador.balas[i].posX += VELOCIDAD_BALA * jugador.balas[i].direccion;
            }
            if(jugador.balas[i].posX < camaraX - LARGO || jugador.balas[i].posX > camaraX + LARGO_PANTALLA) // Fuera de pantalla.
            {
               jugador.balas[i].activa = false;
               continue;
            }
            int fila = (int)(jugador.balas[i].posY / ANCHO);
            for(int col = 0; col < LARGO_MAPA; col++) // Solo revisa la fila donde está la bala.
            {
               if(mapa[fila][col] == '#')
               {
                  float bloqueX = col * LARGO;
                  if(jugador.balas[i].posX + LARGO_BALA > bloqueX && bloqueX + LARGO > jugador.balas[i].posX)
                  {
                     jugador.balas[i].activa = false;
                     break;
                  }
               }
            }
         }

         // 1.2: Enemigos:

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
               if(mapa[fila][j] == '#')
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
         for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++)  // Movimiento de los enemigos C.
         {
            if(fabs(enemigosC[contEnemigos].posX - jugador.posX) <= (float)(LARGO_PANTALLA) / 2)
            {
               al_start_timer(enemigosC[contEnemigos].tempEnemigosC); // Inicia el timer de posicion de enemigos C.
            }
            if(al_get_timer_started(enemigosC[contEnemigos].tempEnemigosC) == true)
            {
               enemigosC[contEnemigos].valorTimerEnemigosC = al_get_timer_count(enemigosC[contEnemigos].tempEnemigosC);
               enemigosC[contEnemigos].posX = enemigosC[contEnemigos].nodoCX + HORIZONTAL_OSCILLATION_RANGE_C * sinf(enemigosC[contEnemigos].valorTimerEnemigosC / OSCILLATION_SPEED_C);
               enemigosC[contEnemigos].posY = enemigosC[contEnemigos].nodoCY + VERTICAL_OSCILLATION_RANGE_C* cosf(enemigosC[contEnemigos].valorTimerEnemigosC / OSCILLATION_SPEED_C) * cosf(enemigosC[contEnemigos].valorTimerEnemigosC / 25);
               enemigosC[contEnemigos].nodoCY += FALLING_SPEED_C;
            }
         }

         for(contEnemigos = 0; contEnemigos < cantidadEnemigosE; contEnemigos++)  // Movimiento de los enemigos E.
         {
            enemigosE[contEnemigos].posY = enemigosE[contEnemigos].nodoE + 150 * sinf(valorTimerEnemigosE / 25);
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
         for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Colision personaje-enemigo C:
         {
            if(fabs(enemigosC[contEnemigos].posX - jugador.posX) <= LARGO * 3 && fabs(enemigosC[contEnemigos].posY - jugador.posY) <= ANCHO * 3)
            {
               if(collide(font, jugador, &enemigosC[contEnemigos].posX, &enemigosC[contEnemigos].posY) == true && iFrames == 0)
               {
                  jugador.vida--; // Resta 1 punto de vida.
                  iFrames = INVINCIBILITY_FRAMES; // Otorga 120 frames de invencibilidad.
               }
            }
         }
         for(contEnemigos = 0; contEnemigos < cantidadEnemigosE; contEnemigos++) // Colision personaje-enemigo E:
         {
            if(fabs(enemigosE[contEnemigos].posX - jugador.posX) <= LARGO * 3 && fabs(enemigosE[contEnemigos].posY - jugador.posY) <= ANCHO * 3)
            {
               if(collide(font, jugador, &enemigosE[contEnemigos].posX, &enemigosE[contEnemigos].posY) == true && iFrames == 0)
               {
                  jugador.vida--; // Resta 1 punto de vida.
                  iFrames = INVINCIBILITY_FRAMES; // Otorga 120 frames de invencibilidad.
               }
            }
         }
         for(contEnemigos = 0; contEnemigos < cantidadEnemigosE; contEnemigos++) // Colision parry propio-enemigo E:
         {
            if(fabs(enemigosE[contEnemigos].posX - jugador.posX) <= LARGO * 3 && fabs(enemigosE[contEnemigos].posY - jugador.posY) <= ANCHO * 3)
            {
               if(collideParry(font, jugador, &enemigosE[contEnemigos].posX, &enemigosE[contEnemigos].posY) == true && parryFrames > 0)
               {
                  al_rest(0.1);
                  al_set_timer_count(tempGravedad, -20);
                  teclaSoltada = true;
                  parryFrames = 0;
                  /*enemigosE[contEnemigos].posX = OUT_OF_BOUNDS;
                  enemigosE[contEnemigos].posY = OUT_OF_BOUNDS;*/
               }
            }
         }

         for(i = 0; i < ANCHO_MAPA; i++) // Revisa las colisiones en cada bloque.
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               puntoX = j*LARGO;
               puntoY = i*ANCHO;
               if(fabs(puntoX - jugador.posX) <= LARGO * 3 && fabs(puntoY - jugador.posY) <= ANCHO * 3) // Revisa solo si la distancia entre el bloque dado y el personaje es menor o igual a cierto rango.
               {
                  if(mapa[i][j] == '#') // Colision personaje-suelo/pared:
                  {
                     if(collide(font, jugador, &puntoX, &puntoY) == true)
                     {
                        // Deshace el movimiento del personaje respecto a la colision.
                        //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 10, 0, "DEBUG: collide = %d", collide(font, jugador, &puntoX, &puntoY));
                        float overlapX = fminf(jugador.posX + LARGO, puntoX + LARGO) - fmaxf(jugador.posX, puntoX);
                        float overlapY = fminf(jugador.posY + ANCHO, puntoY + ANCHO) - fmaxf(jugador.posY, puntoY);

                        if(overlapX < overlapY)
                        {
                           jugador.posX = anularMovimientoX(font, &jugador, jugador.posX, &puntoX);
                        }
                        else
                        {
                           jugador.posY = anularMovimientoY(font, &jugador, jugador.posY, &puntoY, tempGravedad);
                           al_set_timer_count(tempGravedad, 0);
                        }
                        /*if(jugadorEnAire == false || (direccion.Izquierda == false && direccion.Derecha == false))
                        {
                           jugador.posY = anularMovimientoY(font, &jugador, &puntoY);
                           al_set_timer_count(tempGravedad, 0);
                        }
                        if(valorTimerGravedad == 0 && (direccion.Izquierda == true || direccion.Derecha == true))
                        {
                           jugador.posX = anularMovimientoX(font, &jugador, direccion, &puntoX);
                        }*/
                     }
                  }
                  if(mapa[i][j] == '=') // Colision personaje-semiplataforma:
                  {
                     if(collide(font, jugador, &puntoX, &puntoY) == true && jugador.posY <= puntoY && cayendo == true) // Deben chocar, el personaje debe estar mas alto que la plataforma, y el personaje debe estar cayendo estrictamente para abajo.
                     {
                        jugador.posY = anularMovimientoY(font, &jugador, jugador.posY, &puntoY, tempGravedad);
                     }
                  }
                  if(mapa[i][j] == '/') // Colision personaje-pincho:
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
                  if(mapa[i][j] == 'p') // Colision parry propio-objeto parriable:
                  {
                     if(collideParry(font, jugador, &puntoX, &puntoY) == true && parryFrames > 0) // Si se efectua un parry correctamente:
                     {
                        al_rest(0.1);
                        al_set_timer_count(tempGravedad, -20);
                        teclaSoltada = true;
                        parryFrames = 0;
                     }
                  }
                  if(mapa[i][j] == 'H') // Colision personaje-corazon:
                  {
                     if(collide(font, jugador, &puntoX, &puntoY) == true && healCD == 0)
                     {
                        jugador.vida++; // Otorga 1 punto de vida.
                        healCD = INVINCIBILITY_FRAMES;
                     }
                  }
                  if(mapa[i][j] == 'O') // Colision personaje-portal
                  {
                     if(collideParry(font, jugador, &puntoX, &puntoY) == true && direccion.Arriba == true) // Utiliza el cuadrado parry para facilitar la entrada al portal.
                     {
                        al_draw_textf(font, al_map_rgb(255, 255, 255), jugador.posX, jugador.posY + 50, 0, "Transicionando...");
                        nivel++;
                        for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Destruye los timers de los enemigos C:
                        {
                           al_destroy_timer(enemigosC[contEnemigos].tempEnemigosC);
                        }
                        cantidadEnemigosA = 0;
                        cantidadEnemigosC = 0;
                        cantidadEnemigosE = 0;
                        al_rest(1);
                        cargarMapa(CARGADO_DE_MAPA);
                     }
                  }
               }
            }
         }

         if(jugador.vida <= 0) // Logica de game over.
         {
            al_draw_textf(font, al_map_rgb(255, 13, 69), 600,360, 0, "GAME OVER");
            MOVIMIENTO = false;
            GRAVEDAD = false;
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
         if(camaraY < 0)
         {
            camaraY = 0;
         }
         if(camaraX > LARGO_MAPA * LARGO - LARGO_PANTALLA)
         {
            camaraX = LARGO_MAPA * LARGO - LARGO_PANTALLA;
         }
         if(camaraY > ANCHO_MAPA * ANCHO - ANCHO_PANTALLA)
         {
            camaraY = ANCHO_MAPA * ANCHO - ANCHO_PANTALLA;
         }

         // 2: Dibujar el siguiente frame.

         for(i = 0; i < ANCHO_MAPA; i++) // Dibuja el respectivo mapa (PASAR A FUNCION DESPUES)
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               drawX = j*LARGO - camaraX;
               drawY = i*ANCHO - camaraY;
               if(mapa[i][j] == '#') // Suelo
               {
                  //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0, 0, 0.5, 0.2)); // Dibuja el cuadrado suelo.
                  if(i == 0)
                  {
                     al_draw_bitmap(sprites.pasto, drawX, drawY, 0);
                  }
                  if(i - 1 >= 0)
                  {
                     if(mapa[i - 1][j] != '#')  // Dibuja pasto si la casilla superior es vacio.
                     {
                        al_draw_bitmap(sprites.pasto, drawX, drawY, 0);
                     }
                     else // Dibuja tierra si la casilla superior es pasto.
                     {
                        al_draw_bitmap(sprites.tierra, drawX, drawY, 0);
                     }
                  }
               }
               if(mapa[i][j] == '=') // Plataforma atravesable
               {
                  //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0, 0.8, 0.8, 0.2));
                  al_draw_bitmap(sprites.semiplataforma, drawX, drawY, 0);
               }
               if(mapa[i][j] == '/') // Obstaculo
               {
                  //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0.5, 0, 0, 0.2));
                  al_draw_bitmap(sprites.enredadera, drawX, drawY, 0);
               }

               if(mapa[i][j] == 'p') // Parry enemigo
               {
                  //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 0.5, 0.6, 0.2));
                  al_draw_bitmap(sprites.flor, drawX, drawY, 0);
               }
               if(mapa[i][j] == 'H') // Corazon
               {
                  //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0, 0.4, 0, 0.2));
               }
               if(mapa[i][j] == 'O') // Portal
               {
                  //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 0.5, 0, 0.2));
                  al_draw_bitmap(sprites.puerta, drawX, drawY, 0);
               }
               if(mapa[i][j] == 'i') // Punto de inicio
               {
                  //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 1, 1, 0.2));
               }
               if(mapa[i][j] == '+') // Tierra (fondo)
               {
                  al_draw_tinted_bitmap(sprites.tierra, al_map_rgba_f(0.5, 0.5, 0.5, 1), drawX, drawY, 0);
               }
            }
         }

         //if(SPRITES == true)
         for(i = 0; i < MAX_BALAS; i++)
         {
            if(jugador.balas[i].activa == true)
            {
               //drawBalasX
               //drawBalasY
               al_draw_filled_rectangle(jugador.balas[i].posX - camaraX, jugador.balas[i].posY - camaraY, jugador.balas[i].posX - camaraX + LARGO_BALA, jugador.balas[i].posY - camaraY + ANCHO_BALA,al_map_rgb(255, 255, 0));
            }
         }

         for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++) // Enemigos A
         {
            //if(enemigo != OUT_OF_BOUNDS)
            drawEnemigosX = enemigosA[contEnemigos].posX - camaraX;
            drawEnemigosY = enemigosA[contEnemigos].posY - camaraY;
            //al_draw_filled_rectangle(drawEnemigosX, drawEnemigosY, drawEnemigosX + LARGO, drawEnemigosY + ANCHO, al_map_rgba_f(0.8, 0, 0, 0.2));
            if(enemigosA[contEnemigos].direccionMovimientoA == VELOCIDAD_ENEMIGO_A) // Determina la orientacion del sprite respecto a la direccion de movimiento.
            {
               al_draw_bitmap(sprites.enemigoA[enemigosA[contEnemigos].frameA], drawEnemigosX, drawEnemigosY - OFFSET_ENEMIGO_A_Y, ALLEGRO_FLIP_HORIZONTAL);
            }
            else
            {
               al_draw_bitmap(sprites.enemigoA[enemigosA[contEnemigos].frameA], drawEnemigosX, drawEnemigosY - OFFSET_ENEMIGO_A_Y, 0);
            }
            if((int)(valorTimer) % FRAME_RATE_ENEMIGO_A == 0)
            {
               if(enemigosA[contEnemigos].frameA == 0) // Determina la orientacion de ciclado de frames.
               {
                  enemigosA[contEnemigos].cicladoFramesA = 1;
               }
               if(enemigosA[contEnemigos].frameA == FRAMES_ENEMIGO_A - 1)
               {
                  enemigosA[contEnemigos].cicladoFramesA = -1;
               }
               enemigosA[contEnemigos].frameA += enemigosA[contEnemigos].cicladoFramesA; // Cicla a traves de los frames respecto a la orientacion del ciclado.
            }
         }
         for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Enemigos C
         {
            drawEnemigosX = enemigosC[contEnemigos].posX - camaraX;
            drawEnemigosY = enemigosC[contEnemigos].posY - camaraY;
            //al_draw_filled_rectangle(drawEnemigosX, drawEnemigosY, drawEnemigosX + LARGO, drawEnemigosY + ANCHO, al_map_rgba_f(0.4, 0.4, 0.4, 0.2));
            al_draw_rotated_bitmap(sprites.enemigoC, (int)(LARGO / 2), (int)(ANCHO / 2), drawEnemigosX - OFFSET_ENEMIGO_C_X, drawEnemigosY - OFFSET_ENEMIGO_C_Y, -(enemigosC[contEnemigos].posX - enemigosC[contEnemigos].nodoCX) / 200, 0);
         }
         for(contEnemigos = 0; contEnemigos < cantidadEnemigosE; contEnemigos++) // Enemigos E
         {
            drawEnemigosX = enemigosE[contEnemigos].posX - camaraX;
            drawEnemigosY = enemigosE[contEnemigos].posY - camaraY;
            //al_draw_filled_rectangle(drawEnemigosX, drawEnemigosY, drawEnemigosX + LARGO, drawEnemigosY + ANCHO, al_map_rgba_f(1, 0.3, 0.3, 0.2));
            al_draw_rotated_bitmap(sprites.enemigoE, (int)(LARGO / 2), (int)(ANCHO / 2), drawEnemigosX + (int)(LARGO / 2), drawEnemigosY + (int)(ANCHO / 2), valorTimer / SPIN_RATE_ENEMIGO_E, 0);
         }

         if(parryFrames > 0) // Animacion del personaje.
         {
            al_draw_bitmap(sprites.jugador_parry, jugador.posX - camaraX, jugador.posY - camaraY, 0);
         }
         else if(jugadorEnAire == true)
         {
            al_draw_bitmap(sprites.jugador_salto, jugador.posX - camaraX, jugador.posY - camaraY, 0);
         }
         else if(direccion.Izquierda == true || direccion.Derecha == true)
         {
            if(jugador.orientacion == -1)
            {
               al_draw_bitmap(sprites.jugador_corriendo[jugador.frameCorriendo], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_CORRIENDO_Y, ALLEGRO_FLIP_HORIZONTAL);
            }
            else
            {
               al_draw_bitmap(sprites.jugador_corriendo[jugador.frameCorriendo], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_CORRIENDO_Y, 0);
            }
            if((int)(valorTimer) % FRAME_RATE_CORRIENDO == 0)
            {
               jugador.frameCorriendo++; // Aumenta el frame, se devuelve a 0 si se pasa para que siempre cicle hacia la derecha.
               if(jugador.frameCorriendo == FRAMES_CORRIENDO)
               {
                  jugador.frameCorriendo = 0;
               }
            }
         }
         else
         {
            if(jugador.orientacion == -1)
            {
               al_draw_bitmap(sprites.jugador_quieto[jugador.frameQuieto], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_QUIETO_Y, ALLEGRO_FLIP_HORIZONTAL);
            }
            else
            {
               al_draw_bitmap(sprites.jugador_quieto[jugador.frameQuieto], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_QUIETO_Y, 0);
            }
            if((int)(valorTimer) % FRAME_RATE_QUIETO == 0)
            {
               if(jugador.frameQuieto == 0) // Cambia los valores de los frames entre 0 y 1.
               {
                  jugador.frameQuieto++;
               }
               else
               {
                  jugador.frameQuieto--;
               }
            }
         }
         //al_draw_filled_rectangle(x - camaraX, y - camaraY, x - camaraX + LARGO, y - camaraY + ANCHO, al_map_rgb(207, 255, 163));
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
         al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 100, 0, "X: %d", direccion.X);

         for(i = 0; i < ANCHO_MAPA; i++)
         {
            for(j = 0; j < LARGO_MAPA; j++)
            {
               puntoX = j*LARGO;
               puntoY = i*ANCHO;
               if(mapa[i][j] == '#')
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

         for(i = 0; i < MAX_BALAS; i++)
         {
            al_draw_textf(font, al_map_rgb(255, 255, 255), i * 10, 250, 0, "%d,  ", jugador.balas[i].activa);
         }


         // 4. Ajustar ciertas variables al final de un frame.

         direccion.Arriba = false; // Reinicia la direccion (esta se obtiene cada frame).
         direccion.Abajo = false;
         direccion.Izquierda = false;
         direccion.Derecha = false;
         direccion.X = false;

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
         if(dashFrames > 0)
         {
            dashFrames--;
         }
         if(parryFrames > 0)
         {
            parryFrames--;
         }
         if(jugador.disparoCD > 0)
         {
            jugador.disparoCD--;
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
   for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++)
   {
      al_destroy_timer(enemigosC[contEnemigos].tempEnemigosC);
   }
   al_destroy_event_queue(queue);
   sprites_deinit();

   return 0;
}

void must_init(bool test, const char *description)
{
   if(test) return;

   printf("couldn't initialize %s\n", description);
   exit(1);
}

bool generalCollide(float x1, float y1, float largo1, float ancho1, float x2, float y2, float largo2, float ancho2)
{
   int chequeoColision = 0;

   if(x1+largo1>x2)
   {
      chequeoColision++;
   }
   if(x2+largo2>x1)
   {
      chequeoColision++;
   }
   if(y1+ancho1>y2)
   {
      chequeoColision++;
   }
   if(y2+ancho2>y1)
   {
      chequeoColision++;
   }

   if(chequeoColision==4)
   {
      return true;
   }

   return false;
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

float anularMovimientoX(ALLEGRO_FONT* font, entidad *entidad, float posXAnterior, float *sueloX)
{
   if(posXAnterior < *sueloX) // Si venia por la izquierda:
   {
      entidad->posX = *sueloX - LARGO; 
   }
   else // Si venia por la derecha:
   {
      entidad->posX = *sueloX + LARGO; 
   }

   return entidad->posX;
}

float anularMovimientoY(ALLEGRO_FONT* font, entidad *entidad, float posYAnterior, float *sueloY, ALLEGRO_TIMER* tempGravedad)
{
   if(posYAnterior < *sueloY) // Si caia desde arriba:
   {
      entidad->posY = *sueloY - ANCHO; // Se coloca sobre el bloque.
   }
   else // Si subia desde abajo:
   {
      entidad->posY = *sueloY + ANCHO; 
      al_set_timer_count(tempGravedad, 0); // Se golpea la cabeza (empieza a bajar inmediatamente).
      //bonksoundeffect
   }

   return entidad->posY;
}

void cargarMapa(VARIABLES_CARGARMAPA)
//void cargarMapa(char *ruta_archivo, entidad jugador)
{
   FILE *contenidoMapa;
   int i, j; 
   char valorRecibido;

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

      case 3:
      contenidoMapa = fopen("mapa3.txt", "r");
      must_init(contenidoMapa, "mapa3");
      break;

      default:
      printf("no existe el nivel\n");
      exit(1);
   }

   for(i = 0; i < ANCHO_MAPA; i++)
   {
      for(j = 0; j < LARGO_MAPA; j++)
      {
         if(fscanf(contenidoMapa, " %c", &valorRecibido) != EOF)
         {
            mapa[i][j] = valorRecibido;
            if(valorRecibido == 'A') // Define las variables miembro de las casillas del arreglo de enemigos
            {
               if(*cantidadEnemigosA < MAX_ENEMIGOS)
               {
                  // revisar esto, por ejemplo: si coloco mas enemigos que el maximo, se cae
                  // la solucion: llegado a esta parte, recorrer el arreglo de enemigos y buscar un espacio para el nuevo enemigo
                  //tipo_enemigo
                  enemigosA[*cantidadEnemigosA].posX = j*LARGO;
                  enemigosA[*cantidadEnemigosA].posY = i*ANCHO;
                  enemigosA[*cantidadEnemigosA].vida = VIDA_ENEMIGO_A;
                  enemigosA[*cantidadEnemigosA].direccionMovimientoA = -VELOCIDAD_ENEMIGO_A;
                  enemigosA[*cantidadEnemigosA].colisionEnemigoA = false; // No es necesario inicializar puntoColisionA.
                  enemigosA[*cantidadEnemigosA].frameA = 2;
                  enemigosA[*cantidadEnemigosA].cicladoFramesA = 1;
                  enemigosA[*cantidadEnemigosA].contadorSaltitoA = -1;
                  (*cantidadEnemigosA)++;
               }
               else
               {
                  printf("Advertencia: Hay mas enemigos A que los que soporta el arreglo\n");
               }
            }
            if(valorRecibido == 'C')
            {
               if(*cantidadEnemigosC < MAX_ENEMIGOS)
               {
                  enemigosC[*cantidadEnemigosC].posX = j*LARGO;
                  enemigosC[*cantidadEnemigosC].posY = i*ANCHO;
                  enemigosC[*cantidadEnemigosC].nodoCY = enemigosC[*cantidadEnemigosC].posY - 80;
                  enemigosC[*cantidadEnemigosC].nodoCX = enemigosC[*cantidadEnemigosC].posX;
                  enemigosC[*cantidadEnemigosC].tempEnemigosC = al_create_timer(1.0 / TARGET_FPS); // Temporizador de posicion de enemigos C.
                  must_init(enemigosC[*cantidadEnemigosC].tempEnemigosC, "tempEnemigosC");
                  (*cantidadEnemigosC)++;
               }
               else
               {
                  printf("Advertencia: Hay mas enemigos C que los que soporta el arreglo\n");
               }
            }
            if(valorRecibido == 'E')
            {
               if(*cantidadEnemigosE < MAX_ENEMIGOS)
               {
                  enemigosE[*cantidadEnemigosE].posX = j*LARGO;
                  enemigosE[*cantidadEnemigosE].posY = i*ANCHO;
                  enemigosE[*cantidadEnemigosE].nodoE = enemigosE[*cantidadEnemigosE].posY;
                  (*cantidadEnemigosE)++;
               }
               else
               {
                  printf("Advertencia: Hay mas enemigos E que los que soporta el arreglo\n");
               }
            }
            if(valorRecibido == 'i') // Define el punto de inicio del jugador en el caso del cargado de una casilla 'i'
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

   fclose(contenidoMapa);

   return;
}

ALLEGRO_BITMAP* sprite_grab(int x, int y, int largo, int ancho)
{
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._sheet, x, y, largo, ancho);
    must_init(sprite, "sprite grab");
    return sprite;
}

void sprites_init()
{
   sprites._sheet = al_load_bitmap("Sprite-0001.png");
   must_init(sprites._sheet, "spritesheet");

   sprites.tierra = sprite_grab(0, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.pasto = sprite_grab(LARGO, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.semiplataforma = sprite_grab(LARGO * 2, 0, LARGO_SEMIPLATAFORMA, ANCHO_SEMIPLATAFORMA);

   sprites.enredadera = sprite_grab(LARGO * 3, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.flor = sprite_grab(LARGO * 4, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.puerta = sprite_grab(LARGO * 5, 0, LARGO_BLOQUE, ANCHO_BLOQUE);

   sprites.enemigoA[0] = sprite_grab(0, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[1] = sprite_grab(LARGO, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[2] = sprite_grab(LARGO * 2, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[3] = sprite_grab(LARGO * 3, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[4] = sprite_grab(LARGO * 4, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[5] = sprite_grab(LARGO * 5, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[6] = sprite_grab(LARGO * 6, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);      

   sprites.enemigoC = sprite_grab(LARGO * 7, 52, LARGO_ENEMIGO_C, ANCHO_ENEMIGO_C);
   sprites.enemigoE = sprite_grab(LARGO * 7, 0, LARGO_ENEMIGO_E, ANCHO_ENEMIGO_E);

   sprites.jugador_quieto[0] = sprite_grab(0, 164, 52, ANCHO_BLOQUE * 2);
   sprites.jugador_quieto[1] = sprite_grab(56, 164, 52, ANCHO_BLOQUE * 2);

   sprites.jugador_corriendo[0] = sprite_grab(0, 252, 72, 88);
   sprites.jugador_corriendo[1] = sprite_grab(76, 252, 60, 88);
   sprites.jugador_corriendo[2] = sprite_grab(140, 252, 64, 88);
   sprites.jugador_corriendo[3] = sprite_grab(208, 252, 56, 88);
   sprites.jugador_corriendo[4] = sprite_grab(268, 252, 68, 88);

   sprites.jugador_salto = sprite_grab(LARGO * 4, ANCHO, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_parry = sprite_grab(LARGO * 5, ANCHO, LARGO_BLOQUE, ANCHO_BLOQUE);

   return;
}

void sprites_deinit()
{
   al_destroy_bitmap(sprites.tierra);
   al_destroy_bitmap(sprites.pasto);
   al_destroy_bitmap(sprites.semiplataforma);

   al_destroy_bitmap(sprites.enredadera);
   al_destroy_bitmap(sprites.flor);
   al_destroy_bitmap(sprites.puerta);

   al_destroy_bitmap(sprites.enemigoA[0]);
   al_destroy_bitmap(sprites.enemigoA[1]);
   al_destroy_bitmap(sprites.enemigoA[2]);
   al_destroy_bitmap(sprites.enemigoA[3]);
   al_destroy_bitmap(sprites.enemigoA[4]);
   al_destroy_bitmap(sprites.enemigoA[5]);
   al_destroy_bitmap(sprites.enemigoA[6]);
   al_destroy_bitmap(sprites.enemigoC);
   al_destroy_bitmap(sprites.enemigoE);

   al_destroy_bitmap(sprites.jugador_quieto[0]);
   al_destroy_bitmap(sprites.jugador_quieto[1]);
   al_destroy_bitmap(sprites.jugador_corriendo[0]);
   al_destroy_bitmap(sprites.jugador_corriendo[1]);
   al_destroy_bitmap(sprites.jugador_corriendo[2]);
   al_destroy_bitmap(sprites.jugador_corriendo[3]);
   al_destroy_bitmap(sprites.jugador_corriendo[4]);
   al_destroy_bitmap(sprites.jugador_salto);
   al_destroy_bitmap(sprites.jugador_parry);

   al_destroy_bitmap(sprites._sheet);
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
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

#include <allegro5/allegro_ttf.h>
#include <allegro5/altime.h> // de inmediato sigue sprites de enemigo muerto (explosion) y de moneda y moneda recogida
#include <allegro5/bitmap.h> // hacer que se muestre un sprite de secreto encima de pasto en zonas de secreto
#include <allegro5/bitmap_draw.h> // ajustar tamanho interno de flores
#include <allegro5/events.h>
#include <allegro5/keycodes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro5.h> // PROBLEMAS:
#include <allegro5/color.h> // El salto permite que cuando el jugador se caiga al fondo de la pantalla, pueda saltar de nuevo cuando valorTimerGravedad es 0.
#include <allegro5/timer.h> // El parry no tiene cooldown despues de fallarlo.
#include <allegro5/allegro_audio.h> // La funcion anularMovimientoY lleva al personaje al tope del bloque.
#include <allegro5/allegro_acodec.h> // Se descoloca el cuadrado parry al moverse (puede ser por el scrolling con la camara o por la falta de optimizacion de revision de colision de enemigos)
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h> // Sigue: Interaccion con otros elementos (llaves, corazones extra), pasar enemigos a un arreglo de estructura, implementar joystick
#include <allegro5/allegro_native_dialog.h>

#define KEY_SEEN     1
#define KEY_RELEASED 2

#define LARGO 40 // Largo de un bloque
#define ANCHO 40 // Ancho de un bloque

#define LARGO_MAPA 160 // Largo de un subnivel. Normalmente caben 32 bloques en la pantalla
#define ANCHO_MAPA 18

#define SPEED_FACTOR 8
#define TERMINAL_VELOCITY 30 // 30 px/seg.
#define VIDA_INICIAL 5
#define INVINCIBILITY_FRAMES 120
#define DASH_FRAMES 30
#define DASH_SPEED 21
#define PARRY_FRAMES 32

#define MAX_ENEMIGOS 100
#define MAX_BALAS 50
#define VELOCIDAD_BALA 18
#define VELOCIDAD_CHASER 9
#define LARGO_BALA 8
#define ANCHO_BALA 8
#define DANHO_BALA 5
#define VARIACION_ALTURA_BALA 10
#define COOLDOWN_DISPARO_0 10

#define VIDA_ENEMIGO_A 10
#define VELOCIDAD_ENEMIGO_A 4
#define PUNTAJE_ENEMIGO_A 200

#define VIDA_ENEMIGO_C 5
#define HORIZONTAL_OSCILLATION_RANGE_C 75
#define VERTICAL_OSCILLATION_RANGE_C 25
#define OSCILLATION_SPEED_C 25
#define FALLING_SPEED_C 1.5
#define PUNTAJE_ENEMIGO_C 400

#define OSCILLATION_RANGE_E 125
#define OSCILLATION_SPEED_E 25
#define PUNTAJE_ENEMIGO_E 400

#define MONEDAS_JUEGO 15
#define MAX_MONEDAS 5

#define VIDA_DIANA 25
#define MAX_DIANAS 10

#define REGRESO_DE_PORTAL -200

#define MAX_SOMBREROS 10
#define LARGO_SOMBRERO 200
#define ANCHO_SOMBRERO 200
#define ATRAVESAR_SOMBRERO_CD 60

#define LARGO_BLOQUE 40
#define ANCHO_BLOQUE 40
#define LARGO_SEMIPLATAFORMA 40
#define ANCHO_SEMIPLATAFORMA 20
#define CENTRO_FLOR_X 30
#define CENTRO_FLOR_Y 30
#define ANGULO_BASE_FLOR 0.025
#define SPIN_RATE_FLOR 25
#define FRAMES_MONEDA 6
#define FRAME_RATE_MONEDA 5
#define LARGO_ENEMIGO_A 40
#define ANCHO_ENEMIGO_A 48
#define FRAMES_ENEMIGO_A 7
#define FRAME_RATE_ENEMIGO_A 5
#define LARGO_ENEMIGO_C 64
#define ANCHO_ENEMIGO_C 88
#define LARGO_ENEMIGO_E 48
#define ANCHO_ENEMIGO_E 48
#define CENTRO_ENEMIGO_E_X 24
#define CENTRO_ENEMIGO_E_Y 24
#define SPIN_RATE_ENEMIGO_E 10
#define FRAMES_EXPLOSION 4
#define FRAME_RATE_EXPLOSION 5
#define FRAMES_QUIETO 2
#define FRAME_RATE_QUIETO 20
#define FRAMES_LOCK 2
#define FRAME_RATE_LOCK 20
#define FRAMES_CORRIENDO 5 // Para personaje corriendo y disparando o no.
#define FRAME_RATE_CORRIENDO 5
#define FRAMES_DISPARO 3
#define FRAME_RATE_DISPARO 5
#define FRAMES_SALTO_PARRY 4
#define FRAME_RATE_SALTO_PARRY 8
#define CENTRO_BALA_X 4
#define CENTRO_BALA_Y 4

#define OFFSET_TOPE_Y 12
#define OFFSET_FLOR_X -20
#define OFFSET_FLOR_Y -20
#define OFFSET_META_X 28
#define OFFSET_META_Y 56
#define OFFSET_ENEMIGO_A_Y 8
#define OFFSET_ENEMIGO_C_X -10
#define OFFSET_ENEMIGO_C_Y 10
#define OFFSET_ENEMIGO_E_X 4
#define OFFSET_ENEMIGO_E_Y 4
#define OFFSET_EXPLOSION_X 44
#define OFFSET_EXPLOSION_Y 44
#define OFFSET_QUIETO_X 4
#define OFFSET_QUIETO_Y 52
#define OFFSET_CORRIENDO_X 4
#define OFFSET_CORRIENDO_Y 48
#define OFFSET_CORRIENDO_DISPARO_HORIZONTAL_Y 48
#define OFFSET_CORRIENDO_DISPARO_DIAGONAL_Y 44
#define OFFSET_DISPARO_X 12
#define OFFSET_DISPARO_DIAGONAL_ARRIBA_Y 40
#define OFFSET_DISPARO_DIAGONAL_ABAJO_Y 40
#define OFFSET_DISPARO_ARRIBA_Y 60
#define OFFSET_DISPARO_ABAJO_Y 56
#define OFFSET_DISPARO_HORIZONTAL_X 4
#define OFFSET_DISPARO_HORIZONTAL_Y 52
#define OFFSET_AGACHADO_X 28
#define OFFSET_AGACHADO_Y 4
#define OFFSET_AGACHADO_DISPARO_Y 8
#define OFFSET_DASH_X 28
#define OFFSET_DASH_SUELO_Y 36 // (mas alto de lo necesario para que de el efecto de vuelo)
#define OFFSET_DASH_AIRE_Y 28
#define OFFSET_SALTO_X 8
#define OFFSET_SALTO_Y 16
#define OFFSET_PARRY_X 20
#define OFFSET_PARRY_Y 28
#define OFFSET_GOLPE_X 52
#define OFFSET_GOLPE_Y 84

#define LARGO_PORTADA_ZONA_1 204
#define ANCHO_PORTADA_ZONA_1 128
#define LARGO_PORTADA_ZONA_2 228
#define ANCHO_PORTADA_ZONA_2 152
#define OFFSET_PORTADA_ZONA_X 40
#define OFFSET_PORTADA_ZONA_Y 40

#define POS_X_ZONA_1 760
#define POS_Y_ZONA_1 240
#define POS_X_ZONA_2 760
#define POS_Y_ZONA_2 240

#define MAX_PUNTAJES 10
#define MAX_CARACTERES 10

#define PENALIZACION_GOLPE 1000
#define FRAMES_GOLPE 15
#define ALTURA_GOLPE -8

#define DURACION_VICTORIA 120

#define OPCIONES_TIENDA 4
#define ARMAS_TIENDA 2
#define PRECIO_ARMA 4

#define VARIABLES_CARGARMAPA char mapa[ANCHO_MAPA][LARGO_MAPA], entidad monedasMapa[MAX_MONEDAS], int *cantidadMonedas, entidad dianas[MAX_DIANAS], int *cantidadDianas, portal *portalSalida, sombreros sombrerosHorizontales[MAX_SOMBREROS], sombreros sombrerosVerticales[MAX_SOMBREROS], int *cantidadSombrerosHorizontalesEntrada, int *cantidadSombrerosHorizontalesSalida, int *cantidadSombrerosVerticalesEntrada, int *cantidadSombrerosVerticalesSalida, int nivel, int zona, bool *GRAVEDAD, entidad *jugador, entidad enemigosA[MAX_ENEMIGOS], int *cantidadEnemigosA, entidad enemigosC[MAX_ENEMIGOS], int *cantidadEnemigosC, entidad enemigosE[MAX_ENEMIGOS], int *cantidadEnemigosE
#define CARGADO_DE_MAPA mapa, monedasMapa, &cantidadMonedas, dianas, &cantidadDianas, &portalSalida, sombrerosHorizontales, sombrerosVerticales, &cantidadSombrerosHorizontalesEntrada, &cantidadSombrerosHorizontalesSalida, &cantidadSombrerosVerticalesEntrada, &cantidadSombrerosVerticalesSalida, nivel, zona, &GRAVEDAD, &jugador, enemigosA, &cantidadEnemigosA, enemigosC, &cantidadEnemigosC, enemigosE, &cantidadEnemigosE

typedef struct
{
   bool Arriba;
   bool Abajo;
   bool Izquierda;
   bool Derecha;
   bool Espacio;
   bool X;
   bool Z;
}
entrada;

typedef struct
{
   float posX;
   float posY;
   float velocidad;
   entrada direccion;
   int numeroRebotes;
   float angulo;
   float offsetX;
   float offsetY;
   bool activa;
}
bala;

typedef struct
{
   float posX; // Generales
   float posY;
   int vida;

   float posParryX; // Exclusivos al jugador
   float posParryY;
   entrada direccion;
   int monedas;
   bala balas[MAX_BALAS];
   float variacionAlturaBala;
   int direccionVariacionBala;
   int disparoCD;
   bool agachado;
   bool lock;
   int direccionDash;
   int cooldownPortal; // Cooldown para prevenir entrar a un portal inmediatamente despues de entrar por otro.
   int orientacion;
   int armaEquipada;
   int frameQuieto;
   int frameLock;
   int frameCorriendo;
   int frameDisparo;
   int frameSaltoParry;

   bool activo; // Interruptor que decide si el enemigo o la moneda esta activa o no.
   int frame;
   int frameExplosion;

   int direccionMovimientoA; // Exclusivos a los enemigos
   bool colisionEnemigoA;
   float puntoColisionA;
   int frameA;
   int cicladoFramesA;

   ALLEGRO_TIMER* tempEnemigosC;
   float valorTimerEnemigosC;
   float nodoCX;
   float nodoCY;

   float nodoE;
}
entidad;

typedef struct
{
   float posX;
   float posY;
   bool activo;
}
portal;

typedef struct
{
   entidad entrada;
   entidad salida;
}
sombreros;

typedef struct
{
   ALLEGRO_BITMAP* _sheet;
   ALLEGRO_BITMAP* fondo1;
   ALLEGRO_BITMAP* fondo2;
   ALLEGRO_BITMAP* fondo_menu;

   ALLEGRO_BITMAP* tierra;
   ALLEGRO_BITMAP* pasto;
   ALLEGRO_BITMAP* agua;
   ALLEGRO_BITMAP* tope_agua;

   ALLEGRO_BITMAP* madera;
   ALLEGRO_BITMAP* alfombra;
   ALLEGRO_BITMAP* pinchos;
   ALLEGRO_BITMAP* tope_pinchos;

   ALLEGRO_BITMAP* bloque_tutorial;
   ALLEGRO_BITMAP* semiplataforma_tutorial;

   ALLEGRO_BITMAP* moneda[FRAMES_MONEDA];

   ALLEGRO_BITMAP* semiplataforma;
   ALLEGRO_BITMAP* flor;
   ALLEGRO_BITMAP* puerta;
   ALLEGRO_BITMAP* portal_local;
   ALLEGRO_BITMAP* sombrero_entrada;
   ALLEGRO_BITMAP* sombrero_salida;
   ALLEGRO_BITMAP* meta;

   ALLEGRO_BITMAP* enemigoA[FRAMES_ENEMIGO_A];
   ALLEGRO_BITMAP* enemigoC;
   ALLEGRO_BITMAP* enemigoE;
   ALLEGRO_BITMAP* enemigoE_pinchos;

   ALLEGRO_BITMAP* explosion[FRAMES_EXPLOSION];

   ALLEGRO_BITMAP* jugador_quieto[FRAMES_QUIETO];
   ALLEGRO_BITMAP* jugador_lock[FRAMES_LOCK];

   ALLEGRO_BITMAP* jugador_dash;

   ALLEGRO_BITMAP* jugador_corriendo[FRAMES_CORRIENDO];
   ALLEGRO_BITMAP* jugador_corriendo_disparo_horizontal[FRAMES_CORRIENDO];
   ALLEGRO_BITMAP* jugador_corriendo_disparo_diagonal[FRAMES_CORRIENDO];

   ALLEGRO_BITMAP* jugador_disparo_horizontal[FRAMES_DISPARO];
   ALLEGRO_BITMAP* jugador_disparo_diagonal_arriba[FRAMES_DISPARO];
   ALLEGRO_BITMAP* jugador_disparo_diagonal_abajo[FRAMES_DISPARO];
   ALLEGRO_BITMAP* jugador_disparo_arriba[FRAMES_DISPARO];
   ALLEGRO_BITMAP* jugador_disparo_abajo[FRAMES_DISPARO];

   ALLEGRO_BITMAP* jugador_agachado;
   ALLEGRO_BITMAP* jugador_agachado_disparo[FRAMES_DISPARO];

   ALLEGRO_BITMAP* jugador_salto[FRAMES_SALTO_PARRY];
   ALLEGRO_BITMAP* jugador_parry[FRAMES_SALTO_PARRY];

   ALLEGRO_BITMAP* jugador_golpe;

   ALLEGRO_BITMAP* bala0;
   ALLEGRO_BITMAP* bala1;
   ALLEGRO_BITMAP* bala2;
   ALLEGRO_BITMAP* bala3;

   ALLEGRO_BITMAP* pasto_mapa;
   ALLEGRO_BITMAP* arbol_mapa;
   ALLEGRO_BITMAP* jugador_mapa_quieto;
   ALLEGRO_BITMAP* jugador_mapa_der;
   ALLEGRO_BITMAP* jugador_mapa_arrder;
   ALLEGRO_BITMAP* jugador_mapa_arr;
   ALLEGRO_BITMAP* jugador_mapa_arrizq;
   ALLEGRO_BITMAP* jugador_mapa_izq;
   ALLEGRO_BITMAP* jugador_mapa_abaizq;
   ALLEGRO_BITMAP* jugador_mapa_aba;
   ALLEGRO_BITMAP* jugador_mapa_abader;
   ALLEGRO_BITMAP* portada_zona_1;
   ALLEGRO_BITMAP* portada_zona_2;
   ALLEGRO_BITMAP* tienda;

   ALLEGRO_BITMAP* logo_peashooter;
   ALLEGRO_BITMAP* logo_chaser;
   ALLEGRO_BITMAP* logo_roundabout;
   ALLEGRO_BITMAP* logo_ricochet;
}
_sprites;
_sprites sprites;

typedef struct
{
   char nombre[MAX_CARACTERES];
   int puntaje;
}
_puntaje;

typedef struct
{
   bool activo;
   int seleccion;
   const char *opciones[OPCIONES_TIENDA];
}
Menu;
 
Menu menutienda = {.activo = false, .seleccion = 0, .opciones = {"Salir", "Peashooter", "Chaser", "Roundabout"}};
 
void abrir_menu(Menu *m) 
{
   m->activo = true;
   m->seleccion = 0;
}
 
void manejar_input_menu(Menu *m, ALLEGRO_EVENT *ev, entidad *jugador, bool armas[ARMAS_TIENDA]) 
{
   if(!m->activo) 
   {
      return;
   }
 
   if(ev->type == ALLEGRO_EVENT_KEY_DOWN) 
   {
      switch(ev->keyboard.keycode) 
      {
         case ALLEGRO_KEY_UP:
         m->seleccion = (m->seleccion - 1 + OPCIONES_TIENDA) % OPCIONES_TIENDA;
         break;
 
         case ALLEGRO_KEY_DOWN:
         m->seleccion = (m->seleccion + 1) % OPCIONES_TIENDA;
         break;
 
         case ALLEGRO_KEY_ENTER:
         // Ejecutar accion según m->seleccion
         if (m->seleccion == 0) 
         {
            m->activo = false; // Reanudar
         }
         else if(m->seleccion == 1)
         {
            jugador->armaEquipada = 0;
         }
         else if(m->seleccion == 2) 
         { /* !? chaser */ 
            if(armas[0] == true && jugador->monedas >= PRECIO_ARMA)
            {
               jugador->monedas -= PRECIO_ARMA;
               jugador->armaEquipada = 1;
               armas[0] = false;
            }
            if(armas[0] == false)
            {
               jugador->armaEquipada = 1;
            }
         }
         else if(m->seleccion == 3) 
         { /* !? roundabout */
            if(armas[1] == true && jugador->monedas >= PRECIO_ARMA)
            {
               jugador->monedas -= PRECIO_ARMA;
               jugador->armaEquipada = 2;
               armas[1] = false;
            }
            if(armas[1] == false)
            {
               jugador->armaEquipada = 2;
            }
         }
         break;
 
         case ALLEGRO_KEY_ESCAPE:
         m->activo = false; // cerrar sin elegir
         break;
      }
   }
}
 
void dibujar_menu(Menu *m, ALLEGRO_FONT *font, entidad jugador, bool armas[ARMAS_TIENDA]) 
{
   int i;
 
   if(!m->activo) 
   {
      return;
   }
   // Fondo semitransparente
   al_draw_filled_rectangle(400, 200, 880, 520, al_map_rgba(0, 0, 0, 180));
   al_draw_rectangle(400, 200, 880, 520, al_map_rgb(255,255,255), 2);
 
   for(i = 0; i < OPCIONES_TIENDA; i++)
   {
      ALLEGRO_COLOR colorDisponible = (i == m->seleccion)
         ? al_map_rgb(255, 255, 0)   // resaltado
         : al_map_rgb(255, 255, 255);
      ALLEGRO_COLOR colorComprado = (i == m->seleccion)
         ? al_map_rgb(122, 122, 0)   // resaltado
         : al_map_rgb(255, 255, 255);
      if(i == 0) // Salir: siempre se dibuja, sin estado de compra
      {
         al_draw_text(font, colorDisponible, 500, 300 + i * 40, ALLEGRO_ALIGN_CENTER, m->opciones[i]);
      }
      else if(i == 1) // Peashooter: arma inicial, siempre disponible, no usa armas[]
      {
         if(jugador.armaEquipada == 0)
         {
            al_draw_textf(font, colorComprado, 500, 300 + i * 40, ALLEGRO_ALIGN_CENTER, "%s [EQUIPADO]", m->opciones[i]);
         }
         else
         {
            al_draw_text(font, colorComprado, 500, 300 + i * 40, ALLEGRO_ALIGN_CENTER, m->opciones[i]);
         }
      }
      else // Chaser, Roundabout, Ricochet -> armas[0], armas[1], armas[2]
      {
         if(armas[i - 2] == true)
         {
            al_draw_text(font, colorDisponible, 500, 300 + i * 40, ALLEGRO_ALIGN_CENTER, m->opciones[i]);
         }
         else if(jugador.armaEquipada == i - 1)
         {
            al_draw_textf(font, colorComprado, 500, 300 + i * 40, ALLEGRO_ALIGN_CENTER, "%s [EQUIPADO]", m->opciones[i]);
         }
         else
         {
            al_draw_text(font, colorComprado, 500, 300 + i * 40, ALLEGRO_ALIGN_CENTER, m->opciones[i]);
         }
      }
   }
   al_draw_textf(font, al_map_rgb(255, 255, 255), 700, 330, ALLEGRO_ALIGN_CENTER, "DESCRIPCION:");
   switch (m->seleccion)
   {
      case 1:
      al_draw_textf(font, al_map_rgb(255, 255, 255), 700, 350, ALLEGRO_ALIGN_CENTER, "Tu tipica arma de uso comun.");
      al_draw_textf(font, al_map_rgb(255, 255, 255), 700, 360, ALLEGRO_ALIGN_CENTER, "Danho promedio hacia donde apuntes.");
      break;
      case 2:
      al_draw_textf(font, al_map_rgb(255, 255, 255), 700, 350, ALLEGRO_ALIGN_CENTER, "Gran rango a coste de menor danho.");
      al_draw_textf(font, al_map_rgb(255, 255, 255), 700, 360, ALLEGRO_ALIGN_CENTER, "No requiere apuntar.");
      break;
      case 3:
      al_draw_textf(font, al_map_rgb(255, 255, 255), 700, 350, ALLEGRO_ALIGN_CENTER, "Gran cobertura con danho promedio.");
      al_draw_textf(font, al_map_rgb(255, 255, 255), 700, 360, ALLEGRO_ALIGN_CENTER, "Apunta hacia atras para el mayor rango.");
      break;
   }
}

// crear estructura balas o municion
// definir una cantidad de balas para el jugador, de tal manera de ir recargando las balas
void must_init(bool test, const char *description);

bool generalCollide(float x1, float y1, float largo1, float ancho1, float x2, float y2, float largo2, float ancho2);
bool collide(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
bool collideAnticipado(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
bool collideParry(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
bool collideSuelo(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY);
float anularMovimientoX(entidad *entidad, float posXAnterior, float *sueloX);
float anularMovimientoY(entidad *entidad, float posYAnterior, float *sueloY, ALLEGRO_TIMER* tempGravedad);

bool balaDiagonal(entidad jugador, int numeroBala);
entidad obtenerEnemigoMasCercano(entidad jugador, entidad enemigosA[MAX_ENEMIGOS], int cantidadEnemigosA, entidad enemigosC[MAX_ENEMIGOS], int cantidadEnemigosC);

void cargarMapa(VARIABLES_CARGARMAPA);

void logicaSombrerosHorizontalesEntrada(sombreros sombrerosVerticales[MAX_SOMBREROS], entidad entidad, int nivel, int cont);
void logicaSombrerosHorizontalesSalida(sombreros sombrerosVerticales[MAX_SOMBREROS], entidad entidad, int nivel, int cont);
void logicaSombrerosVerticalesEntrada(sombreros sombrerosVerticales[MAX_SOMBREROS], entidad entidad, int nivel, int cont);
void logicaSombrerosVerticalesSalida(sombreros sombrerosVerticales[MAX_SOMBREROS], entidad entidad, int nivel, int cont);

void golpearJugador(int *vida, int *iFrames, int *puntajeZona, int *dashFrames, int *parryFrames, int *golpeFrames, ALLEGRO_TIMER* tempGravedad);

void inicializarRanking(_puntaje puntajes[MAX_PUNTAJES], int *cantidadPuntajes);
// escribir funcion inicializarpuntaje que asigne al arreglo de puntajes los numeros en ranking.txt
void actualizarPuntaje(_puntaje ranking[MAX_PUNTAJES], char nombreIngresado[MAX_CARACTERES], int puntajeIngresado, int *cantidadPuntajes);

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
   ALLEGRO_TIMER* tempVictoria = al_create_timer(1.0 / TARGET_FPS); // Temporizador de posicion de enemigos E.
   must_init(tempVictoria, "tempVictoria");

   ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
   must_init(queue, "queue");

   al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST); //
   al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST); // **** ANTIALIASING ****
   al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR); //

   ALLEGRO_DISPLAY* disp = al_create_display(LARGO_PANTALLA, ANCHO_PANTALLA);
   must_init(disp, "display");

   ALLEGRO_FONT* font = al_create_builtin_font();
   must_init(font, "font");

   must_init(al_init_font_addon(), "font addon");
   must_init(al_init_ttf_addon(), "ttf");

   ALLEGRO_FONT* font48 = al_load_font("BigPixelDemo-75ww.ttf", 48, 0);
   must_init(font48, "font20");

   ALLEGRO_MENU* menu = al_create_menu();
   must_init(menu, "menu");

   must_init(al_init_primitives_addon(), "primitives");

   must_init(al_init_native_dialog_addon(), "dialog addon");

   must_init(al_init_image_addon(), "image addon");
   sprites_init();

    /*ALLEGRO_BITMAP* mysha = al_load_bitmap("mysha.png");
    must_init(mysha, "mysha");*/
   al_register_event_source(queue, al_get_keyboard_event_source());
   al_register_event_source(queue, al_get_display_event_source(disp));
   al_register_event_source(queue, al_get_timer_event_source(timer));

   bool MOVIMIENTO = true; // Variables interruptor para activar y desactivar ciertas propiedades del juego.
   bool GRAVEDAD = true;
   bool DANHO = false;
   bool DISPARO = false;
   bool HITBOXES = false;
   bool SPRITES = true;
   bool DEBUG = false;

   bool enMenu = true;   // !? Empieza en el menú al iniciar el juego.
   int opcionMenu = 0;   // 0 = "Jugar", 1 = "Salir"
   bool ingresandoNombre = false; // true mientras se escribe el nombre en pantalla, dentro del menú.

   bool done = false; // Cierra el juego cuando esta como true.
   bool redraw = true;
   ALLEGRO_EVENT event;

   entidad jugador;
   jugador.agachado = false;
   jugador.lock = false;
   jugador.orientacion = 1;
   jugador.vida = VIDA_INICIAL;
   jugador.variacionAlturaBala = 0;
   jugador.direccionVariacionBala = VARIACION_ALTURA_BALA;
   jugador.direccion.Arriba = false;
   jugador.direccion.Abajo = false;
   jugador.direccion.Izquierda = false;
   jugador.direccion.Derecha = false;
   jugador.direccion.Espacio = false;
   jugador.direccion.X = false;
   jugador.monedas = 3;
   jugador.armaEquipada = 0;
   jugador.frameQuieto = 0;
   jugador.frameLock = 0;
   jugador.frameCorriendo = 0;
   jugador.frameDisparo = 0;
   jugador.frameSaltoParry = 0;

   entidad enemigosA[MAX_ENEMIGOS]; // Se declaran los arreglos de enemigos con sus cantidades.
   int cantidadEnemigosA = 0;
   entidad enemigosC[MAX_ENEMIGOS];
   int cantidadEnemigosC = 0;
   float valorTimerEnemigosC; // Obtiene el valor del temporizador de posicion de enemigos C.
   entidad enemigosE[MAX_ENEMIGOS];
   int cantidadEnemigosE = 0;
   float valorTimerEnemigosE; // Obtiene el valor del temporizador de posicion de enemigos E.

   entidad monedasMapa[MAX_MONEDAS];
   int cantidadMonedas = 0;

   entidad dianas[MAX_DIANAS];
   int cantidadDianas = 0;
   //bool monedasActivas[MONEDAS_JUEGO];

   int iFrames = 0;
   int dashFrames = 0;
   int parryFrames = 0;
   int golpeFrames = 0;
   int healCD = 0;

   int x = 480;
   int y = 480;
   //float theta;

   float valorTimer; // Obtiene el valor del timer de frames.
   int valorTimerGravedad; // Obtiene el valor del temporizador de gravedad.
   bool jugadorEnAire = true; // Revisa si esta cayendo el cuadrado personaje.
   float posYAnteriorFrame;
   bool cayendo = true; // Bandera que permite o denega el choque con semiplataformas.
   bool primeraVezSalto = false; // explicar luego

   bool primeraVezDash = false; // Evita que se pueda hacer dash hasta que se toque el piso de nuevo.

   bool teclaSoltada = false; // Se activa cuando la tecla es soltada en el aire.
   bool puedeHacerParry = true; // Forma parte de las condiciones para hacer parry.

   int i, j, cont, contEnemigos; // Contadores generales reutilizables.
   float puntoX, puntoY; // Reciben los valores de i y j para traspasarlos a variables flotantes que puedan ser traspasadas a las funciones de colision.

   int nivel = 0, zona; // Numero de nivel.

   bool zona1Completada = false;
   bool zona2Completada = false;
   bool zona3Completada = false;

   portal portalSalida; // Sostiene la posicion del portal de salida.

   float desplazamientoBala; // Desplaza la bala, se reduce si el disparo es diagonal.

   float camaraX = 0; // Variables de camara.
   float camaraY = 0;
   float drawX, drawY, drawEnemigosX, drawEnemigosY, drawBalasX, drawBalasY, offsetBalaX, offsetBalaY; // Con camara(x, y), determinan la posicion en la pantalla para dibujar las entidades.

   float anguloFlores = 0.05; // Angulo para dibujar las flores.
   int frameMeta = 0;

   float transparenciaBloque = 1; // Determina la transparencia del bloque de secreto.
   bool atravesando = false; // Bandera de atravieso de bloques de secreto.
   float colorSecreto; // Determina la apariencia y color resultante de los bloques de secreto.

   sombreros sombrerosHorizontales[MAX_SOMBREROS];
   sombreros sombrerosVerticales[MAX_SOMBREROS];
   int cantidadSombrerosHorizontalesEntrada = 0;
   int cantidadSombrerosHorizontalesSalida = 0;
   int cantidadSombrerosVerticalesEntrada = 0;
   int cantidadSombrerosVerticalesSalida = 0;

   char mapa[ANCHO_MAPA][LARGO_MAPA];
   bool columnaEnredaderas = false;

   int puntaje = 0, puntajeZona = 0, cantidadPuntajes;
   char nombre[MAX_CARACTERES] = "---";
   _puntaje ranking[MAX_PUNTAJES];

   float valorTimerVictoria;

   bool armas[ARMAS_TIENDA];

   int framesEscape = 0;

   bool flag = 0; // BANDERA DE PRUEBA

   bool primeraVezDebug = false;

   // Inicializacion general

   cargarMapa(CARGADO_DE_MAPA);

   posYAnteriorFrame = jugador.posY;

   inicializarRanking(ranking, &cantidadPuntajes);

   /*for(i = 0; i < MONEDAS_JUEGO; i++)
   {
      monedasActivas[i] = true;
   }*/

   for(i = 0; i < ARMAS_TIENDA; i++)
   {
      if(i == 0)
      {
         armas[i] = false;
      }
      else
      {
         armas[i] = true;
      }
   }

   for(i = 0; i < MAX_BALAS; i++)
   {
      jugador.balas[i].activa = false;
      jugador.balas[i].numeroRebotes = 0;
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
         if(enMenu)
         {
            redraw = true;
            break;
         }
         if(!menutienda.activo) // No leer input de movimiento mientras la tienda esta abierta
         {
         if(dashFrames > 0 || golpeFrames > 0 || al_get_timer_started(tempVictoria) == true)
         {
            // Caso opuesto: dashFrames == 0 && golpeFrames == 0 && al_get_timer_started(tempVictoria) == false
            MOVIMIENTO = false;
         }
         if(key[ALLEGRO_KEY_DOWN])
         {
            if(nivel == 0)
            {
               jugador.posY += SPEED_FACTOR;
            }
            else
            {
               jugador.agachado = true;
            }
            jugador.direccion.Abajo = true;
         }
         else
         {
            jugador.direccion.Abajo = false;
            jugador.agachado = false;
         }
         if(key[ALLEGRO_KEY_C]) // "Lock": Detiene el movimiento para permitir apuntar com mayor facilidad.
         {
            if(jugador.agachado == true)
            {
               //jugador.agachado = false;
            }
            //jugador.lock = true;
         }
         else
         {
            jugador.lock = false;
         }
         if(jugador.agachado == true || jugador.lock == true)
         {
            MOVIMIENTO = false;
         }
         else
         {
            MOVIMIENTO = true;
         }
         if(key[ALLEGRO_KEY_UP])
         {
            if(nivel == 0)
            {
               jugador.posY -= SPEED_FACTOR;
            }
            jugador.direccion.Arriba = true;
         }
         else
         {
            jugador.direccion.Arriba = false;
         }
         if(key[ALLEGRO_KEY_LEFT])
         {
            if(dashFrames == 0 && golpeFrames == 0 && al_get_timer_started(tempVictoria) == false)
            {
               jugador.posX = jugador.posX - SPEED_FACTOR;
               jugador.orientacion = -1;
            }
            jugador.direccion.Izquierda = true;
         }
         else
         {
            jugador.direccion.Izquierda = false;
         }
         if(key[ALLEGRO_KEY_RIGHT])
         {
            if(dashFrames == 0 && golpeFrames == 0 && al_get_timer_started(tempVictoria) == false)
            {
               jugador.posX = jugador.posX + SPEED_FACTOR;
               jugador.orientacion = 1;
            }
            jugador.direccion.Derecha = true;
         }
         else
         {
            jugador.direccion.Derecha = false;
         }
         if(key[ALLEGRO_KEY_SPACE])
         {
            if(MOVIMIENTO == true)
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
            }
            jugador.direccion.Espacio = true;
         }
         else
         {
            if(jugadorEnAire == true)
            {
               teclaSoltada = true;
            }
            jugador.direccion.Espacio = false;
         }
         if(key[ALLEGRO_KEY_Z])
         {
            if(nivel != 0)
            {
               if(jugador.disparoCD == 0 && al_get_timer_started(tempVictoria) == false)
               {
                  for(i = 0; i < MAX_BALAS; i++) // Busca el primer slot libre del arreglo.
                  {
                     if(jugador.balas[i].activa == false)
                     {
                        jugador.balas[i].posX = jugador.posX;
                        if(jugador.agachado == true)
                        {
                           jugador.balas[i].posY = jugador.posY + (int)(ANCHO / 2) + jugador.variacionAlturaBala;
                        }
                        else
                        {
                           jugador.balas[i].posY = jugador.posY + (int)(ANCHO / 4) + jugador.variacionAlturaBala;
                        }
                        if(jugador.agachado == false) // Obtencion de direccion normal en caso de que el jugador no este agachado.
                        {
                           if(jugador.orientacion == -1) // Determina la direccion de movimiento de la bala, la cual solo cambia cuando esta se sale de la pantalla.
                           {
                              jugador.balas[i].direccion.Izquierda = true;
                           }
                           else if(jugador.orientacion == 1)
                           {
                              jugador.balas[i].direccion.Derecha = true;
                           }
                           if(jugador.direccion.Arriba == true && jugador.agachado == false)
                           {
                              jugador.balas[i].direccion.Arriba = true;
                           }
                           else if(jugador.direccion.Abajo == true && jugador.agachado == false)
                           {
                              jugador.balas[i].direccion.Abajo = true;
                           }
                           if(jugador.direccion.Izquierda == false && jugador.direccion.Derecha == false) // Deshabilita las direcciones laterales si esta disparando directo hacia arriba.
                           {
                              if(jugador.direccion.Arriba == true || jugador.direccion.Abajo == true)
                              {
                                 jugador.balas[i].direccion.Izquierda = false;
                                 jugador.balas[i].direccion.Derecha = false;
                              }
                           }
                        }
                        else // Solo puede disparar horizontalmente en caso contrario.
                        {
                           if(jugador.orientacion == -1)
                           {
                              jugador.balas[i].direccion.Izquierda = true;
                           }
                           else if(jugador.orientacion == 1)
                           {
                              jugador.balas[i].direccion.Derecha = true;
                           }
                        }
                        jugador.balas[i].velocidad = VELOCIDAD_BALA;
                        jugador.balas[i].activa = true;
                        jugador.disparoCD = COOLDOWN_DISPARO_0;
                        break;
                     }
                  }
                  if(jugador.variacionAlturaBala == VARIACION_ALTURA_BALA || jugador.variacionAlturaBala == -VARIACION_ALTURA_BALA)
                  {
                     jugador.direccionVariacionBala *= -1;
                  }
                  jugador.variacionAlturaBala += jugador.direccionVariacionBala;
               }
            }
            jugador.direccion.Z = true;
         }
         else
         {
            jugador.direccion.Z = false;
         }
         if(key[ALLEGRO_KEY_X])
         {
            if(nivel != 0 && primeraVezDash == false && jugador.direccion.X == false)
            {
               dashFrames = DASH_FRAMES;
               primeraVezDash = true;
            }
            jugador.direccion.X = true;
         }
         else
         {
            jugador.direccion.X = false;
         }
         } // fin if(!menutienda.activo)
         if(key[ALLEGRO_KEY_F1]) // Tecla para informacion de debug.
         {
            if(primeraVezDebug == false)
            {
               if(DEBUG == false)
               {
                  DEBUG = true;
               }
               else
               {
                  DEBUG = false;
               }
               primeraVezDebug = true;
            }
         }
         else
         {
            primeraVezDebug = false;
         }
         if(key[ALLEGRO_KEY_ESCAPE])
         {
            framesEscape++;
            if(framesEscape >= 30)
            {
               done = true;
            }
         }
         else
         {
            framesEscape = 0;
         }
         for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
         {
            key[i] &= KEY_SEEN;
         }
         redraw = true;
         break;

         case ALLEGRO_EVENT_KEY_DOWN:

         if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
         {
            menutienda.activo = false; // ESCAPE siempre cierra la tienda sin comprar
         }
         manejar_input_menu(&menutienda, &event, &jugador, armas);

         if(enMenu && !ingresandoNombre)
         {
            if(event.keyboard.keycode == ALLEGRO_KEY_UP || event.keyboard.keycode == ALLEGRO_KEY_DOWN)
            {
               opcionMenu = !opcionMenu; // Como solo hay 2 opciones, alterna entre 0 y 1.
            }

            if(event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_SPACE)
            {
               if(opcionMenu == 0) // "Jugar"
               {
                  ingresandoNombre = true; // En vez de leer por terminal, se abre el campo de texto en pantalla.
                  nombre[0] = '\0';
               }
               else // "Salir"
               {
                  done = true;
               }
            }
         }

         key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
         break;

         case ALLEGRO_EVENT_KEY_CHAR:

         if(enMenu && ingresandoNombre)
         {
            if(event.keyboard.keycode == ALLEGRO_KEY_ENTER)
            {
               if(strlen(nombre) > 0) // No deja confirmar con el nombre vacío.
               {
                  ingresandoNombre = false;
                  enMenu = false;
               }
            }
            else if(event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
            {
               int largoNombre = strlen(nombre);
               if(largoNombre > 0)
               {
                  nombre[largoNombre - 1] = '\0';
               }
            }
            else if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
               ingresandoNombre = false; // Cancela y vuelve al menú, sin salir del juego.
               nombre[0] = '\0';
            }
            else if(event.keyboard.unichar >= 32 && event.keyboard.unichar < 127) // Caracteres imprimibles ASCII.
            {
               int largoNombre = strlen(nombre);
               if(largoNombre < MAX_CARACTERES - 1)
               {
                  nombre[largoNombre] = (char)event.keyboard.unichar;
                  nombre[largoNombre + 1] = '\0';
               }
            }
         }

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
         al_clear_to_color(al_map_rgb(255, 255, 255));

         if(enMenu)
         {
            al_draw_bitmap(sprites.fondo_menu, 0, 0, 0);
            al_draw_textf(font48, al_map_rgb(255, 255, 255), 150, 50, 0, "CUPHEAD'S COMMISSION");

            if(ingresandoNombre)
            {
               al_draw_textf(font48, al_map_rgb(255, 255, 255), 150, 120, 0, "INGRESAR NOMBRE (max 10 caracteres):");

               // El guion bajo simula un cursor parpadeante mientras se escribe.
               al_draw_textf(font48, al_map_rgb(255, 255, 0), 150, 180, 0,
                  "%s%s", nombre, ((al_get_timer_count(timer) / 20) % 2 == 0) ? "_" : "");
            }
            else
            {
               al_draw_textf(font48, opcionMenu == 0 ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255), 150, 120, 0, "JUGAR");

               al_draw_textf(font48, opcionMenu == 1 ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255), 150, 160, 0, "SALIR");
            }
         }
         else
         {
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
            valorTimerVictoria = al_get_timer_count(tempVictoria);
            if(jugador.posY > posYAnteriorFrame)
            {
               cayendo = true;
            }
            else if(jugador.posY < posYAnteriorFrame)
            {
               cayendo = false;
            }
            if(nivel == 0 || dashFrames > 0)
            {
               GRAVEDAD = false;
            }
            else
            {
               GRAVEDAD = true;
            }
            if(nivel == 0 || al_get_timer_started(tempVictoria) == true)
            {
               DANHO = false;
            }
            else
            {
               DANHO = true;
            }
            // Si posY no cambió (parado quieto), cayendo conserva su valor anterior — no oscila.
            posYAnteriorFrame = jugador.posY;
            if(GRAVEDAD == false)
            {
               al_stop_timer(tempGravedad);
               al_set_timer_count(tempGravedad, 0);
            }
            else
            {
               al_start_timer(tempGravedad);
            }
            if(parryFrames > 0)
            {
               puedeHacerParry = false;
            }
            else
            {
               puedeHacerParry = true;
            }
            if(nivel == -1)
            {
               zona = -1;
            }
            else if(nivel == 1 || nivel == 2 || nivel == 3)
            {
               zona = 1;
            }
            else if(nivel == 4 || nivel == 5 || nivel == 6)
            {
               zona = 2;
            }
            else if(nivel == 7 || nivel == 8 || nivel == 9)
            {
               zona = 3;
            }
            else if(nivel == 10)
            {
               zona = 4;
            }
            else if(nivel == 11)
            {
               zona = 5;
            }
            else
            {
               zona = 0;
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

            if(golpeFrames > 0)
            {
               MOVIMIENTO = true;
               DISPARO = false;
            }

            if(dashFrames > 0) // Logica de dash.
            {
               if(dashFrames == DASH_FRAMES)
               {
                  jugador.direccionDash = jugador.orientacion;
               }
               MOVIMIENTO = false;
               parryFrames = 0;
               jugador.posX += DASH_SPEED * jugador.direccionDash;
               dashFrames--;
            }
            if(dashFrames == 1)
            {
               MOVIMIENTO = true;
               GRAVEDAD = true;
               al_set_timer_count(tempGravedad, 0);
            }

            /*if(parryFrames > 0) // Dibuja el cuadrado parry.
            {
               al_draw_filled_rectangle(jugador.posParryX - camaraX, jugador.posParryY - camaraY, jugador.posParryX - camaraX + LARGO * 2, jugador.posParryY - camaraY + ANCHO * 2, al_map_rgb(255, 0, 255));
            }
            else
            {
               al_draw_filled_rectangle(jugador.posParryX - camaraX, jugador.posParryY - camaraY, jugador.posParryX - camaraX + LARGO * 2, jugador.posParryY - camaraY + ANCHO * 2, al_map_rgb(100, 0, 100));
            }*/

            // Actualiza el objetivo más cercano en cada frame.
            entidad enemigoMasCercano = obtenerEnemigoMasCercano(jugador, enemigosA, cantidadEnemigosA, enemigosC, cantidadEnemigosC);
            for(i = 0; i < MAX_BALAS; i++)
            {
               if(jugador.balas[i].activa == true) // Mueve las balas respecto a su direccion de movimiento, unica a cada bala del arreglo.
               {
                  if(balaDiagonal(jugador, i) == true) // Reduce el desplazamiento de bala si es diagonal.
                  {
                     desplazamientoBala = VELOCIDAD_BALA * sqrt(2) / 2;
                  }
                  else
                  {
                     desplazamientoBala = VELOCIDAD_BALA;
                  }
                  switch(jugador.armaEquipada)
                  {
                     case 1:
                     {
                        int enemigoCercano = -1;
                        int tipoArreglo = 0; // 0 = Ninguno, 1 = Arreglo A, 2 = Arreglo C
                        float distanciaMinima = 999999.0f; 
                        
                        // ---------------------------------------------------------
                        // PASO 1A: Escanear el arreglo 'enemigosA'
                        // ---------------------------------------------------------
                        for (int j = 0; j < MAX_ENEMIGOS; j++)
                        {
                           if (enemigosA[j].activo)
                           {
                                 float dx = enemigosA[j].posX - jugador.balas[i].posX;
                                 float dy = enemigosA[j].posY - jugador.balas[i].posY;
                                 float distancia = sqrt((dx * dx) + (dy * dy));
                                 
                                 if (distancia < distanciaMinima)
                                 {
                                    distanciaMinima = distancia;
                                    enemigoCercano = j;
                                    tipoArreglo = 1; // Marcamos que el más cercano hasta ahora es del tipo A
                                 }
                           }
                        }

                        // ---------------------------------------------------------
                        // PASO 1B: Escanear el arreglo 'enemigosC'
                        // ---------------------------------------------------------
                        for (int k = 0; k < MAX_ENEMIGOS; k++)
                        {
                           if (enemigosC[k].activo)
                           {
                                 float dx = enemigosC[k].posX - jugador.balas[i].posX;
                                 float dy = enemigosC[k].posY - jugador.balas[i].posY;
                                 float distancia = sqrt((dx * dx) + (dy * dy));
                                 
                                 // Si este enemigo C está aún más cerca que el mejor que encontramos en A
                                 if (distancia < distanciaMinima)
                                 {
                                    distanciaMinima = distancia;
                                    enemigoCercano = k;
                                    tipoArreglo = 2; // Actualizamos la bandera para saber que ahora es del tipo C
                                 }
                           }
                        }
                        
                        // ---------------------------------------------------------
                        // PASO 2: Mover la bala hacia el enemigo seleccionado
                        // ---------------------------------------------------------
                        if (tipoArreglo != 0) // Si se encontró algún enemigo en cualquiera de los arreglos
                        {
                           float objetivoX = 0;
                           float objetivoY = 0;

                           // Extraemos las coordenadas del objetivo dependiendo del arreglo ganador
                           if (tipoArreglo == 1)
                           {
                                 objetivoX = enemigosA[enemigoCercano].posX;
                                 objetivoY = enemigosA[enemigoCercano].posY;
                           }
                           else if (tipoArreglo == 2)
                           {
                                 objetivoX = enemigosC[enemigoCercano].posX;
                                 objetivoY = enemigosC[enemigoCercano].posY;
                           }

                           // Calculamos el vector de dirección final
                           float dx = objetivoX - jugador.balas[i].posX;
                           float dy = objetivoY - jugador.balas[i].posY;
                           float magnitud = sqrt((dx * dx) + (dy * dy));
                           
                           if (magnitud > 0)
                           {
                                 float velX = (dx / magnitud) * VELOCIDAD_CHASER;
                                 float velY = (dy / magnitud) * VELOCIDAD_CHASER;
                                 
                                 jugador.balas[i].posX += velX;
                                 jugador.balas[i].posY += velY;
                           }
                        }
                        else 
                        {
                           // COMPORTAMIENTO POR DEFECTO: Viaja recto si no hay objetivos activos
                           if (jugador.balas[i].direccion.Arriba)    jugador.balas[i].posY -= VELOCIDAD_CHASER;
                           if (jugador.balas[i].direccion.Abajo)     jugador.balas[i].posY += VELOCIDAD_CHASER;
                           if (jugador.balas[i].direccion.Izquierda) jugador.balas[i].posX -= VELOCIDAD_CHASER;
                           if (jugador.balas[i].direccion.Derecha)   jugador.balas[i].posX += VELOCIDAD_CHASER;
                        }
                        
                        break;
                     }
                     case 2:
                     {
                        // 1. Reducir la velocidad constantemente para crear el efecto de desaceleración y retorno (boomerang).
                        jugador.balas[i].velocidad -= 0.4;

                        // 2. Calcular el desplazamiento real en este frame.
                        float movRoundabout = jugador.balas[i].velocidad;
                        if(balaDiagonal(jugador, i) == true)
                        {
                           // Normalizar la velocidad si viaja en diagonal para que no vaya mas rapido.
                           movRoundabout = jugador.balas[i].velocidad * (sqrt(2) / 2); 
                        }

                        // 3. Aplicar el movimiento según la dirección original con la que se disparo.
                        if(jugador.balas[i].direccion.Arriba == true)
                        {
                           jugador.balas[i].posY -= movRoundabout;
                        }
                        if(jugador.balas[i].direccion.Abajo == true)
                        {
                           jugador.balas[i].posY += movRoundabout;
                        }
                        if(jugador.balas[i].direccion.Izquierda == true)
                        {
                           jugador.balas[i].posX -= movRoundabout;
                        }
                        if(jugador.balas[i].direccion.Derecha == true)
                        {
                           jugador.balas[i].posX += movRoundabout;
                        }
                        break;
                     }
                     default:
                     {
                        if(jugador.balas[i].direccion.Arriba == true)
                        {
                           jugador.balas[i].posY -= desplazamientoBala;
                        }
                        if(jugador.balas[i].direccion.Abajo == true)
                        {
                           jugador.balas[i].posY += desplazamientoBala;
                        }
                        if(jugador.balas[i].direccion.Izquierda == true)
                        {
                           jugador.balas[i].posX -= desplazamientoBala;
                        }
                        if(jugador.balas[i].direccion.Derecha == true)
                        {
                           jugador.balas[i].posX += desplazamientoBala;
                        }
                        break;
                     }
                  }
               }
               if(jugador.balas[i].posX < camaraX - LARGO || jugador.balas[i].posX > camaraX + LARGO_PANTALLA) // Despawnea la bala si esta fuera de la camara.
               {
                  if(jugador.armaEquipada != 3)
                  {
                     jugador.balas[i].activa = false; // Desactiva la bala y sus direcciones para que no interfiera con el movimiento de las proximas.
                     jugador.balas[i].direccion.Arriba = false;
                     jugador.balas[i].direccion.Abajo = false; // TAMBIEN DESPAWNEAR SI SE SALE DE LA PANTALLA POR ARRIBA O POR ABAJO
                     jugador.balas[i].direccion.Izquierda = false;
                     jugador.balas[i].direccion.Derecha = false;
                     jugador.balas[i].velocidad = 0;
                  }
                  else
                  {
                     if(jugador.balas[i].direccion.Derecha == true)
                     {
                        jugador.balas[i].direccion.Derecha = false;
                        jugador.balas[i].direccion.Izquierda = true;
                     }
                     else
                     {
                        jugador.balas[i].direccion.Izquierda = false;
                        jugador.balas[i].direccion.Derecha = true;
                     }
                     jugador.balas[i].numeroRebotes++;
                     if(jugador.balas[i].numeroRebotes > 3)
                     {
                        jugador.balas[i].activa = false;
                        jugador.balas[i].direccion.Arriba = false;
                        jugador.balas[i].direccion.Abajo = false;
                        jugador.balas[i].direccion.Izquierda = false;
                        jugador.balas[i].direccion.Derecha = false;
                     }
                  }
                  continue;
               }
               int fila = (int)(jugador.balas[i].posY / ANCHO); // !? CAMBIAR EN CASO DE AGREGAR LOCK
               if(fila >= 0 && fila <= ANCHO_MAPA)
               {
                  for(int col = 0; col < LARGO_MAPA; col++) // Solo revisa la fila donde esta la bala.
                  {
                     if(mapa[fila][col] == '#')
                     {
                        float bloqueX = col * LARGO;
                        if(jugador.balas[i].posX + LARGO_BALA > bloqueX && bloqueX + LARGO > jugador.balas[i].posX)
                        {
                           jugador.balas[i].activa = false;
                           jugador.balas[i].direccion.Arriba = false;
                           jugador.balas[i].direccion.Abajo = false;
                           jugador.balas[i].direccion.Izquierda = false;
                           jugador.balas[i].direccion.Derecha = false;
                        }
                     }
                  }
               }
            }

            for(i = 0; i < MAX_BALAS; i++) // Revisa las colisiones entre balas y enemigos susceptibles a golpes.
            {
               for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++) // Enemigos A
               {
                  if(jugador.balas[i].activa == true && enemigosA[contEnemigos].activo == true) // Si bala y enemigo estan activos:
                  {
                     if(generalCollide(jugador.balas[i].posX, jugador.balas[i].posY, LARGO_BALA, ANCHO_BALA, enemigosA[contEnemigos].posX, enemigosA[contEnemigos].posY, LARGO, ANCHO))
                     {
                        enemigosA[contEnemigos].vida -= DANHO_BALA;
                        jugador.balas[i].activa = false; // Despawn de bala.
                        jugador.balas[i].direccion.Arriba = false;
                        jugador.balas[i].direccion.Abajo = false;
                        jugador.balas[i].direccion.Izquierda = false;
                        jugador.balas[i].direccion.Derecha = false;
                        if(enemigosA[contEnemigos].vida <= 0)
                        {
                           enemigosA[contEnemigos].activo = false;
                           puntajeZona += PUNTAJE_ENEMIGO_A;
                        }
                     }
                  }
               }
               for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Enemigos C
               {
                  if(jugador.balas[i].activa == true && enemigosC[contEnemigos].activo == true)
                  {
                     if(generalCollide(jugador.balas[i].posX, jugador.balas[i].posY, LARGO_BALA, ANCHO_BALA, enemigosC[contEnemigos].posX, enemigosC[contEnemigos].posY, LARGO, ANCHO))
                     {
                        enemigosC[contEnemigos].vida -= DANHO_BALA;
                        jugador.balas[i].activa = false; // Despawn de bala.
                        jugador.balas[i].direccion.Arriba = false;
                        jugador.balas[i].direccion.Abajo = false;
                        jugador.balas[i].direccion.Izquierda = false;
                        jugador.balas[i].direccion.Derecha = false;
                        if(enemigosC[contEnemigos].vida <= 0)
                        {
                           enemigosC[contEnemigos].activo = false;
                           puntajeZona += PUNTAJE_ENEMIGO_C;
                        }
                     }
                  }
               }
               for(cont = 0; cont < cantidadDianas; cont++)
               {
                  if(jugador.balas[i].activa == true && dianas[cont].activo == true)
                  {
                     if(generalCollide(jugador.balas[i].posX, jugador.balas[i].posY, LARGO_BALA, ANCHO_BALA, dianas[cont].posX, dianas[cont].posY, 40, 200) == true)
                     {
                        dianas[cont].vida -= DANHO_BALA;
                        jugador.balas[i].activa = false; // Despawn de bala.
                        jugador.balas[i].direccion.Arriba = false;
                        jugador.balas[i].direccion.Abajo = false;
                        jugador.balas[i].direccion.Izquierda = false;
                        jugador.balas[i].direccion.Derecha = false;
                        if(dianas[cont].vida <= 0)
                        {
                           dianas[cont].activo = false;
                        }
                     }
                  }
               }
            }

            for(cont = 0; cont < cantidadDianas; cont++)
            {
               if(dianas[cont].activo == true)
               {
                  if(generalCollide(jugador.posX, jugador.posY, LARGO, ANCHO, dianas[cont].posX, dianas[cont].posY, 40, 200) == true)
                  {
                     jugador.posX = dianas[cont].posX - LARGO;
                  }
               }
            }

            // 1.2: Enemigos:

            for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++) // Movimiento de los enemigos A.
            {
               if(enemigosA[contEnemigos].activo == true) // Si esta activo:
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

                  int fila = enemigosA[contEnemigos].posY / ANCHO; // Revisa las colisiones de bloques en los enemigos A.
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
                  logicaSombrerosHorizontalesEntrada(sombrerosHorizontales, enemigosC[contEnemigos], nivel, contEnemigos);
                  logicaSombrerosHorizontalesSalida(sombrerosHorizontales, enemigosC[contEnemigos], nivel, contEnemigos);
                  logicaSombrerosVerticalesEntrada(sombrerosHorizontales, enemigosC[contEnemigos], nivel, contEnemigos);
                  logicaSombrerosVerticalesSalida(sombrerosHorizontales, enemigosC[contEnemigos], nivel, contEnemigos);
                  if(fabs(enemigosA[contEnemigos].posX - jugador.posX) <= LARGO * 3 && fabs(enemigosA[contEnemigos].posY - jugador.posY) <= ANCHO * 3)
                  {
                     if(collide(font, jugador, &enemigosA[contEnemigos].posX, &enemigosA[contEnemigos].posY) == true && iFrames == 0 && DANHO == true)
                     {
                        golpearJugador(&jugador.vida, &iFrames, &puntajeZona, &dashFrames, &parryFrames, &golpeFrames, tempGravedad);
                     }
                  }
               }
               if(enemigosA[contEnemigos].cooldownPortal > 0)
               {
                  enemigosA[contEnemigos].cooldownPortal--;
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
               if(enemigosC[contEnemigos].activo == true) // Si esta activo:
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
                  logicaSombrerosHorizontalesEntrada(sombrerosHorizontales, enemigosC[contEnemigos], nivel, contEnemigos);
                  logicaSombrerosHorizontalesSalida(sombrerosHorizontales, enemigosC[contEnemigos], nivel, contEnemigos);
                  logicaSombrerosVerticalesEntrada(sombrerosHorizontales, enemigosC[contEnemigos], nivel, contEnemigos);
                  logicaSombrerosVerticalesSalida(sombrerosHorizontales, enemigosC[contEnemigos], nivel, contEnemigos);
                  if(fabs(enemigosC[contEnemigos].posX - jugador.posX) <= LARGO * 3 && fabs(enemigosC[contEnemigos].posY - jugador.posY) <= ANCHO * 3)
                  {
                     if(collide(font, jugador, &enemigosC[contEnemigos].posX, &enemigosC[contEnemigos].posY) == true && iFrames == 0 && DANHO == true)
                     {
                        golpearJugador(&jugador.vida, &iFrames, &puntajeZona, &dashFrames, &parryFrames, &golpeFrames, tempGravedad);
                     }
                  }
               }
               if(enemigosC[contEnemigos].cooldownPortal > 0)
               {
                  enemigosC[contEnemigos].cooldownPortal--;
               }
            }

            for(contEnemigos = 0; contEnemigos < cantidadEnemigosE; contEnemigos++)  // Movimiento de los enemigos E.
            {
               enemigosE[contEnemigos].posY = enemigosE[contEnemigos].nodoE + OSCILLATION_RANGE_E * sinf(valorTimerEnemigosE / OSCILLATION_SPEED_E);
               if(fabs(enemigosE[contEnemigos].posX - jugador.posX) <= LARGO * 3 && fabs(enemigosE[contEnemigos].posY - jugador.posY) <= ANCHO * 3)
               {
                  if(enemigosE[contEnemigos].activo == true) // Si esta activo:
                  {
                     if(collide(font, jugador, &enemigosE[contEnemigos].posX, &enemigosE[contEnemigos].posY) == true && iFrames == 0 && DANHO == true)
                     {
                        golpearJugador(&jugador.vida, &iFrames, &puntajeZona, &dashFrames, &parryFrames, &golpeFrames, tempGravedad);
                     }
                  }
                  if(collideParry(font, jugador, &enemigosE[contEnemigos].posX, &enemigosE[contEnemigos].posY) == true && parryFrames > 0)
                  {
                     al_rest(0.1);
                     al_set_timer_count(tempGravedad, -20);
                     teclaSoltada = true;
                     parryFrames = 0;
                     if(enemigosE[contEnemigos].activo == true)
                     {
                        puntajeZona += PUNTAJE_ENEMIGO_E;
                     }
                     enemigosE[contEnemigos].activo = false;
                  }
               }
            }

            for(cont = 0; cont < cantidadMonedas; cont++)
            {
               if(monedasMapa[cont].activo == true)
               {
                  if(fabs(monedasMapa[cont].posX - jugador.posX) <= LARGO * 3 && fabs(monedasMapa[cont].posY - jugador.posY) <= ANCHO * 3)
                  {
                     if(generalCollide(jugador.posX, jugador.posY, LARGO, ANCHO, monedasMapa[cont].posX - 10, monedasMapa[cont].posY - 10, 60, 60) == true)
                     {
                        jugador.monedas++;
                        monedasMapa[cont].activo = false;
                        /*switch(nivel) // !? AGREGAR CASOS DE ZONA 3
                        {
                           case 1:
                           monedasActivas[cont] = false;
                           break;
                           case 2:
                           monedasActivas[cont + 1] = false;
                           break;
                           case 3:
                           monedasActivas[cont + 3] = false;
                           break;
                           case 4:
                           if(cont == 0)
                           {
                              monedasActivas[5] = false;
                           }
                           else
                           {
                              monedasActivas[6] = false;
                           }
                           break;
                           case 5:
                           monedasActivas[cont + 7] = false;
                           break;
                           case 6:
                           monedasActivas[cont + 8] = false;
                           break;
                        }*/
                     }
                  }
               }
            }

            for(cont = 0; cont < cantidadSombrerosHorizontalesEntrada; cont++)
            {
               if(fabs(sombrerosHorizontales[cont].entrada.posX - jugador.posX) <= LARGO * 5 && fabs(sombrerosHorizontales[cont].entrada.posY - jugador.posY) <= ANCHO * 5)
               {
                  if(jugador.cooldownPortal == 0)
                  {
                     if(generalCollide(jugador.posX, jugador.posY, LARGO, ANCHO, sombrerosHorizontales[cont].entrada.posX, sombrerosHorizontales[cont].entrada.posY, LARGO, ANCHO_SOMBRERO) == true)
                     {
                        switch(nivel)
                        {
                           case 5:
                           {
                              switch(cont)
                              {
                                 case 0:
                                 jugador.posX = sombrerosHorizontales[1].salida.posX;
                                 jugador.posY = sombrerosHorizontales[1].salida.posY;
                                 break;
                                 case 1:
                                 jugador.posX = sombrerosHorizontales[0].salida.posX;
                                 jugador.posY = sombrerosHorizontales[0].salida.posY;
                                 break;
                                 case 2:
                                 jugador.posX = sombrerosHorizontales[2].salida.posX;
                                 jugador.posY = sombrerosHorizontales[2].salida.posY;
                                 break;
                              }
                              break;
                           }
                           case 6:
                           {
                              switch(cont)
                              {
                                 case 0:
                                 jugador.posX = sombrerosHorizontales[1].salida.posX;
                                 jugador.posY = sombrerosHorizontales[1].salida.posY;
                                 break;
                                 case 1:
                                 jugador.posX = sombrerosHorizontales[2].salida.posX;
                                 jugador.posY = sombrerosHorizontales[2].salida.posY;
                                 break;
                                 case 2:
                                 jugador.posX = sombrerosHorizontales[0].salida.posX;
                                 jugador.posY = sombrerosHorizontales[0].salida.posY;
                                 break;
                              }
                              break;
                           }
                           default:
                           {
                              jugador.posX = sombrerosHorizontales[cont].salida.posX;
                              jugador.posY = sombrerosHorizontales[cont].salida.posY;
                              break;
                           }
                        }
                        jugador.cooldownPortal = ATRAVESAR_SOMBRERO_CD;
                     }
                  }
               }
            }
            for(cont = 0; cont < cantidadSombrerosHorizontalesSalida; cont++)
            {
               if(fabs(sombrerosHorizontales[cont].salida.posX - jugador.posX) <= LARGO * 5 && fabs(sombrerosHorizontales[cont].salida.posY - jugador.posY) <= ANCHO * 5)
               {
                  if(jugador.cooldownPortal == 0)
                  {
                     if(generalCollide(jugador.posX, jugador.posY, LARGO, ANCHO, sombrerosHorizontales[cont].salida.posX, sombrerosHorizontales[cont].salida.posY, LARGO, ANCHO_SOMBRERO) == true)
                     {
                        switch(nivel)
                        {
                           case 5:
                           {
                              switch(cont)
                              {
                                 case 0:
                                 jugador.posX = sombrerosHorizontales[1].entrada.posX;
                                 jugador.posY = sombrerosHorizontales[1].entrada.posY;
                                 break;
                                 case 1:
                                 jugador.posX = sombrerosHorizontales[0].entrada.posX;
                                 jugador.posY = sombrerosHorizontales[0].entrada.posY;
                                 break;
                                 case 2:
                                 jugador.posX = sombrerosHorizontales[2].entrada.posX;
                                 jugador.posY = sombrerosHorizontales[2].entrada.posY;
                                 break;
                              }
                              break;
                           }
                           case 6:
                           {
                              switch(cont)
                              {
                                 case 0:
                                 jugador.posX = sombrerosHorizontales[2].entrada.posX;
                                 jugador.posY = sombrerosHorizontales[2].entrada.posY;
                                 break;
                                 case 1:
                                 jugador.posX = sombrerosHorizontales[0].entrada.posX;
                                 jugador.posY = sombrerosHorizontales[0].entrada.posY;
                                 break;
                                 case 2:
                                 jugador.posX = sombrerosHorizontales[1].entrada.posX;
                                 jugador.posY = sombrerosHorizontales[1].entrada.posY;
                                 break;
                              }
                              break;
                           }
                           default:
                           {
                              jugador.posX = sombrerosHorizontales[cont].entrada.posX;
                              jugador.posY = sombrerosHorizontales[cont].entrada.posY;
                              break;
                           }
                        }
                        jugador.cooldownPortal = ATRAVESAR_SOMBRERO_CD;
                     }
                  }
               }
            }
            for(cont = 0; cont < cantidadSombrerosVerticalesEntrada; cont++)
            {
               if(jugador.cooldownPortal == 0)
               {
                  if(fabs(sombrerosVerticales[cont].entrada.posX - jugador.posX) <= LARGO * 5 && fabs(sombrerosVerticales[cont].entrada.posY - jugador.posY) <= ANCHO * 5)
                  {
                     if(generalCollide(jugador.posX, jugador.posY, LARGO, ANCHO, sombrerosVerticales[cont].entrada.posX, sombrerosVerticales[cont].entrada.posY, LARGO_SOMBRERO, ANCHO) == true)
                     {
                        switch(nivel)
                        {
                           case 4:
                           {
                              switch(cont)
                              {
                                 case 0:
                                 jugador.posX = sombrerosVerticales[2].salida.posX;
                                 jugador.posY = sombrerosVerticales[2].salida.posY;
                                 break;
                                 case 1:
                                 jugador.posX = sombrerosVerticales[0].salida.posX;
                                 jugador.posY = sombrerosVerticales[0].salida.posY;
                                 break;
                                 case 2:
                                 jugador.posX = sombrerosVerticales[1].salida.posX;
                                 jugador.posY = sombrerosVerticales[1].salida.posY;
                                 break;
                              }
                              break;
                           }
                           case 6:
                           {
                              switch(cont)
                              {
                                 case 0:
                                 jugador.posX = sombrerosVerticales[2].salida.posX;
                                 jugador.posY = sombrerosVerticales[2].salida.posY;
                                 break;
                                 case 1:
                                 jugador.posX = sombrerosVerticales[0].salida.posX;
                                 jugador.posY = sombrerosVerticales[0].salida.posY;
                                 break;
                                 case 2:
                                 jugador.posX = sombrerosVerticales[1].salida.posX;
                                 jugador.posY = sombrerosVerticales[1].salida.posY;
                                 break;
                              }
                              break;
                           }
                           default:
                           {
                              jugador.posX = sombrerosVerticales[cont].salida.posX;
                              jugador.posY = sombrerosVerticales[cont].salida.posY;
                              if(valorTimerGravedad > 0)
                              {
                                 al_set_timer_count(tempGravedad, -valorTimerGravedad);
                              }
                              break;
                           }
                        }
                        jugador.cooldownPortal = ATRAVESAR_SOMBRERO_CD;
                     }
                  }
               }
            }
            for(cont = 0; cont < cantidadSombrerosVerticalesSalida; cont++)
            {
               if(fabs(sombrerosVerticales[cont].salida.posX - jugador.posX) <= LARGO * 5 && fabs(sombrerosVerticales[cont].salida.posY - jugador.posY) <= ANCHO * 5)
               {
                  if(jugador.cooldownPortal == 0)
                  {
                     if(generalCollide(jugador.posX, jugador.posY, LARGO, ANCHO, sombrerosVerticales[cont].salida.posX, sombrerosVerticales[cont].salida.posY, LARGO_SOMBRERO, ANCHO) == true)
                     {
                        switch(nivel)
                        {
                           case 4:
                           {
                              switch(cont)
                              {
                                 case 0:
                                 jugador.posX = sombrerosVerticales[1].entrada.posX;
                                 jugador.posY = sombrerosVerticales[1].entrada.posY;
                                 break;
                                 case 1:
                                 jugador.posX = sombrerosVerticales[2].entrada.posX;
                                 jugador.posY = sombrerosVerticales[2].entrada.posY;
                                 break;
                                 case 2:
                                 jugador.posX = sombrerosVerticales[0].entrada.posX;
                                 jugador.posY = sombrerosVerticales[0].entrada.posY;
                                 break;
                              }
                              break;
                           }
                           case 6:
                           {
                              switch(cont)
                              {
                                 case 0:
                                 jugador.posX = sombrerosVerticales[1].entrada.posX;
                                 jugador.posY = sombrerosVerticales[1].entrada.posY;
                                 break;
                                 case 1:
                                 jugador.posX = sombrerosVerticales[2].entrada.posX;
                                 jugador.posY = sombrerosVerticales[2].entrada.posY;
                                 break;
                                 case 2:
                                 jugador.posX = sombrerosVerticales[0].entrada.posX;
                                 jugador.posY = sombrerosVerticales[0].entrada.posY;
                                 break;
                              }
                              break;
                           }
                           default:
                           {
                              jugador.posX = sombrerosVerticales[cont].entrada.posX;
                              jugador.posY = sombrerosVerticales[cont].entrada.posY;
                              if(valorTimerGravedad > 0)
                              {
                                 al_set_timer_count(tempGravedad, -valorTimerGravedad);
                              }
                              break;
                           }
                        }
                        jugador.cooldownPortal = ATRAVESAR_SOMBRERO_CD;
                     }
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
                              jugador.posX = anularMovimientoX(&jugador, jugador.posX, &puntoX);
                           }
                           else
                           {
                              jugador.posY = anularMovimientoY(&jugador, jugador.posY, &puntoY, tempGravedad);
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
                           jugador.posY = anularMovimientoY(&jugador, jugador.posY, &puntoY, tempGravedad);
                        }
                     }
                     if(mapa[i][j] == '/') // Colision personaje-pincho:
                     {
                        if(collide(font, jugador, &puntoX, &puntoY) == true)
                        {
                           al_set_timer_count(tempGravedad, -30);
                           if(iFrames == 0 && DANHO == true)
                           {
                              golpearJugador(&jugador.vida, &iFrames, &puntajeZona, &dashFrames, &parryFrames, &golpeFrames, tempGravedad);
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
                     if(mapa[i][j] == '?') // Colision personaje-bloque de secreto:
                     {
                        if(collide(font, jugador, &puntoX, &puntoY) == true)
                        {
                           atravesando = true;
                        }
                     }
                     if(mapa[i][j] == 'v') // Colision personaje-portal de entrada
                     {
                        if(collideParry(font, jugador, &puntoX, &puntoY) == true && jugador.direccion.Arriba == true) // Utiliza el cuadrado parry para facilitar la entrada al portal.
                        {
                           jugador.posX = portalSalida.posX + 80;
                           jugador.posY = portalSalida.posY;
                        }
                     }
                     if(mapa[i][j] == '^') // Colision personaje-portal local
                     {
                        if(collideParry(font, jugador, &puntoX, &puntoY) == true && jugador.direccion.Arriba == true && primeraVezSalto == false) // Utiliza el cuadrado parry para facilitar la entrada al portal.
                        {
                           jugador.posY = REGRESO_DE_PORTAL;
                        }
                     }
                     if(mapa[i][j] == 'O') // Colision personaje-portal de transicion
                     {
                        if(collideParry(font, jugador, &puntoX, &puntoY) == true && jugador.direccion.Arriba == true)
                        {
                           al_draw_textf(font, al_map_rgb(255, 255, 255), jugador.posX, jugador.posY + 50, 0, "Transicionando...");
                           nivel++;
                           dashFrames = 0;
                           jugador.vida = VIDA_INICIAL;
                           for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Destruye los timers de los enemigos C:
                           {
                              al_destroy_timer(enemigosC[contEnemigos].tempEnemigosC);
                           }
                           cantidadEnemigosA = 0;
                           cantidadEnemigosC = 0;
                           cantidadEnemigosE = 0;
                           cantidadMonedas = 0;
                           cantidadSombrerosHorizontalesEntrada = 0;
                           cantidadSombrerosHorizontalesSalida = 0;
                           cantidadSombrerosVerticalesEntrada = 0;
                           cantidadSombrerosVerticalesSalida = 0;
                           al_rest(1);
                           al_set_timer_count(tempGravedad, 0);
                           dashFrames = 0;
                           parryFrames = 0;
                           cargarMapa(CARGADO_DE_MAPA);
                        }
                     }
                     if(mapa[i][j] == '@') // Colision personaje-portal global
                     {
                        if(jugador.posX >= puntoX) // Si el jugador pasa por la bandera de meta:
                        {
                           al_start_timer(tempVictoria);
                        }
                     }
                     if(mapa[i][j] == 'T')
                     {
                        if(generalCollide(jugador.posX, jugador.posY, LARGO, ANCHO, puntoX, puntoY, 120, 120) == true)
                        {
                           if(jugador.direccion.Z == true)
                           {
                              menutienda.activo = true;
                              jugador.posY += ANCHO * 2;
                           }
                        }
                     }
                     if(mapa[i][j] == '1' || mapa[i][j] == '2') // Colision personaje-portada de zona
                     {
                        if(generalCollide(jugador.posX, jugador.posY, LARGO, ANCHO, puntoX, puntoY, 120, 120) == true)
                        {
                           if(jugador.direccion.Z == true)
                           {
                              if(mapa[i][j] == '1') // !? AGREGAR CASO DE ZONA 3
                              {
                                 nivel = 1;
                              }
                              else if(mapa[i][j] == '2')
                              {
                                 nivel = 4;
                              }
                              for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Destruye los timers de los enemigos C:
                              {
                                 al_destroy_timer(enemigosC[contEnemigos].tempEnemigosC);
                              }
                              cantidadEnemigosA = 0;
                              cantidadEnemigosC = 0;
                              cantidadEnemigosE = 0;
                              cantidadMonedas = 0;
                              cantidadSombrerosHorizontalesEntrada = 0;
                              cantidadSombrerosHorizontalesSalida = 0;
                              cantidadSombrerosVerticalesEntrada = 0;
                              cantidadSombrerosVerticalesSalida = 0;
                              al_rest(1);
                              cargarMapa(CARGADO_DE_MAPA);
                           }
                        }
                     }
                  }
               }
            }

            if(al_get_timer_started(tempVictoria) == true)
            {
               if((nivel == 3 || nivel == 6) && valorTimerVictoria == 1) // Valor arbitrario para asignar puntaje.
               {
                  puntaje += puntajeZona;
               }
               if(nivel == 3)
               {
                  zona1Completada = true;
               }
               if(nivel == 6)
               {
                  zona2Completada = true;
               }
               MOVIMIENTO = false;
               DISPARO = false;
               jugador.direccion.Derecha = true;
               jugador.posX += SPEED_FACTOR;
            }

            if(valorTimerVictoria >= DURACION_VICTORIA && nivel != 0)
            {
               jugador.vida = VIDA_INICIAL;
               if(nivel == 6) // !? COLOCAR ULTIMO NIVEL
               {
                  actualizarPuntaje(ranking, nombre, puntaje, &cantidadPuntajes);
                  puntaje = 0;
               }
               puntajeZona = 0;
               nivel = 0;
               for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Destruye los timers de los enemigos C:
               {
                  al_destroy_timer(enemigosC[contEnemigos].tempEnemigosC);
               }
               cantidadEnemigosA = 0;
               cantidadEnemigosC = 0;
               cantidadEnemigosE = 0;
               cantidadMonedas = 0;
               cantidadSombrerosHorizontalesEntrada = 0;
               cantidadSombrerosHorizontalesSalida = 0;
               cantidadSombrerosVerticalesEntrada = 0;
               cantidadSombrerosVerticalesSalida = 0;
               al_rest(1);
               al_set_timer_count(tempGravedad, 0);
               dashFrames = 0;
               parryFrames = 0;
               cargarMapa(CARGADO_DE_MAPA);
               al_stop_timer(tempVictoria);
               al_set_timer_count(tempVictoria, 0);
               MOVIMIENTO = true;
            }

            if(jugador.vida <= 0) // Logica de game over.
            {
               al_draw_textf(font, al_map_rgb(255, 13, 69), 600,360, 0, "GAME OVER");
               nivel = 0;
               jugador.vida = VIDA_INICIAL;
               puntajeZona = 0;
               dashFrames = 0;
               for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Destruye los timers de los enemigos C:
               {
                  al_destroy_timer(enemigosC[contEnemigos].tempEnemigosC);
               }
               cantidadEnemigosA = 0;
               cantidadEnemigosC = 0;
               cantidadEnemigosE = 0;
               cantidadMonedas = 0;
               cantidadSombrerosHorizontalesEntrada = 0;
               cantidadSombrerosHorizontalesSalida = 0;
               cantidadSombrerosVerticalesEntrada = 0;
               cantidadSombrerosVerticalesSalida = 0;
               al_rest(1);
               al_set_timer_count(tempGravedad, 0);
               dashFrames = 0;
               parryFrames = 0;
               cargarMapa(CARGADO_DE_MAPA);
            }

            if(jugador.posY > ANCHO_PANTALLA + ANCHO) // Si el jugador se "cae" (Se sale de la pantalla por abajo)
            {
               al_set_timer_count(tempGravedad, -30);
               if(iFrames == 0 && DANHO == true)
               {
                  golpearJugador(&jugador.vida, &iFrames, &puntajeZona, &dashFrames, &parryFrames, &golpeFrames, tempGravedad);
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

            switch(zona)
            {
               case 1:
               al_draw_bitmap(sprites.fondo1, 0, 0, 0);
               break;
               case 2:
               al_draw_bitmap(sprites.fondo2, 0, 0, 0);
               break;
            }
            if(nivel != 0)
            {
               al_draw_textf(font, al_map_rgb(255, 255, 255), 1270, 660, ALLEGRO_ALIGN_RIGHT, "Controles:");
               al_draw_textf(font, al_map_rgb(255, 255, 255), 1270, 670, ALLEGRO_ALIGN_RIGHT, "< v ^ > : Movimiento y orientacion");
               al_draw_textf(font, al_map_rgb(255, 255, 255), 1270, 680, ALLEGRO_ALIGN_RIGHT, "X : Dash");
               al_draw_textf(font, al_map_rgb(255, 255, 255), 1270, 690, ALLEGRO_ALIGN_RIGHT, "[  __  ] : Salto");
               al_draw_textf(font, al_map_rgb(255, 255, 255), 1270, 700, ALLEGRO_ALIGN_RIGHT, "[  __  ] en el aire : Parry");
               al_draw_textf(font, al_map_rgb(255, 255, 255), 1270, 710, ALLEGRO_ALIGN_RIGHT, "Z : Disparo");
            }

            if(nivel == 0) // Dibuja el respectivo mapa (!? PASAR A FUNCION DESPUES)
            {
               for(i = 0; i < ANCHO_MAPA; i++)
               {
                  for(j = 0; j < LARGO_MAPA; j++)
                  {
                     drawX = j*LARGO - camaraX;
                     drawY = i*ANCHO - camaraY;
                     if(mapa[i][j] == '.' || mapa[i][j] == '#')
                     {
                        al_draw_bitmap(sprites.arbol_mapa, drawX, drawY, 0);
                     }
                     if(mapa[i][j] == '*')
                     {
                        al_draw_bitmap(sprites.pasto_mapa, drawX, drawY, 0);
                     }
                     if(mapa[i][j] == 'i') // Punto de inicio
                     {
                        al_draw_bitmap(sprites.pasto_mapa, drawX, drawY, 0);
                     }
                  }
               }
               for(i = 0; i < ANCHO_MAPA; i++)
               {
                  for(j = 0; j < LARGO_MAPA; j++)
                  {
                     drawX = j*LARGO - camaraX;
                     drawY = i*ANCHO - camaraY;
                     if(mapa[i][j] == '1')
                     {
                        al_draw_bitmap(sprites.portada_zona_1, drawX - OFFSET_PORTADA_ZONA_X, drawY - OFFSET_PORTADA_ZONA_Y, 0);
                     }
                     if(mapa[i][j] == '2')
                     {
                        al_draw_bitmap(sprites.portada_zona_2, drawX - OFFSET_PORTADA_ZONA_X, drawY - OFFSET_PORTADA_ZONA_Y, 0);
                     }
                     if(mapa[i][j] == 'T')
                     {
                        al_draw_bitmap(sprites.tienda, drawX - 80, drawY - 80, 0);
                     }
                  }
               }
            }
            else
            {
               for(j = 0; j < LARGO_MAPA; j++) // Auto tiling desde el vacio para arriba, para tierra regular y enredaderas.
               {
                  for(i = ANCHO_MAPA - 1; i >= 0; i--)
                  {
                     for(cont = ANCHO_MAPA - 1; cont >= 0; cont--) // Revisa si la columna contiene enredaderas para dibujar ellas en vez de tierra.
                     {
                        if(mapa[cont][j] == '/')
                        {
                           columnaEnredaderas = true;
                           break;
                        }
                     }
                     drawX = j*LARGO - camaraX;
                     drawY = i*ANCHO - camaraY;
                     if(mapa[i][j] == '#' || mapa[i][j] == '/') // Si encuentra tierra o enredaderas:
                     {
                        break; // Rompe el bucle for, continuando a la siguiente columna.
                     }
                     if(mapa[i][j] == '.') // Dibuja tierra o enredaderas segun el valor de columnaEnredaderas.
                     {
                        if(columnaEnredaderas == true)
                        {
                           al_draw_bitmap(sprites.agua, drawX, drawY, 0);
                        }
                        else
                        {
                           switch(zona)
                           {
                              case 1:
                              al_draw_bitmap(sprites.tierra, drawX, drawY, 0);
                              break;
                              case 2:
                              al_draw_bitmap(sprites.madera, drawX, drawY, 0);
                              break;
                              default:
                              al_draw_bitmap(sprites.bloque_tutorial, drawX, drawY, 0);
                              break;
                           }
                        }
                     }
                  }
                  columnaEnredaderas = false;
               }
               for(i = ANCHO_MAPA - 1; i >= 0; i--) // Dibuja el respectivo mapa (!? PASAR A FUNCION DESPUES)
               {
                  for(j = 0; j < LARGO_MAPA; j++)
                  {
                     drawX = j*LARGO - camaraX;
                     drawY = i*ANCHO - camaraY;
                     colorSecreto = 0.5 + transparenciaBloque / 2;
                     if(mapa[i][j] == '#') // Suelo
                     {
                        if(i == 0)
                        {
                           switch(zona)
                           {
                              case 1:
                              al_draw_bitmap(sprites.tierra, drawX, drawY, 0);
                              break;
                              case 2:
                              al_draw_bitmap(sprites.madera, drawX, drawY, 0);
                              break;
                              default:
                              al_draw_bitmap(sprites.bloque_tutorial, drawX, drawY, 0);
                              break;
                           }
                        }
                        if(i - 1 >= 0)
                        {
                           if(mapa[i - 1][j] != '#' && mapa[i - 1][j] != '+') // Dibuja pasto si la casilla superior es vacio.
                           {
                              switch(zona)
                              {
                                 case 1:
                                 al_draw_bitmap(sprites.pasto, drawX, drawY, 0);
                                 break;
                                 case 2:
                                 al_draw_bitmap(sprites.alfombra, drawX, drawY, 0);
                                 break;
                                 default:
                                 al_draw_bitmap(sprites.bloque_tutorial, drawX, drawY, 0);
                                 break;
                              }
                           }
                           else // Dibuja tierra si la casilla superior es pasto.
                           {
                              switch(zona)
                              {
                                 case 1:
                                 al_draw_bitmap(sprites.tierra, drawX, drawY, 0);
                                 break;
                                 case 2:
                                 al_draw_bitmap(sprites.madera, drawX, drawY, 0);
                                 break;
                                 default:
                                 al_draw_bitmap(sprites.bloque_tutorial, drawX, drawY, 0);
                                 break;
                              }
                           }
                        }
                     }
                     if(mapa[i][j] == '+') // Tierra (auto tiling secundario)
                     {
                        for(cont = 0; i >= cont; cont++)
                        {
                           /*if(mapa[i - cont][j] == '#')
                           {
                              break;
                           }*/
                           switch(zona)
                           {
                              case 1:
                              al_draw_bitmap(sprites.tierra, drawX, drawY, 0);
                              break;
                              case 2:
                              al_draw_bitmap(sprites.madera, drawX, drawY, 0);
                              break;
                              default:
                              al_draw_bitmap(sprites.bloque_tutorial, drawX, drawY, 0);
                              break;
                           }
                        }
                     }
                     if(mapa[i][j] == 'x') // Tierra (fondo)
                     {
                        for(cont = 0; i >= cont; cont++)
                        {
                           if(mapa[i - cont][j] == '#' || mapa[i - cont][j] == '-')
                           {
                              break;
                           }
                           switch(zona)
                           {
                              case 1:
                              al_draw_tinted_bitmap(sprites.tierra, al_map_rgba_f(0.5, 0.5, 0.5, 1), drawX, (i - cont)*ANCHO - camaraY, 0);
                              break;
                              case 2:
                              al_draw_tinted_bitmap(sprites.madera, al_map_rgba_f(0.5, 0.5, 0.5, 1), drawX, (i - cont)*ANCHO - camaraY, 0);
                              break;
                              default:
                              al_draw_tinted_bitmap(sprites.bloque_tutorial, al_map_rgba_f(0.5, 0.5, 0.5, 1), drawX, (i - cont)*ANCHO - camaraY, 0);
                              break;
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
                        switch(zona)
                        {
                           case 1:
                           al_draw_bitmap(sprites.agua, drawX, drawY, 0);
                           al_draw_bitmap(sprites.tope_agua, drawX, drawY - OFFSET_TOPE_Y, 0);
                           break;
                           case 2:
                           al_draw_bitmap(sprites.pinchos, drawX, drawY, 0);
                           al_draw_bitmap(sprites.tope_pinchos, drawX, drawY  - ANCHO, 0);
                           break;
                        }
                     }
                     if(mapa[i][j] == 'p') // Parry enemigo
                     {
                        switch(zona) 
                        {
                           case 1:
                           //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 0.5, 0.6, 0.2));
                           al_draw_rotated_bitmap(sprites.flor, CENTRO_FLOR_X, CENTRO_FLOR_Y, drawX - OFFSET_FLOR_X, drawY - OFFSET_FLOR_Y, ANGULO_BASE_FLOR + anguloFlores, 0);
                           break;
                           default:
                           al_draw_bitmap(sprites.enemigoE, drawX, drawY, 0);
                           break;
                        }
                     }
                     if(mapa[i][j] == 'H') // Corazon
                     {
                        //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(0, 0.4, 0, 0.2));
                     }
                     if(mapa[i][j] == '?') // Bloque de secreto
                     {
                        al_draw_tinted_bitmap(sprites.tierra, al_map_rgba_f(colorSecreto, colorSecreto, colorSecreto, 1), drawX, drawY, 0);
                     }
                     if(mapa[i][j] == 'v') // Portal local de entrada
                     {
                        al_draw_tinted_bitmap(sprites.tierra, al_map_rgba_f(colorSecreto, colorSecreto, colorSecreto, 1), drawX, drawY, 0);
                        al_draw_tinted_bitmap(sprites.portal_local, al_map_rgba_f(colorSecreto, colorSecreto, colorSecreto, 1 - transparenciaBloque), drawX, drawY, 0);
                     }
                     if(mapa[i][j] == '^') // Portal local de salida
                     {
                        al_draw_tinted_bitmap(sprites.portal_local, al_map_rgba_f(0.5, 0.5, 0.5, 1), drawX, drawY, 0);
                     }
                     if(mapa[i][j] == 'O') // Portal
                     {
                        //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 0.5, 0, 0.2));
                        al_draw_bitmap(sprites.puerta, drawX, drawY, 0);
                     }
                     if(mapa[i][j] == '@') // Meta
                     {
                        al_draw_bitmap(sprites.meta, drawX - OFFSET_META_X, drawY - OFFSET_META_Y, 0);
                     }
                     if(mapa[i][j] == 'i') // Punto de inicio
                     {
                        //al_draw_filled_rectangle(drawX, drawY, drawX + LARGO, drawY + ANCHO, al_map_rgba_f(1, 1, 1, 0.2));
                        if(nivel == 0)
                        {
                           al_draw_bitmap(sprites.pasto_mapa, drawX, drawY, 0);
                        }
                     }
                  }
               }
               for(i = ANCHO_MAPA - 1; i >= 0; i--) // Elementos de mayor prioridad.
               {
                  for(j = 0; j < LARGO_MAPA; j++)
                  {
                     drawX = j*LARGO - camaraX;
                     drawY = i*ANCHO - camaraY;
                     if(mapa[i][j] == ')')
                     {
                        al_draw_rotated_bitmap(sprites.sombrero_entrada, 176, 176, drawX + 20, drawY + 80, ALLEGRO_PI * 3 / 2, 0);
                     }
                     if(mapa[i][j] == '(')
                     {
                        al_draw_rotated_bitmap(sprites.sombrero_salida, 176, 176, drawX, drawY + 80, ALLEGRO_PI / 2, 0);
                     }
                     if(mapa[i][j] == 'u')
                     {
                        al_draw_bitmap(sprites.sombrero_entrada, drawX - 80, drawY - 60, 0);
                     }
                     if(mapa[i][j] == 'n')
                     {
                        if(i == 15 && j == 146)
                        {
                           al_draw_bitmap(sprites.sombrero_salida, drawX - 80, drawY - 60, 0);
                        }
                        else
                        {
                           al_draw_rotated_bitmap(sprites.sombrero_salida, 176, 176, drawX + 80, drawY, ALLEGRO_PI, 0);
                        }
                     }
                  }
               }
            }
            if(nivel == 0)
            {
               switch(jugador.armaEquipada)
               {
                  case 0:
                  al_draw_bitmap(sprites.logo_peashooter, 0, 660, 0);
                  al_draw_textf(font, al_map_rgb(0, 0, 0), 0, 710, 0, "Peashooter");
                  break;
                  case 1:
                  al_draw_bitmap(sprites.logo_chaser, 0, 660, 0);
                  al_draw_textf(font, al_map_rgb(0, 0, 0), 0, 710, 0, "Chaser");
                  break;
                  case 2:
                  al_draw_bitmap(sprites.logo_roundabout, 0, 660, 0);
                  al_draw_textf(font, al_map_rgb(0, 0, 0), 0, 710, 0, "Roundabout");
                  break;
               }
            }
            else
            {
               switch(jugador.armaEquipada)
               {
                  case 0:
                  al_draw_bitmap(sprites.logo_peashooter, 0, 558, 0);
                  al_draw_textf(font, al_map_rgb(0, 0, 0), 0, 618, 0, "Peashooter");
                  break;
                  case 1:
                  al_draw_bitmap(sprites.logo_chaser, 0, 558, 0);
                  al_draw_textf(font, al_map_rgb(0, 0, 0), 0, 618, 0, "Chaser");
                  break;
                  case 2:
                  al_draw_bitmap(sprites.logo_roundabout, 0, 558, 0);
                  al_draw_textf(font, al_map_rgb(0, 0, 0), 0, 618, 0, "Roundabout");
                  break;
               }
            }

            if((int)(valorTimer) % SPIN_RATE_FLOR == 0) // Animaciones de elementos estaticos.
            {
               anguloFlores *= -1;
            }

            //if(SPRITES == true)
            for(i = 0; i < MAX_BALAS; i++)
            {
               if(jugador.balas[i].activa == true)
               {
                  drawBalasX = jugador.balas[i].posX - camaraX;
                  drawBalasY = jugador.balas[i].posY - camaraY;
                  if(jugador.balas[i].direccion.Arriba == true)
                  {
                     if(jugador.balas[i].direccion.Izquierda == true)
                     {
                        jugador.balas[i].angulo = ALLEGRO_PI * 5 / 4;
                        jugador.balas[i].offsetX = -24;
                        jugador.balas[i].offsetY = -28;
                        switch(jugador.armaEquipada)
                        {
                           case 0:
                           al_draw_rotated_bitmap(sprites.bala0, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 1:
                           al_draw_rotated_bitmap(sprites.bala1, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 2:
                           al_draw_rotated_bitmap(sprites.bala2, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 3:
                           al_draw_rotated_bitmap(sprites.bala3, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                        }
                     }
                     else if(jugador.balas[i].direccion.Derecha == true)
                     {
                        jugador.balas[i].offsetX = 20;
                        jugador.balas[i].offsetY = -24;
                        jugador.balas[i].angulo = ALLEGRO_PI * 7 / 4;
                        switch(jugador.armaEquipada)
                        {
                           case 0:
                           al_draw_rotated_bitmap(sprites.bala0, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 1:
                           al_draw_rotated_bitmap(sprites.bala1, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 2:
                           al_draw_rotated_bitmap(sprites.bala2, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 3:
                           al_draw_rotated_bitmap(sprites.bala3, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                        }
                     }
                     else
                     {
                        jugador.balas[i].angulo = ALLEGRO_PI * 3 / 2;
                        jugador.balas[i].offsetX = 0;
                        jugador.balas[i].offsetY = -36;
                        switch(jugador.armaEquipada)
                        {
                           case 0:
                           al_draw_rotated_bitmap(sprites.bala0, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 1:
                           al_draw_rotated_bitmap(sprites.bala1, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 2:
                           al_draw_rotated_bitmap(sprites.bala2, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 3:
                           al_draw_rotated_bitmap(sprites.bala3, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                        }
                     }
                  }
                  else if(jugador.balas[i].direccion.Abajo == true)
                  {
                     if(jugador.balas[i].direccion.Izquierda == true)
                     {
                        jugador.balas[i].angulo = ALLEGRO_PI * 3 / 4;
                        switch(jugador.armaEquipada)
                        {
                           case 0:
                           al_draw_rotated_bitmap(sprites.bala0, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 1:
                           al_draw_rotated_bitmap(sprites.bala1, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 2:
                           al_draw_rotated_bitmap(sprites.bala2, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 3:
                           al_draw_rotated_bitmap(sprites.bala3, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                        }
                     }
                     else if(jugador.balas[i].direccion.Derecha == true)
                     {
                        jugador.balas[i].angulo = ALLEGRO_PI / 4;
                        switch(jugador.armaEquipada)
                        {
                           case 0:
                           al_draw_rotated_bitmap(sprites.bala0, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 1:
                           al_draw_rotated_bitmap(sprites.bala1, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 2:
                           al_draw_rotated_bitmap(sprites.bala2, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 3:
                           al_draw_rotated_bitmap(sprites.bala3, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                        }
                     }
                     else
                     {
                        jugador.balas[i].angulo = ALLEGRO_PI / 2;
                        switch(jugador.armaEquipada)
                        {
                           case 0:
                           al_draw_rotated_bitmap(sprites.bala0, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 1:
                           al_draw_rotated_bitmap(sprites.bala1, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 2:
                           al_draw_rotated_bitmap(sprites.bala2, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                           case 3:
                           al_draw_rotated_bitmap(sprites.bala3, 4, 4, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, jugador.balas[i].angulo, 0);
                           break;
                        }
                     }
                  }
                  else if(jugador.balas[i].direccion.Izquierda == true)
                  {
                     jugador.balas[i].offsetX = 4;
                     jugador.balas[i].offsetY = 4;
                     switch(jugador.armaEquipada)
                     {
                        case 0:
                        al_draw_bitmap(sprites.bala0, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, ALLEGRO_FLIP_HORIZONTAL);
                        break;
                        case 1:
                        al_draw_bitmap(sprites.bala1, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, ALLEGRO_FLIP_HORIZONTAL);
                        break;
                        case 2:
                        al_draw_bitmap(sprites.bala2, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, ALLEGRO_FLIP_HORIZONTAL);
                        break;
                        case 3:
                        al_draw_bitmap(sprites.bala3, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, ALLEGRO_FLIP_HORIZONTAL);
                        break;
                     }
                  }
                  else
                  {
                     jugador.balas[i].offsetX = 32;
                     jugador.balas[i].offsetY = 4;
                     switch(jugador.armaEquipada)
                     {
                        case 0:
                        al_draw_bitmap(sprites.bala0, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, 0);
                        break;
                        case 1:
                        al_draw_bitmap(sprites.bala1, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, 0);
                        break;
                        case 2:
                        al_draw_bitmap(sprites.bala2, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, 0);
                        break;
                        case 3:
                        al_draw_bitmap(sprites.bala3, drawBalasX - jugador.balas[i].offsetX, drawBalasY - jugador.balas[i].offsetY, 0);
                        break;
                     }
                  }
                  //al_draw_filled_rectangle(drawBalasX, drawBalasY - camaraY, drawBalasX + LARGO_BALA, drawBalasY + ANCHO_BALA, al_map_rgb(255, 255, 0));
               }
            }

            for(contEnemigos = 0; contEnemigos < cantidadEnemigosA; contEnemigos++) // Enemigos A
            {
               drawEnemigosX = enemigosA[contEnemigos].posX - camaraX;
               drawEnemigosY = enemigosA[contEnemigos].posY - camaraY;
               if(enemigosA[contEnemigos].activo == true)
               {
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
               else if(enemigosA[contEnemigos].frameExplosion < FRAMES_EXPLOSION)
               {
                  al_draw_bitmap(sprites.explosion[enemigosA[contEnemigos].frameExplosion], drawEnemigosX - OFFSET_EXPLOSION_X, drawEnemigosY - OFFSET_EXPLOSION_Y, 0);
               }
               if((int)(valorTimer) % FRAME_RATE_EXPLOSION == 0)
               {
                  enemigosA[contEnemigos].frameExplosion++;
               }
            }
            for(contEnemigos = 0; contEnemigos < cantidadEnemigosC; contEnemigos++) // Enemigos C
            {
               drawEnemigosX = enemigosC[contEnemigos].posX - camaraX;
               drawEnemigosY = enemigosC[contEnemigos].posY - camaraY;
               if(enemigosC[contEnemigos].activo == true)
               {
                   //al_draw_filled_rectangle(drawEnemigosX, drawEnemigosY, drawEnemigosX + LARGO, drawEnemigosY + ANCHO, al_map_rgba_f(0.4, 0.4, 0.4, 0.2));
                  al_draw_rotated_bitmap(sprites.enemigoC, (int)(LARGO / 2), (int)(ANCHO / 2), drawEnemigosX - OFFSET_ENEMIGO_C_X, drawEnemigosY - OFFSET_ENEMIGO_C_Y, -(enemigosC[contEnemigos].posX - enemigosC[contEnemigos].nodoCX) / 200, 0);
               }
               else if(enemigosC[contEnemigos].frameExplosion < FRAMES_EXPLOSION)
               {
                  al_draw_bitmap(sprites.explosion[enemigosC[contEnemigos].frameExplosion], drawEnemigosX - OFFSET_EXPLOSION_X, drawEnemigosY - OFFSET_EXPLOSION_Y, 0);
               }
               if((int)(valorTimer) % FRAME_RATE_EXPLOSION == 0)
               {
                  enemigosC[contEnemigos].frameExplosion++;
               }
            }
            for(contEnemigos = 0; contEnemigos < cantidadEnemigosE; contEnemigos++) // Enemigos E
            {
               drawEnemigosX = enemigosE[contEnemigos].posX - camaraX;
               drawEnemigosY = enemigosE[contEnemigos].posY - camaraY;
               al_draw_rotated_bitmap(sprites.enemigoE, CENTRO_ENEMIGO_E_X, CENTRO_ENEMIGO_E_Y, drawEnemigosX + (int)(LARGO / 2), drawEnemigosY + (int)(ANCHO / 2), valorTimer / SPIN_RATE_ENEMIGO_E, 0);
               if(enemigosE[contEnemigos].activo == true)
               {
                  //al_draw_filled_rectangle(drawEnemigosX, drawEnemigosY, drawEnemigosX + LARGO, drawEnemigosY + ANCHO, al_map_rgba_f(1, 0.3, 0.3, 0.2));
                  al_draw_rotated_bitmap(sprites.enemigoE_pinchos, CENTRO_ENEMIGO_E_X, CENTRO_ENEMIGO_E_Y, drawEnemigosX + (int)(LARGO / 2), drawEnemigosY + (int)(ANCHO / 2), valorTimer / SPIN_RATE_ENEMIGO_E, 0);
               }
               /*else if(enemigosE[contEnemigos].frameExplosion < FRAMES_EXPLOSION)
               {
                  al_draw_tinted_bitmap(sprites.explosion[enemigosE[contEnemigos].frameExplosion], al_map_rgba_f(1, 0.3, 0.3, 0.2), drawEnemigosX - 22, drawEnemigosY - 2, 0);
               }*/
               if((int)(valorTimer) % FRAME_RATE_EXPLOSION == 0)
               {
                  enemigosE[contEnemigos].frameExplosion++;
                  if(enemigosE[contEnemigos].frameExplosion == FRAME_RATE_EXPLOSION)
                  {
                     enemigosE[contEnemigos].frameExplosion = 0;
                  }
               }
            }

            for(cont = 0; cont < cantidadMonedas; cont++)
            {
               drawX = monedasMapa[cont].posX - camaraX;
               drawY = monedasMapa[cont].posY - camaraY;
               if(monedasMapa[cont].activo == true)
               {
                  al_draw_bitmap(sprites.moneda[monedasMapa[cont].frame], drawX, drawY, 0);
                  if((int)(valorTimer) % FRAME_RATE_MONEDA == 0)
                  {
                     monedasMapa[cont].frame++;
                     if(monedasMapa[cont].frame == FRAME_RATE_EXPLOSION)
                     {
                        monedasMapa[cont].frame = 0;
                     }
                  }
               }
            }

            if(nivel == 0)  // Animacion del personaje dentro del mapa global.
            {
               if(jugador.direccion.Arriba == true && jugador.direccion.Derecha == true)
               {
                  al_draw_bitmap(sprites.jugador_mapa_arrder, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
               else if(jugador.direccion.Arriba == true && jugador.direccion.Izquierda == true)
               {
                  al_draw_bitmap(sprites.jugador_mapa_arrizq, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
               else if(jugador.direccion.Abajo == true && jugador.direccion.Izquierda == true)
               {
                  al_draw_bitmap(sprites.jugador_mapa_abaizq, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
               else if(jugador.direccion.Abajo == true && jugador.direccion.Derecha == true)
               {
                  al_draw_bitmap(sprites.jugador_mapa_abader, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
               else if(jugador.direccion.Derecha == true)
               {
                  al_draw_bitmap(sprites.jugador_mapa_der, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
               else if(jugador.direccion.Arriba == true)
               {
                  al_draw_bitmap(sprites.jugador_mapa_arr, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
               else if(jugador.direccion.Izquierda == true)
               {
                  al_draw_bitmap(sprites.jugador_mapa_izq, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
               else if(jugador.direccion.Abajo == true)
               {
                  al_draw_bitmap(sprites.jugador_mapa_aba, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
               else
               {
                  al_draw_bitmap(sprites.jugador_mapa_quieto, jugador.posX - camaraX, jugador.posY - camaraY, 0);
               }
            }
            else
            {
               //al_draw_filled_rectangle(jugador.posX - camaraX, jugador.posY - camaraY, jugador.posX - camaraX + LARGO, jugador.posY - camaraY + ANCHO, al_map_rgba_f(1, 1, 0, 0.8));
               if(golpeFrames > 0)
               {
                  if(jugador.orientacion == -1)
                  {
                     al_draw_bitmap(sprites.jugador_golpe, jugador.posX - camaraX - OFFSET_GOLPE_X, jugador.posY - camaraY - OFFSET_GOLPE_Y, ALLEGRO_FLIP_HORIZONTAL);
                  }
                  else
                  {
                     al_draw_bitmap(sprites.jugador_golpe, jugador.posX - camaraX - OFFSET_GOLPE_X, jugador.posY - camaraY - OFFSET_GOLPE_Y, 0);
                  }
               }
               else if(parryFrames > 0) // Animacion del personaje fuera del mapa global.
               {
                  if(jugador.orientacion == -1)
                  {
                     al_draw_bitmap(sprites.jugador_parry[jugador.frameSaltoParry], jugador.posX - camaraX - OFFSET_PARRY_X, jugador.posY - camaraY - OFFSET_PARRY_Y, ALLEGRO_FLIP_HORIZONTAL);
                  }
                  else
                  {
                     al_draw_bitmap(sprites.jugador_parry[jugador.frameSaltoParry], jugador.posX - camaraX - OFFSET_PARRY_X, jugador.posY - camaraY - OFFSET_PARRY_Y, 0);
                  }
                  if((int)(valorTimer) % FRAME_RATE_SALTO_PARRY == 0)
                  {
                     jugador.frameSaltoParry++; // Aumenta el frame, se devuelve a 0 si se pasa para que siempre cicle hacia la derecha.
                     if(jugador.frameSaltoParry == FRAMES_SALTO_PARRY)
                     {
                        jugador.frameSaltoParry = 0;
                     }
                  }
               }
               else if(dashFrames > 0)
               {
                  if(jugadorEnAire == true)
                  {
                     if(jugador.orientacion == -1)
                     {
                        al_draw_bitmap(sprites.jugador_dash, jugador.posX - camaraX - OFFSET_DASH_X, jugador.posY - camaraY - OFFSET_DASH_AIRE_Y, ALLEGRO_FLIP_HORIZONTAL);
                     }
                     else
                     {
                        al_draw_bitmap(sprites.jugador_dash, jugador.posX - camaraX - OFFSET_DASH_X, jugador.posY - camaraY - OFFSET_DASH_AIRE_Y, 0);
                     }
                  }
                  else
                  {
                     if(jugador.orientacion == -1)
                     {
                        al_draw_bitmap(sprites.jugador_dash, jugador.posX - camaraX - OFFSET_DASH_X, jugador.posY - camaraY - OFFSET_DASH_SUELO_Y, ALLEGRO_FLIP_HORIZONTAL);
                     }
                     else
                     {
                        al_draw_bitmap(sprites.jugador_dash, jugador.posX - camaraX - OFFSET_DASH_X, jugador.posY - camaraY - OFFSET_DASH_SUELO_Y, 0);
                     }
                  }
               }
               else if(jugadorEnAire == true)
               {
                  if(jugador.orientacion == -1)
                  {
                     al_draw_bitmap(sprites.jugador_salto[jugador.frameSaltoParry], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_SALTO_Y, ALLEGRO_FLIP_HORIZONTAL);
                  }
                  else
                  {
                     al_draw_bitmap(sprites.jugador_salto[jugador.frameSaltoParry], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_SALTO_Y, 0);
                  }
                  if((int)(valorTimer) % FRAME_RATE_SALTO_PARRY == 0)
                  {
                     jugador.frameSaltoParry++; // Aumenta el frame, se devuelve a 0 si se pasa para que siempre cicle hacia la derecha.
                     if(jugador.frameSaltoParry == FRAMES_SALTO_PARRY)
                     {
                        jugador.frameSaltoParry = 0;
                     }
                  }
               }
               else if(jugador.agachado == true) // Agachado.
               {
                  if(jugador.direccion.Z == true) // Disparo agachado.
                  {
                     if(jugador.orientacion == -1)
                     {
                        al_draw_bitmap(sprites.jugador_agachado_disparo[jugador.frameDisparo], jugador.posX - OFFSET_AGACHADO_X, jugador.posY - camaraY - OFFSET_AGACHADO_DISPARO_Y, ALLEGRO_FLIP_HORIZONTAL);
                     }
                     else
                     {
                        al_draw_bitmap(sprites.jugador_agachado_disparo[jugador.frameDisparo], jugador.posX - camaraX - OFFSET_AGACHADO_X, jugador.posY - camaraY - OFFSET_AGACHADO_DISPARO_Y, 0);
                     }
                     if((int)(valorTimer) % FRAME_RATE_DISPARO == 0)
                     {
                        jugador.frameDisparo++; // Aumenta el frame, se devuelve a 0 si se pasa para que siempre cicle hacia la derecha.
                        if(jugador.frameDisparo == FRAMES_DISPARO)
                        {
                           jugador.frameDisparo = 0;
                        }
                     }
                  }
                  else
                  {
                     if(jugador.orientacion == -1)
                     {
                        al_draw_bitmap(sprites.jugador_agachado, jugador.posX - camaraX - OFFSET_AGACHADO_X, jugador.posY - camaraY - OFFSET_AGACHADO_Y, ALLEGRO_FLIP_HORIZONTAL);
                     }
                     else
                     {
                        al_draw_bitmap(sprites.jugador_agachado, jugador.posX - camaraX - OFFSET_AGACHADO_X, jugador.posY - camaraY - OFFSET_AGACHADO_Y, 0);
                     }
                  }
               }
               
               else if(jugador.direccion.Izquierda == true || jugador.direccion.Derecha == true)
               {
                  if(jugador.direccion.Z == true)
                  {
                     if(jugador.direccion.Arriba == true)
                     {
                        if(jugador.orientacion == -1)
                        {
                           al_draw_bitmap(sprites.jugador_corriendo_disparo_diagonal[jugador.frameCorriendo], jugador.posX - camaraX - OFFSET_DISPARO_X, jugador.posY - camaraY - OFFSET_CORRIENDO_DISPARO_DIAGONAL_Y, ALLEGRO_FLIP_HORIZONTAL);
                        }
                        else
                        {
                           al_draw_bitmap(sprites.jugador_corriendo_disparo_diagonal[jugador.frameCorriendo], jugador.posX - camaraX - OFFSET_DISPARO_X, jugador.posY - camaraY - OFFSET_CORRIENDO_DISPARO_DIAGONAL_Y, 0);
                        }
                     }
                     else
                     {
                        if(jugador.orientacion == -1)
                        {
                           al_draw_bitmap(sprites.jugador_corriendo_disparo_horizontal[jugador.frameCorriendo], jugador.posX - camaraX - OFFSET_DISPARO_X, jugador.posY - camaraY - OFFSET_CORRIENDO_DISPARO_HORIZONTAL_Y, ALLEGRO_FLIP_HORIZONTAL);
                        }
                        else
                        {
                           al_draw_bitmap(sprites.jugador_corriendo_disparo_horizontal[jugador.frameCorriendo], jugador.posX - camaraX - OFFSET_DISPARO_X, jugador.posY - camaraY - OFFSET_CORRIENDO_DISPARO_HORIZONTAL_Y, 0);
                        }
                     }
                  }
                  else
                  {
                     if(jugador.orientacion == -1)
                     {
                        al_draw_bitmap(sprites.jugador_corriendo[jugador.frameCorriendo], jugador.posX - camaraX - OFFSET_CORRIENDO_X, jugador.posY - camaraY - OFFSET_CORRIENDO_Y, ALLEGRO_FLIP_HORIZONTAL);
                     }
                     else
                     {
                        al_draw_bitmap(sprites.jugador_corriendo[jugador.frameCorriendo], jugador.posX - camaraX - OFFSET_CORRIENDO_X, jugador.posY - camaraY - OFFSET_CORRIENDO_Y, 0);
                     }
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
               else if(jugador.direccion.Z == true) // Animacion de disparo:
               {
                  if(jugador.direccion.Arriba == true) // Diagonal hacia arriba.
                  {
                     if(jugador.direccion.Izquierda == true)
                     {
                        al_draw_bitmap(sprites.jugador_disparo_diagonal_arriba[jugador.frameDisparo], jugador.posX - camaraX - OFFSET_DISPARO_X, jugador.posY - camaraY - OFFSET_DISPARO_DIAGONAL_ARRIBA_Y, ALLEGRO_FLIP_HORIZONTAL);
                     }
                     else if(jugador.direccion.Derecha == true)
                     {
                        al_draw_bitmap(sprites.jugador_disparo_diagonal_arriba[jugador.frameDisparo], jugador.posX - camaraX - OFFSET_DISPARO_X, jugador.posY - camaraY - OFFSET_DISPARO_DIAGONAL_ARRIBA_Y, 0);
                     }
                     else // Hacia arriba.
                     {
                        if(jugador.orientacion == -1)
                        {
                           al_draw_bitmap(sprites.jugador_disparo_arriba[jugador.frameDisparo], jugador.posX - camaraX - OFFSET_DISPARO_X, jugador.posY - camaraY - OFFSET_DISPARO_ARRIBA_Y, ALLEGRO_FLIP_HORIZONTAL);
                        }
                        else
                        {
                           al_draw_bitmap(sprites.jugador_disparo_arriba[jugador.frameDisparo], jugador.posX - camaraX - OFFSET_DISPARO_X, jugador.posY - camaraY - OFFSET_DISPARO_ARRIBA_Y, 0);
                        }
                     }
                  }
                  else if(jugador.direccion.Abajo == true) // Diagonal hacia abajo.
                  {
                     if(jugador.direccion.Izquierda == true)
                     {
                        al_draw_bitmap(sprites.jugador_disparo_diagonal_abajo[jugador.frameDisparo], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_DISPARO_DIAGONAL_ABAJO_Y, ALLEGRO_FLIP_HORIZONTAL);
                     }
                     else if(jugador.direccion.Derecha == true)
                     {
                        al_draw_bitmap(sprites.jugador_disparo_diagonal_abajo[jugador.frameDisparo], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_DISPARO_DIAGONAL_ABAJO_Y, 0);
                     }
                     else // Hacia abajo.
                     {
                        if(jugador.orientacion == -1)
                        {
                           al_draw_bitmap(sprites.jugador_disparo_abajo[jugador.frameDisparo], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_DISPARO_ABAJO_Y, ALLEGRO_FLIP_HORIZONTAL);
                        }
                        else
                        {
                           al_draw_bitmap(sprites.jugador_disparo_abajo[jugador.frameDisparo], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_DISPARO_ABAJO_Y, 0);
                        }
                     }
                  }
                  else
                  {
                     if(jugador.orientacion == -1)
                     {
                        al_draw_bitmap(sprites.jugador_disparo_horizontal[jugador.frameDisparo], jugador.posX - camaraX - OFFSET_DISPARO_HORIZONTAL_X, jugador.posY - camaraY - OFFSET_DISPARO_HORIZONTAL_Y, ALLEGRO_FLIP_HORIZONTAL);
                     }
                     else
                     {
                        al_draw_bitmap(sprites.jugador_disparo_horizontal[jugador.frameDisparo], jugador.posX - camaraX - OFFSET_DISPARO_HORIZONTAL_X, jugador.posY - camaraY - OFFSET_DISPARO_HORIZONTAL_Y, 0);
                     }
                  }
                  if((int)(valorTimer) % FRAME_RATE_DISPARO == 0)
                  {
                     jugador.frameDisparo++; // Aumenta el frame, se devuelve a 0 si se pasa para que siempre cicle hacia la derecha.
                     if(jugador.frameDisparo == FRAMES_DISPARO)
                     {
                        jugador.frameDisparo = 0;
                     }
                  }
               }
               else if(jugador.lock == true) // AGREGAR PRIORIDAD SOBRE ANIMACION CORRIENDO Y ANHADIR SPRITES DE DISPARO EN EL LUGAR
               {
                  if(jugador.orientacion == -1)
                  {
                     al_draw_bitmap(sprites.jugador_lock[jugador.frameLock], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_QUIETO_Y, ALLEGRO_FLIP_HORIZONTAL);
                  }
                  else
                  {
                     al_draw_bitmap(sprites.jugador_lock[jugador.frameLock], jugador.posX - camaraX, jugador.posY - camaraY - OFFSET_QUIETO_Y, 0);
                  }
                  if((int)(valorTimer) % FRAME_RATE_LOCK == 0)
                  {
                     jugador.frameLock++; // Aumenta el frame, se devuelve a 0 si se pasa para que siempre cicle hacia la derecha.
                     if(jugador.frameLock == FRAMES_LOCK)
                     {
                        jugador.frameLock = 0;
                     }
                  }
               }
               else
               {
                  if(jugador.orientacion == -1)
                  {
                     al_draw_bitmap(sprites.jugador_quieto[jugador.frameQuieto], jugador.posX - camaraX - OFFSET_QUIETO_X, jugador.posY - camaraY - OFFSET_QUIETO_Y, ALLEGRO_FLIP_HORIZONTAL);
                  }
                  else
                  {
                     al_draw_bitmap(sprites.jugador_quieto[jugador.frameQuieto], jugador.posX - camaraX - OFFSET_QUIETO_X, jugador.posY - camaraY - OFFSET_QUIETO_Y, 0);
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
            }

            if(nivel != 0 && al_get_timer_started(tempVictoria) == false)
            {
               al_draw_textf(font48, al_map_rgb(0, 0, 0), 0, 614, 0, "Vida: %d", jugador.vida);
               al_draw_textf(font48, al_map_rgb(0, 0, 0), 0, 662, 0, "Puntaje: %d", puntajeZona);
            }
            else if(al_get_timer_started(tempVictoria) == true)
            {
               if((int)(valorTimerVictoria) % 10 == 0 || (int)(valorTimerVictoria + 1) % 10 == 0 || (int)(valorTimerVictoria + 2) % 10 == 0 || (int)(valorTimerVictoria + 3) % 10 == 0 || (int)(valorTimerVictoria + 4) % 10 == 0)
               {
                  al_draw_textf(font48, al_map_rgb(255, 255, 255), 640, 150, ALLEGRO_ALIGN_CENTER, "ZONA");
                  al_draw_textf(font48, al_map_rgb(255, 255, 255), 640, 198, ALLEGRO_ALIGN_CENTER, "COMPLETADA");
                  al_draw_textf(font48, al_map_rgb(255, 255, 255), 640, 400, ALLEGRO_ALIGN_CENTER, "Puntaje obtenido: %d", puntajeZona);
               }
               else
               {
                  al_draw_textf(font48, al_map_rgb(0, 0, 0), 640, 150, ALLEGRO_ALIGN_CENTER, "ZONA");
                  al_draw_textf(font48, al_map_rgb(0, 0, 0), 640, 198, ALLEGRO_ALIGN_CENTER, "COMPLETADA");
                  al_draw_textf(font48, al_map_rgb(0, 0, 0), 640, 400, ALLEGRO_ALIGN_CENTER, "Puntaje obtenido: %d", puntajeZona);
               }
            }
            else
            {
               al_draw_textf(font48, al_map_rgb(0, 0, 0), 1280, 0, ALLEGRO_ALIGN_RIGHT, "RANKING:");
               for(cont = 0; cont < cantidadPuntajes; cont++)
               {
                  if(ranking[cont].puntaje > 0)
                  {
                     al_draw_textf(font48, al_map_rgb(0, 0, 0), 1280, (cont + 1) * 50, ALLEGRO_ALIGN_RIGHT, "%s    %d", ranking[cont].nombre, ranking[cont].puntaje);
                  }
                  else
                  {
                     break;
                  }
               }
            }
            //al_draw_filled_rectangle(x - camaraX, y - camaraY, x - camaraX + LARGO, y - camaraY + ANCHO, al_map_rgb(207, 255, 163));
            //al_draw_bitmap(mysha, 100, 100, 0);
            if(nivel == 0)
            {
               al_draw_bitmap(sprites.moneda[3], 0, 0, 0);
               al_draw_textf(font48, al_map_rgb(1, 1, 1), 50, 0, 0, "x %d", jugador.monedas);
               al_draw_textf(font48, al_map_rgb(0, 0, 0), 1232, 652, ALLEGRO_ALIGN_RIGHT, "Z: Entrar a nivel");
            }
            dibujar_menu(&menutienda, font, jugador, armas); // se dibuja encima de todo, al final

            // 3. Dibujar informacion de debug.

            valorTimerGravedad = al_get_timer_count(tempGravedad);
            if(DEBUG == true)
            {
               al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "X: %.1f Y: %.1f", jugador.posX, jugador.posY);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 250, 0, 0, "Temporizador de gravedad: %d", valorTimerGravedad);
               al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 110, 0, "cayendo: %d", cayendo);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 700, 0, "vida: %d", jugador.vida);
               al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 710, 0, "iFrames: %d", iFrames);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 500, 0, "camaraX: %f", camaraX);
               al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 510, 0, "camaraY: %f", camaraY);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 530, 0, "enemigosA[0].direccionMovimiento: %d", enemigosA[0].direccionMovimientoA);
               al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 540, 0, "enemigosA[0].colisionEnemigo: %d", enemigosA[0].colisionEnemigoA);

               if(jugador.direccion.Arriba == true) // Imprime las direcciones ingresadas.
               {
                  al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 70, 0, "^");
               }
               if(jugador.direccion.Abajo == true)
               {
                  al_draw_textf(font, al_map_rgb(255, 255, 255), 10, 90, 0, "v");
               }
               if(jugador.direccion.Izquierda == true)
               {
                  al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 80, 0, "<");
               }
               if(jugador.direccion.Derecha == true)
               {
                  al_draw_textf(font, al_map_rgb(255, 255, 255), 20, 80, 0, ">");
               }

               /*for(i = 0; i < ANCHO_MAPA; i++)
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
               }*/

               al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 100, 0, "DEBUG: jugadorEnAire = %d", jugadorEnAire);
               al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 110, 0, "DEBUG: teclaSoltada = %d", teclaSoltada);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 120, 0, "DEBUG: puedeHacerParry = %d", puedeHacerParry);
               al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 130, 0, "DEBUG: parryFrames = %d", parryFrames);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 150, 0, "monedas = %d", jugador.monedas);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 350, 170, 0, "DEBUG: transparenciaBloque = %f", transparenciaBloque);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 190, 0, "GRAVEDAD = %d", GRAVEDAD);
               al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 200, 0, "dashFrames: %d", dashFrames);

               al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 220, 0, "DEBUG: MOVIMIENTO = %d", MOVIMIENTO);
               al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 230, 0, "DEBUG: jugador.direccionDash = %d", jugador.direccionDash);
            }

            // 4. Ajustar ciertas variables al final de un frame.

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
            if(golpeFrames > 0)
            {
               golpeFrames--;
            }
            if(jugador.disparoCD > 0)
            {
               jugador.disparoCD--;
            }
            if(jugador.cooldownPortal > 0)
            {
               jugador.cooldownPortal--;
            }
            if(atravesando == true)
            {
               if(transparenciaBloque > 0)
               {
                  transparenciaBloque -= 0.02;
               }
            }
            else
            {
               if(transparenciaBloque < 1)
               {
                  transparenciaBloque += 0.02;
               }
            }
         } // Llave de else de menu.

         al_flip_display();

         redraw = false;
      }
   } 

   //al_destroy_bitmap(mysha);
   al_destroy_font(font);
   al_destroy_font(font48);
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
   if(x1+largo1>=x2 && x1<=x2+largo2 && y1+ancho1>=y2 && y1<=y2+ancho2)
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
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 20, 0, "DEBUG: condicion 1: true");
      chequeoColision++;
   }
   if(*sueloX+LARGO>entidad.posX)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 30, 0, "DEBUG: condicion 2: true");
      chequeoColision++;
   }
   if(entidad.posY+ANCHO>*sueloY)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 40, 0, "DEBUG: condicion 3: true");
      chequeoColision++;
   }
   if(*sueloY+ANCHO>entidad.posY)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 50, 0, "DEBUG: condicion 4: true");
      chequeoColision++;
   }

   if(chequeoColision==4)
   {
      return true;
   }

   return false;
}

bool collideAnticipado(ALLEGRO_FONT *font, entidad entidad, float *sueloX, float *sueloY)
{
   int chequeoColision = 0;

   if(entidad.posX+LARGO+2>*sueloX)
   {
      chequeoColision++;
   }
   if(*sueloX+LARGO+2>entidad.posX)
   {
      chequeoColision++;
   }
   if(entidad.posY+ANCHO+2>*sueloY)
   {
      chequeoColision++;
   }
   if(*sueloY+ANCHO+2>entidad.posY)
   {
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
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 120, 0, "DEBUG: condicion 1: true");
      chequeoColision++;
   }
   if(*sueloX + LARGO > entidad.posParryX)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 130, 0, "DEBUG: condicion 2: true");
      chequeoColision++;
   }
   if(entidad.posParryY + ANCHO * 2 > *sueloY)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 140, 0, "DEBUG: condicion 3: true");
      chequeoColision++;
   }
   if(*sueloY + ANCHO > entidad.posParryY)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 150, 0, "DEBUG: condicion 4: true");
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

   if(entidad.posX+LARGO-5>*sueloX)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 20, 0, "DEBUG: condicion 1: true");
      chequeoColision++;
   }
   if(*sueloX+LARGO-5>entidad.posX)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 30, 0, "DEBUG: condicion 2: true");
      chequeoColision++;
   }
   if(entidad.posY + ANCHO >= *sueloY) // **** Se puede reutilizar en la funcion "collide" (posY+ANCHO+1>*sueloY)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 40, 0, "DEBUG: condicion piso: true");
      chequeoColision++;
   }
   if(*sueloY+ANCHO>entidad.posY)
   {
      //al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 50, 0, "DEBUG: condicion 4: true");
      chequeoColision++;
   }   

   if(chequeoColision == 4)
   {
      return true;
   }

   return false;
}

float anularMovimientoX(entidad *entidad, float posXAnterior, float *sueloX)
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

float anularMovimientoY(entidad *entidad, float posYAnterior, float *sueloY, ALLEGRO_TIMER* tempGravedad)
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

bool balaDiagonal(entidad jugador, int numeroBala)
{
   if(jugador.balas[numeroBala].direccion.Arriba == true || jugador.balas[numeroBala].direccion.Abajo == true)
   {
      if(jugador.balas[numeroBala].direccion.Izquierda == true || jugador.balas[numeroBala].direccion.Derecha == true)
      {
         return true;
      }
   }
   if(jugador.balas[numeroBala].direccion.Izquierda == true || jugador.balas[numeroBala].direccion.Derecha == true)
   {
      if(jugador.balas[numeroBala].direccion.Arriba == true || jugador.balas[numeroBala].direccion.Abajo == true)
      {
         return true;
      }
   }

   return false;
}

entidad obtenerEnemigoMasCercano(entidad jugador, entidad enemigosA[], int cantidadEnemigosA, entidad enemigosC[], int cantidadEnemigosC) 
{
   int i;
   entidad enemigoCercano;
   enemigoCercano.activo = false; // Retorna un enemigo inactivo si no hay ninguno en el mapa
   
   float distanciaMinima = 999999.0f; // Un valor inicial alto
   float distanciaActual;

   // Revisar arreglo de enemigos A
   for(i = 0; i < cantidadEnemigosA; i++)
   {
      if(enemigosA[i].activo == true) 
      {
         distanciaActual = distancia(jugador.posX, enemigosA[i].posX, jugador.posY, enemigosA[i].posY);
         if(distanciaActual < distanciaMinima)
         {
            distanciaMinima = distanciaActual;
            enemigoCercano = enemigosA[i];
         }
      }
   }

   // Revisar arreglo de enemigos C
   for(i = 0; i < cantidadEnemigosC; i++)
   {
      if(enemigosC[i].activo == true)
      {
         distanciaActual = distancia(jugador.posX, enemigosC[i].posX, jugador.posY, enemigosC[i].posY);
         if(distanciaActual < distanciaMinima)
         {
            distanciaMinima = distanciaActual;
            enemigoCercano = enemigosC[i];
         }
      }
   }

   return enemigoCercano;
}

void cargarMapa(VARIABLES_CARGARMAPA)
{
   FILE *contenidoMapa;
   int i, j, contMonedasMapa = 0;
   float puntoX, puntoY;
   char valorRecibido;

   switch(nivel)
   {
      case -1:
      contenidoMapa = fopen("mapa-1.txt", "r");
      must_init(contenidoMapa, "mapa-1");
      break;

      case 0:
      contenidoMapa = fopen("mapa0.txt", "r");
      must_init(contenidoMapa, "mapa0");
      break;

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

      case 4:
      contenidoMapa = fopen("mapa4.txt", "r");
      must_init(contenidoMapa, "mapa4");
      break;

      case 5:
      contenidoMapa = fopen("mapa5.txt", "r");
      must_init(contenidoMapa, "mapa5");
      break;

      case 6:
      contenidoMapa = fopen("mapa6.txt", "r");
      must_init(contenidoMapa, "mapa6");
      break;

      default:
      printf("no existe el nivel\n");
      exit(1);
   }

   if(nivel == 0)
   {
      *GRAVEDAD = false;
   }
   else
   {
      *GRAVEDAD = true;
   }

   for(i = 0; i < ANCHO_MAPA; i++)
   {
      for(j = 0; j < LARGO_MAPA; j++)
      {
         if(fscanf(contenidoMapa, " %c", &valorRecibido) != EOF)
         {
            mapa[i][j] = valorRecibido;
            puntoX = j * LARGO;
            puntoY = i * ANCHO;
            if(valorRecibido == 'A') // Define las variables miembro de las casillas del arreglo de enemigos
            {
               if(*cantidadEnemigosA < MAX_ENEMIGOS)
               {
                  // revisar esto, por ejemplo: si coloco mas enemigos que el maximo, se cae
                  // la solucion: llegado a esta parte, recorrer el arreglo de enemigos y buscar un espacio para el nuevo enemigo
                  //tipo_enemigo
                  enemigosA[*cantidadEnemigosA].activo = true;
                  enemigosA[*cantidadEnemigosA].posX = puntoX;
                  enemigosA[*cantidadEnemigosA].posY = puntoY;
                  enemigosA[*cantidadEnemigosA].vida = VIDA_ENEMIGO_A;
                  enemigosA[*cantidadEnemigosA].direccionMovimientoA = VELOCIDAD_ENEMIGO_A;
                  enemigosA[*cantidadEnemigosA].colisionEnemigoA = false; // No es necesario inicializar puntoColisionA.
                  enemigosA[*cantidadEnemigosA].cooldownPortal = 0;
                  enemigosA[*cantidadEnemigosA].frameA = 2;
                  enemigosA[*cantidadEnemigosA].cicladoFramesA = 1;
                  enemigosA[*cantidadEnemigosA].frameExplosion = 0;
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
                  enemigosC[*cantidadEnemigosC].activo = true;
                  enemigosC[*cantidadEnemigosC].posX = puntoX;
                  enemigosC[*cantidadEnemigosC].posY = puntoY;
                  enemigosC[*cantidadEnemigosC].vida = VIDA_ENEMIGO_C;
                  enemigosC[*cantidadEnemigosC].nodoCY = enemigosC[*cantidadEnemigosC].posY - 80;
                  enemigosC[*cantidadEnemigosC].nodoCX = enemigosC[*cantidadEnemigosC].posX;
                  enemigosC[*cantidadEnemigosC].tempEnemigosC = al_create_timer(1.0 / TARGET_FPS); // Temporizador de posicion de enemigos C.
                  must_init(enemigosC[*cantidadEnemigosC].tempEnemigosC, "tempEnemigosC");
                  enemigosC[*cantidadEnemigosC].cooldownPortal = 0;
                  enemigosC[*cantidadEnemigosC].frameExplosion = 0;
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
                  enemigosE[*cantidadEnemigosE].activo = true;
                  enemigosE[*cantidadEnemigosE].posX = puntoX;
                  enemigosE[*cantidadEnemigosE].posY = puntoY;
                  enemigosE[*cantidadEnemigosE].nodoE = enemigosE[*cantidadEnemigosE].posY;
                  enemigosE[*cantidadEnemigosE].frameExplosion = 0;
                  (*cantidadEnemigosE)++;
               }
               else
               {
                  printf("Advertencia: Hay mas enemigos E que los que soporta el arreglo\n");
               }
            }
            if(valorRecibido == '$')
            {
               if(*cantidadMonedas < MAX_MONEDAS)
               {
                  /*switch(nivel)
                  {
                     case 1:
                     if(monedasActivas[0] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     break;
                     case 2:
                     if(monedasActivas[1] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     if(monedasActivas[2] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     break;
                     case 3:
                     if(monedasActivas[3] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     if(monedasActivas[4] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     break;
                     case 4:
                     if(monedasActivas[6] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     if(monedasActivas[5] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     break;
                     case 5:
                     if(monedasActivas[7] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     break;
                     case 6:
                     if(monedasActivas[8] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     if(monedasActivas[9] == true)
                     {
                        monedasMapa[*cantidadMonedas].activo = true;
                        monedasMapa[*cantidadMonedas].posX = puntoX;
                        monedasMapa[*cantidadMonedas].posY = puntoY;
                        (*cantidadMonedas)++;
                     }
                     break;
                  }*/
                  monedasMapa[*cantidadMonedas].activo = true;
                  monedasMapa[*cantidadMonedas].posX = puntoX;
                  monedasMapa[*cantidadMonedas].posY = puntoY;
                  monedasMapa[*cantidadMonedas].frame = 0;
                  (*cantidadMonedas)++;
               }
               contMonedasMapa++;
            }
            if(valorRecibido == '|')
            {
               dianas[*cantidadDianas].activo = true;
               dianas[*cantidadDianas].posX = puntoX;
               dianas[*cantidadDianas].posY = puntoY;
               dianas[*cantidadDianas].vida = VIDA_DIANA;
               (*cantidadDianas)++;
            }
            if(valorRecibido == '^')
            {
               portalSalida->posX = puntoX;
               portalSalida->posY = puntoY;
               portalSalida->activo = true;
            }
            if(valorRecibido == ')')
            {
               if(*cantidadSombrerosHorizontalesEntrada < MAX_SOMBREROS)
               {
                  sombrerosHorizontales[*cantidadSombrerosHorizontalesEntrada].entrada.posX = puntoX;
                  sombrerosHorizontales[*cantidadSombrerosHorizontalesEntrada].entrada.posY = puntoY;
                  (*cantidadSombrerosHorizontalesEntrada)++;
               }
            }
            if(valorRecibido == '(')
            {
               if(*cantidadSombrerosHorizontalesSalida < MAX_SOMBREROS)
               {
                  sombrerosHorizontales[*cantidadSombrerosHorizontalesSalida].salida.posX = puntoX;
                  sombrerosHorizontales[*cantidadSombrerosHorizontalesSalida].salida.posY = puntoY;
                  (*cantidadSombrerosHorizontalesSalida)++;
               }
            }
            if(valorRecibido == 'u')
            {
               if(*cantidadSombrerosVerticalesEntrada < MAX_SOMBREROS)
               {
                  sombrerosVerticales[*cantidadSombrerosVerticalesEntrada].entrada.posX = puntoX;
                  sombrerosVerticales[*cantidadSombrerosVerticalesEntrada].entrada.posY = puntoY;
                  (*cantidadSombrerosVerticalesEntrada)++;
               }
            }
            if(valorRecibido == 'n')
            {
               if(*cantidadSombrerosVerticalesEntrada < MAX_SOMBREROS)
               {
                  sombrerosVerticales[*cantidadSombrerosVerticalesSalida].salida.posX = puntoX;
                  sombrerosVerticales[*cantidadSombrerosVerticalesSalida].salida.posY = puntoY;
                  (*cantidadSombrerosVerticalesSalida)++;
               }
            }
            if(valorRecibido == 'i') // Define el punto de inicio del jugador en el caso del cargado de una casilla 'i'
            {
               jugador->posX = puntoX;
               jugador->posY = puntoY;
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

void logicaSombrerosHorizontalesEntrada(sombreros sombrerosHorizontales[MAX_SOMBREROS], entidad entidad, int nivel, int cont)
{
   if(entidad.cooldownPortal == 0)
   {
      if(fabs(sombrerosHorizontales[cont].entrada.posX - entidad.posX) <= LARGO * 3 && fabs(sombrerosHorizontales[cont].entrada.posY - entidad.posY) <= ANCHO * 3)
      {
         if(generalCollide(entidad.posX, entidad.posY, LARGO, ANCHO, sombrerosHorizontales[cont].entrada.posX, sombrerosHorizontales[cont].entrada.posY, LARGO, ANCHO_SOMBRERO) == true)
         {
            switch(nivel)
            {
               case 4:
               {
                  switch(cont)
                  {
                     case 0:
                     entidad.posX = sombrerosHorizontales[2].salida.posX;
                     entidad.posY = sombrerosHorizontales[2].salida.posY;
                     break;
                     case 1:
                     entidad.posX = sombrerosHorizontales[0].salida.posX;
                     entidad.posY = sombrerosHorizontales[0].salida.posY;
                     break;
                     case 2:
                     entidad.posX = sombrerosHorizontales[1].salida.posX;
                     entidad.posY = sombrerosHorizontales[1].salida.posY;
                     break;
                  }
                  break;
               }
               case 6:
               {
                  switch(cont)
                  {
                     case 0:
                     entidad.posX = sombrerosHorizontales[2].salida.posX;
                     entidad.posY = sombrerosHorizontales[2].salida.posY;
                     break;
                     case 1:
                     entidad.posX = sombrerosHorizontales[0].salida.posX;
                     entidad.posY = sombrerosHorizontales[0].salida.posY;
                     break;
                     case 2:
                     entidad.posX = sombrerosHorizontales[1].salida.posX;
                     entidad.posY = sombrerosHorizontales[1].salida.posY;
                     break;
                  }
                  break;
               }
               default:
               {
                  entidad.posX = sombrerosHorizontales[cont].salida.posX;
                  entidad.posY = sombrerosHorizontales[cont].salida.posY;
                  break;
               }
            }
            entidad.cooldownPortal = ATRAVESAR_SOMBRERO_CD;
         }
      }
   }
}

void logicaSombrerosHorizontalesSalida(sombreros sombrerosHorizontales[MAX_SOMBREROS], entidad entidad, int nivel, int cont)
{
   if(entidad.cooldownPortal == 0)
   {
      if(fabs(sombrerosHorizontales[cont].salida.posX - entidad.posX) <= LARGO * 3 && fabs(sombrerosHorizontales[cont].salida.posY - entidad.posY) <= ANCHO * 3)
      {
         if(generalCollide(entidad.posX, entidad.posY, LARGO, ANCHO, sombrerosHorizontales[cont].salida.posX, sombrerosHorizontales[cont].salida.posY, LARGO, ANCHO_SOMBRERO) == true)
         {
            switch(nivel)
            {
               case 4:
               {
                  switch(cont)
                  {
                     case 0:
                     entidad.posX = sombrerosHorizontales[2].entrada.posX;
                     entidad.posY = sombrerosHorizontales[2].entrada.posY;
                     break;
                     case 1:
                     entidad.posX = sombrerosHorizontales[0].entrada.posX;
                     entidad.posY = sombrerosHorizontales[0].entrada.posY;
                     break;
                     case 2:
                     entidad.posX = sombrerosHorizontales[1].entrada.posX;
                     entidad.posY = sombrerosHorizontales[1].entrada.posY;
                     break;
                  }
                  break;
               }
               case 6:
               {
                  switch(cont)
                  {
                     case 0:
                     entidad.posX = sombrerosHorizontales[2].entrada.posX;
                     entidad.posY = sombrerosHorizontales[2].entrada.posY;
                     break;
                     case 1:
                     entidad.posX = sombrerosHorizontales[0].entrada.posX;
                     entidad.posY = sombrerosHorizontales[0].entrada.posY;
                     break;
                     case 2:
                     entidad.posX = sombrerosHorizontales[1].entrada.posX;
                     entidad.posY = sombrerosHorizontales[1].entrada.posY;
                     break;
                  }
                  break;
               }
               default:
               {
                  entidad.posX = sombrerosHorizontales[cont].entrada.posX;
                  entidad.posY = sombrerosHorizontales[cont].entrada.posY;
                  break;
               }
            }
            entidad.cooldownPortal = ATRAVESAR_SOMBRERO_CD;
         }
      }
   }
}

void logicaSombrerosVerticalesEntrada(sombreros sombrerosVerticales[MAX_SOMBREROS], entidad entidad, int nivel, int cont)
{
   if(entidad.cooldownPortal == 0)
   {
      if(fabs(sombrerosVerticales[cont].entrada.posX - entidad.posX) <= LARGO * 3 && fabs(sombrerosVerticales[cont].entrada.posY - entidad.posY) <= ANCHO * 3)
      {
         if(generalCollide(entidad.posX, entidad.posY, LARGO, ANCHO, sombrerosVerticales[cont].entrada.posX, sombrerosVerticales[cont].entrada.posY, LARGO_SOMBRERO, ANCHO) == true)
         {
            switch(nivel)
            {
               case 4:
               {
                  switch(cont)
                  {
                     case 0:
                     entidad.posX = sombrerosVerticales[2].salida.posX;
                     entidad.posY = sombrerosVerticales[2].salida.posY;
                     break;
                     case 1:
                     entidad.posX = sombrerosVerticales[0].salida.posX;
                     entidad.posY = sombrerosVerticales[0].salida.posY;
                     break;
                     case 2:
                     entidad.posX = sombrerosVerticales[1].salida.posX;
                     entidad.posY = sombrerosVerticales[1].salida.posY;
                     break;
                  }
                  break;
               }
               case 6:
               {
                  switch(cont)
                  {
                     case 0:
                     entidad.posX = sombrerosVerticales[2].salida.posX;
                     entidad.posY = sombrerosVerticales[2].salida.posY;
                     break;
                     case 1:
                     entidad.posX = sombrerosVerticales[0].salida.posX;
                     entidad.posY = sombrerosVerticales[0].salida.posY;
                     break;
                     case 2:
                     entidad.posX = sombrerosVerticales[1].salida.posX;
                     entidad.posY = sombrerosVerticales[1].salida.posY;
                     break;
                  }
                  break;
               }
               default:
               {
                  entidad.posX = sombrerosVerticales[cont].salida.posX;
                  entidad.posY = sombrerosVerticales[cont].salida.posY;
                  break;
               }
            }
            entidad.cooldownPortal = ATRAVESAR_SOMBRERO_CD;
         }
      }
   }
}

void logicaSombrerosVerticalesSalida(sombreros sombrerosVerticales[MAX_SOMBREROS], entidad entidad, int nivel, int cont)
{
   if(entidad.cooldownPortal == 0)
   {
      if(fabs(sombrerosVerticales[cont].salida.posX - entidad.posX) <= LARGO * 3 && fabs(sombrerosVerticales[cont].salida.posY - entidad.posY) <= ANCHO * 3)
      {
         if(generalCollide(entidad.posX, entidad.posY, LARGO, ANCHO, sombrerosVerticales[cont].salida.posX, sombrerosVerticales[cont].salida.posY, LARGO_SOMBRERO, ANCHO) == true)
         {
            switch(nivel)
            {
               case 4:
               {
                  switch(cont)
                  {
                     case 0:
                     entidad.posX = sombrerosVerticales[2].entrada.posX;
                     entidad.posY = sombrerosVerticales[2].entrada.posY;
                     break;
                     case 1:
                     entidad.posX = sombrerosVerticales[0].entrada.posX;
                     entidad.posY = sombrerosVerticales[0].entrada.posY;
                     break;
                     case 2:
                     entidad.posX = sombrerosVerticales[1].entrada.posX;
                     entidad.posY = sombrerosVerticales[1].entrada.posY;
                     break;
                  }
                  break;
               }
               case 6:
               {
                  switch(cont)
                  {
                     case 0:
                     entidad.posX = sombrerosVerticales[2].entrada.posX;
                     entidad.posY = sombrerosVerticales[2].entrada.posY;
                     break;
                     case 1:
                     entidad.posX = sombrerosVerticales[0].entrada.posX;
                     entidad.posY = sombrerosVerticales[0].entrada.posY;
                     break;
                     case 2:
                     entidad.posX = sombrerosVerticales[1].entrada.posX;
                     entidad.posY = sombrerosVerticales[1].entrada.posY;
                     break;
                  }
                  break;
               }
               default:
               {
                  entidad.posX = sombrerosVerticales[cont].entrada.posX;
                  entidad.posY = sombrerosVerticales[cont].entrada.posY;
                  break;
               }
            }
            entidad.cooldownPortal = ATRAVESAR_SOMBRERO_CD;
         }
      }
   }
}
/*void ordenarSombreros(int nivel)
{
   switch(nivel)
   {
      case 4:
      intercambiarSombreros(&sombrerosVerticales[0], &sombrerosVerticales[2]);
      intercambiarSombreros(&sombrerosVerticales[0], &sombrerosVerticales[1]);

      case 5:
      intercambiarSombreros(&sombrerosHorizontales[0])
   }
}

void intercambiarSombreros(int *sombreroX, int *sombreroY)
{

}*/

void golpearJugador(int *vida, int *iFrames, int *puntajeZona, int *dashFrames , int *parryFrames, int *golpeFrames, ALLEGRO_TIMER* tempGravedad)
{
   (*vida)--; // Resta 1 punto de vida.
   *iFrames = INVINCIBILITY_FRAMES; // Otorga 120 frames de invencibilidad.
   al_set_timer_count(tempGravedad, ALTURA_GOLPE);
   *dashFrames = 1;
   *parryFrames = 0;
   if(*puntajeZona < PENALIZACION_GOLPE) // Reduce el puntaje por 1000, lo ajusta a 0 si es menor a la penalizacion.
   {
      *puntajeZona = 0;
   }
   else
   {
      (*puntajeZona) -= PENALIZACION_GOLPE; // probar sin parentesis si se buguea
   }
   *golpeFrames = FRAMES_GOLPE;

   return;
}

void inicializarRanking(_puntaje ranking[MAX_PUNTAJES], int *cantidadPuntajes)
{
   FILE *contenidoArchivo;
   int i;

   if((contenidoArchivo = fopen("ranking.txt", "r")) == 0) // Abre el archivo.
   {
      printf("Error al abrir el archivo\n");
      exit(1);
   }

   // fscanf lee primero la cadena (%s) y luego el entero (%d)
   while(fscanf(contenidoArchivo, "%9s %d", ranking[*cantidadPuntajes].nombre, &ranking[*cantidadPuntajes].puntaje) == 2 && *cantidadPuntajes < MAX_PUNTAJES) 
   {
      (*cantidadPuntajes)++;
   }

   fclose(contenidoArchivo);

   // Impresión de resultados
   for(i = 0; i < *cantidadPuntajes; i++)
   {
      printf("%d. %s - %d pts\n", i + 1, ranking[i].nombre, ranking[i].puntaje);
   }

   return;
}

void actualizarPuntaje(_puntaje ranking[MAX_PUNTAJES], char nombreIngresado[MAX_CARACTERES], int puntajeIngresado, int *cantidadPuntajes)
{
   FILE* contenidoArchivo;
   int contPuntajes = 0, puntajeAReemplazar;

   if((contenidoArchivo = fopen("ranking.txt", "w")) == 0) // Abre el archivo.
   {
      printf("Error al abrir el archivo\n");
      exit(1);
   }

   if(*cantidadPuntajes < MAX_PUNTAJES)
   {
      (*cantidadPuntajes)++;
   }

   for(contPuntajes = 0; contPuntajes < *cantidadPuntajes; contPuntajes++)
   {
      if(puntajeIngresado > ranking[contPuntajes].puntaje)
      {
         puntajeAReemplazar = contPuntajes;
         for(contPuntajes = MAX_PUNTAJES - 1; contPuntajes > puntajeAReemplazar; contPuntajes--) // - 1 para evitar elementos fuera del arreglo.
         {
            ranking[contPuntajes] = ranking[contPuntajes - 1]; // Hasta llegar al puntaje a reemplazar.
         }
         strcpy(ranking[puntajeAReemplazar].nombre, nombreIngresado);
         ranking[puntajeAReemplazar].puntaje = puntajeIngresado;
         break;
      }
   }

   for(contPuntajes = 0; contPuntajes < *cantidadPuntajes; contPuntajes++)
   {
      // Escribe el nombre y puntaje separados por espacio y un salto de línea
      fprintf(contenidoArchivo, "%s %d\n", ranking[contPuntajes].nombre, ranking[contPuntajes].puntaje);
   }

   fclose(contenidoArchivo); // Cierra el archivo guardado

   printf("\nPuntajes ordenados:\n");
   for(contPuntajes = 0; contPuntajes < *cantidadPuntajes; contPuntajes++) // Impresion de arreglo.
   {
      printf("%s\t", ranking[contPuntajes].nombre);
      printf("%d\n", ranking[contPuntajes].puntaje);
   }

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
   sprites.fondo1 = al_load_bitmap("fondo1.png");
   must_init(sprites._sheet, "fondo1");
   sprites.fondo2 = al_load_bitmap("fondo2.png");
   must_init(sprites._sheet, "fondo2");
   sprites.fondo_menu = al_load_bitmap("fondo_menu.png");
   must_init(sprites.fondo_menu, "fondo_menu");

   sprites.tierra = sprite_grab(0, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.pasto = sprite_grab(LARGO, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.agua = sprite_grab(LARGO * 3, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.tope_agua = sprite_grab(LARGO, LARGO, LARGO_BLOQUE, 12);

   sprites.madera = sprite_grab(LARGO * 4, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.alfombra = sprite_grab(0, ANCHO, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.pinchos = sprite_grab(LARGO * 12, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.tope_pinchos = sprite_grab(LARGO * 13, 0, LARGO_BLOQUE, ANCHO_BLOQUE);
   
   sprites.bloque_tutorial = sprite_grab(LARGO * 6, ANCHO, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.semiplataforma_tutorial = sprite_grab(40, 60, 40, 20);

   sprites.moneda[0] = sprite_grab(308, 424, 48, 52);
   sprites.moneda[1] = sprite_grab(360, 424, 48, 52);
   sprites.moneda[2] = sprite_grab(412, 424, 48, 52);
   sprites.moneda[3] = sprite_grab(464, 424, 48, 52);
   sprites.moneda[4] = sprite_grab(516, 424, 48, 52);
   sprites.moneda[5] = sprite_grab(564, 424, 48, 52);

   sprites.semiplataforma = sprite_grab(LARGO * 2, 0, LARGO_SEMIPLATAFORMA, ANCHO_SEMIPLATAFORMA);
   sprites.flor = sprite_grab(344, 0, 60, 60);
   sprites.puerta = sprite_grab(LARGO * 5, 0, LARGO_BLOQUE, ANCHO_BLOQUE * 2);
   sprites.portal_local = sprite_grab(LARGO * 2, ANCHO, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.sombrero_entrada = sprite_grab(864, 432, 352, 352);
   sprites.sombrero_salida = sprite_grab(1232, 432, 352, 352);
   sprites.meta = sprite_grab(212, 364, 92, 96);

   sprites.enemigoA[0] = sprite_grab(0, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[1] = sprite_grab(LARGO, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[2] = sprite_grab(LARGO * 2, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[3] = sprite_grab(LARGO * 3, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[4] = sprite_grab(LARGO * 4, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[5] = sprite_grab(LARGO * 5, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);
   sprites.enemigoA[6] = sprite_grab(LARGO * 6, ANCHO * 2, LARGO_ENEMIGO_A, ANCHO_ENEMIGO_A);

   sprites.enemigoC = sprite_grab(LARGO * 7, 48, LARGO_ENEMIGO_C, ANCHO_ENEMIGO_C);
   sprites.enemigoE = sprite_grab(LARGO * 7, 0, LARGO_ENEMIGO_E, ANCHO_ENEMIGO_E);
   sprites.enemigoE_pinchos = sprite_grab(560, 0, LARGO_ENEMIGO_E, ANCHO_ENEMIGO_E);

   sprites.explosion[0] = sprite_grab(1096, 0, 84, 84);
   sprites.explosion[1] = sprite_grab(1184, 0, 84, 84);
   sprites.explosion[2] = sprite_grab(1272, 0, 84, 84);
   sprites.explosion[3] = sprite_grab(1360, 0, 84, 84);

   sprites.jugador_quieto[0] = sprite_grab(0, 136, 60, 92);
   sprites.jugador_quieto[1] = sprite_grab(60, 136, 52, 92);

   sprites.jugador_lock[0] = sprite_grab(820, 36, 80, 92);
   sprites.jugador_lock[1] = sprite_grab(904, 36, 76, 92);

   sprites.jugador_dash = sprite_grab(984, 64, 108, 64);

   sprites.jugador_corriendo[0] = sprite_grab(0, 232, 72, 88);
   sprites.jugador_corriendo[1] = sprite_grab(76, 232, 60, 88);
   sprites.jugador_corriendo[2] = sprite_grab(140, 232, 64, 88);
   sprites.jugador_corriendo[3] = sprite_grab(208, 232, 56, 88);
   sprites.jugador_corriendo[4] = sprite_grab(268, 232, 68, 88);

   sprites.jugador_corriendo_disparo_horizontal[0] = sprite_grab(340, 232, 76, 88);
   sprites.jugador_corriendo_disparo_horizontal[1] = sprite_grab(420, 232, 64, 88);
   sprites.jugador_corriendo_disparo_horizontal[2] = sprite_grab(488, 232, 72, 88);
   sprites.jugador_corriendo_disparo_horizontal[3] = sprite_grab(564, 232, 64, 88);
   sprites.jugador_corriendo_disparo_horizontal[4] = sprite_grab(632, 232, 72, 88);

   sprites.jugador_corriendo_disparo_diagonal[0] = sprite_grab(708, 236, 76, 84);
   sprites.jugador_corriendo_disparo_diagonal[1] = sprite_grab(788, 236, 72, 84);
   sprites.jugador_corriendo_disparo_diagonal[2] = sprite_grab(864, 236, 80, 84);
   sprites.jugador_corriendo_disparo_diagonal[3] = sprite_grab(948, 236, 76, 84);
   sprites.jugador_corriendo_disparo_diagonal[4] = sprite_grab(1028, 236, 76, 84);

   sprites.jugador_disparo_horizontal[0] = sprite_grab(116, 136, 80, 92);
   sprites.jugador_disparo_horizontal[1] = sprite_grab(200, 136, 80, 92);
   sprites.jugador_disparo_horizontal[2] = sprite_grab(280, 136, 76, 92);

   sprites.jugador_disparo_diagonal_arriba[0] = sprite_grab(360, 140, 76, 88);
   sprites.jugador_disparo_diagonal_arriba[1] = sprite_grab(440, 140, 68, 88);
   sprites.jugador_disparo_diagonal_arriba[2] = sprite_grab(524, 140, 72, 88);

   sprites.jugador_disparo_diagonal_abajo[0] = sprite_grab(600, 136, 72, 92);
   sprites.jugador_disparo_diagonal_abajo[1] = sprite_grab(676, 136, 76, 92);
   sprites.jugador_disparo_diagonal_abajo[2] = sprite_grab(756, 136, 72, 92);

   sprites.jugador_disparo_arriba[0] = sprite_grab(832, 128, 64, 100);
   sprites.jugador_disparo_arriba[1] = sprite_grab(900, 128, 64, 100);
   sprites.jugador_disparo_arriba[2] = sprite_grab(968, 128, 64, 100);

   sprites.jugador_disparo_abajo[0] = sprite_grab(1036, 132, 64, 104);
   sprites.jugador_disparo_abajo[1] = sprite_grab(1104, 132, 64, 104);
   sprites.jugador_disparo_abajo[2] = sprite_grab(1172, 132, 64, 104);

   sprites.jugador_agachado = sprite_grab(352, 80, 108, 44);
   sprites.jugador_agachado_disparo[0] = sprite_grab(464, 76, 116, 48);
   sprites.jugador_agachado_disparo[1] = sprite_grab(584, 76, 116, 48);
   sprites.jugador_agachado_disparo[2] = sprite_grab(704, 76, 116, 48);

   sprites.jugador_salto[0] = sprite_grab(396, 324, 52, 52);
   sprites.jugador_salto[1] = sprite_grab(452, 324, 56, 72);
   sprites.jugador_salto[2] = sprite_grab(512, 324, 56, 52);
   sprites.jugador_salto[3] = sprite_grab(572, 324, 52, 72);

   sprites.jugador_parry[0] = sprite_grab(628, 324, 76, 96);
   sprites.jugador_parry[1] = sprite_grab(708, 324, 84 ,84);
   sprites.jugador_parry[2] = sprite_grab(796, 324, 76, 100);
   sprites.jugador_parry[3] = sprite_grab(876, 324, 88, 92);

   sprites.jugador_golpe = sprite_grab(1108, 232, 92, 124);

   sprites.bala0 = sprite_grab(404, 0, 44, 16);
   sprites.bala1 = sprite_grab(404, 16, 36, 32);
   sprites.bala2 = sprite_grab(448, 0, 28, 16);
   sprites.bala3 = sprite_grab(440, 16, 20, 20);

   sprites.pasto_mapa = sprite_grab(LARGO * 3, ANCHO, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.arbol_mapa = sprite_grab(LARGO * 4, ANCHO, LARGO_BLOQUE, ANCHO_BLOQUE);

   sprites.jugador_mapa_quieto = sprite_grab(0, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_mapa_der = sprite_grab(44, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_mapa_arrder = sprite_grab(88, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_mapa_arr = sprite_grab(132, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_mapa_arrizq = sprite_grab(176, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_mapa_izq = sprite_grab(220, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_mapa_abaizq = sprite_grab(264, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_mapa_aba = sprite_grab(308, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);
   sprites.jugador_mapa_abader = sprite_grab(352, ANCHO * 8, LARGO_BLOQUE, ANCHO_BLOQUE);

   sprites.portada_zona_1 = sprite_grab(0, 364, LARGO_PORTADA_ZONA_1, ANCHO_PORTADA_ZONA_1);
   sprites.portada_zona_2 = sprite_grab(0, 496, LARGO_PORTADA_ZONA_2, ANCHO_PORTADA_ZONA_2);

   sprites.tienda = sprite_grab(232, 480, 200, 212);

   sprites.logo_peashooter = sprite_grab(608, 0, 48, 48);
   sprites.logo_chaser = sprite_grab(656, 0, 48, 48);
   sprites.logo_roundabout = sprite_grab(704, 0, 48, 48);
   sprites.logo_ricochet = sprite_grab(752, 0, 48, 48);

   return;
}

void sprites_deinit()
{
   al_destroy_bitmap(sprites.tierra);
   al_destroy_bitmap(sprites.pasto);
   al_destroy_bitmap(sprites.agua);
   al_destroy_bitmap(sprites.tope_agua);

   al_destroy_bitmap(sprites.madera);
   al_destroy_bitmap(sprites.alfombra);
   al_destroy_bitmap(sprites.pinchos);
   al_destroy_bitmap(sprites.tope_pinchos);

   al_destroy_bitmap(sprites.bloque_tutorial);
   al_destroy_bitmap(sprites.semiplataforma_tutorial);

   al_destroy_bitmap(sprites.moneda[0]);
   al_destroy_bitmap(sprites.moneda[1]);
   al_destroy_bitmap(sprites.moneda[2]);
   al_destroy_bitmap(sprites.moneda[3]);
   al_destroy_bitmap(sprites.moneda[4]);
   al_destroy_bitmap(sprites.moneda[5]);

   al_destroy_bitmap(sprites.semiplataforma);
   al_destroy_bitmap(sprites.flor);
   al_destroy_bitmap(sprites.puerta);
   al_destroy_bitmap(sprites.portal_local);
   al_destroy_bitmap(sprites.meta);

   al_destroy_bitmap(sprites.enemigoA[0]);
   al_destroy_bitmap(sprites.enemigoA[1]);
   al_destroy_bitmap(sprites.enemigoA[2]);
   al_destroy_bitmap(sprites.enemigoA[3]);
   al_destroy_bitmap(sprites.enemigoA[4]);
   al_destroy_bitmap(sprites.enemigoA[5]);
   al_destroy_bitmap(sprites.enemigoA[6]);

   al_destroy_bitmap(sprites.enemigoC);
   al_destroy_bitmap(sprites.enemigoE);

   al_destroy_bitmap(sprites.explosion[0]);
   al_destroy_bitmap(sprites.explosion[1]);
   al_destroy_bitmap(sprites.explosion[2]);
   al_destroy_bitmap(sprites.explosion[3]);

   al_destroy_bitmap(sprites.jugador_quieto[0]);
   al_destroy_bitmap(sprites.jugador_quieto[1]);

   al_destroy_bitmap(sprites.jugador_lock[0]);
   al_destroy_bitmap(sprites.jugador_lock[1]);

   al_destroy_bitmap(sprites.jugador_corriendo[0]);
   al_destroy_bitmap(sprites.jugador_corriendo[1]);
   al_destroy_bitmap(sprites.jugador_corriendo[2]);
   al_destroy_bitmap(sprites.jugador_corriendo[3]);
   al_destroy_bitmap(sprites.jugador_corriendo[4]);

   al_destroy_bitmap(sprites.jugador_salto[0]);
   al_destroy_bitmap(sprites.jugador_salto[1]);
   al_destroy_bitmap(sprites.jugador_salto[2]);
   al_destroy_bitmap(sprites.jugador_salto[3]);

   al_destroy_bitmap(sprites.jugador_dash);

   al_destroy_bitmap(sprites.jugador_parry[0]);
   al_destroy_bitmap(sprites.jugador_parry[1]);
   al_destroy_bitmap(sprites.jugador_parry[2]);
   al_destroy_bitmap(sprites.jugador_parry[3]);

   al_destroy_bitmap(sprites.jugador_golpe);

   al_destroy_bitmap(sprites.bala0);
   al_destroy_bitmap(sprites.bala1);
   al_destroy_bitmap(sprites.bala2);
   al_destroy_bitmap(sprites.bala3);

   al_destroy_bitmap(sprites.pasto_mapa);
   al_destroy_bitmap(sprites.arbol_mapa);
   al_destroy_bitmap(sprites.jugador_mapa_quieto);
   al_destroy_bitmap(sprites.jugador_mapa_der);
   al_destroy_bitmap(sprites.jugador_mapa_arrder);
   al_destroy_bitmap(sprites.jugador_mapa_arr);
   al_destroy_bitmap(sprites.jugador_mapa_arrizq);
   al_destroy_bitmap(sprites.jugador_mapa_izq);
   al_destroy_bitmap(sprites.jugador_mapa_abaizq);
   al_destroy_bitmap(sprites.jugador_mapa_aba);
   al_destroy_bitmap(sprites.jugador_mapa_abader);
   al_destroy_bitmap(sprites.portada_zona_1);
   al_destroy_bitmap(sprites.portada_zona_2);
   al_destroy_bitmap(sprites.tienda);

   al_destroy_bitmap(sprites.logo_peashooter);
   al_destroy_bitmap(sprites.logo_chaser);
   al_destroy_bitmap(sprites.logo_roundabout);
   al_destroy_bitmap(sprites.logo_ricochet);

   al_destroy_bitmap(sprites._sheet);
   al_destroy_bitmap(sprites.fondo1);
   al_destroy_bitmap(sprites.fondo2);
}

float angulo(float x1, float x2, float y1, float y2)
{
   float anguloRad, anguloDeg, pendiente; 

   pendiente = (y2-y1)/(x2-x1);
   anguloRad = atan(pendiente);
   anguloDeg = anguloRad * (180/ALLEGRO_PI);

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
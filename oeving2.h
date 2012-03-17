/*****************************************************************************
 * 
 * Øving 2 UCSysDes
 *
 * Headerfil
 *
 *****************************************************************************/

#ifndef OEVING_2_H /* vanlig måte å unngå headerrekursjon på */
#define OEVING_2_H

#include <avr32/ap7000.h> /* inkludere Atmel-headerfil */
#include <sys/interrupts.h>

/* legg andre "includes" her, sammen med "defines" og prototyper */
#define ABDAC_INT_LEVEL 0
#define BUTTONS_INT_LEVEL 0

#define SW0 0x1
#define SW1 0x2
#define SW2 0x4
#define SW3 0x8
#define SW4 0x10
#define SW5 0x20
#define SW6 0x40
#define SW7 0x80

#define SND_COIN         0
#define SND_MACHINEGUN   1
#define SND_POWERUP      2
#define SND_NOISE        3
#define SND_NYAN         4
#define SND_MONSTER      5
#define SND_GAME_OVER    6
#define SND_DEATH        7
#define SND_SILENCE      8

#define BASS_LENGTH 32
#define LEAD_LENGTH 256
#define TICK_LENGTH 2800

/* prototyper */
int main (int argc, char *argv[]);
static void initIntc(void);
static void initButtons(void);
static void initLeds(void);
static void initAudio(void);
static void initHardware (void);

static void button_isr(void);
static void abdac_isr(void);


void sound0(void);
void sound1(void);
void sound2(void);
void sound3(void);
void sound4(void);
void sound5(void);
void sound6(void);
void sound7(void);
void silence(void);

void activate_sound(int sound);
short render_sample(void);
#endif

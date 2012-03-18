/*****************************************************************************
 * 
 * Øving 2 UCSysDes
 *
 * Authors: Rune Holmgren & Sigve Sebastian Farstad
 *
 *****************************************************************************/

#include <stdlib.h>
#include <math.h>
#include "oeving2.h"

volatile avr32_pio_t *piob = &AVR32_PIOB;
volatile avr32_pio_t *pioc = &AVR32_PIOC;
volatile avr32_abdac_t *abdac = &AVR32_ABDAC;
volatile avr32_pm_t *pm = &AVR32_PM;

int selected;
float volume = 1.f;

short sin_tab[1024];
short sample;

void (*sounds[9])(void) = {sound0, sound1, sound2, sound3, sound4, sound5, sound6, sound7, silence};

int lead[LEAD_LENGTH];
int bass[BASS_LENGTH];
int lead_period_tracker = 0;
int lead_note_tracker = 0;
int bass_period_tracker = 0;
int bass_note_tracker = 0;
/* since our note data is so short, we can just keep it here :) */
const char* bass_input = "BBNNDDPPAAMMFFRR??KKDDPP==II==II";
const char* lead_input = "hhjjdeeadca\0aaccd\0dcacehjehceacaeehhjehceadedcacddacehcdcaccaacchhjjdeeadca\0aaccd\0dcacehjehceacaeehhjehceadedcacddacehcdcacca\0a\0aa\\^aa\\^aceafefha\0aa\\^a\\feca\\YZ\\aa\\^aa\\^aacea\\^\\a\0a`a\\^afefhaa``aa\\^aa\\^aceafefha\0aa\\^a\\feca\\YZ\\aa\\^aa\\^aacea\\^\\a\0a`a\\^afefhaacc";

int t = 0;

int last_known_button_state = 0;



int main (int argc, char *argv[]) {

    int i;
    /* pre-render some tables */
    //a sine table
    for(i=0;i<1024;i++) {
        sin_tab[i] = sin(M_PI/1024*i)*10000;
    }	
    //the frequency table for the lead voice
    for(i=0;i<LEAD_LENGTH;i++){
        lead[i] = 12000.f/(440*pow(2,(lead_input[i]-112)/12.f));
    }
    //the frequency table for the bass voice
    for(i=0;i<BASS_LENGTH;i++){ 
        bass[i] = 12000.f/(440*pow(2,(bass_input[i]-112)/12.f));
    }

    activate_sound(SND_SILENCE); //silence from the start
    initHardware(); //init the hardware as late as possible
    while(1); //main chillaxing loop :D
    return EXIT_SUCCESS;
}

void initHardware (void) {
    initIntc();
    initLeds();
    initButtons();
    initAudio();
}

void initIntc(void) {
    set_interrupts_base((void *)AVR32_INTC_ADDRESS);
}

void initButtons(void) {
    pioc->per |= 0xFF;
    pioc->puer |= 0xFF;
    pioc->ier |= 0xFF;

    register_interrupt(button_isr, AVR32_PIOC_IRQ/32, AVR32_PIOC_IRQ % 32, BUTTONS_INT_LEVEL);
    init_interrupts();
}

void initLeds(void) {
    piob->per = 0xFF;
    piob->oer = 0xFF;
}

void initAudio(void) {
    register_interrupt( abdac_isr, AVR32_ABDAC_IRQ/32, AVR32_ABDAC_IRQ % 32, ABDAC_INT_LEVEL);

    piob->PDR.p20 = 1;
    piob->PDR.p21 = 1;

    piob->ASR.p20 = 1;
    piob->ASR.p21 = 1;

    pm->GCCTRL[6].diven = 0;
    pm->GCCTRL[6].pllsel = 0;
    pm->GCCTRL[6].oscsel = 0;
    pm->GCCTRL[6].cen = 1;

    abdac->CR.en = 1;
    abdac->IER.tx_ready = 1;
}

void clear_leds(void){
    piob->codr = 0xFF;
}

void set_active_led(int led){
    piob->sodr = pow(2,led); //set the active led
}

void activate_sound(int sound){
    volume = 1.0f;
    selected = sound;
    clear_leds();
    if(sound != SND_SILENCE){
        set_active_led(selected);
    }
}

void button_isr(void) {
    pioc->isr; //reset interupt
    clear_leds();


    int button_state = pioc->pdsr;
    button_state = ~button_state;
    int button_pushed = last_known_button_state^button_state;
    button_pushed = button_pushed&button_state;
    last_known_button_state = button_state;

    switch(button_pushed){
        case SW0:
            t = 20000;
            activate_sound(SND_COIN);
            break;
        case SW1:
            t = 20000;
            activate_sound(SND_MACHINEGUN);
            break;
        case SW2:
            t = 20000;
            activate_sound(SND_POWERUP);
            break;
        case SW3:
            t = 20000;
            activate_sound(SND_NOISE);
            break;
        case SW4:
            t = 1; 
            lead_period_tracker = 0;
            bass_period_tracker = 0;
            lead_note_tracker= 0;
            bass_note_tracker = 0;
            activate_sound(SND_NYAN);
            break;
        case SW5:
            t = 20000;
            activate_sound(SND_MONSTER);
            break;
        case SW6:
            t=60000;
            activate_sound(SND_GAME_OVER);
            break;
        case SW7:
            t=50000;
            activate_sound(SND_DEATH);
            break;
    }
    set_active_led(selected);
    return;
}

void abdac_isr(void) {
    (*sounds[selected])();
    if(!t){
        activate_sound(SND_SILENCE);
    }
    return;
}

void sound0(void){
    sample = (t*1024)%(4194304)<<(t/2000);
    abdac->SDR.channel0 = volume*sample;
    abdac->SDR.channel1 = volume*sample;
    t--;
    return;
}
void sound1(void){

    sample = t&0x80?10000:-10000;
    abdac->SDR.channel0 = (short)(volume*sample);
    abdac->SDR.channel1 = (short)(volume*sample);
    t--;
    volume = t&0x800?1:0;
    return;
}
void sound2(void){
    sample = 10000*(t%(t/(t>>9|t>>13))); //tribute to xpansive at pouet.net
    abdac->SDR.channel0 = sample;
    abdac->SDR.channel1 = sample;
    t--;
    return;
}
void sound3(void){
    abdac->SDR.channel0 = rand();
    abdac->SDR.channel1 = rand();
    t--;
    return;
}
void sound4(void){
    sample = render_sample();
    abdac->SDR.channel0 = sample;
    abdac->SDR.channel1 = sample;
    return;
}
void sound5(void){
    int bouncer1 = ((60000-t)/(8+(60000-t)/300)%2)*2-1;
    int bouncer2 = ((60000-t)/(64+t/6000)%4)*2-1;
    sample = bouncer1*4000 + bouncer2*4000;
    abdac->SDR.channel0 = sample;
    abdac->SDR.channel1 = sample;
    t--;
    return;
}
void sound6(void){
    sample = sin_tab[((t/1000)*(60000-t))%1024] + sin_tab[((t/1000)*(60000-t)+256)%1024] + sin_tab[((60000-t)/1000)%1024] ; 
    abdac->SDR.channel0 = sample;
    abdac->SDR.channel1 = sample;
    t--;	

    return;
}
void sound7(void){
    int tp = 50000 - t;
    short sample = sin_tab[((4+tp/300)*tp)%1024] + sin_tab[(((4000+tp/300)*tp))%1024];
    if(tp > 45000) sample = sample*(50000-tp)/5000;
    abdac->SDR.channel0 = sample;
    abdac->SDR.channel1 = sample;
    t--;	
    return;
}
void silence(void){
    return;
}

short render_sample(void){
    //two lines were faster than one
    sample = lead_period_tracker<lead[lead_note_tracker]*0.5f?10000:-10000;
    sample += bass_period_tracker<bass[bass_note_tracker]*0.5f?10000:-10000;
    t++;
    lead_period_tracker++;
    bass_period_tracker++;
    if(t>TICK_LENGTH){
        t=1;
        lead_note_tracker++;
        bass_note_tracker++;
    }
    /* if we reack the end of the track, repeat */
    if(lead_note_tracker >  LEAD_LENGTH-1){
        lead_note_tracker = 0;
    }
    if(bass_note_tracker > BASS_LENGTH-1){
        bass_note_tracker = 0;
    }

    /* if we reach the end of period designated by the lead and bass frequency tables, repeat */
    if(lead_period_tracker > lead[lead_note_tracker]){
        lead_period_tracker = 0;
    }
    if(bass_period_tracker > bass[bass_note_tracker]){
        bass_period_tracker = 0;
    }
    return sample;
}

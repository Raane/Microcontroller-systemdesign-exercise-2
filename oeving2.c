/*****************************************************************************
 * 
 * Øving 2 UCSysDes
 *
 *****************************************************************************/

#include "oeving2.h"
#include <math.h>

volatile avr32_pio_t *piob = &AVR32_PIOB;
volatile avr32_pio_t *pioc = &AVR32_PIOC;
volatile avr32_abdac_t *abdac = &AVR32_ABDAC;
volatile avr32_pm_t *pm = &AVR32_PM;

int selected;
float volume = 1.f;

int v = 0;
short sin_tab[1024];
int u = 0;
int v_2 = 0;
int u_2 = 0;
int pitch = 256;
int t;
int last_known_button_state;
short sample;

#define BASS_LENGTH 32
#define LEAD_LENGTH 256
#define TICK_LENGTH 4000

int lead[LEAD_LENGTH];
int bass[BASS_LENGTH];
const char* bass_input = "BBNNDDPPAAMMFFRR??KKDDPP==II==II";
const char* lead_input = "hhjjdeeadca\0aaccd\0dcacehjehceacaeehhjehceadedcacddacehcdcaccaacchhjjdeeadca\0aaccd\0dcacehjehceacaeehhjehceadedcacddacehcdcacca\0a\0aa\\^aa\\^aceafefha\0aa\\^a\\feca\\YZ\\aa\\^aa\\^aacea\\^\\a\0a`a\\^afefhaa``aa\\^aa\\^aceafefha\0aa\\^a\\feca\\YZ\\aa\\^aa\\^aacea\\^\\a\0a`a\\^afefhaacc";


short render_sample(void){
    sample = u<lead[v]*0.5f?10000:-10000;
    sample += u_2<bass[v_2]*0.5f?10000:-10000;
    t++;
    u++;
    u_2++;
    if(t>TICK_LENGTH){
        t=1;
        v++;
        v_2++;
    }
    if(v>LEAD_LENGTH-1){
        v = 0;
    }
    if(v_2>BASS_LENGTH-1){
        v_2 = 0;
    }
    if(u>lead[v]){
        u=0;
    }
    if(u_2>bass[v_2]){
        u_2=0;
    }
    return sample;
}

int main (int argc, char *argv[]) {
    int i;
	for(i=0;i<1024;i++) {	
		sin_tab[i] = sin(M_PI/1024*i)*10000;
	}	
    for(i=0;i<LEAD_LENGTH;i++){
        lead[i] = 20000.f/(440*pow(2,(lead_input[i]-112)/12.f));
    }
    for(i=0;i<BASS_LENGTH;i++){
        bass[i] = 20000.f/(440*pow(2,(bass_input[i]-112)/12.f));
    }
    initHardware();
    selected = -1;
    last_known_button_state = 0x00;		
    while(1);
    return 0;
}

/* funksjon for å initialisere maskinvaren, må utvides */
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
    piob->per |= 0xFF;
    piob->puer |= 0xFF;
    piob->ier |= 0xFF;
    register_interrupt(button_isr, AVR32_PIOB_IRQ/32, AVR32_PIOB_IRQ % 32, BUTTONS_INT_LEVEL);
    init_interrupts();  //søk på 4 40 scale
}

void initLeds(void) {
    pioc->per = 0xFF;
    pioc->oer = 0xFF;
}

void initAudio(void) {
    register_interrupt( abdac_isr, AVR32_ABDAC_IRQ/32, AVR32_ABDAC_IRQ % 32, ABDAC_INT_LEVEL);

    piob->PDR.p20 = 1;
    piob->PDR.p21 = 1;

    piob->ASR.p20 = 1;
    piob->ASR.p21 = 1;

    pm->GCCTRL[6].diven = 0;
    pm->GCCTRL[6].pllsel = 0;
    pm->GCCTRL[6].oscsel = 1;
    pm->GCCTRL[6].cen = 1;

    abdac->CR.en = 1;
    abdac->IER.tx_ready = 1;
}

void activate_sound(int sound){
    abdac->CR.en = 1;
    volume = 1.0f;
    t = 20000;
    if(sound == 4){ //special case for music
       t = 1; 
       v = 0;
       v_2 = 0;
       u = 0;
       u_2 = 0;
    }
    selected = sound;
    pioc->codr = 0xFF; //clear leds
    if(sound != -1){
        pioc->sodr = pow(2,selected);
    }else{
        abdac->CR.en = 0;
    }
}

void button_isr(void) {
    pioc->codr = 0xFF; //clear leds
    piob->isr; //reset interupt


    int button_state = piob->pdsr;
    button_state = button_state^0xFF;
    int button_pushed = last_known_button_state^button_state;
    button_pushed = button_pushed&button_state;
    last_known_button_state = button_state;

	switch(button_pushed){
		case 0x01:
			activate_sound(0);
			break;
		case 0x02:
			activate_sound(1);
			break;
		case 0x04:
			activate_sound(2);
			break;
		case 0x08:
			activate_sound(3);
			break;
		case 0x10:
			activate_sound(4);
			break;
		case 0x20:
			activate_sound(5);
			break;
		case 0x40:
			t=60000;
			activate_sound(6);
			break;
		case 0x80:
			t=50000;
			activate_sound(7);
			break;
	}
    pioc->sodr = pow(2,selected);
    return ;
}

void abdac_isr(void) {
    switch(selected){
        case 0:
            sound0();
            break;
        case 1:
            sound1();
            break;
        case 2:
            sound2();
            break;
        case 4:
            sound4();
            break;
        case 6:
            sound6();
            break;
        case 7:
            sound7();
            break;
        case 3:
        case 5:
        default:
            silence();
    }
    if(!t)activate_sound(-1);
    abdac->isr;
    return;
}

void sound0(void){
    short sample = (t*1024)%(2048*2048)<<(t/2000);
    abdac->SDR.channel0 = volume*sample;
    abdac->SDR.channel1 = volume*sample;
    t--;
    return ;
}
void sound1(void){

    short sample = t&0x80?10000:-10000;
    abdac->SDR.channel0 = (short)(volume*sample);
    abdac->SDR.channel1 = (short)(volume*sample);
    t--;
    volume = t&0x800?1:0;
    return ;
}
void sound2(void){
    short sample = 10000*(t%(t/(t>>9|t>>13)));
    abdac->SDR.channel0 = (short)(volume*sample);
    abdac->SDR.channel1 = (short)(volume*sample);
    t--;
    return ;
}
void sound3(void){

    return ;
}
void sound4(void){
    short sample = render_sample();
    abdac->SDR.channel0 = render_sample();
    abdac->SDR.channel1 = render_sample();
    return ;
}
void sound5(void){

    return ;
}
void sound6(void){
	short sample = sin_tab[((t/1000)*(60000-t))%1024] + sin_tab[((t/1000)*(60000-t)+256)%1024] + sin_tab[((60000-t)/1000)%1024] ; 
	abdac->SDR.channel0 = sample;
	abdac->SDR.channel1 = sample;
	t--;	

    return ;
}
void sound7(void){
	int tp = 50000 - t;
	short sample = sin_tab[((4+tp/300)*tp)%1024] + sin_tab[(((4000+tp/300)*tp))%1024];
	if(tp > 45000) sample = sample*(50000-tp)/5000;
	abdac->SDR.channel0 = sample;
	abdac->SDR.channel1 = sample;
	t--;	
    return ;
}

void silence(void){
    abdac->SDR.channel0 = 0;
    abdac->SDR.channel1 = 0;
    return ;
}

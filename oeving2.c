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

int i;
int last_known_button_state;

int main (int argc, char *argv[]) {
	initHardware();
	//pioc->codr = 0xAA;
	//pioc->sodr = 0x55;
	//	__int_handler *interruptroutine(void);
	int i = 0;
	selected = 1;
//	pioc->sodr = 0x01;
	last_known_button_state = 0x00;		
	while(1){
	//	i = 1;
		//		pioc->codr = 0xFF; //clear leds
		//		int buttons = piob->pdsr^0xFF;
		//		pioc->sodr = 0x55;

		//		pioc->sodr = i;
	}
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
	register_interrupt(button_isr, AVR32_PIOB_IRQ/32, AVR32_PIOB_IRQ % 32, 1);
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

	/* (...) */
}

void button_isr(void) {
	//debounce
/*	int c = 1000;
	while(c--);*/

	pioc->codr = 0xFF; //clear leds
	piob->isr; //reset interupt
	
	
	int button_state = piob->pdsr;
	button_state = button_state^0xFF;
	int button_pushed = last_known_button_state^button_state;
	button_pushed = button_pushed&button_state;
	last_known_button_state = button_state;
	
	if(button_pushed==0x01) selected = 0; 
	if(button_pushed==0x02) selected = 1;
	if(button_pushed==0x04) selected = 2;
	if(button_pushed==0x08) selected = 3;
	if(button_pushed==0x10) selected = 4;
	if(button_pushed==0x20) selected = 5;
	if(button_pushed==0x40) selected = 6;
	if(button_pushed==0x80) selected = 7;

	pioc->sodr = pow(2,selected);
	return ;
}


void abdac_isr(void) {
	if(selected==0){
		sound0();
	}
}

void sound0(void){
	abdac->SDR.channel0 = (i*1024)%(2048*2048);
	abdac->SDR.channel1 = (i*1024)%(2048*2048);
	i++;
	return ;
}
void sound1(void){
	
	return ;
}
void sound2(void){
	
	return ;
}
void sound3(void){
	
	return ;
}
void sound4(void){
	
	return ;
}
void sound5(void){
	
	return ;
}
void sound6(void){
	
	return ;
}
void sound7(void){
	
	return ;
}

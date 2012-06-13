/**
 * @file
 * @author Ethan LaMaster <ealamast@ncsu.edu>
 * @version 1.0
 *
 * @section Description
 *
 * Simple "button pressing" code.
 *
 * Use the buttons to control the output of P_OUT.
 *
 */


#include <avr/io.h>

#define P_OUT PORTC

unsigned int btn_shift[8];
char btn_toggle;
char btn_pressed;

int main (void)
{
	int i;

	board_init();

	PORTE.DIRSET = 0xff;
	PORTE.OUT = 0x00;
	PORTD.DIRCLR = 0x3f;
	PORTR.DIRCLR = 0x03;
	P_OUT.DIRSET = 0xff;
	P_OUT.OUT = 0x00;

	TCE0.CTRLA = (~TC0_CLKSEL_gm) | TC_CLKSEL_DIV1_gc;		// Use the main system clock as timer clock source
	TCE0.CTRLB = TC_WGMODE_NORMAL_gc;						// Normal timer mode
	TCE0.INTCTRLA = TC_OVFINTLVL_MED_gc;					// Set overflow interrupt priority level to medium
	TCE0.PER = 2000;										// Set timer period. 32000 generates an overflow interrupt
															// once every 1ms.
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;							// Enable medium priority interrupts
	sei();

	for(i=0; i<8; i++) {
		btn_shift[i] = 0;
	}

	for(;;) {
		__asm__ __volatile__("nop");
	}
}

ISR(TCE0_OVF_vect)
{
	int i;
	unsigned char button_array = ((~PORTD_IN) & 0x3f) | (((~PORTR_IN) & 0x03) << 6);

	for(i=0; i<8; i++) {
		btn_shift[i] <<= 1;
		btn_shift[i] |= button_array & (1 << i) ? 1 : 0;
		if(btn_shift[i] == ~0 && !(btn_pressed & (1 << i))) {
			btn_toggle ^= 1 << i;
			btn_pressed |= 1 << i;
		}

		if(btn_shift[i] == 0) {
			btn_pressed &= ~(1 << i);
		}
	}

	P_OUT.OUT = btn_toggle;
	PORTE.OUT = btn_toggle;
}

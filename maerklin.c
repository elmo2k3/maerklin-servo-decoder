#include <avr/io.h>
#include <avr/interrupt.h>
#include "maerklin.h"

volatile uint8_t timerflag;
volatile uint8_t decoded;
	
static volatile uint32_t decoded_data;

uint8_t maerklin(void)
{
	uint8_t pressed_key = 0;
	if(decoded == 1)
	{
		switch(decoded_data)
		{
			case 17409: // Keyboard 15, Taste 13 rot
						pressed_key = KEY_1_OPEN;
						break;
			case 17505: // Keyboard 15, Taste 13 gruen
						pressed_key = KEY_1_CLOSE;
						break;
			
			case 17433: // Keyboard 15, Taste 14 rot
						pressed_key = KEY_2_OPEN;
						break;
			case 17529: // Keyboard 15, Taste 14 gruen
						pressed_key = KEY_2_CLOSE;
						break;
			
			case 17415: // Keyboard 15, Taste 15 rot
						pressed_key = KEY_3_OPEN;
						break;
			case 17511: // Keyboard 15, Taste 15 gruen
						pressed_key = KEY_3_CLOSE;
						break;
			
			case 17439: // Keyboard 15, Taste 16 rot
						break;
			case 17535: // Keyboard 15, Taste 16 gruen
						break;
		}
		decoded=0;				// bereit für nächste adresse
		return pressed_key;
	}
	return 0;
}

void maerklin_init(void)
{
	MCUCR=(1<<ISC00);
	GIMSK=(1<<INT0);
	TCNT0=0;
	TIMSK|=(1<<TOIE0);
	TCCR0=(1<<CS01);
	timerflag=1;
	decoded=0;	
	timerflag=0;
}

ISR(INT0_vect) // jede 0.5us
{
	uint8_t low_time;
	static uint8_t bitcount,high_time;
	if(!(PIND&(1<<2))) // pin 0 fallende Flanke
	{
		high_time=TCNT0;
	}
	else				// steigende Flanke
	{
		low_time=TCNT0;
		TCNT0=0;		// timer rücksetzen
		if(timerflag==0) // TCNT0 war größer als 255, 255us ohne steigende flanke
		{
			TCCR0=(1<<CS01);			// Timer0 prescaler 8 und starten;
			timerflag=1;				// timerflag setzen
			bitcount=0;
			decoded_data = 0;
		}
		else if(low_time<200) // jetzt high_time auswerten
		{
			if(high_time > 10 && high_time < 40) // 0 erkannt, eigentlich 13us
			{
				decoded_data = (decoded_data<<1);
			}
			else if(high_time > 150 && high_time < 200) // 1 erkannt, eigentlich 91us
			{
				decoded_data = (decoded_data<<1) | 1;
			}
			if(++bitcount == 17)
			{
				decoded = 1;
				bitcount = 0;
				TCCR0=0;
				timerflag=0;
			}
		}
		else
		{
			timerflag = 0;
			TCCR0=0;
		}
	}
}
ISR(TIMER0_OVF_vect)
{
	TCCR0=0;
	timerflag=0;
}

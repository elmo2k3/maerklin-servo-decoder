#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#include "main.h"
#include "uart.h"
#include "maerklin.h"

#define LED_RED PD7
#define LED_GREEN PD6

#define TASTER PB0
#define SERVO_1 PB1
#define SERVO_2 PB2
#define SERVO_3 PB3

#define SMOOTHNESS 4

static uint16_t servo_start_save[3], servo_end_save[3], servo_last[3] EEMEM;

volatile static uint8_t state;
volatile static uint16_t adc_servo[3];
volatile static uint16_t servo_to[3];
volatile static uint16_t servo_start[3], servo_end[3];
volatile static uint8_t warmup;
static uint8_t servo_reached[3];

void read_start_stop_from_eeprom(void)
{
	uint16_t servo;
	uint8_t i;
	
	for(i=0;i<3;i++) {
		servo_start[i] = eeprom_read_word(&servo_start_save[i]);
		servo_end[i] = eeprom_read_word(&servo_end_save[i]);
		servo = eeprom_read_word(&servo_last[i]);
		
		if(servo > 0 && servo < 1024) {
			adc_servo[i] = servo;
			servo_to[i] = servo;
		} else {
			adc_servo[i] = 512; // mittelstellung
			servo_to[i] = 512;
		}
	}
	servo_reached[0] = 1;
	servo_reached[1] = 1;
	servo_reached[2] = 1;
	warmup = 1;
	_delay_ms(100);
	warmup = 0;
}

void write_start_stop_to_eeprom(void)
{
	uint8_t i;

	uart_puts("writing start and stop positions\r\n");
	for(i=0;i<3;i++) {
		eeprom_write_word(&servo_start_save[i], servo_start[i]);
		eeprom_write_word(&servo_end_save[i], servo_end[i]);
	}
}

int main(void)
{
	uint8_t key_pressed;

	DDRB = (1<<SERVO_1) | (1<<SERVO_2) | (1<<SERVO_3);

	PORTB = (1<<TASTER); // pullup on

	DDRD = (1<<LED_RED) | (1<<LED_GREEN);
	PORTD = (1<<LED_RED) | (1<<LED_GREEN); // leds off

	TCCR1A = 0;
	TCCR1B = (1<<CS10) | (1<<WGM12);

	TIMSK |= (1<<OCIE1A);

	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	uart_init(UART_BAUD_SELECT(38400, F_CPU));
	
	maerklin_init();
	sei();
	
	OCR1A = F_CPU/1000; // 2ms
	
	uart_puts("Maerklin servo decoder <www.bjoern-b.de>\r\n");
	
	read_start_stop_from_eeprom();
	wdt_enable(WDTO_500MS);
	for (;;)
	{
		wdt_reset();
		if(debounce(PINB, TASTER)) {
			switch(++state) {
				case 1:
					GIMSK &=~(1<<INT0);			// INT0 ausschalten
					PORTD &= ~(1<<LED_GREEN);
					PORTD |= (1<<LED_RED);
					break;
				case 2:
					PORTD &= ~(1<<LED_RED);
					PORTD |= (1<<LED_GREEN);
					break;
				case 3:
					PORTD |= (1<<LED_RED);
					PORTD |= (1<<LED_GREEN);
					write_start_stop_to_eeprom();
					GIMSK |=(1<<INT0);			// INT0 wieder an
					state = 0;
					break;
			}
		}
		else if(uart_data())
		{
			key_pressed = uart_getchar();
			if(key_pressed == '1')
			{
				uart_puts("Pressed Key 1 open\r\n");
				servo_to[0] = servo_start[0];
			}
			else if(key_pressed == '2')
			{
				uart_puts("Pressed Key 1 close\r\n");
				servo_to[0] = servo_end[0];
			}
			else if(key_pressed == '3')
			{
				uart_puts("Pressed Key 2 open\r\n");
				servo_to[1] = servo_start[1];
			}
			else if(key_pressed == '4')
			{
				uart_puts("Pressed Key 2 close\r\n");
				servo_to[1] = servo_end[1];
			}
			else if(key_pressed == '5')
			{
				uart_puts("Pressed Key 3 open\r\n");
				servo_to[2] = servo_start[2];
			}
			else if(key_pressed == '6')
			{
				uart_puts("Pressed Key 3 close\r\n");
				servo_to[2] = servo_end[2];
			}
		}
		else
		{
			if((key_pressed = maerklin()) == KEY_1_OPEN)
			{
				uart_puts("Pressed Key 1 open\r\n");
				servo_to[0] = servo_start[0];
			}
			else if(key_pressed == KEY_1_CLOSE)
			{
				uart_puts("Pressed Key 1 close\r\n");
				servo_to[0] = servo_end[0];
			}
			else if(key_pressed == KEY_2_OPEN)
			{
				uart_puts("Pressed Key 2 open\r\n");
				servo_to[1] = servo_start[1];
			}
			else if(key_pressed == KEY_2_CLOSE)
			{
				uart_puts("Pressed Key 2 close\r\n");
				servo_to[1] = servo_end[1];
			}
			else if(key_pressed == KEY_3_OPEN)
			{
				uart_puts("Pressed Key 3 open\r\n");
				servo_to[2] = servo_start[2];
			}
			else if(key_pressed == KEY_3_CLOSE)
			{
				uart_puts("Pressed Key 3 close\r\n");
				servo_to[2] = servo_end[2];
			}
		}
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}
}


ISR(SIG_OUTPUT_COMPARE1A)
{
	static uint8_t debounce = 10;
	static uint16_t smoothness = SMOOTHNESS;
	uint8_t i;
	
	if (state == 0) {
		if(--smoothness == 0) {
			for(i=0;i<3;i++) {
				if(adc_servo[i] > servo_to[i]) {
					adc_servo[i]--;
					servo_reached[i] = 0;
				}
				else if(adc_servo[i] < servo_to[i]) {
					adc_servo[i]++;
					servo_reached[i] = 0;
				}
				else {
					if(!servo_reached[i]) {
						servo_reached[i] = 1;
						uart_puts("writing position for servo\r\n");
						eeprom_write_word(&servo_last[i], adc_servo[i]);
					}
				}
			}
			smoothness = SMOOTHNESS;
		}
	}
	else // state != 0
		ADCSRA |= (1<<ADSC);

	if(--debounce == 0) {
		if(!servo_reached[0] || state != 0 || warmup) {
			PORTB |= (1<<SERVO_1);
			_delay_ms(0.6 + (float)adc_servo[0]/1024.0 * 2.4); // 0.6 - 3
			PORTB &= ~(1<<SERVO_1);
		}
		if(!servo_reached[1] || state != 0 || warmup) {
			PORTB |= (1<<SERVO_2);
			_delay_ms(0.6 + (float)adc_servo[1]/1024.0 * 2.4); // 0.6 - 3
			PORTB &= ~(1<<SERVO_2);
		}
		if(!servo_reached[2] || state != 0 || warmup) {
			PORTB |= (1<<SERVO_3);
			_delay_ms(0.6 + (float)adc_servo[2]/1024.0 * 2.4); // 0.6 - 3
			PORTB &= ~(1<<SERVO_3);
		}
		debounce = 10;
	}
}

ISR(SIG_ADC)
{
	static uint8_t toggle_adc = 0;
	if(state == 1) {
		servo_start[toggle_adc] = ADC;
		adc_servo[toggle_adc] = ADC;
	}
	else if(state == 2) {
		servo_end[toggle_adc] = ADC;
		adc_servo[toggle_adc] = ADC;
	}

	if(toggle_adc == 0) {
		ADMUX |= (1<<MUX0);
	} else if(toggle_adc == 1) {
		ADMUX &= ~(1<<MUX0);
		ADMUX |= (1<<MUX1);
	}
	toggle_adc++;
	if(toggle_adc > 2) {
		ADMUX &= ~((1<<MUX0) | (1<<MUX1));
		toggle_adc = 0;
	}
}


/*
TITLE	: UART Communication: ATmega 2560 & ESP32
DATE	: 2019/11/12
AUTHOR	: e-Yantra Team

AIM: To send data on UART#0 of ATMega 2560 to ESP32
*/


#define F_CPU 14745600		// Define Crystal Frequency of eYFi-Mega Board
#include <avr/io.h>				// Standard AVR IO Library
#include <util/delay.h>			// Standard AVR Delay Library
#include <avr/interrupt.h>		// Standard AVR Interrupt Library
#include "uart.h"				// Third Party UART Library

#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>

#include "/home/rajiv/Life/iitb/684/lab/Experiments/3_LCD_Interfacing/3_LCD_Interfacing/lcd.h"

#define PIN_LED_GREEN	PH5		// Macro for Pin Number of Green Led
#define MESSAGE			"Tx from ATmega 2560> Hello ESP32!\n" // Message to be sent on UART #0

volatile unsigned int count = 0;	// Used in ISR of Timer2 to store ms elasped
unsigned int seconds = 0;			// Stores seconds elasped
char rx_byte;
char lcd_array[100];
int idx = 0;

// structs to store request states
struct scan_req {
	int plot, id, serverTime, completeIn;
};

struct fetch_req {
	int id, serverTime, completeIn;
};

struct scan_resp {
	int id, plot, timeTaken;
};

struct fetch_resp {
	int id, timeTaken;
	char *type;
};

void init_timer2(void){
	cli();	// Turn off global interrupts

	//Setup Timer2 to fire every 1ms
	TCCR2B = 0x00;        						// Cut off Clock Source to disbale Timer2 while we set it up
	TCNT2  = 130;         						// Reset Timer Count to 130 out of 255
	TIFR2  &= ~(1 << TOV2);        				// Timer2 INT Flag Reg: Clear Timer Overflow Flag
	TIMSK2 |= (1 << TOIE2);        				// Timer2 INT Reg: Timer2 Overflow Interrupt Enable
	TCCR2A = 0x00;        						// Timer2 Control Reg A: Wave Gen Mode normal
	TCCR2B |= (1 << CS22) | (1 << CS20);        // Timer2 Control Reg B: Timer Prescaler set to 128 and Start Timer2

	sei();	// Turn on global interrupts
}


//Timer2 Overflow Interrupt Vector
ISR(TIMER2_OVF_vect) {
  count++;	// increment after 1 ms               
  
  // increment seconds variable after 1000 ms
  if(count > 999){
	seconds++;
	
	// uart0_puts(MESSAGE);    // Send data on UART #0 after 1 second

    count = 0;          
  }
  
  TCNT2 = 130;           	// Reset Timer to 130 out of 255
  TIFR2  &= ~(1 << TOV2);	// Timer2 INT Flag Reg: Clear Timer Overflow Flag
};


void init_led(void){
	DDRH    |= (1 << PIN_LED_GREEN);    
	PORTH   |= (1 << PIN_LED_GREEN);    
}

void led_greenOn(void){
	PORTH &= ~(1 << PIN_LED_GREEN);

}

void led_greenOff(void){
	PORTH |= (1 << PIN_LED_GREEN);
}


char uart0_readByte(void){

	uint16_t rx;
	uint8_t rx_status, rx_data;

	rx = uart0_getc();
	rx_status = (uint8_t)(rx >> 8);
	rx = rx << 8;
	rx_data = (uint8_t)(rx >> 8);

	if(rx_status == 0 && rx_data != 0){
		return rx_data;
	} else {
		return -1;
	}

}

int main(void) {
	lcd_port_config();					// Initialize the LCD port
	lcd_init();							// Initialize the LCD	

	init_led();
	init_timer2();

	uart0_init(UART_BAUD_SELECT(115200, F_CPU));
	uart0_flush();
	// lcd_string(1, 1, "Welcome");

	lcd_string(0, 0, "init");

	while(1){
		// uart0_puts("Qwertyuiop");
		rx_byte = uart0_readByte();
		// if(rx_byte == -1) {
		// 	lcd_string(0, 0, "gand maro");
		// }
		if(isalnum(rx_byte)) {
				// lcd_clear();
				// lcd_string(0, 0, "receiving...");
			//	uart0_putc(rx_byte);
			// 	led_greenOn();
			if(idx < 100)
				lcd_array[idx++] = rx_byte;
		}
//		lcd_string(0, 0, lcd_array);
		if(rx_byte == '-') {
			idx = 0;
			if(lcd_array[0]=='s') {
				lcd_clear();
				lcd_string(0, 0, "scan");

				struct scan_req *req_info = malloc(sizeof(struct scan_req));
				char *curr = strtok(lcd_array, " ");
				int curr_int;
				sscanf(curr, &curr_int);
				req_info->plot = curr_int;
				curr = strtok(NULL, " ");
				sscanf(curr, &curr_int);
				req_info->id = curr_int;
				curr = strtok(NULL, " ");
				sscanf(curr, &curr_int);
				req_info->serverTime = curr_int;
				curr = strtok(NULL, " ");
				sscanf(curr, &curr_int);
				req_info->completeIn = curr_int;
				
				struct scan_resp *resp_info = malloc(sizeof(struct scan_resp));
				// pass req_info and resp_info to the motion function
				// _delay_ms(5000);
				resp_info->id = 100;
				resp_info->plot = 13;
				resp_info->timeTaken = 30;
				char resp_vals[100];
				sprintf(resp_vals, "%d %d %d", resp_info->id, resp_info->plot, resp_info->timeTaken);
				uart0_puts(resp_vals);
				lcd_clear();
				lcd_string(0, 0, resp_vals);
				free(req_info);
				free(resp_info);
			}
			else if(lcd_array[0]=='f') {
				lcd_clear();
				lcd_string(0, 0, "fetch");
			}
		
			// parse values and pass to the motion function which will be blocking
			// after finding motion, response parameters are set
			// response params are sent to esp32 as uart
		}
		// // Turn On green led if seconds elasped are even else turn it off
		// if((seconds % 2) == 0){
		// 	led_greenOn();
			
		// } else {
		// 	led_greenOff();isalnum
		// }

	}

	return 0;	
}

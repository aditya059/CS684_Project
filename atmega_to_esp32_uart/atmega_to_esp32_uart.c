// /*
// TITLE	: UART Communication: ATmega 2560 & ESP32
// DATE	: 2019/11/12
// AUTHOR	: e-Yantra Team

// AIM: To send data on UART#0 of ATMega 2560 to ESP32
// */


#define F_CPU 14745600		// Define Crystal Frequency of eYFi-Mega Board
#include <avr/io.h>				// Standard AVR IO Library
#include <util/delay.h>			// Standard AVR Delay Library
#include <avr/interrupt.h>		// Standard AVR Interrupt Library
#include "uart.h"				// Third Party UART Library

#include "firebird_avr.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include "lcd.h"

#define PIN_LED_GREEN	PH5		// Macro for Pin Number of Green Led
#define MESSAGE			"Tx from ATmega 2560> Hello ESP32!\n" // Message to be sent on UART #0

#define angle_resolution 4.090			//resolution used for angle rotation
#define distance_resolution 5.338		//resolution used for distance traversal

// related to path planning
#define SIZE 9
#define NO_OF_PLOTS 16

#define NORTH 'n'
#define SOUTH 's'
#define WEST 'w'
#define EAST 'e'

#define GREEN 'G'
#define RED 'R'

#define SCAN 'S'

#define DESTINATION 'D'

// To store the eBot's Current and Goal location
typedef struct
{
	int x, y;
} Point;

Point curr_loc = {9, 4}, med_loc = {4, 8};

// To store the direction in which eBot is currently facing
unsigned char dir_flag = NORTH;

unsigned long int ShaftCountLeft = 0; 	//to keep track of left position encoder 
unsigned long int ShaftCountRight = 0; 	//to keep track of right position encoder			
unsigned int Degrees; 					//to accept angle in degrees for turning

volatile unsigned int count = 0;	// Used in ISR of Timer2 to store ms elasped
unsigned int seconds = 0;			// Stores seconds elasped
char rx_byte;
char lcd_array[100];
int idx = 0;

unsigned char left_wl_sensor_data, center_wl_sensor_data, right_wl_sensor_data;

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

/**
 * @brief      Function to configure motor pins
 */
void motors_pin_config(void) {
	motors_dir_ddr_reg |= (1 << motors_RB_pin) | (1 << motors_RF_pin) | (1 << motors_LF_pin) | (1 << motors_LB_pin) ;			// motor pin as output
	motors_dir_port_reg &=  ~( (1 << motors_RB_pin) | (1 << motors_RF_pin) | (1 << motors_LF_pin) | (1 << motors_LB_pin) );		// stop motor initially
}

/**
 * @brief      Function to configure left and right channel pins of the L293D Motor Driver IC for PWM
 */
void pwm_pin_config(void){
	motors_pwm_ddr_reg |= (1 << motors_pwm_R_pin) | (1 << motors_pwm_L_pin);	// left and right channel pin as output
	motors_pwm_port_reg |= (1 << motors_pwm_R_pin) | (1 << motors_pwm_L_pin);	// turn on left and right channel
}

/**
 * @brief      Function to configure left and right encoder pins
 */
void position_encoder_pin_config (void)
{
 	position_encoder_ddr_reg  &= ~(1 << left_encoder_pin | 1 << right_encoder_pin);  	//Set the direction of the encoder pins as input
 	position_encoder_port_reg |= (1 << left_encoder_pin | 1 << right_encoder_pin); 		//Enable internal pull-up for encoder pins
}

/**
 * @brief      Function to configure external interrupt for encoder pins
 */
void position_encoder_interrupt_config (void)
{
 	// all interrupts have to be disabled before configuring interrupts
	cli();	// Disable Interrupts Globally
	
	// Turn ON INT4 and INT5 (alternative function of PE4 and PE5 i.e Left and Right Encoder Pin)
	EIMSK_reg |= (1 << interrupt_left_encoder_pin | 1 << interrupt_right_encoder_pin);

	// Falling Edge detection on INT4 and INT5 pins
	EICRB_reg |= (1 << interrupt_ISC_left_bit1 | 1 << interrupt_ISC_right_bit1);
	EICRB_reg &= ~(1 << interrupt_ISC_left_bit0 | 1 << interrupt_ISC_right_bit0);

	sei();	// Enable Interrupts Globally
}

/**
 * @brief      Function to initialize Timer 5 in FAST PWM mode for speed control of motors of Firebird-V
 *
 */
void timer5_init() {
	TCCR5B_reg = 0x00;	//Stop
	
	TCNT5H_reg = 0xFF;	//Counter higher 8-bit value to which OCR5xH value is compared with
	TCNT5L_reg = 0x01;	//Counter lower 8-bit value to which OCR5xH value is compared with
	
	OCR5AH_reg = 0x00;	//Output compare register high value for Left Motor
	OCR5AL_reg = 0xFF;	//Output compare register low value for Left Motor
	
	OCR5BH_reg = 0x00;	//Output compare register high value for Right Motor
	OCR5BL_reg = 0xFF;	//Output compare register low value for Right Motor
	
	// Clear on Compare
	TCCR5A_reg |= (1 << COMA1_bit) | (1 << COMB1_bit);
	TCCR5A_reg &= ~( (1 << COMA0_bit) | (1 << COMB0_bit));

	// Configure for FAST PWM
	TCCR5A_reg |= (1 << WGM0_bit);
	TCCR5A_reg &= ~(1 << WGM1_bit);
	TCCR5B_reg |= (1 << WGM2_bit);
	TCCR5B_reg &= ~(1 << WGM3_bit);

	// Set Prescalar to 64
	TCCR5B_reg |= (1 << CS1_bit) | (1 << CS0_bit);
	TCCR5B_reg &= ~(1 << CS2_bit);
}

//----------------------------- VELOCITY FUNCTION ----------------------------------------------

/**
 * @brief      Function to control the speed of both the motors of Firebird-V
 *
 * @param[in]  left_motor   Left motor speed 0 to 255
 * @param[in]  right_motor  Right motor speed 0 to 255
 */
void velocity (unsigned char left_motor, unsigned char right_motor) {
	OCR5AL_reg = left_motor;
	OCR5BL_reg = right_motor;
}

//----------------------------- INTERRUPT SERVICE ROUTINES ----------------------------------------------

/**
 * @brief      ISR for right position encoder
 */
ISR(INT5_vect)  
{
	ShaftCountRight++;  //increment right shaft position count
}

/**
 * @brief      ISR for left position encoder
 */
ISR(INT4_vect)
{
	ShaftCountLeft++;  //increment left shaft position count
}

//----------------------------- MOTION RELATED FUNCTIONS ----------------------------------------------

/**
 * @brief      Function to make Firebird-V move forward.
 */
void forward (void) //both wheels forward
{
  	motors_dir_port_reg &=  ~( (1 << motors_RB_pin) | (1 << motors_LB_pin) );	// Make LB and RB LOW
	motors_dir_port_reg |= (1 << motors_RF_pin) | (1 << motors_LF_pin) ;		// Make LF and RF HIGH
}

/**
 * @brief      Function to make Firebird-V move backward.
 */
void back (void) //both wheels backward
{
  	motors_dir_port_reg &=  ~( (1 << motors_RF_pin) | (1 << motors_LF_pin) );	// Make LF and RF LOW
	motors_dir_port_reg |= ((1 << motors_RB_pin) | (1 << motors_LB_pin)) ;		// Make LB and RB HIGH
}

/**
 * @brief      Function to make Firebird-V rotate left.
 */
void left (void) //Left wheel backward, Right wheel forward
{
  	motors_dir_port_reg &=  ~( (1 << motors_RB_pin) | (1 << motors_LF_pin) );	// Make LF and RB LOW
	motors_dir_port_reg |= (1 << motors_RF_pin) | (1 << motors_LB_pin) ;		// Make LB and RF HIGH
}

/**
 * @brief      Function to make Firebird-V rotate right.
 */
void right (void) //Left wheel forward, Right wheel backward
{
  	motors_dir_port_reg &=  ~( (1 << motors_LB_pin) | (1 << motors_RF_pin) );	// Make LB and RF LOW
	motors_dir_port_reg |= (1 << motors_LF_pin) | (1 << motors_RB_pin) ;		// Make LF and RB HIGH
}

/**
 * @brief      Function to make Firebird-V rotate soft left.
 */
void soft_left (void) //Left wheel stationary, Right wheel forward
{
	motors_dir_port_reg &=  ~( (1 << motors_LB_pin) | (1 << motors_RF_pin) | (1 << motors_LF_pin));	// Make LF, LB and RF LOW
	motors_dir_port_reg |= (1 << motors_RF_pin) ;	// Make RF HIGH
}

/**
 * @brief      Function to make Firebird-V rotate soft right.
 */
void soft_right (void) //Left wheel forward, Right wheel is stationary
{
 	motors_dir_port_reg &=  ~( (1 << motors_LB_pin) | (1 << motors_RF_pin) | (1 << motors_RB_pin));	// Make LB, RF and RB LOW
	motors_dir_port_reg |= (1 << motors_LF_pin) ;	// Make LF HIGH
}

/**
 * @brief      Function to make Firebird-V rotate backward left.
 */
void soft_left_2 (void) //Left wheel backward, right wheel stationary
{
 	motors_dir_port_reg &=  ~( (1 << motors_LF_pin) | (1 << motors_RF_pin) | (1 << motors_RB_pin));	// Make LF, RF and RB LOW
	motors_dir_port_reg |= (1 << motors_LB_pin) ;	// Make LB HIGH
}

/**
 * @brief      Function to make Firebird-V rotate backward right.
 */
void soft_right_2 (void) //Left wheel stationary, Right wheel backward
{
	motors_dir_port_reg &=  ~( (1 << motors_LF_pin) | (1 << motors_RF_pin) | (1 << motors_LB_pin));	// Make LF, RF and LB LOW
	motors_dir_port_reg |= (1 << motors_RB_pin) ;	// Make RB HIGH
}

/**
 * @brief      Function to make Firebird-V stop.
 */
void stop (void)
{
  	motors_dir_port_reg &=  ~( (1 << motors_LF_pin) | (1 << motors_RF_pin) | (1 << motors_LB_pin) | (1 << motors_RB_pin));	// Make LF, RF, LB and RB LOW
}

//----------------------------- ENCODER RELATED FUNCTIONS ----------------------------------------------

/**
 * @brief      Function to rotate Firebird-V by specified degrees
 * @param[in]  Degrees   Rotation angle 0 to 360
 */
void angle_rotate(unsigned int Degrees)
{
	 float ReqdShaftCount = 0;
	 unsigned long int ReqdShaftCountInt = 0;

	 ReqdShaftCount = (float) Degrees/ angle_resolution; // division by resolution to get shaft count
	 ReqdShaftCountInt = (unsigned int) ReqdShaftCount;
	 ShaftCountRight = 0; 
	 ShaftCountLeft = 0; 

	 while (1)
	 {
		  if((ShaftCountRight >= ReqdShaftCountInt) || (ShaftCountLeft >= ReqdShaftCountInt))
			break;
	 }
	//  lcd_string(0, 0, "Done!");
	 stop(); //Stop robot
}

/**
 * @brief      Function to move Firebird-V by specified distance
 * @param[in]  DistanceInMM   Distance in mm 0 to 65535
 */
void linear_distance_mm(unsigned int DistanceInMM)
{
	 float ReqdShaftCount = 0;
	 unsigned long int ReqdShaftCountInt = 0;

	 ReqdShaftCount = DistanceInMM / distance_resolution; // division by resolution to get shaft count
	 ReqdShaftCountInt = (unsigned long int) ReqdShaftCount;
  
	 ShaftCountRight = 0;
	 ShaftCountLeft = 0;
	 while(1)
	 {
		  if((ShaftCountRight >= ReqdShaftCountInt) || (ShaftCountLeft >= ReqdShaftCountInt))
			  break;
	 } 
	 stop(); //Stop robot
}

/**
 * @brief      Function to move forward Firebird-V by specified distance
 * @param[in]  DistanceInMM   Distance in mm 0 to 65535
 */
void forward_mm(unsigned int DistanceInMM)
{
	 forward();
	 linear_distance_mm(DistanceInMM);
}

/**
 * @brief      Function to move backward Firebird-V by specified distance
 * @param[in]  DistanceInMM   Distance in mm 0 to 65535
 */
void back_mm(unsigned int DistanceInMM)
{
	 back();
	 linear_distance_mm(DistanceInMM);
}

/**
 * @brief      Function to rotate Firebird-V left by specified degrees
 * @param[in]  Degrees   Rotation angle 0 to 360
 */
void left_degrees(unsigned int Degrees) 
{
	 // 88 pulses for 360 degrees rotation 4.090 degrees per count
	 left(); //Turn left
	 angle_rotate(Degrees);

}

/**
 * @brief      Function to rotate Firebird-V right by specified degrees
 * @param[in]  Degrees   Rotation angle 0 to 360
 */
void right_degrees(unsigned int Degrees)
{
	 // 88 pulses for 360 degrees rotation 4.090 degrees per count
	 right(); //Turn right
	 angle_rotate(Degrees);
}

/**
 * @brief      Function to rotate Firebird-V left by specified degrees
 * @param[in]  Degrees   Rotation angle 0 to 360
 */
void soft_left_degrees(unsigned int Degrees)
{
	 // 176 pulses for 360 degrees rotation 2.045 degrees per count
	 soft_left(); //Turn soft left
	 Degrees=Degrees*2;
	 angle_rotate(Degrees);
}

/**
 * @brief      Function to rotate Firebird-V right by specified degrees
 * @param[in]  Degrees   Rotation angle 0 to 360
 */
void soft_right_degrees(unsigned int Degrees)
{
	 // 176 pulses for 360 degrees rotation 2.045 degrees per count
	 soft_right();  //Turn soft right
	 Degrees=Degrees*2;
	 angle_rotate(Degrees);
}

/**
 * @brief      Function to rotate Firebird-V left by specified degrees
 * @param[in]  Degrees   Rotation angle 0 to 360
 */
void soft_left_2_degrees(unsigned int Degrees)
{
	 // 176 pulses for 360 degrees rotation 2.045 degrees per count
	 soft_left_2(); //Turn reverse soft left
	 Degrees=Degrees*2;
	 angle_rotate(Degrees);
}

/**
 * @brief      Function to rotate Firebird-V right by specified degrees
 * @param[in]  Degrees   Rotation angle 0 to 360
 */
void soft_right_2_degrees(unsigned int Degrees)
{
	 // 176 pulses for 360 degrees rotation 2.045 degrees per count
	 soft_right_2();  //Turn reverse soft right
	 Degrees=Degrees*2;
	 angle_rotate(Degrees);
}

volatile unsigned long int pulse = 0; //to keep the track of the number of pulses generated by the color sensor
volatile unsigned long int red;       // variable to store the pulse count when read_red function is called
volatile unsigned long int blue;      // variable to store the pulse count when read_blue function is called
volatile unsigned long int green;     // variable to store the pulse count when read_green function is called

// void lcd_port_config (void)
// {
// 	DDRC = DDRC | 0xF7; //setting all the LCD pin's direction set as output
// 	PORTC = PORTC & 0x80; //setting all the LCD pins are set to logic 0 except PORTC 7
// }

void color_sensor_pin_config(void)
{
	DDRD  = DDRD | 0xFE; //set PD0 as input for color sensor output
	PORTD = PORTD | 0x01;//Enable internal pull-up for PORTD 0 pin
}

void port_init(void)
{
	// lcd_port_config();//lcd pin configuration
	color_sensor_pin_config();//color sensor pin configuration
}

void color_sensor_pin_interrupt_init(void) //Interrupt 0 enable
{
	cli(); //Clears the global interrupt
	EICRA = EICRA | 0x02; // INT0 is set to trigger with falling edge
	EIMSK = EIMSK | 0x01; // Enable Interrupt INT0 for color sensor
	sei(); // Enables the global interrupt
}

//ISR for color sensor
ISR(INT0_vect)
{
	pulse++; //increment on receiving pulse from the color sensor
}

void init_devices(void)
{
	cli(); //Clears the global interrupt
	port_init();  //Initializes all the ports
	color_sensor_pin_interrupt_init();
	sei();   // Enables the global interrupt
}

//Filter Selection
void filter_red(void)    //Used to select red filter
{
	//Filter Select - red filter
	PORTD = PORTD & 0xBF; //set S2 low
	PORTD = PORTD & 0x7F; //set S3 low
}

void filter_green(void)	//Used to select green filter
{
	//Filter Select - green filter
	PORTD = PORTD | 0x40; //set S2 High
	PORTD = PORTD | 0x80; //set S3 High
}

void filter_blue(void)	//Used to select blue filter
{
	//Filter Select - blue filter
	PORTD = PORTD & 0xBF; //set S2 low
	PORTD = PORTD | 0x80; //set S3 High
}

void filter_clear(void)	//select no filter
{
	//Filter Select - no filter
	PORTD = PORTD | 0x40; //set S2 High
	PORTD = PORTD & 0x7F; //set S3 Low
}

//Color Sensing Scaling
void color_sensor_scaling()		//This function is used to select the scaled down version of the original frequency of the output generated by the color sensor, generally 20% scaling is preferable, though you can change the values as per your application by referring datasheet
{
	//Output Scaling 20% from datasheet
	//PORTD = PORTD & 0xEF;
	PORTD = PORTD | 0x10; //set S0 high
	//PORTD = PORTD & 0xDF; //set S1 low
	PORTD = PORTD | 0x20; //set S1 high
}

void red_read(void) // function to select red filter and display the count generated by the sensor on LCD. The count will be more if the color is red. The count will be very less if its blue or green.
{
	//Red
	filter_red(); //select red filter
	pulse=0; //reset the count to 0
	_delay_ms(100); //capture the pulses for 100 ms or 0.1 second
	red = pulse;  //store the count in variable called red
	
	lcd_cursor(1,1);  //set the cursor on row 1, column 1
	lcd_string(0, 0, "Red Pulses"); // Display "Red Pulses" on LCD
	lcd_numeric_value(2,1,red,5);  //Print the count on second row
	_delay_ms(1000);	// Display for 1000ms or 1 second
	// lcd_wr_command(0x01); //Clear the LCD
}

void green_read(void) // function to select green filter and display the count generated by the sensor on LCD. The count will be more if the color is green. The count will be very less if its blue or red.
{
	//Green
	filter_green(); //select green filter
	pulse=0; //reset the count to 0
	_delay_ms(100); //capture the pulses for 100 ms or 0.1 second
	green = pulse;  //store the count in variable called green
	
	lcd_cursor(1,1);  //set the cursor on row 1, column 1
	lcd_string(0, 0, "Green Pulses"); // Display "Green Pulses" on LCD
	lcd_numeric_value(2,1,green,5);  //Print the count on second row
	_delay_ms(1000);	// Display for 1000ms or 1 second
	lcd_wr_command(0x01); //Clear the LCD
}

void blue_read(void) // function to select blue filter and display the count generated by the sensor on LCD. The count will be more if the color is blue. The count will be very less if its red or green.
{
	//Blue
	filter_blue(); //select blue filter
	pulse=0; //reset the count to 0
	_delay_ms(100); //capture the pulses for 100 ms or 0.1 second
	blue = pulse;  //store the count in variable called blue
	
	lcd_cursor(1,1);  //set the cursor on row 1, column 1
	lcd_string(0, 0, "Blue Pulses"); // Display "Blue Pulses" on LCD
	lcd_numeric_value(2,1,blue,5);  //Print the count on second row
	_delay_ms(1000);	// Display for 1000ms or 1 second
	lcd_wr_command(0x01); //Clear the LCD
}



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

char uart2_readByte(void){

	uint16_t rx;
	uint8_t rx_status, rx_data;

	rx = uart2_getc();
	rx_status = (uint8_t)(rx >> 8);
	rx = rx << 8;
	rx_data = (uint8_t)(rx >> 8);

	if(rx_status == 0 && rx_data != 0){
		return rx_data;
	} else {
		return -1;
	}

}

//Timer2 Overflow Interrupt Vector
ISR(TIMER2_OVF_vect) {
  count++;	// increment after 1 ms               
  
  // increment seconds variable after 1000 ms
  if(count > 999){
	seconds++;
	
	// uart2_puts(MESSAGE);	    // Send data on UART #0 after 1 second

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

void read_color_sensor() {
	red_read(); //display the pulse count when red filter is selected
}




void adc_port_config (void)
{
	adc_sensor_low_ddr_reg		= 0x00;				// set PORTF direction as input
	adc_sensor_low_port_reg		= 0x00;				// set PORTF pins floating
	adc_sensor_high_ddr_reg		= 0x00;				// set PORTK direction as input
	adc_sensor_high_port_reg	= 0x00;				// set PORTK pins floating
}

void adc_init(){
	
	// enable ADC and pre-scalar = 64 (ADEN = 1, ADPS2 = 1, ADPS1 = 1, ADPS0 = 0)
	// and clear ADC start conversion bit, auto trigger enable bit, interrupt flag bit and interrupt enable bit
	ADCSRA_reg	|= ( (1 << ADEN_bit) | (1 << ADPS2_bit) | (1 << ADPS1_bit) );
	ADCSRA_reg	&= ~( (1 << ADSC_bit) | (1 << ADATE_bit) | (1 << ADIF_bit) | (1 << ADIE_bit) | (1 << ADPS0_bit) );
	
	// In ADCSRB, disable Analog Comparator Multiplexer, MUX5 bit and ADC Auto Trigger Source bits
	ADCSRB_reg	&= ~( (1 << ACME_bit) | (1 << MUX5_bit) | (1 << ADTS2_bit) | (1 << ADTS1_bit) | (1 << ADTS0_bit) );
	
	// In ADMUX, set the Reference Selection bits to use the AVCC as reference, and disable the channel selection bits MUX[4:0]
	ADMUX_reg	&= ~( (1 << REFS1_bit) | (1 << MUX4_bit) | (1 << MUX3_bit) | (1 << MUX2_bit) | (1 << MUX1_bit) | (1 << MUX0_bit) );
	ADMUX_reg	|= (1 << REFS0_bit);
	
	// In ADMUX, enable the ADLAR bit for 8-bit ADC result
	ADMUX_reg	|= (1 << ADLAR_bit);
	
	// In ACSR, disable the Analog Comparator by writing 1 to ACD_bit
	ACSR_reg	|= ( 1 << ACD_bit );
}

unsigned char ADC_Conversion(unsigned char channel_num)
{
	unsigned char adc_8bit_data;
	
	// MUX[5:0] bits to select the ADC channel number
	if ( channel_num > 7 )
	{
		ADCSRB_reg |= ( 1 << MUX5_bit );					// set the MUX5 bit for selecting channel if its greater than 7
	}
	channel_num	= channel_num & 0x07;						// retain the last 3 bits from the variable for MUX[2:0] bits
	ADMUX_reg	= ( ADMUX_reg | channel_num );
	
	// set the ADSC bit in ADCSRA register
	ADCSRA_reg		|= ( 1 << ADSC_bit );
	
	//Wait for ADC conversion to complete
	while( ( ADCSRA_reg & ( 1 << ADIF_bit ) ) == 0x00 );
	
	adc_8bit_data = ADCH_reg;
	
	// clear ADIF bit by writing 1 to it
	ADCSRA_reg		|= ( 1 << ADIF_bit );
	
	// clear the MUX5 bit
	ADCSRB_reg		&= ~( 1 << MUX5_bit );
	
	// clear the MUX[4:0] bits
	ADMUX_reg		&= ~( (1 << MUX4_bit) | (1 << MUX3_bit) | (1 << MUX2_bit) | (1 << MUX1_bit) | (1 << MUX0_bit) );
	
	return adc_8bit_data;
}

void readSensor() {
	left_wl_sensor_data = ADC_Conversion(left_wl_sensor_channel);
	center_wl_sensor_data = ADC_Conversion(center_wl_sensor_channel);
	right_wl_sensor_data = ADC_Conversion(right_wl_sensor_channel);
}

void printSensor() {
	lcd_numeric_value(2, 2, left_wl_sensor_data, 3);
	lcd_numeric_value(2, 6, center_wl_sensor_data, 3);
	lcd_numeric_value(2, 10, right_wl_sensor_data, 3);
}

void left_turn_wls(void)
{
	// printf("Left Turn\n");
	// printf("Initial Direction = %c\n", dir_flag);
	left();
	// _delay_ms(100);
	velocity(120, 120);
	while (1)
	{
		// timer++;
		readSensor();
		if (left_wl_sensor_data < 10 && center_wl_sensor_data > 20 && right_wl_sensor_data < 10)
		{
			stop();
			break;
		}
		// _delay_ms(10);
		lcd_clear();
		lcd_numeric_value(1, 1, ShaftCountLeft, 4);
		lcd_numeric_value(1, 4, ShaftCountRight, 4);
	}
	if (dir_flag == NORTH)
		dir_flag = WEST;
	else if (dir_flag == WEST)
		dir_flag = SOUTH;
	else if (dir_flag == SOUTH)
		dir_flag = EAST;
	else
		dir_flag = NORTH;
}

void right_turn_wls(void)
{
	// printf("Left Turn\n");
	// printf("Initial Direction = %c\n", dir_flag);
	left();
	// _delay_ms(100);
	velocity(120, 120);
	while (1)
	{
		// timer++;
		readSensor();
		if (left_wl_sensor_data < 10 && center_wl_sensor_data > 20 && right_wl_sensor_data < 10)
		{
			stop();
			break;
		}
		// _delay_ms(10);
		lcd_clear();
		lcd_numeric_value(1, 1, ShaftCountLeft, 4);
		lcd_numeric_value(1, 4, ShaftCountRight, 4);
	}
	if (dir_flag == NORTH)
		dir_flag = WEST;
	else if (dir_flag == WEST)
		dir_flag = SOUTH;
	else if (dir_flag == SOUTH)
		dir_flag = EAST;
	else
		dir_flag = NORTH;
}

// bool last_state = 0; // 0-left, 1-right

void forward_wls(unsigned char node)
{
	for (unsigned char i = 0; i < node; i++)
	{
		unsigned char t = 0;
		unsigned char s = 1;
		while (1)
		{

			readSensor();

			if (left_wl_sensor_data < 10 && center_wl_sensor_data > 20 && right_wl_sensor_data < 10)
			{ // WBW
				lcd_clear();
				lcd_string(0, 0, "WBW");
				// printSensor();
				velocity(255,255);
				forward();
			}
			else if (left_wl_sensor_data < 10 && center_wl_sensor_data > 20 && right_wl_sensor_data > 20)
			{ // WBB
				lcd_clear();
				lcd_string(0, 0, "WBB");	
				// printSensor();
				velocity(110,110);
				soft_right();
			}
			else if (left_wl_sensor_data < 10 && center_wl_sensor_data < 10 && right_wl_sensor_data > 20)
			{ // WWB
				lcd_clear();
				lcd_string(0, 0, "WWB");	
				// printSensor();
				velocity(110,110);
				right();
			}
			else if (left_wl_sensor_data > 20 && center_wl_sensor_data > 20 && right_wl_sensor_data < 10)
			{ // BBW
				lcd_clear();
				lcd_string(0, 0, "BBW");
				// printSensor();
				velocity(110,110);
				soft_left();
			}
			else if (left_wl_sensor_data > 20 && center_wl_sensor_data < 10 && right_wl_sensor_data < 10)
			{ // BWW
				lcd_clear();
				lcd_string(0, 0, "BWW");
				// printSensor();
				velocity(110,110);
				left();
			}
			else if (left_wl_sensor_data > 20 && center_wl_sensor_data > 20 && right_wl_sensor_data > 20)
			{ // BBB
				// printf("Intersection Reached\n");
				lcd_clear();
				lcd_numeric_value(1, 1, i, 4);
				// printSensor();
//				lcd_string(1, 0, "Destination reached!");
				forward();
				velocity(110,110);
				_delay_ms(300);
				velocity(0, 0);
				if (dir_flag == NORTH)
					curr_loc.x--;
				else if (dir_flag == SOUTH)
					curr_loc.x++;
				else if (dir_flag == WEST)
					curr_loc.y--;
				else
					curr_loc.y++;
				// printf("Current: (%d, %d)\n", curr_loc.x, curr_loc.y);
				break;
			}
			else
			{	 // WWW or any other reading
				lcd_clear();
				lcd_string(0, 0, "WWW");
				// printSensor();
				if (t % 4 == 0)
				{
					velocity(110,110);
					left();
				}
				else if (t % 4 == 1)
				{
					velocity(110,110);
					right();
				}
				else if (t % 4 == 2)
				{
					velocity(110,110);
					right();
				}
				else
				{
					velocity(110,110);
					left();
				}
				_delay_ms(s * 5);
				if (t % 4 == 3)
					s *= 2;
				t++;
				continue;
			}
			t = 0;
			s = 1;
			// _delay_ms(10);
		}
	}
}

int main(void) {

	adc_port_config();
	adc_init();

	lcd_port_config();					// Initialize the LCD port
	lcd_init();							// Initialize the LCD	

	init_led();
	init_timer2();

	uart2_init(UART_BAUD_SELECT(115200, F_CPU));
	uart2_flush();

    init_devices();
    lcd_set_4bit();
	color_sensor_scaling();

	motors_pin_config();
	pwm_pin_config();
	position_encoder_pin_config();
	position_encoder_interrupt_config();

	timer5_init();

	float BATT_Voltage, BATT_V;
	BATT_V = ADC_Conversion(batt_sensor_channel);
	BATT_Voltage = ( ( BATT_V * 100 ) * 0.07902 ) + 0.7;
	lcd_numeric_value(1, 1, BATT_Voltage, 4);

	_delay_ms(1000);

	while(1) {
		forward_wls(3);
		left_turn_wls();
		break;
		// readSensor();
		// if (left_wl_sensor_data < 10 && center_wl_sensor_data > 20 && right_wl_sensor_data < 10) {
		// 	// WBW
		// 	lcd_clear();
		// 	lcd_string(0, 0, "WBW");
		// 	// printSensor();
		// } else if (left_wl_sensor_data < 10 && right_wl_sensor_data > 20) {
		// 	// WBB or WWB
		// 	lcd_clear();
		// 	lcd_string(0, 0, "WBB or WWB");
		// 	// printSensor();
		// } else if (left_wl_sensor_data > 20 && right_wl_sensor_data < 10) { 
		// 	// BWW & BBW
		// 	lcd_clear();
		// 	lcd_string(0, 0, "BWW & BBW");
		// 	// printSensor();
		// } else if (left_wl_sensor_data > 20 && center_wl_sensor_data > 20 && right_wl_sensor_data > 20) { 
		// 	// BBB
		// 	lcd_clear();
		// 	lcd_string(0, 0, "BBB");
		// 	// printSensor();
		// } else {
		// 	// WWW
		// 	lcd_clear();
		// 	lcd_string(0, 0, "WWW");
		// 	// printSensor();
		// }
		// _delay_ms(500);
		// rx_byte = uart2_readByte();
		// if(isalnum(rx_byte)) {
		// 	if(idx < 100)
		// 		lcd_array[idx++] = rx_byte;
		// }
		// if(rx_byte == '-') {
		// 	idx = 0;
		// 	if(lcd_array[0]=='s') {
		// 		lcd_clear();
		// 		lcd_string(0, 0, "scan");
		// 		_delay_ms(500);
		// 		// forward_mm(240);
		// 		// left_degrees(90);
		// 		// forward_mm(720);
		// 		// right_degrees(90);
		// 		// lcd_clear();
		// 		// read_color_sensor();
		// 		// right_degrees(90);
		// 		// forward_mm(720);
		// 		// right_degrees(90);
		// 		// forward_mm(240);

		// 		// struct scan_req *req_info = malloc(sizeof(struct scan_req));
		// 		// char *curr = strtok(lcd_array, " ");
		// 		// int curr_int;
		// 		// sscanf(curr, &curr_int);
		// 		// req_info->plot = curr_int;
		// 		// curr = strtok(NULL, " ");
		// 		// sscanf(curr, &curr_int);
		// 		// req_info->id = curr_int;
		// 		// curr = strtok(NULL, " ");
		// 		// sscanf(curr, &curr_int);
		// 		// req_info->serverTime = curr_int;
		// 		// curr = strtok(NULL, " ");
		// 		// sscanf(curr, &curr_int);
		// 		// req_info->completeIn = curr_int;
				
		// 		// struct scan_resp *resp_info = malloc(sizeof(struct scan_resp));
		// 		// pass req_info and resp_info to the motion function
		// 		// _delay_ms(5000);
		// 		// resp_info->id = 100;
		// 		// resp_info->plot = 13;
		// 		// resp_info->timeTaken = 30;
		// 		// char resp_vals[100];
		// 		// sprintf(resp_vals, "%d %d %d", resp_info->id, resp_info->plot, resp_info->timeTaken);
		// 		uart2_puts(MESSAGE);
		// 		// lcd_clear();
		// 		// lcd_string(0, 0, resp_vals);
		// 		// free(req_info);
		// 		// free(resp_info);
		// 	}
		// 	else if(lcd_array[0]=='f') {
		// 		lcd_clear();
		// 		lcd_string(0, 0, "fetch");
		// 	}
		// }	
	}

	return 0;	
}

/*
 * GccApplication150.c
 *
 * Created: 10-Oct-19 10:08:34 PM
 *  Author: Animesh_Sharma
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <math.h> //included to support power function
#include "lcd.c"



char display_row[17] = "Dist:    .  cm  ";

unsigned int ADC_Conversion(unsigned char);
unsigned int ADC_Value = 0;
float distance_in_cm = 0;

//Function to configure LCD port
void lcd_port_config (void)
{
	DDRC = DDRC | 0xF7;   //all the LCD pin's direction set as output
	PORTC = PORTC & 0x80; // all the LCD pins are set to logic 0 except PORTC 7
}

//ADC pin configuration
void adc_pin_config (void)
{
	DDRK = 0x7F;  //set PORTK7 direction as input
	PORTK = 0x00; //set PORTK7 pins floating
}

// ultarasonic trigger configuration
void ultrsonic_trigger_config(void)
{
	DDRB = DDRB | 0x10;   // PB4 direction set as output
	PORTB = PORTB & 0x00; // PB4 set to logic 0
}

//Function to Initialize PORTS
void port_init()
{
	lcd_port_config();
	adc_pin_config();
	ultrsonic_trigger_config();
}

//Function to Initialize ADC
void adc_init()
{
	ADCSRA = 0x00;
	ADCSRB = 0x00;		//MUX5 = 0
	ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//This Function accepts the Channel Number and returns the corresponding Analog Value
unsigned int ADC_Conversion(unsigned char Ch)
{
	//unsigned char a;
	unsigned int a = 0,b = 0;
	
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}

	Ch = Ch & 0x07;
	ADMUX= 0x20| Ch;
	ADCSRA = ADCSRA | 0x40;	    //Set start conversion bit
	while((ADCSRA&0x10)==0);	    //Wait for ADC conversion to complete
	b = (unsigned int)(ADCL>>6);   //read & adjust ADCL result to read as a right adjusted result
	a = (unsigned int)(ADCH<<2);   //read & adjust ADCH result to read as a right adjusted result
	a = a | b;
	ADCSRA = ADCSRA|0x10; //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	ADCSRB = 0x00;
	return a;
}

// This Function prints the Analog Value Of Corresponding Channel No. at required Row
// and Coloumn Location.
void print_sensor(char row, char coloumn,unsigned char channel)
{
	ADC_Value = ADC_Conversion(channel);
	lcd_print(row, coloumn, ADC_Value, 4);
}

//--------------------------------------------------------------------------------
// Ultrasonic sensor are connected in chaining mode. This function rise a
// trigger pulse of >20usec to command ringing.
//--------------------------------------------------------------------------------
void ultrasonic_trigger(void)
{
	PORTB = PORTB | 0x10;  // make high the Trigger input for Ultrasonic sensor
	_delay_us(50);         // Wait for >20usec
	PORTB = PORTB & 0xEF;  // make low the Trigger input for Ultrasonic sensor
}

void init_devices (void)
{
	cli(); //Clears the global interrupts
	port_init();
	adc_init();
	sei(); //Enables the global interrupts
}

//Main Function
int main(void)
{
	float distance_in_cm;
	unsigned int distance_in_cm_int;
	unsigned int distance_in_cm_fraction;

	init_devices();                  // initialise required modules
	
	lcd_set_4bit();                  // initialise LCD
	lcd_init();                      // initialise LCD

	lcd_string(display_row);          // display message on 1st row
	_delay_ms(150);
	
	while(1)
	{
		ultrasonic_trigger();            // call ultrasonic triggering after enery 150msec
		_delay_ms(150);

		ADC_Value = ADC_Conversion(15);
		
		distance_in_cm = ADC_Value * 1.268;  // where, 5V/1024 = 0.00488/step & 9.85mV/2.54cm = 0.00385mV/cm
		// for distance in cm, we get 0.00488/0.00385 = 1.267 as const multiplier

		distance_in_cm_int = floor(distance_in_cm);        // seperate integer part from float value
		distance_in_cm_fraction = (distance_in_cm * 100);  // seperate fractional part from float value

		lcd_print(2, 7, distance_in_cm_int, 3);            // print integer part on LCD
		lcd_print(2, 11, distance_in_cm_fraction, 2);       // print fractional part on LCD


	}
}
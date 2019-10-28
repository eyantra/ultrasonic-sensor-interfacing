/*
 * ud.h
 *
 * Created: 26-Oct-19 11:56:54 PM
 * Modified: 28-Oct-19 10:44:06 PM
 * Author: Debdut
 * Modifier: Debdut
 */ 

#ifndef UD_H_
#define UD_H_

#define UD_PORT PORTD //SENSOR PORT REG
#define UD_PD DDRD //SENSOR PORT DATA DIRECTION REG
#define UD_TRIG_PIN PIND1 //SENSOR TRIGGER PIN
#define UD_ECHO_PIN PIND0 //SENSOR ECHO PIN
#define UD_PDR PIND //SENSOR INPUT DATA REG

/*
 * Clock Cycles and Time related formulas:
 * Clock Cycles per Micro Second = CPU Frequency / 1MHz { EX: 16000000UL / 1000000UL = 16 cycles }
 * Micro Seconds to Clock Cycles = Micro Seconds * Clock Cycles per Micro Second { EX: 1000 * (16000000UL / 1000000UL) = 16000 cycles }
 * Clock Cycles to Micro Seconds = Clock Cycles / Clock Cycles per Micro Second { EX: 16000 / (16000000UL / 1000000UL) = 1000 us }
 */
#ifndef _CT_F
#define _CT_F

#define _CPM() (F_CPU / 1000000UL) //CLOCK CYCLES PER MICRO SECOND
#define _MTC(us) ((us) * _CPM()) //MICRO SECONDS TO CLOCK CYCLES
#define _CTM(cl) ((cl) / _CPM()) //CLOCK CYCLES TO MICRO SECONDS

#endif

#define UD_MAX_ECHO_TIME 23400 //APPROX MAX TIME IN MICRO SECONDS IF SENSOR IS GETTING IT'S MAX DISTANCE READING OF ~400cm
#define UD_TIMEOUT -1 //UD_TIMEOUT TO -1
#define UD_MAX_CYCLE _MTC(1000) //CYCLES IN 1000 MICRO SECONDS

#include <stdbool.h>

void ud_port_config(void);
void ud_init();

void ud_wake();
bool ud_getData();
uint16_t ultrasonicReading();

uint32_t ud_getPulse();

uint16_t ud_distanceCm; //GLOBAL VARIABLE FOR STORING DISTANCE IN cm

//Function to config sensor port
void ud_port_config(void)
{
	UD_PD |= (1 << UD_TRIG_PIN); //SET PIN DIRECTION TO OUTPUT
	UD_PD &= ~(1 << UD_ECHO_PIN); //SET PIN DIRECTION TO INPUT
	UD_PORT &= ~(1 << UD_TRIG_PIN); //WRITE LOW TO OUTPUT
	UD_PORT &= ~(1 << UD_ECHO_PIN); //DISABLE INPUT PULLUP
}

//Function to initialize sensor
void ud_init()
{
	ud_port_config();
}

//Function to wake sensor
void ud_wake()
{
	UD_PORT |= (1 << UD_TRIG_PIN); //WRITE HIGH TO OUTPUT
	_delay_us(10); //DELAY FOR 10us
	UD_PORT &= ~(1 << UD_TRIG_PIN); //WRITE LOW TO OUTPUT
}

//Function to get sensor data
bool ud_getData()
{
	uint32_t pulseWidthTimeUs;
	
	ud_wake();
	pulseWidthTimeUs = ud_getPulse();
	if(pulseWidthTimeUs >= UD_MAX_ECHO_TIME)
	{
		ud_distanceCm = UD_MAX_ECHO_TIME * 0.034 / 2;
		return true;
	}
	
	if(pulseWidthTimeUs != UD_TIMEOUT)
	{
		ud_distanceCm = pulseWidthTimeUs * 0.034 / 2;
		return true;
	}
	
	return false;
}

//Function to get distance reading
uint16_t ultrasonicReading()
{
	if(ud_getData())
		return ud_distanceCm;
	else
		return (uint16_t)NAN;
}

//Function to get echo pulse duration in clock cycles
uint32_t ud_getPulse()
{
	uint32_t cycles = 0;
	uint32_t pulseWidthCycles = 0;
	
	//Wait for if any earlier echo high pulse to end
	while(((UD_PDR & (1 << UD_ECHO_PIN)) >> UD_ECHO_PIN) == 1)
	{
		if(cycles++ >= UD_MAX_CYCLE)
			return UD_TIMEOUT;
	}
	
	//Wait for the echo high pulse to start
	while(((UD_PDR & (1 << UD_ECHO_PIN)) >> UD_ECHO_PIN) != 1)
	{
		if(cycles++ >= UD_MAX_CYCLE)
			return UD_TIMEOUT;
	}
	
	//Timing critical code is starting
	cli(); //CLEAR INTERRUPT
	
	//Wait for the echo high pulse to stop
	while(((UD_PDR & (1 << UD_ECHO_PIN)) >> UD_ECHO_PIN) == 1)
	{
		if(cycles++ >= UD_MAX_CYCLE)
			return UD_TIMEOUT;
			
		pulseWidthCycles++;
	}
	
	//Timing critical code is ending
	sei(); //SET INTERRUPT

	return _CTM(pulseWidthCycles * 24 + 16);
	/* Previous loop is ~20 clock cycles long and 
	   also have 16 clocks between the previous pulse edge 
	   and the start of the loop */
}

#endif
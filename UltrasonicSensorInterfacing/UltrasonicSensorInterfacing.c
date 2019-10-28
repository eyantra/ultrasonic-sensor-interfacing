/*
 * SensorInterfacing.c
 *
 * Created: 27-Sep-19 9:04:13 AM
 * Modified: 28-Oct-19 11:01:02 AM
 * Author: Debdut
 * Modifier: Debdut
 */

#define F_CPU 14745600UL
//#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include "ud.h"

void port_init();
void init_devices(void);

void port_init()
{
	lcd_init();
	ud_init();
}

void init_devices(void)
{
	cli();
	port_init();
	sei();
}

int main(void)
{
	init_devices();
	
	lcd_cursor(1,3);
	lcd_string("HACTOBER-2K19");
	lcd_cursor(2,4);
	lcd_string("OPENSOURCE");
	_delay_ms(2000);
	lcd_clear();
	
	lcd_cursor(1,5);
	lcd_string("DISTANCE");
	lcd_cursor(2,10);
	lcd_string("CM");
	
	while(1)
	{	
		lcd_print(2,6,ultrasonicReading(),4);
		_delay_ms(500);
	}
}
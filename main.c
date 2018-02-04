/******************************************************************************
* License		: GNU GPL
*******************************************************************************/
#define F_CPU 8000000UL									//XTAL 8MHz
#define HDC1000_start  0x80								// device address 0x40 <<1 = 0x80

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/sleep.h>
#include "i2cmaster.h"
#include <math.h>
#include "diskio.h"
#include "ff.h"
char text[32];											// global text buffer

void HDC1000_init(void)
{	
	i2c_init();										// initialize I2C library
	i2c_start_wait(HDC1000_start+I2C_WRITE);		// set device address and write mode
	i2c_write(0x02);                        
	i2c_write(0x10);
	i2c_write(0x00);
	i2c_stop();
}

void mesure(void)										// start mesure parameter
{	
	
	float teplota,vlhkost;
	int hum1,hum2,temp1,temp2;
	uint16_t temp,hum;
		
	// start measuring
	i2c_start_wait(HDC1000_start+I2C_WRITE);		// set device address and write mode
	i2c_write(0x00);
	_delay_ms(20);
	
	// read value
	i2c_start_wait(HDC1000_start+I2C_READ);			// set device address and read mode
	temp1 = i2c_readAck();							//LSB temperature
	temp2 = i2c_readAck();							// MSB Temperature
	hum1 = i2c_readAck();							//LSB Humidity
	hum2 = i2c_readNak();							// MSB humidity
	//
	i2c_stop();
	
	temp = (temp1 << 8) | temp2;					// 2x8bit to one 16bit
	teplota=(temp* (160.0/65536.0) - 40);			//temperature calculation
	hum = (hum1 << 8) | hum2;
	vlhkost=((hum/65536.0)* 100);					//humidity calculation

	sprintf(text,"Vlhkost:%.1f \nTeplota:%.1f ",vlhkost,teplota); // solution convert to char
	
}

void blink_fail(void)
{
	PORTD |=  (1<<PD1);
	_delay_ms(200);
	PORTD &= ~(1<<PD1);
}

void blink_ok(void)
{
	PORTD |= (1<<PD0);
	_delay_ms(200);
	PORTD &= ~(1<<PD0);
}

int main()
{
	DDRC = 0x83;
	
	
	
	DSTATUS status = STA_NOINIT;
	FIL file;
	static FATFS fs;

	while(status)
	{
		status = disk_initialize(0);
		if(status) blink_fail();
		else blink_ok();
	}
	HDC1000_init();
	f_mount(0,&fs);
	f_open(&file,"graf.txt",FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&file,f_size(&file));

	short int i = 0;

	while(1)
	{
		
		for (i = 0;i<8;i++)
		{ 
			mesure();
			f_printf(&file,text,i);
			f_sync(&file);
			blink_ok();
		}

		f_close(&file);
		

		break;
	}

	return 0;
}

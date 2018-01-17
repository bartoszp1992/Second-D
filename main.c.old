/*
 * main.c
 *
 *	Bart's second-D
 *
 *	for ATmega 328P
 *	Created on: 10 sty 2018
 *	Author: Bart
 *
 *	Ver 3.0
 *	-added Display
 */


// #define F_CPU 1000000UL (no use in eclipse- go to properties)
#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "hd44780.h"

#define fire (1<<PC2)
#define plus (1<<PC1)
#define minus (1<<PC0)
#define pwm PB1
#define stb PC5
#define adcin PC3
#define ind4 PD0
#define ind3 PD1
#define ind2 PD2
#define ind1 PD3
#define buz PD4
#define warm PD7
#define bl PD6

//Pins description
// 25(PC2)button fire
// 24(PC1)button plus
// 23(PC0)button minus

// 15(PB1) PWM Out(mosfet gate)

// 28(PC5) standby(green)
// 26(PC3) ADC3 negative input

// 02(PD0) ind4 (blue)
// 03(PD1) ind3 (blue)
// 04(PD2) ind2 (blue)
// 05(PD3) ind1 (blue)
// 13(PD7) warming(red)
// 06(PD4) buzzer
// 12(PD6) backlight

//beeps description
// 2- locked
// 3- unlocked
// 1-menu enter/exit/set/check battery
// 4- no atomizer
// 5- low battery
//long- short circuit

//add bytes for save values
EEMEM unsigned char d; //for duty
EEMEM unsigned char p; // for preheat duty
EEMEM unsigned char t; // for preheat time
int itoa();
int sprintf(char *str, const char *format, ...);

int main(void)
{

//Variables and constants
//main
int duty, dutyD, step, adj_counter, counter, phstep, phd;
step = 1; //step for duty change
duty = 30; //default duty
dutyD = duty / 2.55;
adj_counter = 20;//after which steps adjust set to fast mode
counter = 1;


char buffer[10];


//preheat time inters
phstep = 1; //preheat adjust step
phd = 255; //default ph duty

//times
int t_blink, warmDelay, t_splash, t_splashD, t_display, t_press, v_check, t_adj_s, t_adj_f, v_display, t_unlock, tph, tph1, tph2, tph3, tph4;

t_blink = 120; // blinking at notifications
t_splash = 80; // time for splash and other transitions/ 1 state lock blinking
t_press = 200;// waiting after press(menu)
v_display = 1500; // display battery state time/0 state lock blinking
t_adj_s = 35; //slow power adjust time
t_adj_f = 12; //fast power adjust time/preheat adjust time
t_unlock = 800; //unlocking time
t_splashD = 5000;
t_display = 1000;
warmDelay = 240;
//preheat times
tph1 = 100;
tph2 = 200;
tph3 = 500;
tph4 = 800;
tph = 1;//default preheat time

//voltages
int voltage, low_voltage, actualDecrease, min_l_v, l_v, fl_v, v_dcr, max_v_dcr, i_v, min_v_dcr;
low_voltage = 2800;
min_l_v = 2900; // minimum load voltage


//modes
int menu_toggle, opt, mode, lock, pha, tpha;
menu_toggle = 0;
opt = 4; //default optin after enter menu(4- lock)
mode = 0; //default mode(normal)
lock = 1; //lock on by default
pha = 0; //preheat adjust mode
tpha = 0; //preheat time adjust mode

//frequently changed
max_v_dcr = 980;//(on one cell)default: 650 for short protection- voltage decrease at max battery current
min_v_dcr = 50; //10- for single, 50 for serial//(on one cell)no atomizer- min voltage decrease
v_check = 350; //(us) 2000 single, 350 for serial(for 100uF cap) checking load voltages- time for align cell capacitor

//LCD
int dutyDisplay1, dutyDisplay2, voltageDisplay, modeDisplay;
dutyDisplay1 = 0;
dutyDisplay2 = 5;
voltageDisplay = 10;
modeDisplay = 64;

//define ports
//B- PWM PB1(pin15)- OC1A
DDRB = 0x3F;  // 111 111
PORTB = 0x00; // 000 000

//D- indicator, warming led.
DDRD = 0xFF;  // 1111 1111 all D as Out
PORTD = 0x00; // 0000 0000

//C- voltage meter, standby led(PC5, pin28), (PC6 reset- dont use), buttons
DDRC = 0x60;  // 110 0 000 -PC6 reset, PC5 standby led, others for voltage meter
PORTC = 0x07; // 000 0 111 - buttons high

//define hardware PWM

	//mode3, fast PWM
TCCR1A |= (1<<WGM10);
TCCR1B |= (1<<WGM12);

	//compare output mode(set for pin15)-- moved to warming
//TCCR1A |= (1<<COM1A1);

	//prescaler/freq(freq= clock/prescaler/256)
//TCCR1B |= (1<<CS10); // no prescaler
TCCR1B |= (1<<CS11);// /8

OCR1A = 0; //PWM out as low

//define ADC
//prescaler
ADCSRA |= (1<<ADPS1);
ADCSRA |= (1<<ADPS2);
//adc ON
ADCSRA |= (1<<ADEN);

	//ref select- internal
//SFIOR |= (1<<ACME);

//pin select
ADMUX |= (1<<MUX1);
ADMUX |= (1<<MUX0);

	//set Vcc ref
ADMUX |= (1<<REFS0);

//read last values from eeprom
duty = eeprom_read_byte(&d); //duty
phd = eeprom_read_byte(&p); // preheat duty
tph = eeprom_read_byte(&t); // preheat time


//splash
PORTC |= (1<<stb);
PORTD |= (1<<bl);

lcd_init();
lcd_clrscr();
lcd_puts("Ampera NOVA");
lcd_goto(64);
lcd_puts("Bart's second v3");
_delay_ms(t_splashD);

PORTC &= ~(1<<stb);
lcd_init();

while(1)
{
//pwm off
TCCR1A &= ~(1<<COM1A1);

// standby light on
PORTC |= (1<<stb);
PORTD &= ~(1<<warm);

//backlight on
PORTD |= (1<<bl);

//low battery check
ADCSRA |= (1<<ADSC); //start
while(ADCSRA & (1<<ADSC));
voltage = ADC * 10;

while(voltage < low_voltage )
	{
	OCR1A = 0;
	TCCR1A &= ~(1<<COM1A1);

	//low battery blinking
	lcd_clrscr();
	lcd_goto(modeDisplay);
	lcd_puts("Low voltage");

	PORTD ^= (1<<warm) | (1<<bl);
	_delay_ms(t_blink);

	//check again
	ADCSRA |= (1<<ADSC); //start
	while(ADCSRA & (1<<ADSC));
	voltage = ADC * 10;
	}

if(mode != 2)
{
//DD
lcd_goto(dutyDisplay1);
lcd_puts("Duty:");
dutyD = duty / 2.55 ;
itoa(dutyD, buffer, 10);
lcd_goto(dutyDisplay2);
lcd_puts(buffer);
lcd_puts("   ");
}
else
{
	lcd_goto(dutyDisplay1);
	lcd_puts("          ");
}



//DV
lcd_goto(voltageDisplay);
sprintf(buffer, "%dmV", voltage);
lcd_puts(buffer);

//DM
lcd_goto(modeDisplay);

if(mode == 2) lcd_puts("mechanical      ");
else if(mode == 1) lcd_puts("preheat         ");
else if(mode == 0) lcd_puts("normal          ");

_delay_ms(10);

// lock
while(lock == 1)
	{
	TCCR1A &= ~(1<<COM1A1);
	PORTD &= ~(1<<bl);

	lcd_init();
	lcd_clrscr();

	if(mode != 2)
	{
	//DD
	lcd_goto(dutyDisplay1);
	lcd_puts("Duty:");
	dutyD = duty / 2.55 ;
	itoa(dutyD, buffer, 10);
	lcd_goto(dutyDisplay2);
	lcd_puts(buffer);
	lcd_puts("   ");
	}
	else
	{
		lcd_goto(dutyDisplay1);
		lcd_puts("          ");
	}


	//DV
	lcd_goto(voltageDisplay);
	sprintf(buffer, "%dmV", voltage);
	lcd_puts(buffer);

	//DM
	lcd_goto(modeDisplay);
	lcd_puts("locked");

	PORTC |= (1<<stb);
	_delay_ms(t_splash);
	PORTC &= ~(1<<stb);
	_delay_ms(v_display);


	if (!(PINC & fire) && !(PINC & minus) && (PINC & plus))
		{
		lcd_init();
		lcd_clrscr();
		lcd_goto(modeDisplay);
		lcd_puts("Unlocking...");
		PORTD |= (1<<bl);
		_delay_ms(t_unlock);
		if(!(PINC & fire) && !(PINC & minus) && (PINC & plus))
			{
			menu_toggle = 0;
			lock = 0;
			PORTC |= (1<<stb);
			lcd_clrscr();
			lcd_puts("Ampera NOVA");
			lcd_goto(64);
			lcd_puts("Bart's second v3");
			_delay_ms(t_display);
			}
		}
	}

// menu

if (!(PINC & fire) && !(PINC & minus) && (PINC & plus) && (menu_toggle == 0))
	{
	menu_toggle = 1;
	opt = 4;
	lcd_clrscr();
	lcd_puts("Menu");
	_delay_ms(t_display);
	}

while(menu_toggle == 1)
	{
	if (!(PINC & plus) && (opt < 4))
		{
		opt = opt +1;
		PORTD |= (1<<buz);
		_delay_ms(t_press);
		PORTD &= ~(1<<buz);
		}

	if (!(PINC & minus) && (opt > 1))
		{
		opt = opt -1;
		PORTD |= (1<<buz);
		_delay_ms(t_press);
		PORTD &= ~(1<<buz);
		}

	if ( opt == 1 )
	{
		lcd_goto(modeDisplay);
		lcd_puts("<   mechanical  ");
	}
	if ( opt == 2 )
	{
		lcd_goto(modeDisplay);
		lcd_puts("<    preheat   >");
	}
	if ( opt == 3 )
	{
		lcd_goto(modeDisplay);
		lcd_puts("<    normal    >");
	}
	if ( opt == 4 )
	{
		lcd_goto(modeDisplay);
		lcd_puts("      lock     >");
	}

	//opt1- mechanic
	if ((opt == 1) && !(PINC & fire) && (PINC & minus))
		{
		mode = 2;
		_delay_ms(t_display);

		menu_toggle = 0;
		}


	//opt2- preheat
	if ((opt == 2) && !(PINC & fire) && (PINC & minus))
		{
		mode = 1;
		pha = 1;
		_delay_ms(t_press);

		//adjust ph power
		while(pha == 1)
			{

			lcd_goto(dutyDisplay1);
			lcd_puts("Preheat duty:");
			dutyD = phd / 2.55 ;
			itoa(dutyD, buffer, 10);
			lcd_goto(modeDisplay);
			lcd_puts(buffer);
			lcd_puts("                 ");

			if(!(PINC & fire))
				{
				pha = 0;
				tpha = 1;
				_delay_ms(t_press);
				}

			if(!(PINC & plus) && (phd <= 255 - phstep))
				{
				phd = phd + step;
				_delay_ms(t_adj_f);
				}

			if(!(PINC & minus) && (phd >= 0 + phstep))
				{
				phd = phd - step;
				_delay_ms(t_adj_f);
				}
			}
		//adjust preheat time
		while(tpha == 1)
			{
			if ( tph > 4 ) tph = 1;

			if ( tph == 1 )
			{
				lcd_goto(dutyDisplay1);
				lcd_puts("Preheat time:             ");
				itoa(tph1, buffer, 10);
				lcd_goto(modeDisplay);
				lcd_puts(buffer);
				lcd_puts("ms                ");
				_delay_ms(t_press);
			}
			if ( tph == 2 )
			{
				lcd_goto(dutyDisplay1);
				lcd_puts("Preheat time:             ");
				itoa(tph2, buffer, 10);
				lcd_goto(modeDisplay);
				lcd_puts(buffer);
				lcd_puts("ms                ");
				_delay_ms(t_press);
			}
			if ( tph == 3 )
			{
				lcd_goto(dutyDisplay1);
				lcd_puts("Preheat time:             ");
				itoa(tph3, buffer, 10);
				lcd_goto(modeDisplay);
				lcd_puts(buffer);
				lcd_puts("ms                ");
				_delay_ms(t_press);
			}
			if ( tph == 4 )
			{
				lcd_goto(dutyDisplay1);
				lcd_puts("Preheat time:             ");
				itoa(tph4, buffer, 10);
				lcd_goto(modeDisplay);
				lcd_puts(buffer);
				lcd_puts("ms                ");
				_delay_ms(t_press);
			}

			if(!(PINC & fire))
				{
				tpha = 0;
				menu_toggle = 0;
				_delay_ms(t_display);
				}

			if(!(PINC & plus) && (tph < 4 ))
				{
				tph = tph + 1;
				_delay_ms(t_press);
				}

			if(!(PINC & minus) && (tph > 1 ))
				{
				tph = tph - 1;
				_delay_ms(t_press);
				}
			}
		}

//opt3 normal
	if ((opt == 3) && !(PINC & fire) && (PINC & minus))
		{
		mode = 0;
		menu_toggle = 0;
		_delay_ms(t_display);
		}

	//opt4 lock
	if ((opt == 4) && !(PINC & fire) && (PINC & minus) && (lock == 0))
		{
		lock = 1;
		menu_toggle = 0;
		_delay_ms(t_press);
		}
//menu exit
	if (!(PINC & fire) && !(PINC & minus) && (PINC & plus) && (menu_toggle == 1))
		{
		menu_toggle = 0;
		}
	}

// setting duty

// reset counter
counter = 1;

//plus
while(!(PINC & plus) && (PINC & minus) && (PINC & fire) && (duty <= 252 - step) && (mode != 2))
	{
	PORTD |= (1<<warm);
	duty = duty + step;
	//slow mode
	if(counter < adj_counter)
		{
		_delay_ms(t_adj_s);
		PORTD &= ~(1<<warm);
		_delay_ms(t_adj_s);

		lcd_goto(dutyDisplay1);
		lcd_puts("Duty:");
		dutyD = duty / 2.55 ;
		itoa(dutyD, buffer, 10);
		lcd_goto(dutyDisplay2);
		lcd_puts(buffer);
		lcd_puts("   ");

		}
	//fast mode
	if(counter >= adj_counter)
		{
		_delay_ms(t_adj_f);
		PORTD |= (1<<warm);

		lcd_goto(dutyDisplay1);
		lcd_puts("Duty:");
		dutyD = duty / 2.55 ;
		itoa(dutyD, buffer, 10);
		lcd_goto(dutyDisplay2);
		lcd_puts(buffer);
		lcd_puts("   ");
		}

	counter = counter + 1;
	}

//minus
while(!(PINC & minus) && (PINC & plus) && (PINC & fire) && (duty >= 26 + step) && (mode != 2))
	{
	PORTD |= (1<<warm);
	duty = duty - step;


	if(counter < adj_counter)
		{
		_delay_ms(t_adj_s);
		PORTD &= ~(1<<warm);
		_delay_ms(t_adj_s);

		lcd_goto(dutyDisplay1);
		lcd_puts("Duty:");
		dutyD = duty / 2.55 ;
		itoa(dutyD, buffer, 10);
		lcd_goto(dutyDisplay2);
		lcd_puts(buffer);
		lcd_puts("   ");
		}

	if(counter >= adj_counter)
		{
		_delay_ms(t_adj_f);
		PORTD |= (1<<warm);

		lcd_goto(dutyDisplay1);
		lcd_puts("Duty:");
		dutyD = duty / 2.55 ;
		itoa(dutyD, buffer, 10);
		lcd_goto(dutyDisplay2);
		lcd_puts(buffer);
		lcd_puts("   ");
		}

	counter = counter + 1;
}


//warming
while(!(PINC & fire) && (PINC & minus) && (PINC & plus))
	{
	//pwm on
	PORTD |= (1<<warm);
	TCCR1A |= (1<<COM1A1);

	//mechanical warming
	while(!(PINC & fire) && (mode == 2) && (PINC & minus) && (PINC & plus))
		{
		OCR1A = 255;
		PORTD = (1<<warm) | (1<<ind1) | (1<<ind2) | (1<<ind3) |(1<<ind4) | (1<<bl);
		}

	//save duty
	eeprom_write_byte(&d, duty);
	_delay_ms(20);

	//save preheat duty
	eeprom_write_byte(&p, phd);
	_delay_ms(20);

	//save preheat time
	eeprom_write_byte(&t, tph);
	_delay_ms(20);

	//check voltage decrease
		//idle Voltage
	OCR1A = 0;
	_delay_us(v_check);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	i_v = ADC * 10;
	OCR1A = 0;

	//full load voltage
	OCR1A = 255;
	_delay_us(v_check);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	fl_v = ADC * 10;
	OCR1A = 0;

		//decrease voltage
	v_dcr = i_v - fl_v;

	//actual load
	OCR1A = duty;
	_delay_us(v_check);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	l_v = ADC * 10;
	OCR1A = 0;

	//anti-short protection
	while(v_dcr >= max_v_dcr && !(PINC & fire) && (PINC & minus) && (PINC & plus) && (mode != 2))
		{
		OCR1A = 0;
		TCCR1A &= ~(1<<COM1A1);

		lcd_goto(modeDisplay);
		lcd_puts("Short circuit        ");
		_delay_ms(t_display);

		}

	//no atomizer
	while(v_dcr <= min_v_dcr && !(PINC & fire) && (PINC & minus) && (PINC & plus) && (mode != 2))
	 	 {
		OCR1A = 0;
		lcd_goto(modeDisplay);
		lcd_puts("No atomizer        ");
		_delay_ms(t_display);
	 	 }

	//low battery
	while(l_v <= min_l_v && !(PINC & fire) && (PINC & minus) && (PINC & plus) && (mode != 2))
		{
		OCR1A = 0;
		lcd_goto(modeDisplay);
		lcd_puts("Low battery        ");
		_delay_ms(t_display);
		}

	//normal warming
	while(v_dcr < max_v_dcr && l_v > min_l_v && !(PINC & fire) && (mode == 0) && (PINC & minus) && (PINC & plus))
		{
		OCR1A = duty;
		_delay_ms(warmDelay);
		ADCSRA |= (1<<ADSC);
		while(ADCSRA & (1<<ADSC));
		l_v = ADC * 10;


		OCR1A = 0;
		_delay_us(v_check);
		ADCSRA |= (1<<ADSC);
		while(ADCSRA & (1<<ADSC));
		i_v = ADC * 10;
		OCR1A = 0;

		OCR1A = 255;
		_delay_us(v_check);
		ADCSRA |= (1<<ADSC);
		while(ADCSRA & (1<<ADSC));
		fl_v = ADC * 10;
		OCR1A = 0;

		v_dcr = i_v - fl_v;

		actualDecrease = i_v - l_v;

		lcd_goto(voltageDisplay);
		sprintf(buffer, "%dmV", i_v);
		lcd_puts(buffer);

		itoa(actualDecrease, buffer, 10);
		lcd_goto(modeDisplay);
		lcd_puts("Load:");
		lcd_puts(buffer);
		lcd_puts("mV   ");

		}

	//preheat warming
	while(v_dcr < max_v_dcr && l_v >= min_l_v && !(PINC & fire) && (mode == 1) && (PINC & minus) && (PINC & plus))
		{
		OCR1A = phd;
		if (tph == 1) _delay_ms(tph1);
		if (tph == 2) _delay_ms(tph2);
		if (tph == 3) _delay_ms(tph3);
		if (tph == 4) _delay_ms(tph4);


		while(v_dcr < max_v_dcr && l_v >= min_l_v && !(PINC & fire) && (mode == 1))
			{
			OCR1A = duty;
			_delay_ms(warmDelay);
			ADCSRA |= (1<<ADSC);
			while(ADCSRA & (1<<ADSC));
			l_v = ADC * 10;

			OCR1A = 0;
			_delay_us(v_check);
			ADCSRA |= (1<<ADSC);
			while(ADCSRA & (1<<ADSC));
			i_v = ADC * 10;
			OCR1A = 0;

			OCR1A = 255;
			_delay_us(v_check);
			ADCSRA |= (1<<ADSC);
			while(ADCSRA & (1<<ADSC));
			fl_v = ADC * 10;
			OCR1A = 0;

			v_dcr = i_v - fl_v;

			actualDecrease = i_v - l_v;

			lcd_goto(voltageDisplay);
			sprintf(buffer, "%dmV", i_v);
			lcd_puts(buffer);

			itoa(actualDecrease, buffer, 10);
			lcd_goto(modeDisplay);
			lcd_puts("Load:");
			lcd_puts(buffer);
			lcd_puts("mV   ");

			}// afterheat close
		}//prehat warming close

	//pwm off
	TCCR1A &= ~(1<<COM1A1);

	}//warming close
	OCR1A = 0;
}//1 close
}//uC close

/*
 * main.c
 *
 *  Created on: 17 sty 2018
 *      Author: bartosz21
 *      for ATmega 328P
 *
 *      Second-D
 *      v3.1.2
 *
 *      changelog:
 *      3.1.1- rewrited for 8x2 disply
 *      3.1.2- correct amperometer formula
 *
 *      Pinout:
 *      25(PC2) fire
 *      24(PC1) plus
 *      23(PC0) minus
 *
 *      15(PB1) PWM Out
 *
 *      28(PC5) standby LED(green)
 *      26(PC3) ADC input
 *
 *      13(PD7) warming LED(red)
 *      12(PD6) backlight
 *
 *      02-05(PD0-PD3) Display out
 *      10(PB7)- RS line
 *      11(PD5)- E line
 */


#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "hd44780.h"

#define fire (1<<PC2)
#define plus (1<<PC1)
#define minus (1<<PC0)
#define pwm PB1
#define stb PC5
#define warm PD7
#define backlight PD6


//add EEPROM bytes
EEMEM unsigned char d; //for duty
EEMEM unsigned char p; // for preheat duty
EEMEM unsigned char t; // for preheat time
EEMEM float c; // for cell indicator
EEMEM float m; // for max decrease

//display functions define
int itoa();
int sprintf(char *str, const char *format, ...);
int dtoa();

//main function
int main(void){
	//Variables
	//main
	int counter;
	float dutyReset = 30;
	float duty = 30;
	int preheatDuty = 30;
	int percentDuty = duty / 2.55;
	int slowSteps = 15; //after which steps adjust set to fast mode

	//display
	char buffer[10];

	int dutyDisplay = 0;
	//int logo = 6;
	int voltageDisplay = 4;
	int modeDisplay = 64;
	int currentDisplay = 68;

	//times
	int lowBlink = 120;//blinking at low input
	int lockBlink = 80;//green blink when locked
	int press = 200;//button press delay
	int lockPause = 1500;
	int slowAdjust = 70;
	int fastAdjust = 10;
	int unlocking = 800; //unlocking delay
	int splash = 1500;//splash screen
	int warmingDelay = 240;
	int reload = 350; //capacitor reload(us)(350 for 100uF)

	//preheat presets
	int preheatTime = 1;//default preheat time
	int pht1 = 100;
	int pht2 = 200;
	int pht3 = 500;
	int pht4 = 800;
	int pht5 = 1500;

	//currencies
	//float maxCurrent = 20.0;
	float fullCurrent;
	float loadCurrent;
	float cellIndicatorReset = 25;
	float cellIndicator = cellIndicatorReset;//C = I(A) / U(V) decrease

	//power
	//float power;

	//voltages
	float voltage;
	float lowVoltage = 5.6;
	//float loadDecrease;
	float fullDecrease;
	float load;
	float fullLoad;
	float idle;
	float minDecrease = 0.1;
	float maxDecreaseReset = 1.4;
	float maxDecrease = maxDecreaseReset;

	//modes
	unsigned int menuToggle = 0;
	int option;
	int mode = 0; //defalut mode
	/*
	 * 0-pwm
	 * 1-preheat
	 * 2-mech
	 */
	int locked = 1;
	int preheatDutyAdjust  = 0;
	int preheatTimeAdjust = 0;
	int calibrate = 0;
	int maxDecreaseSet = 0;
	//end of variables

	//ports
	//B- PWM PB1(pin15)- OC1A
	DDRB = 0x3F;  // 111 111
	PORTB = 0x00; // 000 000

	//D- indicator, warming led.
	DDRD = 0xFF;  // 1111 1111 all D as Out
	PORTD = 0x00; // 0000 0000

	//C- voltage meter, standby led(PC5, pin28), (PC6 reset- dont use), buttons
	DDRC = 0x60;  // 110 0 000 -PC6 reset, PC5 standby led, others for voltage meter
	PORTC = 0x07; // 000 0 111 - buttons high


	//hardware PWM define
	TCCR1A |= (1<<WGM10);//mode-fastPWM
	TCCR1B |= (1<<WGM12);
	//prescaler-freq= clock/prescaler/256
	TCCR1B |= (1<<CS10); //1
	//TCCR1B |= (1<<CS11); //8
	OCR1A = 0;//pwm out LOW
	TCCR1A &= ~(1<<COM1A1); //pwm off

	//ADC define
	ADCSRA |= (1<<ADPS1);//prescaler
	ADCSRA |= (1<<ADPS2);
	ADCSRA |= (1<<ADEN);//ON
	ADMUX |= (1<<MUX1);//pin select
	ADMUX |= (1<<MUX0);
	ADMUX |= (1<<REFS0);//reference voltage


	//read saved values from eeprom
	duty = eeprom_read_byte(&d); //duty
	preheatDuty = eeprom_read_byte(&p); // preheat duty
	preheatTime = eeprom_read_byte(&t); // preheat time
	cellIndicator = eeprom_read_float(&c); // cell indicator
	maxDecrease = eeprom_read_float(&m); // maxDecrease

	if (cellIndicator > 250) cellIndicator = cellIndicatorReset;
	if (duty > 255) duty = dutyReset;


	//splash
	PORTC |= (1<<stb);
	PORTD |= (1<<warm);
	PORTD |= (1<<backlight);
	lcd_init();
	lcd_clrscr();
	lcd_puts("Bart's");
	lcd_goto(modeDisplay);
	lcd_puts("Second 3");
	_delay_ms(splash);
	lcd_clrscr();


	while(1){//main loop

		TCCR1A &= ~(1<<COM1A1);

		PORTC |= (1<<stb);
		PORTD &= ~(1<<warm);
		PORTD |= (1<<backlight);


		ADCSRA |= (1<<ADSC); //start
		while(ADCSRA & (1<<ADSC));
		voltage = (float)ADC / 50;

		while (voltage < lowVoltage)
		{
			OCR1A = 0;
			TCCR1A &= ~(1<<COM1A1);

			lcd_clrscr();
			lcd_goto(modeDisplay);
			lcd_puts("Low bat!");

			PORTD ^= (1<<warm) | (1<<backlight);
			_delay_ms(lowBlink);

			ADCSRA |= (1<<ADSC); //start
			while(ADCSRA & (1<<ADSC));
			voltage = (float)ADC / 50;

		}

		//Display duty
		if(mode != 2){
			lcd_goto(dutyDisplay);
			percentDuty = duty / 2.55;
			sprintf(buffer, ">%d", percentDuty);
			lcd_puts(buffer);
		}
		else{
			lcd_goto(dutyDisplay);
			lcd_puts("  ");
		}

		//display logo
		//lcd_goto(logo);
		//lcd_puts("Bart");

		//Display voltage
		lcd_goto(voltageDisplay);
		sprintf(buffer, "%1.1fV", voltage);
		lcd_puts(buffer);

		//Display mode
		lcd_goto(modeDisplay);
		if(mode == 2) lcd_puts("m   ");
		else if(mode == 1) lcd_puts("ph  ");
		else if(mode == 0) lcd_puts("pwm ");

		//Display current
		if (mode != 2){
			lcd_goto(currentDisplay);
			lcd_puts("0A");
		}


		//lock
		while(locked == 1){
			TCCR1A &= ~(1<<COM1A1);
			PORTD &= ~(1<<backlight);

			lcd_goto(dutyDisplay);
			lcd_puts("   ");

			//Display voltage
			lcd_goto(voltageDisplay);
			sprintf(buffer, "%1.1fV", voltage);
			lcd_puts(buffer);

			//DM
			lcd_goto(modeDisplay);
			lcd_puts("locked  ");

			PORTC |= (1<<stb);
			_delay_ms(lockBlink);
			PORTC &= ~(1<<stb);
			_delay_ms(lockPause);

			if(!(PINC & fire) && !(PINC & minus) && (PINC & plus)){
				lcd_init();
				lcd_clrscr();
				lcd_goto(modeDisplay);
				lcd_puts("hold... ");
				PORTD |= (1<<backlight);
				_delay_ms(unlocking);

				if(!(PINC & fire) && !(PINC & minus) && (PINC & plus)){
					menuToggle = 0;
					locked = 0;
					PORTC |= (1<<stb);
					PORTD |= (1<<warm);
					PORTD |= (1<<backlight);
					lcd_init();
					lcd_clrscr();
					lcd_puts("Bart's");
					lcd_goto(modeDisplay);
					lcd_puts("Second 3");
					_delay_ms(splash);
					lcd_clrscr();
				}
			}
		}

		//menu
		if(!(PINC & fire) && !(PINC & minus) && (PINC & plus) && (menuToggle == 0)){
			menuToggle = 1;
			option = 4;
			lcd_clrscr();
			lcd_puts("menu");
			_delay_ms(unlocking);
		}

		while(menuToggle == 1){
			if(!(PINC & plus) && (option < 6)){
				option = option + 1;
				_delay_ms(press);
			}

			if(!(PINC & minus) && (option > 1)){
				option = option - 1;
				_delay_ms(press);
			}

			lcd_goto(modeDisplay);

			if (option == 1) lcd_puts(">M      ");
			if (option == 2) lcd_puts(">P M    ");
			if (option == 3) lcd_puts(">N P M  ");
			if (option == 4) lcd_puts(">L N P M");
			if (option == 5) lcd_puts(">c L N P");
			if (option == 6) lcd_puts(">a c L N");


			if (!(PINC & fire) && (PINC & minus)){
				if (option == 1) mode = 2; //mech
				if (option == 2){//preheat
					mode = 1;
					preheatDutyAdjust = 1;
					_delay_ms(press);

					while(preheatDutyAdjust == 1){
						lcd_goto(dutyDisplay);
						lcd_puts("PH duty:");
						percentDuty = preheatDuty / 2.55;
						itoa(percentDuty, buffer, 10);
						lcd_goto(modeDisplay);
						lcd_puts("<");
						lcd_puts(buffer);
						lcd_puts(">    ");

						if(!(PINC & fire))
						{
							preheatDutyAdjust = 0;
							preheatTimeAdjust = 1;
							_delay_ms(press);
						}

						if(!(PINC & plus) && (preheatDuty < 255)){
							preheatDuty = preheatDuty + 1;
							_delay_ms(fastAdjust);
						}

						if(!(PINC & minus) && (preheatDuty > 0)){
							preheatDuty = preheatDuty - 1;
							_delay_ms(fastAdjust);
						}
					}

					while(preheatTimeAdjust == 1){
						if (preheatTime > 5) preheatTime = 1;
						lcd_goto(dutyDisplay);
						lcd_puts("PH time:");
						lcd_goto(modeDisplay);

						if(preheatTime == 1){
							itoa(pht1, buffer, 10);
							lcd_puts("<");
							lcd_puts(buffer);
							lcd_puts("ms> ");
							_delay_ms(press);
						}

						if(preheatTime == 2){
							itoa(pht2, buffer, 10);
							lcd_puts("<");
							lcd_puts(buffer);
							lcd_puts("ms> ");
							_delay_ms(press);
						}

						if(preheatTime == 3){
							itoa(pht3, buffer, 10);
							lcd_puts("<");
							lcd_puts(buffer);
							lcd_puts("ms> ");
							_delay_ms(press);
						}

						if(preheatTime == 4){
							itoa(pht4, buffer, 10);
							lcd_puts("<");
							lcd_puts(buffer);
							lcd_puts("ms> ");
							_delay_ms(press);
						}

						if(preheatTime == 5){
							itoa(pht5, buffer, 10);
							lcd_puts("<");
							lcd_puts(buffer);
							lcd_puts("ms> ");
							_delay_ms(press);
						}

						if(!(PINC & fire)){
							preheatTimeAdjust = 0;
							menuToggle = 0;
							lcd_clrscr();
							_delay_ms(press);
						}

						if(!(PINC & plus) && (preheatTime < 5)){
							preheatTime = preheatTime + 1;
							_delay_ms(press);
						}

						if(!(PINC & minus) && (preheatTime > 1)){
							preheatTime = preheatTime - 1;
							_delay_ms(press);
						}

					}

				}
				if (option == 3) mode = 0; //pwm
				if (option == 4) locked = 1;
				if (option == 5){
					calibrate = 1;
					_delay_ms(press);
					while(calibrate == 1){
						lcd_goto(dutyDisplay);
						lcd_puts("amp cal:");
						sprintf(buffer, "<%1.f>", cellIndicator);
						lcd_goto(modeDisplay);
						lcd_puts(buffer);
						lcd_puts("    ");

						if(!(PINC & fire) && (PINC & plus) && (PINC & minus)){
							calibrate = 0;
							_delay_ms(press);
						}

						if(!(PINC & plus) && (cellIndicator < 40) && (PINC & minus) && (PINC & fire)){
							cellIndicator = cellIndicator + 1;
							_delay_ms(press);
						}
						if(!(PINC & minus) && (cellIndicator > 10) && (PINC & plus) && (PINC & fire)){
							cellIndicator = cellIndicator - 1;
							_delay_ms(press);
						}
						if(!(PINC & minus) && (PINC & plus) && !(PINC & fire)){
							cellIndicator = cellIndicatorReset;
							_delay_ms(press);
						}

					}
				}
				if (option == 6){
					maxDecreaseSet = 1;
					_delay_ms(press);
					while(maxDecreaseSet == 1){
						lcd_goto(dutyDisplay);
						lcd_puts("max dec:");
						sprintf(buffer, "<%1.1fV>", maxDecrease);
						lcd_goto(modeDisplay);
						lcd_puts(buffer);
						lcd_puts("   ");

						if(!(PINC & fire) && (PINC & plus) && (PINC & minus)){
							maxDecreaseSet = 0;
							_delay_ms(press);
						}

						if(!(PINC & plus) && (maxDecrease < 1.9) && (PINC & minus) && (PINC & fire)){
							maxDecrease = maxDecrease + 0.1;
							_delay_ms(press);
						}
						if(!(PINC & minus) && (maxDecrease > 0.1) && (PINC & plus) && (PINC & fire)){
							maxDecrease = maxDecrease - 0.1;
							_delay_ms(press);
						}
						if(!(PINC & minus) && (PINC & plus) && !(PINC & fire)){
							maxDecrease = maxDecreaseReset;
							_delay_ms(press);
						}

					}
				}

				menuToggle = 0;
				lcd_clrscr();
				_delay_ms(press);
			}
		}

		//setting duty//
		counter = 1;

		//plus
		while(!(PINC & plus) && (PINC & minus) && (PINC & fire) && (duty < 250) && (mode != 2)){
			duty = duty + 1;

			if(counter < slowSteps){
				_delay_ms(slowAdjust);
				PORTD ^= (1<<warm);

				lcd_goto(dutyDisplay);
				percentDuty = duty / 2.55;
				sprintf(buffer, ">%d", percentDuty);
				lcd_puts(buffer);
			}
			if(counter > slowSteps){
				_delay_ms(fastAdjust);
				PORTD ^= (1<<warm);

				lcd_goto(dutyDisplay);
				percentDuty = duty / 2.55;
				sprintf(buffer, ">%d", percentDuty);
				lcd_puts(buffer);
			}
			counter = counter + 1;
		}

		while(!(PINC & minus) && (PINC & plus) && (PINC & fire) && (duty > 27) && (mode != 2)){
			duty = duty - 1;

			if(counter < slowSteps){
				_delay_ms(slowAdjust);
				PORTD ^= (1<<warm);

				lcd_goto(dutyDisplay);
				percentDuty = duty / 2.55;
				sprintf(buffer, ">%d", percentDuty);
				lcd_puts(buffer);
			}
			if(counter > slowSteps){
				_delay_ms(fastAdjust);
				PORTD ^= (1<<warm);

				lcd_goto(dutyDisplay);
				percentDuty = duty / 2.55;
				sprintf(buffer, ">%d", percentDuty);
				lcd_puts(buffer);
			}
			counter = counter + 1;

		}

		//warming
		while(!(PINC & fire) && (PINC & minus) && (PINC & plus)){
			PORTD |= (1<<warm);
			TCCR1A |= (1<<COM1A1);


			eeprom_write_byte(&d, duty);
			_delay_ms(20);
			eeprom_write_float(&c, cellIndicator);
			_delay_ms(20);
			eeprom_write_float(&m, maxDecrease);
			_delay_ms(20);
			eeprom_write_byte(&p, preheatDuty);
			_delay_ms(20);
			eeprom_write_byte(&t, preheatTime);
			_delay_ms(20);


			//mech
			while(!(PINC & fire) && (mode == 2) && (PINC & minus) && (PINC & plus)){
				OCR1A = 255;

				ADCSRA |= (1<<ADSC); //start
				while(ADCSRA & (1<<ADSC));
				voltage = (float)ADC / 50;

				lcd_goto(voltageDisplay);
				sprintf(buffer, "%1.1fV", voltage);
				lcd_puts(buffer);

				lcd_goto(currentDisplay);
				lcd_puts("    ");



			}

			//check idle
			OCR1A = 0;
			_delay_us(reload);
			ADCSRA |= (1<<ADSC); //start
			while(ADCSRA & (1<<ADSC));
			idle = (float)ADC / 50;

			//full load
			OCR1A = 255;
			_delay_us(reload);
			ADCSRA |= (1<<ADSC); //start
			while(ADCSRA & (1<<ADSC));
			fullLoad = (float)ADC / 50;
			OCR1A = 0;

			fullDecrease = idle - fullLoad;

			//load
			OCR1A = duty;
			_delay_us(reload);
			ADCSRA |= (1<<ADSC); //start
			while(ADCSRA & (1<<ADSC));
			load = (float)ADC / 50;
			OCR1A = 0;

			//loadDecrease = idle - load;

			//anti-short
			while((fullDecrease > maxDecrease) && !(PINC & fire) && (PINC & plus) && (PINC & minus) && (mode !=2)){
				OCR1A = 0;
				TCCR1A &= ~(1<<COM1A1);
				lcd_clrscr();
				lcd_goto(modeDisplay);
				lcd_puts("Short!!");
				_delay_ms(splash);
			}

			//no atomizer
			while((fullDecrease < minDecrease) && !(PINC & fire) && (PINC & plus) && (PINC & minus) && (mode !=2)){
				OCR1A = 0;
				TCCR1A &= ~(1<<COM1A1);
				lcd_clrscr();
				lcd_goto(modeDisplay);
				lcd_puts("No load");
				_delay_ms(splash);
			}

			//low battery
			while((load < lowVoltage) && !(PINC & fire) && (PINC & plus) && (PINC & minus) && (mode !=2)){
				OCR1A = 0;
				TCCR1A &= ~(1<<COM1A1);
				lcd_clrscr();
				lcd_goto(modeDisplay);
				lcd_puts("Low bat!");
				_delay_ms(splash);
			}

			//pwm warming
			while(fullDecrease < maxDecrease && load > lowVoltage && !(PINC & fire) && (mode == 0) && (PINC & minus) && (PINC & plus)){
				OCR1A = duty;
				_delay_ms(warmingDelay);

				//check idle
				OCR1A = 0;
				_delay_us(reload);
				ADCSRA |= (1<<ADSC); //start
				while(ADCSRA & (1<<ADSC));
				idle = (float)ADC / 50;

				//full load
				OCR1A = 255;
				_delay_us(reload);
				ADCSRA |= (1<<ADSC); //start
				while(ADCSRA & (1<<ADSC));
				fullLoad = (float)ADC / 50;
				OCR1A = 0;

				fullDecrease = idle - fullLoad;

				//load
				OCR1A = duty;
				_delay_us(reload);
				ADCSRA |= (1<<ADSC); //start
				while(ADCSRA & (1<<ADSC));
				load = (float)ADC / 50;
				OCR1A = 0;

				//loadDecrease = idle - load;

				//Display voltage
				lcd_goto(voltageDisplay);
				sprintf(buffer, "%1.1fV", load);
				lcd_puts(buffer);

				//display ampers
				_delay_us(reload);
				fullCurrent = cellIndicator * fullDecrease;
				loadCurrent = fullCurrent * (duty * 0.004);

				lcd_goto(currentDisplay);
				sprintf(buffer, "%1.fA", loadCurrent);
				lcd_puts(buffer);
				lcd_puts("   ");


			}

			//preheat warming
			while(fullDecrease < maxDecrease && load > lowVoltage && !(PINC & fire) && (mode == 1) && (PINC & minus) && (PINC & plus)){
				OCR1A = preheatDuty;
				if (preheatTime == 1) _delay_ms(pht1);
				if (preheatTime == 2) _delay_ms(pht2);
				if (preheatTime == 3) _delay_ms(pht3);
				if (preheatTime == 4) _delay_ms(pht4);
				if (preheatTime == 5) _delay_ms(pht5);

				while(fullDecrease < maxDecrease && load > lowVoltage && !(PINC & fire) && (mode == 1) && (PINC & minus) && (PINC & plus)){
					OCR1A = duty;
					_delay_ms(warmingDelay);

					//check idle
					OCR1A = 0;
					_delay_us(reload);
					ADCSRA |= (1<<ADSC); //start
					while(ADCSRA & (1<<ADSC));
					idle = (float)ADC / 50;

					//full load
					OCR1A = 255;
					_delay_us(reload);
					ADCSRA |= (1<<ADSC); //start
					while(ADCSRA & (1<<ADSC));
					fullLoad = (float)ADC / 50;
					OCR1A = 0;

					fullDecrease = idle - fullLoad;

					//load
					OCR1A = duty;
					_delay_us(reload);
					ADCSRA |= (1<<ADSC); //start
					while(ADCSRA & (1<<ADSC));
					load = (float)ADC / 50;
					OCR1A = 0;

					//loadDecrease = idle - load;

					//Display voltage
					lcd_goto(voltageDisplay);
					sprintf(buffer, "%1.1fV", load);
					lcd_puts(buffer);

					//display ampers
					_delay_us(reload);
					fullCurrent = cellIndicator * fullDecrease;
					loadCurrent = fullCurrent * (duty * 0.004);

					lcd_goto(currentDisplay);
					sprintf(buffer, "%1.fA", loadCurrent);
					lcd_puts(buffer);
					lcd_puts("   ");
				}
			}//preheat warming
			//pwm off
			TCCR1A &= ~(1<<COM1A1);
			lcd_clrscr();
		}//warming
	}//main loop
}//program





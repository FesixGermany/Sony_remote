/*
 * ATtiny85 Sony NEX/Alpha Remote Control
 *
 * David Johnson-Davies - www.technoblogy.com - 12th April 2015
 * ATtiny85 @ 1 MHz (internal oscillator; BOD disabled)
 *
 * CC BY 4.0
 * Licensed under a Creative Commons Attribution 4.0 International license: 
 * http://creativecommons.org/licenses/by/4.0/
 *
 * 
 * Modified by Fesix
 * Converted from Arduino C to standard AVR C, some formatting of code and comments added.
 * Original code had a third button on PB5 for video, change defines as needed for functionality of buttons.
 * 
 * Original source: http://www.technoblogy.com/show?VFT
 *
 * Note: IR LED stays lit until one button is pressed the first time.
 *
 */ 

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// Pin assignments
#define IRout 			PB1   // OC0B
#define ButtonRight		PB4
#define ButtonLeft		PB3
#define LED 			PB0

// Assign button functions, look next segment 'Remote Control' and replace as needed
#define ButtonRightCode ShutterCode
#define ButtonLeftCode	TwoSecsCode

// Remote control
const int Address = 0x1E3A;		// Destination of code
const int ShutterCode = 0x2D;	// Activate shutter without delay
const int TwoSecsCode = 0x37;	// Activate shutter with two second delay
const int VideoCode = 0x48;		// Start video recording

// IR transmitter *****************************************

const int top = 24;    // 1000000/25 = 40kHz
const int match = 18;  // pulses with approx 25% mark/space ratio

// Set up Timer/Counter0 to output PCM on OC0B
void SetupPCM ()
{
	TCCR0A |= (3 << COM0B0) | (3 << WGM00);				// Inverted output on OC0B and Fast PWM
	TCCR0B |= (1 << WGM02) | (1 << CS00);				// Fast PWM and divide by 1
	OCR0A = top;										// 40kHz
	OCR0B = top;										// Keep output low
}

// Set up pin change interrupts on button pins
void SetupPinChange ()
{
	PCMSK |= (1 << ButtonRight) | (1 << ButtonLeft);	// Pin change mask register
	GIMSK |= (1 << PCIE);								// Pin change interrupt enable
}

void Pulse (int carrier, int gap)
{
	int count = carrier;								// First send carrier pulses
	OCR0B = match;										// Generate pulses
	for (char i = 0; i < 2; i++)
	{
		for (int c = 0; c < count; c++)
		{
			do ; while ((TIFR & (1 << TOV0)) == 0);		// Counter0 overflow flag
			TIFR |= (1 << TOV0);
		}
		count = gap;									// Now send no pulses (gap)
		OCR0B = top;									// Output low because OCR0B equals OCR0A
	}
}

void SendSony (unsigned long code)
{
	TCNT0 = 0;											// Start counting from 0
	// Send Start pulse
	Pulse (96, 24);
	// Send 20 bits
	for (int Bit = 0; Bit < 20; Bit++)
	{
		if (code & ((unsigned long) 1<<Bit))			// If bit of code is 1
			Pulse (48, 24);
		else Pulse (24, 24);							// If bit of code is 0
	}
}

// Transmit code and light LED
void Transmit (int address, int command)
{
	unsigned long code = (unsigned long) address << 7 | command;	// Join address and command
	PORTB |= (1 << LED);								// Control LED on begin transmit
	SendSony (code);									// Send code
	Pulse (0, 430);										// Wait 11ms and send again (430*25us)
	SendSony (code);
	Pulse (0, 430);										// Wait 11ms and send again (430*25us)
	SendSony (code);
	PORTB &= ~(1 << LED);								// Control LED off transmit complete
}

// Interrupt **********************************************

// Use PCINT0 just to wake up from sleep
ISR (PCINT0_vect)
{
}

 
 // Main program ******************************************
 
 int main (void)
 {
	 DDRB &= ~(1 << ButtonRight) | ~(1 << ButtonLeft);	// Inputs
	 PORTB |= (1 << ButtonRight) | (1 << ButtonLeft) ;	// Internal pullups for buttons
	 DDRB |= (1 << IRout) | (1 << LED);					// Outputs
	 
	 set_sleep_mode(SLEEP_MODE_PWR_DOWN);				// MCUCR |= (1 << SM1);
	 
	 ADCSRA &= ~(1<<ADEN);								// Disable ADC to save power
	 
	 SetupPCM(); 
	 SetupPinChange();
	 
	 // Main loop
	 while (1)
	 {
		// Go to sleep
		sleep_enable();									// Equals MCUCR |= (1 << SE);
		sei();
		sleep_cpu();
		
		// Come here after pin change interrupt wakes us from sleep
		// Check which button is pressed and transmit
		if (!(PINB & (1 << ButtonRight)))
			Transmit (Address, ShutterCode);
		else if (!(PINB & (1 << ButtonLeft)))
			Transmit (Address, TwoSecsCode);
		// Wait for all buttons to be released
		do ; while (!(PINB & (1 << ButtonRight)) || !(PINB & (1 << ButtonLeft)));
	}
}

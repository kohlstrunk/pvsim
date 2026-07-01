#include <avr/io.h>

// Pin-Belegung und IO-Makros

#define TBIT     	104    					// ideal:104/9600baud 
#define TOUT 		250		 				// xTBIT max.8bit/25ms warten bis RX-Abbruch
#define TABT		40						// +10µs=50 Abtastposition [µs], 70 @ Pullup=30k

// Pin-Belegung
#define UBPin		5						// ADC0
#define PWMBPin		4						// OCR1B
#define I1Pin		3						// ADC3
#define I2Pin    	2						// ADC1
#define PWMAPin		1						// OCR1A
#define RTXPin		0						// RTX

// Pin-Steuerung
//#define LED_T 	PIN_TOGGLE(B,LEDPin)
//#define LED_H	 	PIN_HIGH(B,LEDPin)
//#define LED_L		PIN_LOW(B,LEDPin)
#define PWMA_L		PIN_LOW(B,PWMAPin)
#define PWMB_L		PIN_LOW(B,PWMBPin)
#define PWMA_IN		PIN_INPUT(B,PWMAPin);
#define PWMB_IN		PIN_INPUT(B,PWMBPin);
#define RTX_S		PIN_GET(B,RTXPin)		// Eingang abtasten
#define RTX_PU  	PIN_PULLUP(B,RTXPin)	// pullup: als Eingang, High
#define RTX_PD 		PIN_LOW(B,RTXPin)		// pulldown: Low, Ausgang

#define PIN_HIGH(port, pin)   do { PORT##port |= (1 << pin); DDR##port |= (1 << pin); } while(0)
#define PIN_LOW(port, pin)    do { PORT##port &= ~(1 << pin);DDR##port |= (1 << pin); } while(0)
#define PIN_PULLUP(port, pin) do { DDR##port &= ~(1 << pin); PORT##port |= (1 << pin); } while(0)
#define PIN_INPUT( port, pin) do { DDR##port &= ~(1 << pin); PORT##port &= ~(1 << pin); } while(0)
#define PIN_GET(port, pin)    ((PIN##port & (1 << pin)) != 0)
#define PIN_TOGGLE(port, pin) do { PORT##port ^= (1 << pin); DDR##port |= (1 << pin); } while(0)
//#define PIN_TOGGLE(port, pin) (PIN##port  |= (1 << pin)) atomar, aber nicht portabel 

// ADC
#define MUXAD0	0							// VCCRef PB5 ADC0
#define MUXAD1	(1<<MUX0)					// VCCRef PB2 ADC1
#define MUXAD3	((1<<MUX1)|(1<<MUX0))		// VCCRef PB3 ADC3

// Interrupt-Steuerung
#define TBIF_S		((TIFR>>OCF0B)&1)		// TB-IF abfragen
#define TBIF_D		(TIFR = (1<<OCF0B))		// schreiben einer 1 auf TB-IF löscht es, ohne zu verODERN
#define PCIF_S		((GIFR>>PCIF)&1)		// PCIF abfragen
#define RXI_E		(PCMSK |=  (1<<RTXPin))	// PORTBx=PCINTx=RTX_Pin
#define RXI_D		(PCMSK &= ~(1<<RTXPin))	// PORTBx=PCINTx=RTX_Pin
#define PCI_E		(GIMSK |=  (1<<PCIE))	// enable pin change interrupt - geht auf INT0
//#define PCI_D		(GIMSK &= ~(1<<PCIE))	// disable pin change interrupt - geht auf INT0

#define CMDMAXI		32//64					// max. Kommandolänge

enum {STA,D0,D1,D2,D3,D4,D5,D6,D7,STP,RDY}; // Serial 
enum {TNO,TYES};							// Transmission 
enum {NEIN,JA};

volatile uint8_t txi,uc,rxi,outran,intran;	// RTX: Taktphasen,Sendepuffer,Transmissionen				
volatile long rtsec,rtusec;					// Sekunden und Mikrosekunden (TBIT-Auflösung) seit Reset

volatile uint8_t cmdf; 						// cmd handshake
char cmd[CMDMAXI+1];						// Kommandopuffer (volatile nicht möglich)


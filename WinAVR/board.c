/* 
Hardwareschicht: ISR, Portzugriffe,  

Hardware: Attiny85 mit Fuse: SELFPRGEN, SPIEN, DIS BODLEVEL, 8MHz 64ms


PB0 - RTX/Bootloader
PB1 - PWM
PB2 - LED 
PB3 - Iout-
PB4 - Iout+ 
PB5 - Ubat, Reset

Funktionen: 
- Soft-UART in ISR
- App-Timing
*/

#include <avr/interrupt.h>
#include "board.h"
#include "daten.h"

void rx(void);
void tx(void);
void tim(void);

ISR (PCINT0_vect){								// lauscht auf PinChange Ereignis 
uint8_t b;										// differenz
	b=TCNT0;									// Timerwert speichern für RX 			
	if (!RTX_S){ 								// HL-Flanke
		if((outran==TNO)&(rxi==RDY)){			// Leitung frei? -> RX	
			if (intran==TNO) intran=TYES;		// Leitung ist für Lesen blockiert
			if (b>=(TBIT-TABT)) OCR0B=b-(TBIT-TABT);else OCR0B=b+TABT;// TimerB vorbereiten Achtung Wertebereich
			if(TBIF_S) TBIF_D;rxi=STA;			// ggf. auch Flag löschen - wirklich wichtig
			}// endif 
		}// Ende RX
}												// Ende PC-IRQ

ISR (TIM0_COMPB_vect){							// TBIT/2
//	tx();rx();									// Soft-UART
	rx();tx();									// kritisch ist RX +-30µs
	tim();										// Timing Zeug
}   

void tim(void){
//uint8_t t0=TCNT0;dt_us=TCNT0-t0;
//uint8_t t0=rtt; dtt=rtt-t0;
//long t0=get_ms();tms=get_ms()-t0;	

	if ((rtusec+=TBIT)>999999L) {rtusec-=1000000L;rtsec++; tick ^= 0x01;} 	// µs-Überlauf -> sec++
//	rtt++;
}

void tx(void){
static uint8_t c=0; uint8_t b=0;
	if (outran!=TNO){										// Senden
		switch(txi){
			case STA: b=0;c=uc;					txi++;	break;	// Startbit
			default:if (c&1)b=1;else b=0;c>>=1;	txi++;	break;	// Datenbits
			case STP: b=1;						txi++;	break;	// Stopbit 
			case RDY: b=1;								break;
			}
		if(b==0){RTX_PD;}if(b==1){RTX_PU;}				// physische Ausgabe
		}// end if
}

void rx(void){
static uint8_t cmdi=0,cmdti,c=0;							// Takt,cmdtimeindex,bits,bitnr
uint8_t b=0;												// byte,
	if (intran!=TNO){//49µs									// Empfangen zuerst 
	  	b=RTX_S;
	  	switch (rxi){
			case STA:cmdti=0;	  c=0;		rxi++;	break;	// Startbit
			default: c>>=1;if(b) c|=0x80;	rxi++;	break;	// Datenbits lesen
			case STP:
				if (c=='\n') c='\0';						// nur NL beendet schnell
				cmd[cmdi++]=c;cmd[cmdi]='\0';rxi++;			// Schreiben samt '\0'
				if (c=='\0') 		  {cmdf=JA;	 cmdi=0;intran=TNO;}		// Abschluss sofort
				if ((cmdi+1)>CMDMAXI) {cmdf=NEIN;cmdi=0;intran=TNO;}break;	// Abbruch 												
			case RDY:if (++cmdti>TOUT){cmdf=JA;	 cmdi=0;intran=TNO;}break;	// Abschluss nach 25ms
			}	
	  	}
}


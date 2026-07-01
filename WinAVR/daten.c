/*
Treiberschicht: Datenerfassung

Zugriff von oben: 	über zeitunkritische Funktionen
Zugriff nach unten: volatile Variablen 

PB0 - RTX/Bootloader
PB1 - OCR1A PWM-Takt 
PB2 - Iout2
PB3 - Iout1 
PB4 - OCR1B
PB5 - Ubat, Reset

Funktionen:
- Messung von Batterie- und Ausgangs-Spannung und -Strom
- PV-Simulator-Regler
- EEPROM-Handler
- Laufzeit-Messfunktionen

Leistung     1    10    20    40    60    80    100
TA7        >38V  >41V  37V   30V
TA6	          42V >40V   37V   33V
TA5               N    41V  >39V   37V    35V
TA4                          39V  >38V   >36V  >34V
TA3                                       35V   34V

*/

#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "board.h"
#include "daten.h"
#include "report.h"

//#define RINT		50		// mOhn Batterie-Innenwiderstand 
//#define R_VIRTUAL	200		// mOhm ->  0.2 Ohm PV-Modul Innenwiderstand

// EEPROM
#define EEPReadDWord(addr)     	eeprom_read_dword((uint32_t *)addr)      // Lesemakro
#define EEPWriteDWord(addr,val)	eeprom_write_dword((uint32_t *)addr,val) // Schreibmakro

long para(int no,int act,long val);	// Parameter/EEProm Manager
uint8_t filter(int,uint8_t);		// Notchfilter
uint8_t set_pwm(uint8_t); 			// erzeugt PWM-Wert aus iout 
int  pv_lut(int);					// Strom->Spannung 8bit, inputfest

int psr(int act){					// Laderegler: regulierend AN, permanent AUS
static uint8_t pssta=PSAUS,psc;		// Status und Power-Sollwert
int oact,rval=0;	
	do{
	  switch (oact=act){
		case PSAUS: psc=0; 			break;	// Runterfahren
		case PSAN: if(ubat_mV>ul_mV) psc=pscale; break;// Ubat OK
		case PSUPD: 
			if (aa){
				if      (ubat_mV>uh_mV) act=PSAN;
				else if (ubat_mV<ul_mV) act=PSAUS;
				}
			if (set_pwm(psc)==0) pssta=PSAUS; else pssta=PSAN;				
			if (psc!=0) psc=pscale;
			break;							
		case PSSTA: rval=pssta;	break;
		}//end switch act
	   }//end do
	while (oact!=act);					// iterative möglich	
	return(rval);
}

uint8_t set_pwm(uint8_t psc){ 			// 8µs bis Aufruf Regler
#define SBIT	6						// pwm=dyn<<SBIT
static long mn=64,ua,ia[NCHAN];			// magig number und ubat akku
static int dyn[NCHAN],ul,il;			// PWM, Zähler I und U
static uint16_t pa[NCHAN];				// pwm gemittelt
static uint8_t mx[NCHAN]={MUXAD3,MUXAD1};// Plus Ubat
static volatile uint8_t *pc[NCHAN] = { &OCR1A, &OCR1B };// PWM-Kanäle
static uint8_t psi,ps,tau,pl,ea;
int a,isca,lut,drop,mod,err;
uint8_t i=0,p;

	for (i=0;i<NCHAN;i++){
		loop_until_bit_is_clear(ADCSRA,ADSC);// <52µs warten
		a=ADCW;
		if(a>999)ea++;a-=IZCAL;if(a<0)a=0;	// Kanal A Strom lesen
		ia[i]+=a;							// Iout mit Offset akkumulieren 
		if (il==iflush){iout_mA[i]=ia[i]>>8;ia[i]=0;}// nach 2s Wert speichern ohne div und mul
		if (i<(NCHAN-1)) ADMUX = mx[i+1]; else ADMUX=MUXAD0;// ggf. Ubat starten
		ADCSRA |= (1<<ADSC);				// nächsten Kanal starten
		isca= (a * mn)>>8;					// 29µs Long! (83µs bei a=-30)
		if (fien) isca=filter(isca,i);		//  6µs Notchfilter 
		lut=pv_lut(isca)<<SBIT;				// 18µs mit 8bit Mul und Flash-Lut
		drop = (a>>dropi)<<SBIT; 			// Virtueller Innenwiderstand	
		mod=lut-drop;		
		if      (ps==0) mod=0;				// 0% -> AUS 
		else if (ps==1){					// temporär 1% 
			if (psc>1) 	mod=(psi<<SBIT);	// >1% Hoch 50ms C laden
//			else		mod=dyn[i]-1;		// <1% -> 0% und AUS
			}
		err = mod-dyn[i];
		dyn[i] = dyn[i] + (err>>tau); 			// 4µs RC-Filter	
		if (dyn[i]<0)  				  	p=0;
		else if (dyn[i]>(PWM_MAX<<SBIT))p=PWM_MAX;
		else 						  	p=(dyn[i]>>SBIT);
		*pc[i]=p; pa[i]+=p;
		if(pl==0) {pwm[i]=pa[i]>>8;pa[i]=0;}

if (dup==i){								// nur selten
	dval0=a;
	dval1=psc;
	dval2=ps;
 	dval3=lut>>SBIT;
	dval4=drop>>SBIT;
	dval5=mod>>SBIT;
	dval6=dyn[0]>>SBIT;
	dup=DUPD;								// erledigt
	}// Ende dupt
		}// Ende for
 	loop_until_bit_is_clear(ADCSRA, ADSC); 	// bis Fertig 13Takte =52µs
	ua += ADCW;								// Ubat akkumulieren
	ADMUX = mx[0];	ADCSRA |= (1<<ADSC);	// KanalA starten    		
	if (++ul==uflush){ubat_mV=ua>>5;ua=0;ul=0;}// nach 2s Wert speichern ohne div und mul
	if (il++==iflush) il=0;					// gilt für beide Kanäle
	pl++;									// pwm Mittelungsdauer
	if (ea>30) ef=1;						// pro Periode 1ms Overload
	if (ef) psc=0;							// Fehler -> Power runter
	if ((++psi&0x7F)==0){ 					// alle 25ms
		ea=0;								// Fehlersammler reset
		if (ps!=psc)						// wenn Tracking nötig
			{
			if (psc>ps) ps++; if(psc<ps) ps--; 	// ps nachführen
			if (ps>0) mn=(6400+PCAL)/ps; else mn=6400;	// 38µs für mn Neuberechnung
			}
		}			
	if (ps<15) tau=7; else if (ps<30) tau=6; else if (ps<50) tau=5; else tau=4;
	return ps;								// 250µs
}

// Notch bei 5k/8 = 625Hz oder 1.6ms (Samplingrate/FSIZE)
#define FSIZE 16	// 2^N damit Ringspeicher schnell adressiert
#define DELAY 0		// maximaler "Delay"

uint8_t filter(int in,uint8_t i){		// Wertebereich 0..255, Kanal#
static uint8_t buffer[FSIZE][NCHAN];	// Ringpuffer
static uint8_t wr[NCHAN];				// Schreibadresse
uint8_t rd,x,y;							// Leseadresse, Gefiltertes

	if (in>0xFF) 	x=0xFF; 
	else if (in<0) 	x=0;
	else 			x=in;

//	rd = (wr[i] - DELAY) & (FSIZE - 1);	// untere 4bit maskieren
  	rd = (wr[i] - dsp) & (FSIZE - 1);	// untere 4bit maskieren
    y = (x + buffer[rd][i]) >> 1;		// mitteln mit verschobenem Wert
    buffer[wr[i]][i] = x;				// 
    wr[i] = (wr[i] + 1) & (FSIZE - 1);	// Adresse incrementieren
    return y;
}

// 8bit LUT und 8bit Mul dank Spezialbehandlung am Ende
#define LSIZE	32
#define FBIT	3						// dx=8
#define DXMAX	((1<<FBIT)-1)			// 0x07
#define DYMAX	((1<<8)/(1<<(8-FBIT)))	// 32
const uint8_t lut[LSIZE] PROGMEM = {
	220,220,220,220,219,219,219,218,218,217,216,216,215,214,213,212,
	211,211,210,208,206,204,200,196,191,185,177,167,153,133,102,0};
//const uint8_t lut[LSIZE] = {
//	220,220,220,220,219,219,219,218,218,217,216,216,215,214,213,212,
//	211,211,210,208,206,204,200,196,191,185,177,167,153,133,102,0};

int pv_lut(int in){						
uint8_t x,i,frac,y1,dy,y;				//  frac 0..(1<<FBIT)

	if(in>0xFF) x=0xFF; else if(in<0) x=0; else x=in;
	i = x>>FBIT; frac = x&DXMAX; 			// quasi Festkomma
	y1 = pgm_read_byte(&lut[i]);
//	y1 = lut[i];	
	if (i<(LSIZE-2)){ 						// i<30: dy*frac<256
		dy = y1-pgm_read_byte(&lut[i+1]);
//		dy = y1-lut[i+1]; 
		y  = y1-((dy*frac)>>FBIT);
		}
	else{									// i=30	über Steigung 		
		dy=frac<<4;
		if (dy<y1) y=y1-dy; else y=0;
		}
	return y;	 			// dx=8 spart Div	
}

long get_uout_mV(uint8_t i){	return (pwm[i]*(long)ubat_mV)/255L;}	// nicht gemessen

// Parameter 
long getpar(int no)				{return para(no,PRCL,0L);}		// Parameter abfragen
void setpar(int no,long val)	{para(no,PSET,val);} 			// Parameter setzen und im EEPROM sichern
void inipar(void){ for (int i=0;i<NPAR;i++) para(i,PINI,0L);}	// Parameter aus dem EEPROM laden
long para(int no,int act,long val){ 
		  // Parameter: ID, SC,  UL,    UH,    AA,   UF,   IF
static long ppma[NPAR]={10L,100L,60000L,60000L,1L,10000L,10000};// Max 
static long ppmi[NPAR]={ 0L,  0L,45000L,45000L,0L,  500L,  100};// Min
static long pp[NPAR]  ={ 0L, 50L,51500L,52500L,0L, 1000L, 5000};// Puffer/Defaults
int adr=4*no;								 
long l;
	switch(act){
		case PINI:l=EEPReadDWord(adr);idle_ms(50);						// 50ms zum Elko wieder aufladen
			if (l==0xFFFFFFFF){EEPWriteDWord(adr,pp[no]);idle_ms(50);}	// EEP lesen und ggf. selbst Initialisieren
			else {pp[no]=l;}							break;			// in Lokalen Puffer (ggf.default-Werte überschreiben)	
		case PSET:if( (val<=ppma[no])&&(val>=ppmi[no])) pp[no]=val;		// plausib-Test
			EEPWriteDWord(adr,pp[no]); idle_ms(50);		break;			// manchmal Absturz																		 
		case PRCL:										break; 			// ist beim Lesen schneller
	}
	return pp[no];
}

void init (void){ // für Register und Variablen
	wdt_disable();										// kommt zu spät - deshalb im Startcode 
	_delay_ms(100);										// bis Spannung hochgefahren
	outran=TNO;txi=RDY;uc=0;intran=TNO;rxi=RDY;cmdf=NEIN;// RTX Variablen setzen			
	RTX_PU;												// Pullups  
// Timer0 als Zeitbasis (neu)							// für RTX			
	TCCR0A = (1<<WGM01); 								// Normal Port Operation, Mode2:CTC TOP=OCRA
	TCCR0B = (1<<CS01);									// 8MHz/8 = 1MHz
	OCR0A = TBIT;OCR0B = TBIT/2;						// A:BAUD-RATE B: Abtastmoment
	TIMSK = (1<<OCIE0B);								// B-IRQ für zeitkritisches Zeug, A-IRQ ungenutzt
// Timer1 PWM via OC1A auf 62,5kHz mit PLL, Normalmode
	PLLCSR = (1<<LSM) | (1<<PLLE);  	// PCK = 32 MHz abh. von LSM , PLL-Enabled
	while(!(PLLCSR & (1<< PLOCK))); 	// warten bis PLOCK ==1
	PLLCSR |= (1<<PCKE);				// PLL-Takt für Timer 
	TCCR1 = (1<<CS11)|(1<<COM1A1)|(1<<PWM1A);	// PCK/2, cleared OC1A on compare,no /OC1A
	DDRB|=(1<<PB1);							// PB1 Ausgang
	OCR1A=0;								// 0V Start
	GTCCR = (1<<PWM1B)|(1<<COM1B1);	PWMB_L;	// PB4 als Oszi-Ausgang								 
	OCR1B=92;
// ADC starten											
	ADCSRA  = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS0);	// AN,Takt/32=50µs
	ADCSRA |= (1<<ADSC);						// Strom starten
	ADCSRB= 0;	// kein Binary-Mode, kein Input-Reversal, keine Triggerquellen
//  Pinchange Interrupt 
	RXI_E;							// RTXI 
	PCI_E;							// incl. vorher Flag löschen 
	sei();							// global Interupts erlauben
	inipar(); 						// Werte aus dem EEProm
	id=getpar(ID);
	aa=getpar(AA);
	uh_mV=getpar(UL);
	ul_mV=getpar(UH);
	uflush=getpar(UF);				// U flush
	iflush=getpar(IF);
	pscale=getpar(SC);				// % Power
	ef=0;							// kein Fehler
	dropi=6;fien=1;dsp=8;			// Debug set werte
}

void uputs(const char * s){			// sendet String s auf TXPin
	if (lof) return;				// Listen Only Command Flag
	RXI_D;							// falls RXPin=TXPin
	outran=TYES;					// RX blockieren
	while(*s!='\0'){uc=(*s++);txi=STA;while(txi!=RDY)idle_ms(0);}	// bis Zeichen gesendet ist		
	outran=TNO;						// Sendung beendet
	RXI_E;							// falls RXPin=TXPin
}

void idle_ms (int ms){			// Wartefunktion gefüllt mit Nötigstem
	if (ms>10000) ms=10000;		// sonst Überlauf
	for(int i=0;i<=(ms<<2);i++){// 200µs für action, 0: ein Durchlauf
		psr(PSUPD);				// Schnelle Schleife
		}
}

char* befehl(void){							// Kommando-Handshake
static uint8_t readf=NEIN;
	if (readf==JA) {readf=NEIN;cmdf=NEIN;}  // wurde gelesen - frei für Neues
	if (cmdf==JA){	readf=JA;return cmd;}	// wird es gelesen haben
	else 					 return "";		// noch nichts da
}

void reset(void){				// Watchdog-Reset auslösen
	uputs("8s");
	wdt_enable(WDTO_8S);
	psr(PSAUS);					// regulär Runterfahren ca.5s
	while (psr(PSSTA)!=PSAUS) psr(PSUPD);
	PWMA_IN;					// als Eingang deaktiviert H-Brücke
	PWMB_IN;					// als Eingang deaktiviert H-Brücke
	while(1);	
}

// http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init1")));
void get_mcusr(void){
  mcusr_mirror = MCUSR; // if wants to examine the reset source,
  MCUSR = 0;			// disable the dog
  wdt_disable();
}


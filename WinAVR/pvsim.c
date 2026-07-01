/*
Anwendungsschicht: Zugriff zur Treiberschicht: Funktionen, Zugriff zur Welt: Serielles Interface

- PV-Modul-Simulation für Hoymiles grid-tie inverter
- Ein- und Ausschaltschwelle für Automatik
- UBL gilt für Volllast -> sonst Anhebung mit Rint nötig
- busfähig, wenn indiziert
- broadcast möglich 

ToDo:
- Wieder am Laptop: pvsim aws und aps raus aus Git
- Nulleinspeisungsfähigkeit über ständige Scale-Änderungen
- Tiefentladungsschutz ohne Coloumb-Counting: Abregelung oder Anhebung der Abschaltschwelle 
  bei Leistungsreduzierung  
- Soll-Leistung Einstellung
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "board.h"
#include "daten.h"			
#include "report.h"

void action(int intervall);
void parse(char* cmd);						// wenig Speicher
int parsespezial(char*);					// ergonomischer
int para2long(char *s,const char*,long*);	// Zahleneingabe
int p2l(char *s,uint8_t ni,long* lval);
void get_analog_values(void);

int main (void){
	init();										
	uputs("ps\n");							// Hallo
	while(1){								// ultraschnelle Schleife 90.000/s
		if (cmdf==JA) parse(befehl());		// 8µs schnelles cmdf-Polling
		psr(PSUPD);							// 230µs
		action(1);							// jede Sekunde zu erledigen, dauert auch eine Weile  
		}						
}							

void action(int iv){						// 1x pro Sekunde
static int i=0;static int ov=1;int nv=tick; // nur jeder iv-te Flanke wird durchgereicht
	if (nv!=ov) {i++; ov=nv;}				// 2-Flankentakt
	if (i!=iv) return; else i=0;			// bis Intervall erreicht 1.... 
}

void parse(char * s){
char *cp;
														
	if (strlen(s)==0) 			return;				// garnix
	cp=strstr(s,"PS"); 								// Präfix und ID
	if (cp!=NULL) cp+=2; else 	return;				// von meinem Typ? 
	if (*cp=='_'){ if (id==0) lof=0; else lof=1;}	// _=alle: ID>0 -> listen only command		
	else{ if ((*cp-'0')!=id) 	return;}			// oder 1..9: ich gemeint?
	cp++;											// weiter zum Befehl
	if (parsespezial(cp)==-1){			 			// nix gefunden
	  switch (*cp){									// sonst aufdröseln
		case 'A': switch(*(cp+1)){
					case 'U': psr(PSAUS);break; 	// AUS 
					case 'N': psr(PSAN); break;		// AN  
					}
				  uputs("\n");		break;
		case 'W': drucke(WERTE);	break;			// Strom, Spannung
		case 'B': drucke(BOARD);	break;			// Interna
		case 'P': drucke(PARA);		break;			// Fehlersuche
		case 'D': drucke(DEBUG);	break;			// Parameter
		case 'R': reset();	  		break;			// WDT-Reset nach 8s 
		case '\0':uputs("PS_ W,B,P,D,R\n");	break;	// Help
		}
	  }
	lof=0;											// Schweigen ggf beenden
}

int parsespezial(char *s){
int rval=-1;										// = Druckpattern
long lval;											// Eingabe ohne Dezimalpunkt
	if (p2l(s,ID,&lval))	{rval=PARA;setpar(ID,lval);reset();}	// ID setzen		
	if (p2l(s,SC,&lval))	{rval=PARA;setpar(SC,lval);if ((pscale>1)&(pscale<110)) pscale = lval;}	// % EEPROM-Wert
	if (p2l(s,UL,&lval))	{rval=PARA;setpar(UL,lval);ul_mV=lval;}	// mV Ubatmin	
	if (p2l(s,UH,&lval))	{rval=PARA;setpar(UH,lval);uh_mV=lval;}	// mV Ubatmax	
	if (p2l(s,AA,&lval))	{rval=PARA;setpar(AA,lval);aa=lval; }	// 0/1		
	if (p2l(s,UF,&lval))	{rval=PARA;setpar(UF,lval);uflush=lval; }	// uflush	
	if (p2l(s,IF,&lval))	{rval=PARA;setpar(IF,lval);iflush=lval; }	// uflush	
	if (p2l(s,TS,&lval))	{pscale = lval;}				// % temporär nur im RAM
	if (p2l(s,DS,&lval))	{rval=DEBUG;dsp=(int8_t)lval;}	// debug setpoint	
	if (p2l(s,DI,&lval))	{rval=DEBUG;dropi=(int8_t)lval;}// udrop shift	
	if (p2l(s,FE,&lval))	{rval=DEBUG;fien=(int8_t)lval;}	// filter enable	
	if (rval>=0) {drucke(rval);}								 	

return rval;													 
}

int p2l(char *s,uint8_t ni,long* lval){	// extrahiert aus Parameter-Kommandos den long-Parameter
char p[4];
//	strcpy_P(p,(PGM_P)pgm_read_word(&lab[ni]));
    strcpy_P(p,lab[ni]);idle_ms(2);
	strcat(p,"=");	
	if (strstr(s,p)==s){				// exakter Vergleich
		*lval=strtol(strstr(s,p)+strlen(p),NULL,10);
		idle_ms(2);return 1;
		}		
	else {idle_ms(2);return 0;}
}


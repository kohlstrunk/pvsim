#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "report.h"
#include "daten.h"			
#include "mem-check.h"

//enum {ID,SC,UL,UH,AA,UF,IF,NPAR,TS,		// perm Parameter + 1x volat.
//		UB,I1,I2,U1,U2,						// Variablen
//		ME,DS,AD,IC,MN,LU,DR,MO,DY,FE,DI,PS	// Debugwerte
//		};
const char lab[][3] PROGMEM ={
	"ID","SC","UL","UH","AA","UF","IF"," ","TS",
	"Ub","I1","I2","U1","U2","P1","P2","PW",
	"me","ds","ad","ic","mn","lu","dr","mo","dy","fe","di","ps","ef"
	};

long formen(long wert,long unten,long oben);
void uprintl(const char* label,int prec,long value,char* unit);
void uprint(uint8_t ni,int prec,long v,char* einheit);

void drucke(int was){						// Leerzeichen als Abstandshalter
long u1,u2,i1,i2,p1,p2;
	if (was==WERTE){	
		if (psr(PSSTA)==PSAN) uputs("AN ");	// psr läuft?
		uprint(UB,2,ubat_mV,"V");			 
		u1=get_uout_mV(0);idle_ms(2);u2=get_uout_mV(1);idle_ms(2);
		uprint(U1,1,u1,"V");				
		uprint(U2,1,u2,"V");			
		i1=iout_mA[0];idle_ms(2);i2=iout_mA[1];idle_ms(2);
		uprint(I1,2,i1,"A");		
		uprint(I2,2,i2,"A");		
		p1=(i1*u1)/1000L;idle_ms(2);p2=(i2*u2)/1000L;idle_ms(2);
		uprint(P1,0,p1,"W");		 
		uprint(P2,0,p2,"W");		 
		uprint(PW,0,p1+p2,"W");		 
		uputs("\n");
		}
	if (was==BOARD){	
		uputs("\n");
		}
	if (was==PARA){
		uprint(ID,0,id*1000L,"");		// Sprecher oder nur Hörer			
		uprint(SC,0,pscale*1000L,"%");	// Skalierung der Leistung		
		uprint(UL,1,ul_mV,"V"); 		// Tiefentladungsschutz		
		uprint(UH,1,uh_mV,"V"); 		// AutoAN-Schwelle		
		uprint(AA,0,aa*1000L,""); 		// Autostart		
		uprint(UF,0,uflush*1000L,""); 	// ubat-Kalib		
		uprint(IF,0,iflush*1000L,""); 	// iout-Kalib		
		uputs("\n");	
		}
	if (was==DEBUG){	
		uprint(ME,0,get_mem_unused()*1000L,""); 	// RAM-Reserve bestimmen		
		dup=DUP0; idle_ms(0);					// debug Werte aktualisieren
		uprint(SC,0,pscale*1000L,"");		// SC		
		uprint(PS,0,dval1*1000L,"");		// psc weitergereicht
		uprint(PS,0,dval2*1000L,"");		// ps langsam
		uprint(DS,0,dsp*1000L,""); 			// Debug Setpoint		
		uprint(FE,0,fien*1000L,"");			// filter enable
		uprint(DI,0,dropi*1000L,"");		// Udrop Berechnung
		uprint(EF,0,ef*1000L,"");			// Error Flag
		uputs("\n");
		uprint(AD,0,dval0*1000L,"");		// 
		uprint(LU,0,dval3*1000L,"");		// 
		uprint(DR,0,dval4*1000L,"");		// 
		uprint(MO,0,dval5*1000L,"");		// 
		uprint(DY,0,dval6*1000L,"");		// 
		uputs("\n");
		dup=DUP1; idle_ms(0);					// debug Werte aktualisieren
		uprint(AD,0,dval0*1000L,"");		// 
		uprint(LU,0,dval3*1000L,"");		// 
		uprint(DR,0,dval4*1000L,"");		// 
		uprint(MO,0,dval5*1000L,"");		// 
		uprint(DY,0,dval6*1000L,"");		// 
		uputs("\n");
		}
	idle_ms(50);
}
void uprint(uint8_t ni,int prec,long v,char* einheit){// 48byte weniger Stack
uint8_t i,j=0,f,kpos=4;	// i:tempindex, j:outindex, flag und prec=4 für mK-Genauigkeit
char s[16];
char buf[3];

    strcpy_P(buf,lab[ni]);
	uputs(buf);
	uputs(" ");
//	uputs( strcpy_P(buf,(PGM_P)pgm_read_word(&lab[ni])) );// 2 Zeichen Identifier aus Flash drucken 

	if(v<0) { s[j++]='-'; v=-v;} 	// Vorzeichen
	for(i=10;i>=1;i--){				// Modulo rechnen, dann den ASCII-Code von '0' addieren
		s[i]=(v%10)+'0';v/=10;
		idle_ms(2);
		}
	for(i=1,f=0;i<(12-kpos);i++){  				
		if (f==1 || s[i]!='0') {	// einfach Hochzählen bei bei führenden Nullen
			s[j++]=s[i];			// relvante Vorkommastellen
			f=1;					// ... erreicht
			}
		idle_ms(2);
		}
	if (prec>0){
		if (f==0) s[j++]='0'; 					// eine führende Null
		s[j++]='.';                				// Komma ausgeben
		for(;i<(12-kpos+prec);i++){				// Nachkommastellen ausgeben
			s[j++]=s[i];	
			idle_ms(2);
			}
		}
	else{										// 0 Watt - Bug
		if (f==0) s[j++]='0'; 					// eine führende Null
		}
	s[j]='\0';                  				// String Terminator
	uputs(s);if(strlen(s)!=0) uputs(" ");
	uputs(einheit); if(strlen(einheit)!=0) uputs(" ");
}

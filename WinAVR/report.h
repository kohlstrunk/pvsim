// Globale Variablen, Definitionen und Funktionen
enum {WERTE,BOARD,PARA,DEBUG,RESET,HELP};	// drucke
enum {ID,SC,UL,UH,AA,UF,IF,NPAR,TS,			// Parameter
	  UB,I1,I2,U1,U2,P1,P2,PW,				// phys Messwerte
	  ME,DS,AD,IC,MN,LU,DR,MO,DY,FE,DI,PS,EF// Debugwerte
	  };

void drucke(int);					 		

extern const char *plab[NPAR];
extern const char lab[][3] PROGMEM;

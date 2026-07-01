// Allgemein
#define NCHAN	2
#define PWM_MAX	220				// 230 liefert Aussetzer an H-Brücke, Timer1
#define PCAL	0//-600			// Ziel: 100%=400W 10.3A 38V
#define IZCAL	30				// Strom-Nullpunkt
// 
enum {PSAUS,PSH,PSR,PSAN,PSUPD,PSSTA};	// PV-Simulator-Regler-Status 
enum {PRCL,PSET,PINI};			// para()
enum {CRESET,CREAD};			// Kommando-Handler
enum {DUP0,DUP1,DUPD};			// Debug update Kanal0/Kanal1/Done
// Funktionen
long getpar(int no);			// Parameter abfragen
void setpar(int no,long val);	// Parameter setzen und im EEPROM sichern
void inipar(void);				// Parameter aus dem EEPROM laden
void init (void);				// Register und Variablen
void reset(void);				// Watchdog-Reset auslösen
void uputs(const char *);		// String senden - nicht verändern
char* befehl(void);				// Handshake mit UART-Lesefunktion
long get_uout_mV(uint8_t);		// aus pwm
int  psr(int);					// Laderegler
void idle_ms(int);				// Aktiv-Wartefunktion

// globale Variablen
uint16_t ubat_mV,ul_mV,uh_mV;	// Ubat 20mV genau
int iout_mA[NCHAN];				// s Mittelwert
uint8_t pwm[NCHAN];				// gemitteltes pwm-> uout
int uflush,iflush;				// schneller RAM-Zugriff			
volatile short tick;			// 0 oder 1,  App-Zyklus
uint8_t lof,pscale,dropi,ef,aa,id;	// listen only command flag

//Debug
int dval0,dval1,dval2,dval3,dval4,dval5,dval6;
int8_t dup,dsp,fien;			// debug set variablen


#include <stdio.h>
#include <stdlib.h>

#pragma config OSC=HS,WDT=OFF,LVP=OFF,CCP2MUX=OFF  // HS Oszillator, Watchdog Timer disabled, Low Voltage Programming

// Define für LCD des neuen, grünen Demo-Boards:
//#define NEUE_PLATINE  // Achtung: define vor include! Bei altem braunem Demo-Board auskommentieren!

#include "p18f452.h"
#include "lcd.h"


unsigned char cm_text[]="cm";
unsigned char no_obj_text[]="--              ";
unsigned char zeit_text[]="Zeit: ";
unsigned char colon_text[]=":";
unsigned char abstand_text[]="Abstand: ";

volatile unsigned int abstand=0; 			// Abstand des Objekts
unsigned char vorzaehler=0;
unsigned char Stunde=23;
unsigned char Minute=59;
unsigned char Sekunde=55;

volatile unsigned int timestamp=0;
enum bool{false,true};
enum bool dist_of=false;


void high_prior_InterruptHandler (void);
void low_prior_InterruptHandler (void);


#pragma code high_prior_InterruptVector = 0x08
void high_prior_InterruptVector(void)
{
	_asm
			goto high_prior_InterruptHandler
	_endasm
}


#pragma code low_prior_InterruptVector = 0x18
void low_prior_InterruptVector(void)
{
	_asm
			goto low_prior_InterruptHandler
	_endasm
}


#pragma code init = 0x30
void init (void)
{
#ifndef Simulator	// LCD-Initialisierung mit Portzuweisung RA<3:1> und RD<3:0>
	lcd_init();		// Alle LCD-Funktionen werden für die Simulation herausgenommen,
	lcd_clear();	// da man sonst hier stecken bleibt.
#endif

	PORTB = 0;
	TRISBbits.TRISB1 = 0;

	T1CON = 0x88;
	T3CON = 0x90;
	
	CCP2CON = 0x05;
	
	RCONbits.IPEN = 1;
	PIE2 = 0x03;
	IPR2 = 0x01;
	PIR2 = 0;
	
	T1CONbits.TMR1ON = 1;
	T3CONbits.TMR3ON = 1;
	
	INTCON = 0xC0;
}


// hochpriorisierte ISR:
// Messung der Dauer des Echoimpulses (an RB3/CCP2) durch Timer 1.
// Steigende Flanke an RB3/CCP2: Capture-Wert speichern; Fallende Flanke: Abstand berechnen
#pragma code
#pragma interrupt high_prior_InterruptHandler
void high_prior_InterruptHandler(void)
{
	if(PIR2bits.CCP2IF == 1)
	{
		if(CCP2CONbits.CCP2M0 == 1)		// Steigende Flanke abfragen und Timer 1 Wert speichern
		{
			timestamp = CCPR2;		
			
			PIR1bits.TMR1IF = 0;
			
			CCP2CONbits.CCP2M0 = 0;
		}
		else 
		{
			dist_of = false;
			
			if(PIR1bits.TMR1IF == 1)
			{
				dist_of = true;
			}
			else
			{
				abstand = (CCPR2 - timestamp) / 58;
			}
			
			CCP2CONbits.CCP2M0 = 1;
		}
		
		PIR2bits.CCP2IF = 0;
	}
	
	return;
}


// niedrigpriorisierte ISR:
// 100ms-Intervalle von Timer 3 verwenden, um die Abstandsmessung darzustellen.
// Die Intervalle dienen zugleich als Zeitbasis für die Uhr.
#pragma code
#pragma interruptlow low_prior_InterruptHandler
void low_prior_InterruptHandler(void)
{
	TMR3H = 0x79;	//TMR3 auf 31.070 zurücksetzen
	TMR3L = 0x5E;
	
	// Hardware: Ausgabe an LCD
	ADCON1bits.PCFG3=1;	// Port Konfiguration
	ADCON1bits.PCFG2=1;	// RA3:RA1 wieder digital I/O für LCD, nur AN0-Eingang analog
	ADCON1bits.PCFG1=1;
	
	if(dist_of == true)
	{
		lcd_gotoxy(1,1);
		lcd_printf(abstand_text);
		lcd_printf (no_obj_text);	
	}
	else
	{
		lcd_gotoxy(1,1);
		lcd_printf(abstand_text);
		lcd_int(abstand);
		lcd_printf (cm_text);	
	}
	
	vorzaehler += 1;
	
	if(vorzaehler > 9)
	{
		vorzaehler = 0;
		Sekunde += 1;
		
		if(Sekunde > 59)
		{
			Sekunde = 0;
			Minute += 1;
			
			if(Minute > 59)
			{
				Minute = 0;
				Stunde += 1;
				
				if(Stunde > 23)
				{
					Stunde = 0;
				}
			}
		}
		
		lcd_gotoxy(2,1);
		lcd_printf(zeit_text);
		lcd_int(Stunde);
		lcd_printf(colon_text);
		lcd_int(Minute);
		lcd_printf(colon_text);
		lcd_int(Sekunde);	
	}
	
	ADCON1bits.PCFG3=0;	// Port Konfiguration
	ADCON1bits.PCFG2=0;	// AAAA AAAA
	ADCON1bits.PCFG1=0;
	
	TMR1H = 0; //TMR1 zurücksetzen
	TMR1L = 0;
	
	PORTBbits.RB1 = 1;
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	PORTBbits.RB1 = 0;
	
	PIR2bits.TMR3IF = 0;
}


void main() {
	init();
	while(1);
	
}

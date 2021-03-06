// Hochschule Mannheim / Institut f?r Mikrocomputertechnik und Embedded Systems
//
// Versuch: PIC2  DA-Wandler durch PWM    Dateiname: PIC2_PWM.c
//
// Eine am Analogeingang RA0/AN0 vorgegebene Spannung wird digitalisiert,
// der Wert AnalogIn=xxxx am LCD angezeigt
// und ?ber eine Pulsweitenmodulation am Ausgang RC2/CCP1 ausgegeben.
// Das dort angeschlossene RC-Glied macht daraus wieder eine Analogspannung,
// die am Eingang RE2/AN7 eingelesen und als Istwert AnalogOut=yyyy angezeigt wird.
//
// 08.12.2011 (Poh) Kommentare f?r LCD (Prototypen in lcd.h)
// 24.05.2011 (Poh) Configuration Bit Settings, Anpassungen f?r NEUE_PLATINE, Includes im Projektverzeichnis
// 07.06.2020 (Poh) #ifdef Umschaltung zwischen Simulation und Hardware mit LCD
// 30.06.2020 (Poh) umstrukturiert / AD-Wandlungsstart 1x
//
// Name/Gruppe: Jacob Sander, Alexander Strasser
//

#pragma config OSC=HS,WDT=OFF,LVP=OFF  // HS Oszillator, Watchdog Timer disabled, Low Voltage Programming

#define Simulator        // zum Gebrauch mit Hardware auskommentieren
// Define f?r LCD des neuen, gr?nen Demo-Boards:
// #define NEUE_PLATINE  // Achtung: define vor include! Bei altem braunem Demo-Board auskommentieren!
#include "p18f452.h"
#include "lcd.h"         // Enth?lt alle Prototypen f?r das LCD


void init();

unsigned int x=0;  // Analogwert AN0 (Vorgabe durch Poti)
unsigned int y=0;  // Analogwert AN7 (Istwert des PWM-Mittelwerts am RC-Glied)
unsigned char Analog_text1[20]="AnalogIn =";  // Analogwert AN0 (Poti)  16 Zeichen pro Zeile
unsigned char Analog_text2[20]="AnalogOut=";  // Analogwert AN7 (Istwert des PWM-Mittelwerts am RC-Glied)
unsigned char leer[]          ="                ";
unsigned char temp = 0x00;

void init()
{
	// IO Ports, sind standartm??ig auf Eingang
	TRISCbits.TRISC2 = 0;	// RC2 als Ausgang f?r PWM
	
	TRISBbits.TRISB0 = 0;	// Ausg?nge f?r LED 1 und 3
	TRISBbits.TRISB2 = 0;
	

#ifndef Simulator	// LCD-Initialisierung mit Portzuweisung RA<3:1> und RD<3:0>
	lcd_init();		// Alle LCD-Funktionen werden f?r die Simulation herausgenommen,
	lcd_clear();	// da man sonst hier stecken bleibt.
#endif

	CCP1CON = 0x0F;	// CCP1 als PWM Modul konfigurieren, 

	T2CON = 0x04;	// Timer 2 Einstellungen
	PORTBbits.RB0 = 1;
	
	PR2 = 0xFF;

	ADCON0 = 0x81;	// A/D-Umsetzer Einstellungen
	ADCON1 = 0x00; 

}

void main()
{
	init();
	while(1)
	{
		if(PORTAbits.RA4 == 0)
		{
			while(PORTAbits.RA4 == 0);
			
			if(T2CON == 0x04))	// Timer 2 auf 16 Prescale und LED setzten
			{
				T2CON = 0x06;
				PORTBbits.RB0 = 0;
				PORTBbits.RB2 = 1;
			}
			else(T2CON == 0x06)	// Timer 2 auf 1 Prescale und LED setzten
			{
				T2CON = 0x04;
				PORTBbits.RB2 = 0;
				PORTBbits.RB0 = 1;
			}
		}
		
		ADCON0bits.GO = 1;	// A/D-Umsetzung durchf?hren
		while(ADCON0bits.DONE == 1);


		// A/D-Converter: Werteverarbeitung Kanal 0 oder 7
		//Analogkanal 0 wurde eingelesen (Poti Sollwert)
		if(!ADCON0bits.CHS2 && !ADCON0bits.CHS1 && !ADCON0bits.CHS0)
		{				
				x = ADRES;			// Berechnung von x
				x = x >> 6;
				
				CCPR1L = ADRESH;	// Duty Cycle f?r PWM  einstellen
				temp = ADRESL >> 2;
				CCP1CON = 0x0F;
				CCP1CON = CCP1CON | temp;

				ADCON0 = 0xB9;		// Channel 7 ausw?hlen
		}
		//Analogkanal 7 wurde eingelesen (RC-Ausgang Istwert)
		else if(ADCON0bits.CHS2 && ADCON0bits.CHS1 && ADCON0bits.CHS0)
		{				
				y = ADRES;			// Berechnung von y
				y = y >> 6;

				ADCON0 = 0x81;		// Channel 0 ausw?hlen
		}

#ifndef Simulator
		// Hardware: Ausgabe an LCD
		ADCON1bits.PCFG3=1;	// Port Konfiguration
		ADCON1bits.PCFG2=1;	// RA3:RA1 wieder digital I/O f?r LCD, nur AN0-Eingang analog
		ADCON1bits.PCFG1=1;

		lcd_gotoxy(1,1);
		lcd_printf(Analog_text1);
		lcd_int(x);
		lcd_printf (leer);
		lcd_gotoxy(2,1);
		lcd_printf(Analog_text2);
		lcd_int(y);
		lcd_printf(leer);

		ADCON1bits.PCFG3=0;	// Port Konfiguration
		ADCON1bits.PCFG2=0;	// AAAA AAAA
		ADCON1bits.PCFG1=0;

#else	// Simulation: PWM-Periode abwarten + Haltepunkt bei bestimmtem Analogwert erm?glichen
		while(!PIR1bits.TMR2IF);	// Eine Periode der PWM (Timer 2) abwarten
		PIR1bits.TMR2IF=0;			// bis der n?chste Analogwert gelesen wird.

		if(y==0x1FA){		// Letzter aus "Stimulus ADRESL pic2pwm.txt" zu lesender AD-Wert
			Nop();		// <-- hier einen Haltepunkt zum Anhalten nach einem Datenzyklus setzten!
		}					// Ohne Haltepunkt wird die Injaktionsdatei zyklisch wiederholt gelesen.
#endif

	}
}

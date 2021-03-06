; Dateiname:
; Beschreibung:
;
;
; Entwickler:
; Version: 1.0    Datum:

	list p=18f452		; Select Processor for Assembler
#include <p18f452.inc>	; Registers / ASM Include File in Search Path  C:\Program Files (x86)\Microchip\MPASM Suite

	config OSC=HS,WDT=OFF,LVP=OFF ;Configuration Bits - defined in include file
	; HS Oszillator, Watchdog Timer disabled, Low Voltage Programming

BANK0 EQU 0x000
BANK1 EQU 0x100
BANK2 EQU 0x200
BANK3 EQU 0x300
BANK4 EQU 0x400
BANK5 EQU 0x500

;//***** Variables *****
;//Bank0
Timer EQU 0x20
;//Bank1
;//Bank2
;//Bank4
;//Bank5

;//***** Vector Table *****
	ORG 0x00
ResetVect
	GOTO Init

	ORG 0x08
IntVectHigh
;//No Interrupts Enabled

	ORG 0x18
IntVectLow
;//No Interrupts Enabled

;//***** Main Program *****
	ORG 0x30
;//Initialisation Code
Init
;//Ports
	;//Anaolog Input (unnötig)
	;//BSF TRISA,RA0
	;//Taster Input (unnötig)
	;//BSF TRISA,RA4
	
	;//LEDs Output
	MOVLW 0xF0
	MOVWF TRISB

	;//Lautsprecher Output
	BCF TRISC,RC2

	;//AD init
	;//CLRF ADCON1
	MOVLW 0x81
	MOVWF ADCON0
	
	

;//Main Application Code
MainLoop
	
	BTFSC PORTA,RA4
	BRA MainLoop ;//Wait for Taster gegen 0

;//start AD
	BSF ADCON0,GO
ADLoop
	BTFSC ADCON0,GO
	BRA ADLoop

;//Bits umsortieren
	;//MOVF ADRESH,0
	SWAPF ADRESH,0

;//Output
	;//LEDs
	MOVWF PORTB
	
	;//Lautsprecher
	BTG PORTC,RC2

;//Verzögerung
	CALL Delay
	
	
	
	BRA MainLoop ;// Main Loop
	

Delay
TIMELoop
	DECFSZ Timer,1
	BRA TIMELoop
	Return

	END
	
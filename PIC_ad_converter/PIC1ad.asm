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

;//Main Application Code
MainLoop

	BRA MainLoop ;//Do.. Forever
	END

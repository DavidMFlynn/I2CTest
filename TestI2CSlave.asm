;====================================================================================================
;
;   Filename:	TestI2CSlave.asm
;   Date:	4/23/2015
;   File Version:	1.0d1
;
;    Author:	David M. Flynn
;    Company:	Oxford V.U.E., Inc.
;    E-Mail:	dflynn@oxfordvue.com
;    Web Site:	http://www.oxfordvue.com/
;
;====================================================================================================
;    I2C Example for PIC12F1822 sample projects
;
;    History:
;
; 1.0d1  4/23/2015	First code
;
;====================================================================================================
; Options
;
;====================================================================================================
;====================================================================================================
; What happens next:
;
;   The system LED blinks once per second
;
;
;  Communications:
;     Send state of SW_1 to remote.
;     Receive remote SW_1 and light LED_1
;
;
;====================================================================================================
;
;  Pin 1 VDD (+5V)		+5V
;  Pin 2 RA5		LED_1 (Active high output)
;  Pin 3 RA4		System LED Active Low/Center switch Active Low
;  Pin 4 RA3/MCLR*/Vpp (Input only)	SW_1  (Active Low input)
;  Pin 5 RA2		SDA
;  Pin 6 RA1/ICSPCLK		SCL
;  Pin 7 RA0/ICSPDAT/AN0
;  Pin 8 VSS (Ground)		Ground
;
;====================================================================================================
;
;
	list	p=12f1822,r=hex,w=0	; list directive to define processor
;
	nolist
	include	p12f1822.inc	; processor specific variable definitions
	list
;
	__CONFIG _CONFIG1,_FOSC_INTOSC & _WDTE_OFF & _MCLRE_OFF & _IESO_OFF
;
;
;
; INTOSC oscillator: I/O function on CLKIN pin
; WDT disabled
; PWRT disabled
; MCLR/VPP pin function is digital input
; Program memory code protection is disabled
; Data memory code protection is disabled
; Brown-out Reset enabled
; CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin
; Internal/External Switchover mode is disabled
; Fail-Safe Clock Monitor is enabled
;
	__CONFIG _CONFIG2,_WRT_OFF & _PLLEN_OFF & _LVP_OFF
;
; Write protection off
; 4x PLL disabled
; Stack Overflow or Underflow will cause a Reset
; Brown-out Reset Voltage (Vbor), low trip point selected.
; Low-voltage programming Disabled ( allow MCLR to be digital in )
;  *** must set apply Vdd before Vpp in ICD 3 settings ***
;
; '__CONFIG' directive is used to embed configuration data within .asm file.
; The lables following the directive are located in the respective .inc file.
; See respective data sheet for additional information on configuration word.
;
	constant	oldCode=0
	constant	useRS232=0
;
#Define	_C	STATUS,C
#Define	_Z	STATUS,Z
;
;
;
;====================================================================================================
	nolist
	include	F1822_Macros.inc
	list
;
;    Port A bits
PortADDRBits	EQU	b'11001111'
#Define	SystemLED	LATA,4	;Output: 0=LED ON, Input: 0 = Switch Pressed
#Define	SysLEDTrisBit	TRISA,4
#Define	SW1BtnBit	PORTA,3
#Define	LED1Bit	LATA,5
;
PortAValue	EQU	b'00000000'
;
;====================================================================================================
;====================================================================================================
;
;Constants
All_In	EQU	0xFF
All_Out	EQU	0x00
;
TMR0Val	EQU	0xB2	;0xB2=100Hz, 0.000128S/Count
LEDTIME	EQU	d'100'	;1.00 seconds
LEDErrorTime	EQU	d'10'
kWDTime	EQU	d'200'	;2 seconds
;
T1CON_Val	EQU	b'00000001'	;PreScale=1,Fosc/4,Timer ON
;
;
DebounceTime	EQU	d'10'
;
;================================================================================================
;***** VARIABLE DEFINITIONS
; there are 128 bytes of ram, Bank0 0x20..0x7F, Bank1 0xA0..0xBF
; there are 256 bytes of EEPROM starting at 0x00 the EEPROM is not mapped into memory but
;  accessed through the EEADR and EEDATA registers
;================================================================================================
;  Bank0 Ram 020h-06Fh 80 Bytes
;
	cblock	0x20
;
	ISR_Temp		;scratch mem
	LED_Time	
	lastc		;part of tickcount timmer
	tickcount		;Timer tick count
;
;
	EEAddrTemp		;EEProm address to read or write
	EEDataTemp		;Data to be writen to EEProm
;
;
	Timer1Lo		;1st 16 bit timer
	Timer1Hi		; one second RX timeiout
;
	Timer2Lo		;2nd 16 bit timer
	Timer2Hi		;
;
	Timer3Lo		;3rd 16 bit timer
	Timer3Hi		;GP wait timer
;
	Timer4Lo		;4th 16 bit timer
	Timer4Hi		; debounce timer
;
	SysFlags
;
	endc
;
;
I2C_ADDRESS	EQU	0x30	; Slave address
RX_ELEMENTS	EQU	.8	; number of allowable array elements
I2C_TX_Init_Val	EQU	0xAB	; value to load into transmit array to send to master
I2C_RX_Init_Val	EQU	0xAA	; value to load into received data array
;
;================================================================================================
;  Bank1 Ram 0A0h-0BFh 32 Bytes
;
;
I2C_Buffers	udata	0xA0
I2C_ARRAY_TX	RES	RX_ELEMENTS	; array to transmit to master
I2C_ARRAY_RX 	RES	RX_ELEMENTS 	; array to receive from master
;
;=======================================================================================================
;  Common Ram 70-7F same for all banks
;      except for ISR_W_Temp these are used for paramiter passing and temp vars
;=======================================================================================================
;
	cblock	0x70
	Param70
	Param71
	Flags
;Globals from I2C_Master.inc
	INDEX_I2C
	GFlags
;
	Param75
	Param76
	Param77
	Param78
	Param79
	Param7A
	Param7B
	Param7C
	Param7D
	Param7E
	Param7F
	endc
;
; Flags bits
;
#Define	I2C_TXLocked	GFlags,0	; Set/cleared by ISR, data is being sent
#Define	I2C_RXLocked	GFlags,1	; Set/cleared by ISR, data is being received
#Define	I2C_NewRXData	GFlags,2	; Set by ISR, The new data is here!
;
;=========================================================================================
;Conditionals
HasISR	EQU	0x80	;used to enable interupts 0x80=true 0x00=false
;
;=========================================================================================
;==============================================================================================
; ID Locations
	__idlocs	0x10d2
;
;==============================================================================================
;============================================================================================
;
;
	ORG	0x000	; processor reset vector
	CLRF	STATUS
	CLRF	PCLATH
  	goto	start	; go to beginning of program
;
;===============================================================================================
; Interupt Service Routine
;
; we loop through the interupt service routing every 0.008192 seconds
;
;
	ORG	0x004	; interrupt vector location
	CLRF	BSR	; bank0
;
;
	btfss	INTCON,T0IF
	goto	IRQ_2
;
	movlw	TMR0Val	;256x39+16 cycles (10,000uS)
	addwf	TMR0,F	; reload TMR0 with -40
	bcf	INTCON,T0IF	; reset interupt flag bit
;
;Decrement timers until they are zero
; 
	call	DecTimer1	;if timer 1 is not zero decrement
	call	DecTimer2
	call	DecTimer3
	call	DecTimer4
;
;-----------------------------------------------------------------
; blink LEDs
	BankSel	TRISA
	BSF	SysLEDTrisBit	;LED off
;
;
	movlb	0
	MOVF	tickcount,F
	SKPNZ
	GOTO	SystemBlinkDone
	DECF	tickcount,F
	SKPNZ
	CALL	ToggleSysLED
	GOTO	SystemBlink_end
;
SystemBlinkDone	MOVF	LED_Time,F
	SKPZ
	CAll	ToggleSysLED
;
SystemBlink_end	
;
;-----------------------------------------------------------------
;
IRQ_2:
;==================================================================================
;-----------------------------------------------------------------------------------------
; I2C Com
IRQ_4	MOVLB	0x00
	btfss 	PIR1,SSP1IF 	; Is this a SSP interrupt?
	goto 	IRQ_4_End 	; if not, bus collision int occurred
	banksel	SSP1STAT						
	btfsc	SSP1STAT,R_NOT_W	; is it a master read:
	goto	I2C_READ	; if so go here
	goto	I2C_WRITE	; if not, go here
I2C_READ_Return:
I2C_WRITE_Return	movlb	0x00
	bcf 	PIR1,SSP1IF	; clear the SSP interrupt flag
IRQ_4_End
;-----------------------------------------------------------------------------------------
; I2C Bus Collision
IRQ_5	MOVLB	0x00
	btfss	PIR2,BCL1IF
	goto	IRQ_5_End
	banksel	SSPBUF						
	clrf	SSPBUF	; clear the SSP buffer
	movlb	0x00	;banksel PIR2
	bcf	PIR2,BCL1IF	; clear the SSP interrupt flag	
	banksel	SSPCON1
	bsf	SSPCON1,CKP	; release clock stretch
	movlb	0x00
;
IRQ_5_End:
;
;--------------------------------------------------------------------
;
	retfie		; return from interrupt
;
;
;==============================================================================================
;==============================================================================================
; This routine runs every 1/2 second. Unless an error is active then every 1/10th
;
ToggleSysLED	MOVF	LED_Time,W
	MOVWF	tickcount
	BankSel	TRISA
	BCF	SysLEDTrisBit	;LED on
	MOVLB	0
	RETURN
;
;==============================================================================================
;==============================================================================================
;
	include	I2C_SLAVE.inc
;
;==============================================================================================
;**********************************************************************************************
;==============================================================================================
;
start	MOVLB	0x01	; select bank 1
	bsf	OPTION_REG,NOT_WPUEN	; disable pullups on port B
	bcf	OPTION_REG,TMR0CS	; TMR0 clock Fosc/4
	bcf	OPTION_REG,PSA	; prescaler assigned to TMR0
	bsf	OPTION_REG,PS0	;111 8mhz/4/256=7812.5hz=128uS/Ct=0.032768S/ISR
	bsf	OPTION_REG,PS1	;101 8mhz/4/64=31250hz=32uS/Ct=0.008192S/ISR
	bsf	OPTION_REG,PS2
;
	MOVLB	0x01	; bank 1
	MOVLW	b'01110000'	;8MHz
	MOVWF	OSCCON
	movlw	b'00010111'	; WDT prescaler 1:65536 period is 2 sec (RESET value)
	movwf	WDTCON 	
;	
;
; setup timer 1 for 0.5uS/count
;
	BANKSEL	T1CON	; bank 0
	MOVLW	T1CON_Val
	MOVWF	T1CON
	bcf	T1GCON,TMR1GE
;
; clear memory to zero
	CALL	ClearRam
;
;
; setup data ports
	BANKSEL	PORTA
	MOVLW	PortAValue
	MOVWF	PORTA	;Init PORTA
	BANKSEL	ANSELA
	MOVLW	0x00
	MOVWF	ANSELA	;digital I/O
	BANKSEL	TRISA
	MOVLW	PortADDRBits
	MOVWF	TRISA
;
	MOVLB	0	;bank 0
	CLRWDT
;
	MOVLW	LEDTIME
	MOVWF	LED_Time
;
	MOVLW	0x01
	MOVWF	tickcount
;
	CLRWDT
;
;
	bsf	INTCON,PEIE	; enable periferal interupts
	bsf	INTCON,T0IE	; enable TMR0 interupt
	bsf	INTCON,GIE	; enable interupts
;
	MOVLB	0x00	;bank 0
	call	Init_I2C	;setup I2C
;
;=========================================================================================
;=========================================================================================
;  Main Loop
;
MainLoop	CLRWDT
	CALL	I2C_DataInturp
;
	CALL	I2C_DataSender
;
	goto	MainLoop
;
;==============================================================
;
I2C_DataInturp	BTFSC	I2C_RXLocked
	RETURN
	BTFSS	I2C_NewRXData	;Data is new?
	RETURN		; No
	BCF	I2C_NewRXData
	CLRF	Param79	;offset
	LOADFSR0	I2C_ARRAY_RX,Param79
	MOVF	INDF0,W
	MOVWF	Param78
;
	MOVLB	0x02
	BTFSS	Param78,0
	BCF	LED1Bit
	BTFSC	Param78,0
	BSF	LED1Bit
	MOVLB	0x00
	RETURN
;
;==============================================================
;
I2C_DataSender	BTFSC	I2C_TXLocked
	RETURN
;
	CLRF	Param78
	BTFSS	SW1BtnBit
	BSF	Param78,0
;
	CLRF	Param79	;offset
	LOADFSR0	I2C_ARRAY_TX,Param79
	MOVF	Param78,W
	MOVWF	INDF0
;
	RETURN
;=========================================================================================
;=========================================================================================
;
	include	F1822_Common.inc
;
;=========================================================================================
;=========================================================================================
;
;
;
;
	END
;

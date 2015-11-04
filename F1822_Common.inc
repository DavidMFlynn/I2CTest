;=========================================================================================
; Commonly used routines PIC16F1822 version
;
;    Filename:      F1822 Common.inc
;    Date:          9/4/2014
;    File Version:  1.0.1
;
;    Author:        David M. Flynn
;    Company:       Oxford V.U.E., Inc.
;    E-Mail:        dflynn@oxfordvue.com
;    Web Site:      http://www.oxfordvue.com/
;
;=========================================================================================
;    History:
;
; 1.0.1 9/4/2014	Updated from F1847 Common.inc
; 1.0   11/16/2013	Updated from F648A Common.inc
;
;=========================================================================================
; Routines:
;
; ClearRam	(2+0) Clears all RAM, call once before initializing variables, FSR0
; CopyToRam	(1+0) copy param memory (EEPROM) to ram, call once, FSR0
; SaveParams	(1+0) copy ram to param memory (EEPROM), FSR0
; TX_TheByte	(0+0) Send one byte to UART
; RX_TheByte	(0+0) Receive one byte from UART
;
; DecTimer4	(0+0) Decrement routine for 16 bit timers, FSR0
; DecTimer3
; DecTimer2
; DecTimer1
;
; TestT4_Zero	Test for 16 bit timers = zero
; TestT3_Zero	If Timer is zero return Z flag,1 else Z=0
; TestT2_Zero
; TestT1_Zero
;
; Delay10uS	(0+0)Delay uS    1 cycle = 1uS, 8Mhz clock version
; Delay100uS
; Delay40uS
; DelayWuS
;
; EEReadW	(0+0) Read EEPROM address in W
; EERead	(0+0) Read EEPROM address in EEAddrTemp
; EEWriteW	(0+0) Write EEPROM address in W, Data in EEDataTemp
; EEWrite	(0+0) Write EEPROM address in EEAdrTemp, Data in EEDataTemp, FSR0
;
; StoreSerIn	Put the byte in W into the serial input buffer, FSR0
; GetSerIn	Get a byte from the serial input buffer, FSR0
; GetSerOutBytes	Get the number of bytes in the serial ouput buffer
; StoreSerOut	Put the byte in W into the serial output buffer, FSR0
; POP_SerOut	Remove the last char stored in the output buffer
; GetSerOut	Get a byte from the serial Output buffer, FSR0
;
;=========================================================================================
; Clears all RAM
; Entry: none
; Exit: none
; RAM used: All
; Calls:(2+0) ClearRam_L2
;
ClearRam	MOVLB	0x00
	MOVLW	0x5F	;Clear 20h-7Eh, 95 bytes
	MOVWF	Param7F
	MOVLW	0x20
	MOVWF	FSR0
	CLRF	FSR0H
	CALL	ClearRam_L2
;
	MOVLW	0x20	;Clear A0h-BFh, 32 bytes
	MOVWF	Param7F
	MOVLW	0xA0
	MOVWF	FSR0
;
ClearRam_L2	CLRF	INDF0
	INCF	FSR0,F
	DECFSZ	Param7F,F
	GOTO	ClearRam_L2
	RETURN
;
	ifndef UesesCopyToRam
	constant UesesCopyToRam=0
	endif
	if UesesCopyToRam
;==========================================================================
; copy param memory to ram
;
CopyToRam:
	BankSel	EEAddrTemp
	MOVLW	nvFirstParamByte
	MOVWF	EEAddrTemp
	MOVLW	FirstRAMParam
	MOVWF	FSR0
	CLRF	FSR0H
CopyToRam_L1	CALL	EERead
	MOVWF	INDF0
	MOVLW	LastRAMParam
	SUBWF	FSR0,W
	SKPNZ
	RETURN
	INCF	FSR0,F
	INCF	EEAddrTemp,F
	GOTO	CopyToRam_L1
;
;===========================================================================
; copy ram to param memory
;
SaveParams:
	BankSel	EEAddrTemp
	MOVLW	nvFirstParamByte
	MOVWF	EEAddrTemp
	MOVLW	FirstRAMParam
	MOVWF	FSR0
	CLRF	FSR0H
SaveParams_L1	MOVF	INDF0,W
	MOVWF	EEDataTemp
	CALL	EEWrite
	MOVLW	LastRAMParam	;last byte
	SUBWF	FSR0,W
	SKPNZ
	RETURN
	INCF	FSR0,F
	INCF	EEAddrTemp,F
	GOTO	SaveParams_L1
;
	endif
;=====================================================================================================
;=========================================================================================================
; Decrement routine for 16 bit timers
;
DecTimer4	movlw	Timer4Hi
	goto	DecTimer
DecTimer3	movlw	Timer3Hi
	goto	DecTimer
DecTimer2	movlw	Timer2Hi
	goto	DecTimer
DecTimer1	movlw	Timer1Hi
;DecTimer
; entry: FSR=Timer(n)Hi
DecTimer	MOVWF	FSR0
	MOVF	INDF0,W	;TimerNHi
	DECF	FSR0,F
	IORWF	INDF0,W	;TimerNLo
	SKPNZ
	RETURN
	MOVLW	0x01
	SUBWF	INDF0,F	;TimerNLo
	INCF	FSR0,F
	MOVLW	0x00
	SUBWFB	INDF0,F	;TimerNHi
	RETURN
;
;==============================================================================================
; Test for 16 bit timers = zero
;If Timer is zero return Z flag,1 else Z=0
;
TestT4_Zero	movf	Timer4Lo,W
	iorwf	Timer4Hi,W
	return
;
TestT3_Zero	movf	Timer3Lo,W
	iorwf	Timer3Hi,W
	return
;
TestT2_Zero	movf	Timer2Lo,W
	iorwf	Timer2Hi,W
	return
;
TestT1_Zero	movf	Timer1Lo,W
	iorwf	Timer1Hi,W
	return	
;
;======================================================================================
;Delay uS    1 cycle = 1uS, 8Mhz clock version
; RAM used: Param77
; Calls:(0) none
Delay10uS	MOVLW	0x05	;(2*3+5)/2=10
	GOTO	DelayWuS
Delay100uS	MOVLW	d'65'	;(28*3+5)/2=100
	GOTO	DelayWuS
Delay40uS	MOVLW	d'25'	;(11*3+5)=40
DelayWuS	MOVWF	Param77
DelayWuS_Loop	DECFSZ	Param77,F
	GOTO	DelayWuS_Loop
	RETURN
;
;==============================================================================================
; Read EEPROM
; entry: EEPROM address to read in W
;        Bank 0 selected
; exit: W=EEDATA, Bank 0 selected
;
EEReadW	movwf	EEAddrTemp
;
;==============================================================================================
; Read EEPROM
; entry: EEPROM address to read in EEAddrTemp
;        Bank 0 selected
; exit: W=EEDATA, Bank 0 selected
;
EERead	MOVF	EEAddrTemp,W	;Data Memory
	BANKSEL	EEADRL
	MOVWF	EEADRL	;Address to read
;
	BCF	EECON1,CFGS	;Deselect Config space
	BCF	EECON1,EEPGD	;Point to DATA memory
	BSF	EECON1,RD	;EE Read
	MOVF	EEDATL,W	;W = EEDATL
	MOVLB	0x00	;Bank 0
	return
;	
;==============================================================================================
; Write EEPROM
; entry: EEPROM address to write in W
;        EEPROM data to write in EEDataTemp
;        Bank 0 selected
; exit: Bank 0 selected
;
EEWriteW	movwf	EEAddrTemp
;
;==============================================================================================
; Write EEPROM
; entry: EEPROM address to write in EEAdrTemp
;        EEPROM data to write in EEDataTemp
;        Bank 0 selected
; exit: Bank 0 selected
;
EEWrite	MOVF	EEAddrTemp,W
	BANKSEL	EEADRL
	MOVWF	EEADRL	;Data Memory Address to write
	BankSel	EEDataTemp
	MOVF	EEDataTemp,W
	BANKSEL	EEDATL
	MOVWF	EEDATL	;Data Memory Value to write
	BCF	EECON1,CFGS	;Deselect Configuration space
	BCF	EECON1,EEPGD	;Point to DATA memory
	BCF	INTCON,GIE	;Disable INTs.
	BSF	EECON1,WREN	;Enable writes
	MOVLW	0x55
	MOVWF	EECON2	;Write 55h
	MOVLW	0xAA
	MOVWF	EECON2	;Write AAh
	BSF	EECON1,WR	;Set WR bit to begin write
	BSF	INTCON,GIE	;Enable Interrupts
	BCF	EECON1,WREN	;Disable writes
EEWriteLoop	BTFSC	EECON1,WR	;Wait for write to complete
	GOTO	EEWriteLoop
	MOVLB	0x00	;Bank 0
	return
;
;=========================================================================================================
	IF useRS232
;=====================================================================================================
; Send one byte
; Entry: Bank0
; Exit: Bank0
;
;
TX_TheByte	MOVF	TXByte,W
	BTFSS	DataSentFlag
	MOVWF	TXREG
	BSF	DataSentFlag
	RETURN
;
;=====================================================================================================
;
RX_TheByte	MOVF	RCSTA,W
	ANDLW	0x06
	SKPZ		;Fame or Overrun Err?
	GOTO	RX_TheByte_Err	; yes
	MOVF	RCREG,W
;
	MOVWF	RXByte
	BSF	DataReceivedFlag
	RETURN
;
RX_TheByte_Err	MOVF	RCREG,W	;bad byte so drop it
	CLRF	RXByte
	BCF	RCSTA,CREN
	BCF	RCSTA,FERR
	BCF	RCSTA,OERR
	BSF	RCSTA,CREN
	RETURN
;
;=========================================================================================================
;=============================================================================================
; Put the byte in W into the serial input buffer
;
; Entry: W = byte to store
; Exit: none
;
StoreSerIn	MOVWF	Param78
	MOVLB	Ser_Buff_Bank
	INCF	Ser_In_Bytes,F
;	
	MOVF	Ser_In_InPtr,W
	ANDLW	0x1F
	INCF	Ser_In_InPtr,F
	ADDLW	low Ser_In_Buff
	MOVWF	FSR0
	MOVLW	high Ser_In_Buff
	MOVWF	FSR0H
	MOVF	Param78,W
	MOVWF	INDF0
	MOVLB	0x00
	RETURN
;
;=============================================================================================
; Get a byte from the serial input buffer
;
; Entry: none
; Exit: W=byte from buffer, Param78,0=valid
;
GetSerIn	MOVLB	Ser_Buff_Bank
	BCF	Param78,0	;no data
	MOVF	Ser_In_Bytes,F
	SKPNZ
	GOTO	Bank0Rtn
	BSF	Param78,0
	DECF	Ser_In_Bytes,F
	MOVF	Ser_In_OutPtr,W
	ANDLW	0x1F
	INCF	Ser_In_OutPtr,F
	ADDLW	low Ser_In_Buff
	MOVWF	FSR0
	MOVLW	high Ser_In_Buff
	MOVWF	FSR0H
	MOVF	INDF0,W
Bank0Rtn	MOVLB	0x00
	RETURN
;
;=============================================================================================
; Get the number of bytes in the serial ouput buffer
;
; Entry: none
; Exit: W=bytes in buffer, Z
;
GetSerOutBytes	MOVLB	Ser_Buff_Bank
	MOVF	Ser_Out_Bytes,W
	MOVLB	0x00
	RETURN
;
;=============================================================================================
; Put the byte in W into the serial output buffer
;
; Entry: W = byte to store
; Exit: Param78 & W = byte stored
;
StoreSerOut	MOVWF	Param78
	MOVLB	Ser_Buff_Bank
	INCF	Ser_Out_Bytes,F
;	
	MOVF	Ser_Out_InPtr,W
	ANDLW	0x1F
	INCF	Ser_Out_InPtr,F
	ADDLW	low Ser_Out_Buff
	MOVWF	FSR0
	MOVLW	high Ser_In_Buff
	MOVWF	FSR0H
	MOVF	Param78,W
	MOVWF	INDF0
	MOVLB	0x00
	RETURN
;
;=============================================================================================
; Remove the last char stored in the output buffer
;
POP_SerOut	MOVLB	Ser_Buff_Bank
	MOVF	Ser_Out_Bytes,F
	SKPNZ
	GOTO	Bank0Rtn
	DECF	Ser_Out_Bytes,F
	DECF	Ser_Out_InPtr,F
	MOVLB	0x00
	RETURN
;
;=============================================================================================
; Get a byte from the serial Output buffer
;
; Entry: none
; Exit: W=byte from buffer, Param78,0=valid
;
GetSerOut	MOVLB	Ser_Buff_Bank
	BCF	Param78,0	;no data
	MOVF	Ser_Out_Bytes,F
	SKPNZ
	GOTO	Bank0Rtn
	BSF	Param78,0
	DECF	Ser_Out_Bytes,F
	MOVF	Ser_Out_OutPtr,W
	ANDLW	0x1F
	INCF	Ser_Out_OutPtr,F
	ADDLW	low Ser_Out_Buff
	MOVWF	FSR0
	MOVLW	high Ser_In_Buff
	MOVWF	FSR0H
	MOVF	INDF0,W
	MOVLB	0x00
	RETURN
;
	ENDIF
;
;

	subtitle	"F1822 Macros"
	page
;===================================================================================
;
;  FileName: F1822 Macros.inc
;  Date: 4/23/2015
;  File Version: 1.3.3
;  
;  Author: David M. Flynn
;  Company: Oxford V.U.E., Inc.
;
;============================================================================================
; Notes:
;
;  These are generic macros for the 16F1822
;
;============================================================================================
; Revision History
;
; 1.3.3  4/23/2015	Removed old comments
; 1.3.2  9/4/2014	Modified for 12F1822
; 1.3.1  11/16/13	Modified for 16F1847
; 1.3    4/28/04	Added mBankISel
; 1.2.4  5/19/03	Added BNE, BEQ
; 1.2.3  5/16/03	Added CMP, BMI and BPL.
; 1.2.2  4/23/03	Changed LCALL to be just CALL when same segment.
; 1.2.1  4/15/03	Added SKPB, SKPNB
; 1.2    2/26/03	Added Instruction set extentions for 14 bit core (16F###) (27 of them)
; 1.1    2/21/03	Added mLED_On, mLED_Off, mPCLGoto
;	Added 18F452 version of mCall#To#
; 1.0.1  2/12/03	Fixed mCallxTox so a return from a segment other than the one
;	called will not cause PCLATH,3/PCLATH,4 problems.
; 1.0    10/1/02	Moved macros to this file
;
;============================================================================================
; Conditionals
;
;============================================================================================
; Name	(additional stack words required), (Return Bank) Description
;============================================================================================
;TestTimerIsZero	(0),(0) pass Timer1Lo,Timer2Lo, etc. returns STATUS,Z set if zero
;
;
;mBankISel	(0),(n/a) Set Status,IRP bit corect for the operand passed.
;
;===================================================================
; Instruction set extentions for 14 bit core (16F###)
;
; opcode	operand	description
;---------------------------------------------------------------
;ADDCF	f,d	Add Carry to File
;ADDDCF	f,d	Add Digit Carry to File
;BC	k	Branch on Carry
;BDC	k	Branch on Digit Carry
;BEQ	k	Branch on Equal to Zero aka BZ
;BMI	k	Branch on W minus (bit 7 set) (W bits 0..6 are zerroed)
;BNC	k	Branch on No Carry
;BNDC	k	Branch on No Digit Carry
;BNE	k	Branch on Not Equal to Zero aka BNZ
;BNZ	k	Branch on No Zero
;BPL	k	Branch on W plus (bit 7 clr) (W bits 0..6 are zerroed)
;BZ	k	Branch on Zero
;CLRC		Clear Carry
;CLRDC		Clear Digit Carry
;CLRZ		Clear Zero flag
;CMP	f	Compare F to W (F-W)
;LCALL	k	Long Call
;LGOTO	k	Long GOTO
;MOVFW	f	Move File to W
;NEGF	f,d	Negate File
;SETC		Set Carry
;SETDC		Set Digit Carry
;SETZ		Set Zero
;SKPB		Skip on Borrow
;SKPC		Skip on Carry
;SKPDC		Skip on Digit Carry
;SKPNB		Skip on Not Borrow
;SKPNC		Skip on No Carry
;SKPNDC		Skip on No Digit Carry
;SKPNZ		Skip on Not Zero
;SKPZ		Skip on Zero
;SUBBF	f,d	Subtract Borrow from File (used after SUBWF)
;SUBCF	f,d	Subtract Carry from File
;SUBDCF	f,d	Subtract Digit Carry from File
;TSTF	f	Test File
;
;======================================================================================
; pass Timer1Lo,Timer2Lo, etc.
; returns STATUS,Z set if zero
;
TestTimerIsZero	macro	T_Number
	CLRF	BSR	;Bank0
	MOVF	T_Number,W
	IORWF	T_Number+1,W
	endm	
;
;-----------------------------------------------------------------------------------------------
; The macro LOADFSR1 loads FSR1 and makes code easier to read.
;------------------------------------- LOADFSR macro -------------------------------------------
;
LOADFSR1	macro 	ADDRESS,INDEX 	; ADDRESS = I2C_ARRAY, INDEX = INDEX_I2C			
	movlw 	low ADDRESS 	; load address 						
	addwf	INDEX,W	; add the index value to determine location in array
	movwf 	FSR1L	; load FSR1L with pointer info
	clrf	FSR1H
	movlw	high ADDRESS
	addwfc	FSR1H,F
	endm		
;
LOADFSR0	macro 	ADDRESS,INDEX 	; ADDRESS = I2C_ARRAY, INDEX = INDEX_I2C			
	movlw 	low ADDRESS 	; load address 						
	addwf	INDEX,W	; add the index value to determine location in array
	movwf 	FSR0L	; load FSR0L with pointer info
	clrf	FSR0H
	movlw	high ADDRESS
	addwfc	FSR0H,F
	endm		
;
;=======================================================================================
; Instruction set extentions for 14 bit core (16F###)
;=======================================================================================
; Add Carry to File
;
;ADDCF	macro	dest,W_or_F
;	BTFSC	STATUS,C
;	INCF	dest,W_or_F
;	endm
;
;==============================================================
; Add Digit Carry to File
;
;ADDDCF	macro	dest,W_or_F
;	BTFSC	STATUS,DC
;	INCF	dest,W_or_F
;	endm
;
;==============================================================
; Branch on Carry
;
;BC	macro	dest
;	BTFSC	STATUS,C
;	GOTO	dest
;	endm
;
;==============================================================
; Branch on Digit Carry
;
;BDC	macro	dest
;	BTFSC	STATUS,DC
;	GOTO	dest
;	endm
;
;==============================================================
; Branch on Equal to Zero aka BZ
;
BEQ	macro	dest
	BTFSC	STATUS,Z
	GOTO	dest
	endm
;
;==============================================================
; Branch on W minus (bit 7 set) (W bits 0..6 are zerroed)
;
BMI	macro	dest
	ANDLW	0x80
	BTFSC	STATUS,Z	;skip if Z
	GOTO	dest
	endm
;
;==============================================================
; Branch on No Carry
;
;BNC	macro	dest
;	BTFSS	STATUS,C
;	GOTO	dest
;	endm
;
;==============================================================
; Branch on No Digit Carry
;
;BNDC	macro	dest
;	BTFSS	STATUS,DC
;	GOTO	dest
;	endm
;
;==============================================================
; Branch on Not Equal to Zero aka BNZ
;
BNE	macro	dest
	BTFSS	STATUS,Z
	GOTO	dest
	endm
;
;==============================================================
; Branch on No Zero
;
;BNZ	macro	dest
;	BTFSS	STATUS,Z
;	GOTO	dest
;	endm
;
;==============================================================
; Branch on W plus (bit 7 clr) (W bits 0..6 are zerroed)
;
BPL	macro	dest
	ANDLW	0x80
	BTFSS	STATUS,Z	;skip if not Z
	GOTO	dest
	endm
;
;==============================================================
; Branch on Zero
;
;BZ	macro	dest
;	BTFSC	STATUS,Z
;	GOTO	dest
;	endm
;
;==============================================================
; Clear Carry
;
;CLRC	macro
;	BCF	STATUS,C
;	endm
;
;==============================================================
; Clear Digit Carry
;
;CLRDC	macro
;	BCF	STATUS,DC
;	endm
;
;==============================================================
; Clear Zero flag
;
;CLRZ	macro
;	BCF	STATUS,Z
;	endm
;
;==============================================================
; Compare F to W (F-W)
;
CMP	macro	f
	SUBWF	f,W
	endm
;
;==============================================================
;==============================================================
; Move File to W
;
;MOVFW	macro	src
;	MOVF	src,W
;	endm
;
;==============================================================
; Negate File
;
;NEGF	macro	dest,W_or_F
;	COMF	dest,F
;	INCF	dest,W_or_F
;	endm
;
;==============================================================
; Set Carry
;
;SETC	macro
;	BSF	STATUS,C
;	endm
;
;==============================================================
; Set Digit Carry
;
;SETDC	macro
;	BSF	STATUS,DC
;	endm
;
;==============================================================
; Set Zero
;
;SETZ	macro
;	BSF	STATUS,Z
;	endm
;
;==============================================================
; Skip on Borrow
;
SKPB	macro
	BTFSC	STATUS,C
	endm
;
;==============================================================
; Skip on Carry
;
;SKPC	macro
;	BTFSS	STATUS,C
;	endm
;
;==============================================================
; Skip on Digit Carry
;
;SKPDC	macro
;	BTFSS	STATUS,DC
;	endm
;
;==============================================================
; Skip on Not Borrow
;
SKPNB	macro
	BTFSS	STATUS,C
	endm
;
;==============================================================
; Skip on No Carry
;
;SKPNC	macro
;	BTFSC	STATUS,C
;	endm
;
;==============================================================
; Skip on No Digit Carry
;
;SKPNDC	macro
;	BTFSC	STATUS,DC
;	endm
;
;==============================================================
; Skip on Not Zero
;
;SKPNZ	macro
;	BTFSC	STATUS,Z
;	endm
;
;==============================================================
; Skip on Zero
;
;SKPZ	macro
;	BTFSS	STATUS,Z
;	endm
;
;==============================================================
; Subtract Borrow from File
;
SUBBF	macro	dest,W_or_F
	BTFSS	STATUS,C
	DECF	dest,W_or_F
	endm
;
;==============================================================
; Subtract Carry from File
;
;SUBCF	macro	dest,W_or_F
;	BTFSC	STATUS,C
;	DECF	dest,W_or_F
;	endm
;
;==============================================================
; Subtract Digit Carry from File
;
;SUBDCF	macro	dest,W_or_F
;	BTFSC	STATUS,DC
;	DECF	dest,W_or_F
;	endm
;
;==============================================================
; Test File
;
;TSTF	macro	src
;	MOVF	src,F
;	endm
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;

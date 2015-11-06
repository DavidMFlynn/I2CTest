;=========================================================================================
; PIC16F1822
; I2C SLAVE DRIVER CODE
;
;    Filename:      I2C SLAVE.inc
;    Date:          11/4/2015
;    File Version:  1.0.3
;
;    Author:        David M. Flynn
;    Company:       Oxford V.U.E., Inc.
;    E-Mail:        dflynn@oxfordvue.com
;    Web Site:      http://www.oxfordvue.com/
;
;=========================================================================================
;   I2C Slave Driver Code for PIC16F1822
;
;    History:
;
; 1.0.3 11/4/2015	Added ACK to R_Data for partial buffer read support.
;	Added PCIE (Stop causes interrupt) to end write 
;	 before all RX_ELEMENTS received.
; 1.0.2 4/25/2015	First woring version
; 1.0.1  9/4/2014	Modified for 12F1822
; 1.0   11/23/2013	Adapted from AN734C
;
; *** Adapted from AN734C ****
; PIC16F1937 
;I2C SLAVE DRIVER CODE 
;Author: Chris Best 
;Microchip Technologies
;DATE: 07/03/2013
;
;=========================================================================================
; Options
;
;   I2C_ADDRESS and RX_ELEMENTS must be set and be the same in the master driver.
;
;=========================================================================================
;=========================================================================================
; What happens next:
;  Sends or receives RX_ELEMENTS bytes of data from the master
;
;=========================================================================================
; Routines:
;
; Init_I2C	initial setup, call once before starting main event loop
; I2C_READ
; I2C_WRITE
;
;=========================================================================================
;
;
;Note: only upper 7 bits of address are used
;I2C_ADDRESS	EQU	0x30	; Slave address
;RX_ELEMENTS	EQU	.8	; number of allowable array elements, in this case 16
;I2C_TX_Init_Val	EQU	0xAA	; value to load into transmit array to send to master
;I2C_RX_Init_Val	EQU	0xAA	; value to load into received data array
;
;------------------------------------ variables-------------------------------------------
;
;	cblock	0x70									; set up in shared memory for easy access
;	INDEX_I2C		; index used to point to array location	
;	GFlags
;	endc
;
;#Define	I2C_TXLocked	GFlags,0	; Set/cleared by ISR, data is being sent
;#Define	I2C_RXLocked	GFlags,1	; Set/cleared by ISR, data is being received
;#Define	I2C_NewRXData	GFlags,2	; Set by ISR, The new data is here!
;
;I2C_Buffers	udata	0xA0
;I2C_ARRAY_TX	RES	RX_ELEMENTS	; array to transmit to master
;I2C_ARRAY_RX 	RES	RX_ELEMENTS 	; array to receive from master
;	
;=========================================================================================
;----------------------------------- interrupt service routines --------------------------
;
;-----------------------------------------------------------------------------------------
; I2C Com
;IRQ_4	MOVLB	0x00
;	btfss 	PIR1,SSP1IF 	; Is this a SSP interrupt?
;	goto 	IRQ_4_End 	; if not, bus collision int occurred
;	banksel	SSP1STAT						
;	btfsc	SSP1STAT,R_NOT_W	; is it a master read:
;	goto	I2C_READ	; if so go here
;	goto	I2C_WRITE	; if not, go here
;I2C_READ_Return
;I2C_WRITE_Return	movlb	0x00
;	bcf 	PIR1,SSP1IF	; clear the SSP interrupt flag
;IRQ_4_End
;-----------------------------------------------------------------------------------------
; I2C Bus Collision
;IRQ_5	MOVLB	0x00
;	btfss	PIR2,BCL1IF
;	goto	IRQ_5_End
;	banksel	SSPBUF						
;	clrf	SSPBUF	; clear the SSP buffer
;	movlb	0x00	;banksel PIR2
;	bcf	PIR2,BCL1IF	; clear the SSP interrupt flag	
;	banksel	SSPCON1
;	bsf	SSPCON1,CKP	; release clock stretch
;	movlb	0x00
;
;IRQ_5_End
;
;=========================================================================================
;
;LOADFSR1	macro 	ADDRESS,INDEX 	; ADDRESS = I2C_ARRAY, INDEX = INDEX_I2C			
;	movlw 	low ADDRESS 	; load address 						
;	addwf	INDEX,W	; add the index value to determine location in array
;	movwf 	FSR1L	; load FSR1L with pointer info
;	clrf	FSR1H
;	movlw	high ADDRESS
;	addwfc	FSR1H,F
;	endm		
;
;=========================================================================================
;	Main
;-----------------------------------------------------------------------------------------
;
;	call	Init_I2C	; set up uC
;
;=========================================================================================
;-----------------------------------------------------------------------------------------
;	Initialize: Sets up register values 
;-----------------------------------------------------------------------------------------
; fill the RX and TX arrays with I2C_TX_Init_Val, I2C_RX_Init_Val
;
Init_I2C:
; for J=0 to RX_ELEMENTS-1
;  I2C_ARRAY_TX(J)=I2C_TX_Init_Val
	movlw	low I2C_ARRAY_TX
	movwf	FSR1L
	movlw	high I2C_ARRAY_TX
	movwf	FSR1H
	movlw	RX_ELEMENTS
	movwf	INDEX_I2C
	movlw	I2C_TX_Init_Val
Init_I2C_L1	movwi	FSR1++
	decfsz	INDEX_I2C,F
	goto	Init_I2C_L1
;
	movlw	low I2C_ARRAY_RX
	movwf	FSR1L
	movlw	high I2C_ARRAY_RX
	movwf	FSR1H
	movlw	RX_ELEMENTS
	movwf	INDEX_I2C
	movlw	I2C_RX_Init_Val
Init_I2C_L2	movwi	FSR1++
	decfsz	INDEX_I2C,F
	goto	Init_I2C_L2
;
;I2C set up
	banksel	SSPSTAT
	bsf	SSP1STAT,SMP	; Slew rate control disabled for standard speed mode
	movlw	b'00110110'	; Enable serial port, I2C slave mode, 7-bit address
	movwf	SSP1CON1
	bsf	SSP1CON2,SEN	; enable clock stretching
	bsf	SSP1CON3,BOEN	; SSPBUF is updated and NACK is generated 	
			; for a received address/data byte, 
			; ignoring the state of SSPOV bit only if BF bit = 0.
	bsf	SSP1CON3,SDAHT	; Minimum of 300 ns hold time
	bsf	SSP1CON3,PCIE	; Stop causes interupt
	movlw	I2C_ADDRESS	; load the slave address
	movwf	SSP1ADD
;
	movlb	0x01
	bsf	PIE2,BCL1IE	; enable SSP interrupts
	bsf	PIE1,SSP1IE	; enable SSP interrupts
	movlb	0x00
	bcf	PIR1,SSP1IF	; clear the SSP interrupt flag	
	bcf	PIR2,BCL1IF	; clear the SSP interrupt flag
	return
;
;=========================================================================================				
; Interrupt Service Routines (ISR)
;=========================================================================================
;
I2C_READ	btfsc	SSP1STAT,D_NOT_A	; was last byte an address or data?
	goto	R_DATA	; if set, it was data
;	
R_ADDRESS	movf	SSP1BUF,W	; dummy read to clear BF bit
	clrf	INDEX_I2C	; clear index pointer
	LOADFSR1	I2C_ARRAY_TX,INDEX_I2C
	movf	INDF1,W	; move value into W to load to SSP buffer
	movwf	SSP1BUF	; load SSP buffer
;
	btfsc	SSP1CON1,WCOL	; did a write collision occur?				
	call	WRITE_COLL	; if so, go clear bit
;
	incf	INDEX_I2C,F	; increment INDEX_I2C 'pointer'
	bsf	I2C_TXLocked	; lock data
	bsf	SSPCON1,CKP	; release clock stretch
	goto	I2C_READ_Return	; Go to EXIT to return from interrupt
;
R_DATA	btfsc	SSP1CON2,ACKSTAT	; ACK* = Master wants another byte
	goto	R_Data_NACK	;  don't send another
;
	movlw	RX_ELEMENTS	; load array elements value
	subwf	INDEX_I2C,W	; if Z = 1, subtract index from number of elements
	btfsc	_C	; did a carry occur after subtraction?
	goto	NO_MEM_OVERWRITE	; if so, Master is trying to write to many bytes
	LOADFSR1	I2C_ARRAY_TX,INDEX_I2C
	movf	INDF1,W	; move value into W to load to SSP buffer
	movwf	SSP1BUF	; load SSP buffer
;
	btfsc	SSPCON1,WCOL	; did a write collision occur?
	call	WRITE_COLL	; if so, go clear bit
;
	incf	INDEX_I2C,F	; increment INDEX_I2C 'pointer'
;
	movlw	RX_ELEMENTS
	subwf	INDEX_I2C,W
	btfsc	_Z
	BCF	I2C_TXLocked	;unlock data
;
	bsf	SSPCON1,CKP	; release clock stretch
	goto	I2C_READ_Return 	; Go to EXIT to return from interrupt
;
R_Data_NACK	BCF	I2C_TXLocked
	goto	I2C_READ_Return
;
;-----------------------------------------------------------------------------------------
;
I2C_WRITE	btfsc	SSP1STAT,D_NOT_A	; was last byte an address or data?
	goto	W_DATA	; if set, it was data
;
W_ADDRESS	movf	SSP1BUF,W	; dummy read to clear the BF bit
	clrf	INDEX_I2C	; clear index pointer
	bsf	SSP1CON1,CKP	; release clock stretch
	BSF	I2C_RXLocked	; lock data
	goto	I2C_WRITE_Return 	; Go to EXIT to return from interrupt
;
W_DATA	btfsc	SSP1STAT,P	;Stop bit?
	goto	W_Data_Stop	;  Yes
	movlw	RX_ELEMENTS	; load array elements value
	subwf	INDEX_I2C,W	; if Z = 1, subtract index from number of elements
	btfsc	_C	; did a carry occur after subtraction?
	goto	NO_MEM_OVERWRITE	; if so, Master is trying to write to many bytes
;
	LOADFSR1	I2C_ARRAY_RX,INDEX_I2C
	movfw	SSP1BUF	; move the contents of the buffer into W
	movwf 	INDF1	; load INDF1 with data to write
	BSF	I2C_NewRXData	;new data is available
;
	incf	INDEX_I2C,F	; increment INDEX_I2C 'pointer'
;
	btfsc	SSP1CON1,WCOL	; did a write collision occur?
	call	WRITE_COLL	; if so, go clear bit
;
	movlw	RX_ELEMENTS
	subwf	INDEX_I2C,W
	btfss	_Z
	goto	W_DATA_1
	BCF	I2C_RXLocked	;unlock data
;
W_DATA_1	bsf	SSPCON1,CKP	; release clock stretch
	goto	I2C_WRITE_Return 	; Go to EXIT to return from interrupt
;
W_Data_Stop	BCF	I2C_RXLocked	;unlock data
	goto	I2C_WRITE_Return
;-----------------------------------------------------------------------------------------
;
NO_MEM_OVERWRITE	movf	SSP1BUF,W	; clear buffer so no overwrite occurs
	bsf	SSP1CON1,CKP	; release clock stretch
	goto	I2C_WRITE_Return 	; Go to EXIT to return from interrupt
;
;-----------------------------------------------------------------------------------------
;
WRITE_COLL	bcf	SSP1CON1,WCOL	; clear WCOL bit
	movf	SSP1BUF,W	; dummy read to clear the BF bit
	return
;
;
;	
		
		
		
		
		
		
		
		
		
		
		
		
		
											; END OF PROGRAM

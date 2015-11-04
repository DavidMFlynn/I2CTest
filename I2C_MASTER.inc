;=========================================================================================
; PIC16F1822
; I2C MASTER DRIVER CODE
;
;    Filename:      I2C MASTER.inc
;    Date:          11/3/2015
;    File Version:  1.0.4
;
;    Author:        David M. Flynn
;    Company:       Oxford V.U.E., Inc.
;    E-Mail:        dflynn@oxfordvue.com
;    Web Site:      http://www.oxfordvue.com/
;
;=========================================================================================
;   I2C Master Driver Code for PIC16F1822
;
;    History:
;
; 1.0.4  11/4/2015	Bug fixes.
; 1.0.3  9/7/2015	Added multi-Slave.
; 1.0.2  4/25/2015	First woring version, single master to single slave, no error checking.
; 1.0.1  9/4/2014	Modified for 12F1822
; 1.0   11/23/2013	Adapted from AN734C
;
; Known issues:
;  Not checking ACK from slave after address.
;  Address is hard coded.
;
; *** Adapted from AN734C ****
; PIC16F1937 
;I2C MASTER DRIVER CODE 
;Author: Chris Best 
;Microchip Technologies
;DATE: 07/03/2013
;
;=========================================================================================
; Options
;
;   I2C_ADDRESS and RX_ELEMENTS must be set and be the same in the slave driver.
;
;=========================================================================================
;=========================================================================================
; What happens next:
;  Sends and receives RX_ELEMENTS bytes of data to/from the slave
;
;INITIALIZE_I2C
; start write seq.	Set I2C_Write_Seq_Active, I2C_Init_Start, I2C_Addr_Pending, INDEX_I2C=0
;Idle_I2C
; set start bit	clr I2C_Init_Start
;I2C_WRITE
; send address	clr I2C_Addr_Pending set I2C_INIT_TRANS_DATA
;Idle_I2C
; 	clr I2C_INIT_TRANS_DATA set I2C_TRANS_DATA, I2C_TXLocked
;I2C_WRITE
; send I2C_ARRAY_TX,INDEX_I2C++ until RX_ELEMENTS bytes sent
;	clr I2C_TRANS_DATA set I2C_TRANS_COMPLETE
;Idle_I2C
; send stop bit	clr I2C_TRANS_COMPLETE set I2C_Stop_Asserted
;I2C_WRITE
; 	clr I2C_Write_Seq_Active, I2C_Stop_Asserted, I2C_TXLocked
;
;Idle_I2C
; start read seq.	set I2C_Read_Seq_Active, I2C_Addr_Pending, I2C_Init_Start
;Idle_I2C
; send start bit	clr I2C_Init_Start
;I2C_READ
; send address	clr I2C_Addr_Pending set I2C_INIT_MSTR_REC
;Idle_I2C
; 	clr I2C_INIT_MSTR_REC set I2C_SET_RCEN, I2C_RXLocked, INDEX_I2C=0
;
;I2C_READ
; enable receive	clr I2C_SET_RCEN set I2C_REC_BYTE
;I2C_READ
; read byte	clr I2C_REC_BYTE set I2C_SET_ACKEN
;Idle_I2C
; sent ack	clr I2C_SET_ACKEN, INDEX_I2C++
;	if INDEX_I2C<RX_ELEMENTS then set I2C_SET_RCEN else set I2C_Recd_All
;
;I2C_READ
; 	clr I2C_Recd_All, I2C_RXLocked set I2C_NewRXData, I2C_REC_COMPLETE
;Idle_I2C
; send stop	clr I2C_REC_COMPLETE set I2C_Stop_Asserted
;I2C_READ
; 	clr I2C_Stop_Asserted, I2C_Read_Seq_Active
;
;Idle_I2C
;	call I2C_Start_Write_Seq or I2C_Start_Read_Seq
;=========================================================================================
; Routines:	
;
; Init_I2C	initial setup, call once before starting main event loop
; Idle_I2C	Sequence control, call from main event loop
; I2C_WRITE	called by ISR to handle interupts while writing to slave
; I2C_READ	called by ISR to handle interupts while reading from slave
;
;=========================================================================================
;
;I2C_SlaveCount	EQU	.3
;RX_ELEMENTS	EQU	.8	; number of allowable array elements
;Note: only upper 7 bits of address are used
;I2C_ADDRESS	EQU	0x30	; slave address
;BRG_VAL	EQU	0x27	; baud rate setting = 100kHz
;I2C_TX_Init_Val	EQU	0xAA	; value to load into transmit array to send to master
;I2C_RX_Init_Val	EQU	0xAA	; value to load into received data array
;
;
;------------------------------------ variables-------------------------------------------
;	I2C_Slave_Idx		;0..I2C_SlaveCount-1
;	I2C_Temp		;temp var
;
;	cblock	0x70									; set up in shared memory for easy access
;	INDEX_I2C		; index used to point to array location
;			;   0..RX_ELEMENTS-1	
;	GFlags
;	GFlags2
;	GFlags3
;	endc
;
;#Define	I2C_TXLocked	GFlags,0	; Set/cleared by ISR, data is being sent
;#Define	I2C_RXLocked	GFlags,1	; Set/cleared by ISR, data is being received
;#Define	I2C_SET_ACKEN	GFlags,2	; sets up acknowledge sequence
;#Define	I2C_Init_Start	GFlags,3	; used to set bit for start sequence
;#Define	I2C_Write_Seq_Active	GFlags,4	; used for write sequence
;#Define	I2C_Stop_Asserted	GFlags,5	; used to set bit for stop sequence
;#Define	I2C_Addr_Pending	GFlags,6	; bit sets up address transmission
;#Define	I2C_INIT_TRANS_DATA	GFlags,7	; bit sets up data transmission
;#Define	I2C_TRANS_DATA	GFlags2,0	; used to monitor transmission
;#Define	I2C_TRANS_COMPLETE	GFlags2,1	; set when transmission is complete
;#Define	I2C_REC_COMPLETE	GFlags2,2	; set when reception is complete
;#Define	I2C_Read_Seq_Active	GFlags2,3	; used for read sequence
;#Define	I2C_INIT_MSTR_REC	GFlags2,4	; initializes reception
;#Define	I2C_SET_RCEN	GFlags2,5	; initializes RCEN bit to receive data
;#Define	I2C_REC_BYTE	GFlags2,6	; sets up receive byte sequence
;#Define	I2C_Recd_All	GFlags2,7	; Set when all bytes heve been rec'd waiting for ack before stop
;#Define	I2C_NewRXData	GFlags3,0	; Set by ISR, The new data is here!
;#Define	I2C_NewDataToTX	GFlags3,1	; set in main to cause send now
;	udata
;
; array to transmit to slaves
;I2C_ARRAY_TX	res	RX_ELEMENTS*I2C_SlaveCount
; array to receive from slaves
;I2C_ARRAY_RX	res	RX_ELEMENTS*I2C_SlaveCount	
;
;
;=========================================================================================
;
;LOADFSR1	macro 	ADDRESS,INDEX 	; ADDRESS = I2C_ARRAY, INDEX = INDEX_I2C			
;	movlw 	low ADDRESS 	; load address 				
;	addwf	INDEX,W	; add the index value to determine location in array
;	movwf 	FSR1L	; load FSR1L with pointer info
;	movlw	high ADDRESS
;	movwf	FSR1H
;	endm
;
;=========================================================================================
;----------------------------------- interrupt service routine ---------------------------
;
;IRQ_4	MOVLB	0x00
;	btfss 	PIR1,SSP1IF 	; Is this a SSP interrupt?
;	goto 	IRQ_4_End 	; if not, bus collision int occurred
;	btfsc	I2C_Write_Seq_Active	; is it a write sequence?
;	goto	I2C_Write	; if so go here
;	goto	I2C_READ	; if not, it is a read sequence
;I2C_READ_Return:
;I2C_Write_Return	movlb	0x00
;	bcf 	PIR1,SSP1IF	; clear the SSP interrupt flag
;IRQ_4_End
;
;-----------------------------------------------------------------------------------------
; I2C Bus Collision
;IRQ_5	MOVLB	0x00
;	btfss	PIR2,BCL1IF
;	goto	IRQ_5_End
;	movlb	0x04	;banksel SSPBUF						
;	clrf	SSP1BUF	; clear the SSP buffer
;	movlb	0x00	;banksel PIR2
;	bcf	PIR2,BCL1IF	; clear the SSP interrupt flag	
;
;IRQ_5_End
;
;=========================================================================================
;=========================================================================================
;	Main
;-----------------------------------------------------------------------------------------
;
;	call	INITIALIZE_I2C	; set up uC
;
;MainLoop
;	call	Idle_I2C
;=========================================================================================
;
Idle_I2C	btfss	I2C_Write_Seq_Active	;Is it a write sequence?
	goto	Idle_I2C_Read	; No, try read
	btfss	I2C_Init_Start	;Is start bit set?
	goto	Idle_I2C_Write_1	; No, check if transmission ready
	bcf	I2C_Init_Start	; Yes, clear flag, send start bit
	banksel	SSP1CON2
	bsf	SSP1CON2,SEN	; set start enable bit
Idle_I2C_B0Rtn	movlb	0x00
	return
;
; set up transmit sequence
;
Idle_I2C_Write_1	btfss	I2C_INIT_TRANS_DATA	;Is bit set?
	goto	Idle_I2C_Write_2	; if not, transmit of data complete
	bcf	I2C_INIT_TRANS_DATA	; if so, clear bit
	bsf	I2C_TRANS_DATA	; set bit for transmission seq in INT routine
	bsf	I2C_TXLocked	; lock the data
	return
;
; set up stop sequence
;
Idle_I2C_Write_2	btfss	I2C_TRANS_COMPLETE	;Is transmission complete?
	return		; No, go back to LOOP
	bcf	I2C_TRANS_COMPLETE	;Clear transmission complete flag
	bsf	I2C_Stop_Asserted	;Send a stop bit
	banksel	SSP1CON2
	bsf	SSP1CON2,PEN	;Begin stop sequence
	BTFSC	SSP1CON2,PEN	;Has stop bit been sent?
	GOTO	$-1	; no, loop back to test again
	goto	Idle_I2C_B0Rtn
;
;=========================================
;  read sequence
;
Idle_I2C_Read	btfss	I2C_Read_Seq_Active	; make sure read sequence is set
	goto	I2C_StartSeq	; if not, go start something
	btfss	I2C_Init_Start	; is start bit set?
	goto	Idle_I2C_Read_1	; if not, go get ready to read
; Send start bit
	bcf	I2C_Init_Start	; if so, clear bit
	banksel	SSPCON2
	bsf	SSPCON2,SEN	; set SEN bit to begin start sequence
	goto	Idle_I2C_B0Rtn
;
; get ready to receive data
;
Idle_I2C_Read_1	btfss	I2C_INIT_MSTR_REC	; Receive first byte
	goto	Idle_I2C_Read_2	; No, go receive data
	bcf	I2C_INIT_MSTR_REC	; if so, clear bit
	bsf	I2C_RXLocked	; lock the data
	clrf	INDEX_I2C
	bsf	I2C_SET_RCEN	; set this to so in next INT RCEN is set
	return
;
Idle_I2C_Read_2	btfsc	I2C_REC_BYTE	;Waiting to receive data?
	return		; Yes
;
Idle_I2C_Read_3	btfss	I2C_SET_ACKEN	; is bit set?
	goto	Idle_I2C_Read_4	; if not, finish read routines
;
	bcf	I2C_SET_ACKEN
;
	movlw	RX_ELEMENTS
	subwf	INDEX_I2C,W	
	btfss	_Z	; RX_ELEMENTS=INDEX_I2C?
	bsf	I2C_SET_RCEN	; No, when ack is done read another byte
	btfsc	_Z	; RX_ELEMENTS=INDEX_I2C?
	bsf	I2C_Recd_All	; Yes, we are ack'ing the last byte
;
	banksel	SSP1CON2
	BTFSC	I2C_SET_RCEN
	bcf	SSP1CON2,ACKDT	;send ACK
	BTFSC	I2C_Recd_All
	bsf	SSP1CON2,ACKDT	;send NAK
	bsf	SSP1CON2,ACKEN	; if so, set ACK bit to acknowledge slave
	goto	Idle_I2C_B0Rtn
;
; finish up read routine
;
Idle_I2C_Read_4	btfss	I2C_REC_COMPLETE	; is bit set?
	return		; if not, go back to LOOP
	bsf	I2C_Stop_Asserted	; if so, set up stop
	bcf	I2C_REC_COMPLETE	; clear receive complete bit
	banksel	SSP1CON2
	bsf	SSP1CON2,PEN	; set stop bit to begin stop sequence
	BTFSC	SSP1CON2,PEN	; has stop bit been sent?
	GOTO	$-1	; no, loop back to test
	goto	Idle_I2C_B0Rtn
;
;==============================================
;
I2C_StartSeq:
	btfss	I2C_NewDataToTX
	goto	I2C_Start_Read_Seq
	goto	I2C_Start_Write_Seq
;
;=========================================================================================
;	Initialize: Sets up register values
;=========================================================================================
; Entry: none
; Exit: bank0
; Calls: none
; Ram used: none
;
Init_I2C:
; for J=0 to RX_ELEMENTS-1
;  I2C_ARRAY_TX(J)=I2C_TX_Init_Val
	movlw	low I2C_ARRAY_TX
	movwf	FSR1L
	movlw	high I2C_ARRAY_TX
	movwf	FSR1H
;
	movlw	RX_ELEMENTS*I2C_SlaveCount
	movwf	INDEX_I2C	;used as count not index
	movlw	I2C_TX_Init_Val
Init_I2C_L1	movwf	INDF1
	incf	FSR1L,F
	decfsz	INDEX_I2C,F
	goto	Init_I2C_L1
;
; for J=0 to RX_ELEMENTS-1
;  I2C_ARRAY_RX(J)=I2C_RX_Init_Val
	movlw	low I2C_ARRAY_RX
	movwf	FSR1L
	movlw	high I2C_ARRAY_RX
	movwf	FSR1H
;
	movlw	RX_ELEMENTS*I2C_SlaveCount
	movwf	INDEX_I2C	;used as count not index
	movlw	I2C_RX_Init_Val
Init_I2C_L2	movwf	INDF1
	incf	FSR1L,F
	decfsz	INDEX_I2C,F
	goto	Init_I2C_L2
;
	CLRF	GFlags
	CLRF	GFlags2
	CLRF	GFlags3
;
;I2C set up
	BANKSEL	SSP1STAT
	bsf	SSP1STAT,SMP	; Slew rate control disabled for standard speed mode
	movlw	b'00101000'	; Enable serial port, I2C master mode, 7-bit address
	movwf	SSP1CON1
	bsf	SSP1CON3,SDAHT	; Minimum of 300 ns hold time
	movlw	BRG_VAL	; load the baud rate value
	movwf	SSP1ADD

	banksel	PIE1
	bsf	PIE1,SSP1IE	; enable SSP interrupts
	bsf	PIE2,BCL1IE	; enable SSP interrupts
	movlb	0x00	; banksel PIR1
	bcf	PIR1,SSP1IF	; clear the SSP interrupt flag
	bcf	PIR2,BCL1IF	; clear the SSP interrupt flag
	return
;
;=======================================================
; Stert write opperation
I2C_Start_Write_Seq	clrf	INDEX_I2C	; clear index
	CLRF	GFlags
	CLRF	GFlags2
	bsf	I2C_Init_Start	; set start bit
	bsf	I2C_Write_Seq_Active	; set WRITE bit
	bsf	I2C_Addr_Pending	; set bit to ready write add seq.
	bcf	I2C_NewDataToTX
	return
;
;======================================================
; Start read opperation
I2C_Start_Read_Seq	clrf	INDEX_I2C	; clear index
	CLRF	GFlags
	CLRF	GFlags2
	bsf	I2C_Init_Start	; set start bit
	bsf	I2C_Addr_Pending	; sets up read address transmission
	bsf	I2C_Read_Seq_Active	; set read sequence bit
	return
;
;=========================================================================================
; 	Interrupt Service Routine (ISR)
;=========================================================================================
;
; DATA TRANSMISSION SEQUENCE OF EVENTS
;
I2C_Write	btfsc	I2C_TRANS_COMPLETE	; is transmission complete?
	goto	I2C_Write_End	; if so, finish transmission
	btfss	I2C_Addr_Pending	; if not, has trans add been sent?
	goto	I2C_Write_TXData	; if so, go start to transmit
	bcf	I2C_Addr_Pending	; if not, send trans add
	bsf	I2C_INIT_TRANS_DATA
	call	I2C_GetSlaveAddr
	banksel	SSP1BUF
	BCF	WREG,0	; Set R/W* to Write
	movwf	SSP1BUF
	goto	I2C_Write_Return
;
;
I2C_Write_TXData	btfss	I2C_TRANS_DATA	; is transmission complete?
	goto	I2C_Write_End	; if so, go finish sequence
;
	movlw	RX_ELEMENTS	; load array elements value
	subwf	INDEX_I2C,W	; if Z = 1, subtract index from number of elements
	btfsc	_C	; did a carry occur after subtraction?
	goto	I2C_Write_1	; if so, array limit reached
;
	call	I2C_Set_TX_PTR
	movf	INDF1,W	; move value into W to load to SSP buffer
	banksel	SSP1BUF
	movwf	SSP1BUF	; load SSP buffer
	incf	INDEX_I2C,F	; increment INDEX_I2C 'pointer'
	btfsc	SSP1CON1,WCOL	; did a write collision occur?
	bcf	SSP1CON1,WCOL	; if so, clear bit
	goto	I2C_Write_Return
;
;
; is transmission complete?
;
I2C_Write_1	bcf	I2C_TRANS_DATA	; clear bit
	bsf	I2C_TRANS_COMPLETE	; get ready to end sequence
	goto	I2C_Write_Return
;
; finish transmit sequence
;
I2C_Write_End	btfss	I2C_Stop_Asserted	; is stop set?
	goto	I2C_Write_Return	; yes, we're done
	bcf	I2C_Stop_Asserted	; if so, clear stop
	bcf	I2C_Write_Seq_Active	; clear bit to prevent more transmissions
	bcf	I2C_TXLocked	; unlock data
	goto	I2C_Write_Return
;
;=========================================================================================
; READ FROM SLAVE SEQUENCE OF EVENTS
;
I2C_READ	btfss	I2C_Addr_Pending	; if not, was read add transmitted to slave?
	goto	I2C_READ_1	; if so, set master to receive
; Send address to slave(s)
	bcf	I2C_Addr_Pending	; clear bit
	bsf	I2C_INIT_MSTR_REC	; set bit to get ready next INT
	call	I2C_GetSlaveAddr
	banksel	SSP1BUF
	BSF	WREG,0	; Set R/W* to Read 
	movwf	SSP1BUF
	goto	I2C_READ_Return
;
; sets up master as a receiver
;
I2C_READ_1	btfss	I2C_SET_RCEN	; is bit set?
	goto	I2C_READ_2	; if not, master is in receive mode
	bcf	I2C_SET_RCEN	; otherwise, get set up
	bsf	I2C_REC_BYTE
	banksel	SSP1CON2
	bsf	SSP1CON2,RCEN	; set RCEN to enable master receive mode
	goto	I2C_READ_Return
;
; read the data byte from slave
;
I2C_READ_2	btfss	I2C_REC_BYTE	;Was data received?
	goto	I2C_READ_3	; No
	bcf	I2C_REC_BYTE
;
	call	I2C_Set_RX_PTR
	banksel	SSP1BUF
	movf	SSP1BUF,W	; move the contents of the buffer into W
	movwf 	INDF1	; load INDF1 with data to write
	incf	INDEX_I2C,F	; increment INDEX_I2C 'pointer'
	bsf	I2C_SET_ACKEN	; set up ack sequence in MAIN
	goto	I2C_READ_Return
;
; get ready to wrap things up
;
I2C_READ_3	btfss	I2C_Recd_All
	goto	I2C_READ_End
	bcf	I2C_Recd_All
	bcf	I2C_RXLocked	; unlock the data
	bsf	I2C_NewRXData	;new data is complete
	bsf 	I2C_REC_COMPLETE
	goto	I2C_READ_Return
;
I2C_READ_End	btfss	I2C_Stop_Asserted
	goto	I2C_READ_Return
	bcf	I2C_Stop_Asserted	; if so, clear stop
	bcf	I2C_Read_Seq_Active	; and we are done
	goto	I2C_READ_Return
;
;=========================================================================================
; Entry: none
; Exit: W=SlaveAddress
;
I2C_GetSlaveAddr:
	BANKSEL	I2C_Slave_Idx
	movf	I2C_Slave_Idx,W
	andlw	0x03
	brw
	retlw	I2C_ADDRESS
	retlw	I2C_ADDRESS+2
	retlw	I2C_ADDRESS+4
	retlw	I2C_ADDRESS+6
;
; Set FSR1 to point to the tx buffer for slave I2C_Slave_Idx
I2C_Set_TX_PTR:
	BANKSEL	I2C_Slave_Idx
	movlw	low I2C_ARRAY_TX
	movwf	FSR1L
	movlw	high I2C_ARRAY_TX
	movwf	FSR1H
;
I2C_Set_TX_PTR_E2	movf	INDEX_I2C,W
	addwf	FSR1L,F
	clrw
	addwfc	FSR1H,F
;
	movf	I2C_Slave_Idx,W
	movwf	I2C_Temp
	SKPNZ
	return
;
I2C_Set_TX_PTR_L1	movlw	RX_ELEMENTS
	addwf	FSR1L,F
	clrw
	addwfc	FSR1H,F
	decfsz	I2C_Temp,F
	goto	I2C_Set_TX_PTR_L1
	return
;
; Set FSR1 to point to the rx buffer for slave I2C_Slave_Idx
I2C_Set_RX_PTR:
	BANKSEL	I2C_Slave_Idx
	movlw	low I2C_ARRAY_RX
	movwf	FSR1L
	movlw	high I2C_ARRAY_RX
	movwf	FSR1H
	goto	I2C_Set_TX_PTR_E2
;	





















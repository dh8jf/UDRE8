/*
Title:			UART
Autor:			Frank Jonischkies
Umgebung:		WinAVR-20070525
*/ 

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "uart.h"

// Globale Variablen
static char TxBuff[TX_BUFFER_SIZE];

static volatile uint8_t TxRead;		// Zeigt auf Zeichen, das gesendet werden soll
static uint8_t TxWrite;				// Zeigt auf nächste Schreibposition im Puffer

// Funktionen Öffentlich
void InitUART(void) {
	UBRRL = (F_CPU/(16L*UART_BAUD_RATE))-1;
	UCSRB |= (1<<TXEN);									// UART TX einschalten
}

void UartPutC(char data) {
	uint8_t temp = (TxWrite + 1) & TX_BUFFER_MASK;
	while(temp == TxRead);									// Warte bis Platz im Puffer
	TxWrite = temp;
	TxBuff[temp] = data;
	UCSRB |= (1<<UDRIE);									// UDRE Interrupt ein
}

void UartPutS(const char *s) {
	while(*s) UartPutC(*s++);
}

void UartPutP(const char *progmem_s) {
	register char c;
	while((c = pgm_read_byte(progmem_s++))) UartPutC(c);
}

// Interrupt Service Routine
ISR(USART_UDRE_vect) {
	if(TxRead != TxWrite) {
		uint8_t temp = (TxRead + 1) & TX_BUFFER_MASK;
		TxRead = temp;
		UDR = TxBuff[temp];									// nächtes Zeichen aus Puffer senden
	} else UCSRB &= ~(1<<UDRIE);							// UDRE Interrupt aus
}

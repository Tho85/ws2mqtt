#include "Arduino.h"

#include "spi_uart.h"
#include "wisafe2_rx.h"
#include "wisafe2_tx.h"
#include "wisafe2_packets.h"

#define DEBUG_SERIAL

bool binModeActivated = false;
uint8_t uartrxbuf[RXBUF_SIZE];
int uartrxbufPos = 0;
int uartrxbufExpectedLength = 0;

void setupSPI() {
	Serial2.setRxBufferSize(1024);
	Serial2.begin(19200, SERIAL_8N1, 16, 17);
}

void sendSPIMessage(uint8_t* msg) {
	int length;
	for(length = 0; length < TXBUF_SIZE; length++)
		if (msg[length] == SPI_STOP_WORD)
			break;
	sendSPIMessage(msg, length+1);
}

void sendSPIMessage(uint8_t* msg, int length) {
	Serial2.write("}");
	Serial2.write(length);
	Serial2.write(msg, length);
}

void receiveSPIMessage(void (*rxCallback)(uint8_t*, int)) {
	while(Serial2.available()) {
		char c = Serial2.read();
#ifdef DEBUG_SERIAL
		if (binModeActivated)
			// Do not spam console with binary data
			Serial.print('%');
		else
			Serial.print(c);
#endif

		if (c == '{') {
			binModeActivated = true;
			uartrxbufPos = 0;
			uartrxbufExpectedLength = 0;
		} else if (binModeActivated) {
			if (uartrxbufExpectedLength) {
				uartrxbuf[uartrxbufPos++] = c;

				if (uartrxbufPos == uartrxbufExpectedLength) {
#ifdef DEBUG_SERIAL
					Serial.println();
#endif
					rxCallback(uartrxbuf, uartrxbufPos);

					binModeActivated = false;
					uartrxbufPos = 0;
					uartrxbufExpectedLength = 0;
				}
			} else {
				uartrxbufExpectedLength = c;
			}
		}

	}

#ifdef DEBUG_SERIAL
	if(Serial.available())
		Serial2.print((char)Serial.read());
#endif
}

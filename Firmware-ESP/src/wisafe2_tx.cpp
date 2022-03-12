#include "Arduino.h"

#include "wisafe2_tx.h"
#include "wisafe2_escape.h"
#include "config.h"
#include "wisafe2_packets.h"
#include "spi_uart.h"

unsigned long lastTxAt = 0;

uint8_t txQueue[TX_QUEUE_SIZE][TXBUF_SIZE] = { 0 };
int txQueueInsertPos = 0;
int txQueueSendPos = 0;

void handleTX(uint8_t* msg, int length) {
	uint8_t txbuf[TXBUF_SIZE];
	memcpy(&txbuf, msg, length);

	escapeMessage(txbuf, length);
	enqueueSPIMessage(txbuf);
}

void enqueueSPIMessage(uint8_t* msg) {
	memcpy(txQueue[txQueueInsertPos++], msg, TXBUF_SIZE);
	txQueueInsertPos %= TX_QUEUE_SIZE;
}

void sendWelcomeMsg() {
	Serial.printf("Sending welcome message\n");

	pkt_tx_welcome_t pkt;
	memset(&pkt, 0, sizeof(pkt));

	pkt.cmd = SPI_TX_RESPONSE_WELCOME;
	pkt.device_id = DEVICE_ID;
	pkt.model = 0x047C;
	pkt.device_type = DEVICE_TYPE_SMOKE;
	pkt.flags = 0x05;
	pkt.firmware = 0x1234;
	pkt.stop = SPI_STOP_WORD;

	handleTX((uint8_t*)&pkt, sizeof(pkt));
}

void sendTestButtonMsg() {
	Serial.printf("Sending test button message\n");

	pkt_tx_event_button_t pkt;
	memset(&pkt, 0, sizeof(pkt));

	pkt.cmd = SPI_TX_EVENT_BUTTON;
	pkt.device_id = DEVICE_ID;
	pkt.device_type = DEVICE_TYPE_ALL;
	pkt._unknown = 0x01;
	pkt.model = 0x047C;
	pkt.stop = SPI_STOP_WORD;

	handleTX((uint8_t*)&pkt, sizeof(pkt));
}

void sendDiagnosticRequest() {
	Serial.printf("Sending diagnostic request\n");

	uint8_t pkt_tx_diagnostic_request[2] = { SPI_TX_DIAGNOSTIC_REQUEST , SPI_STOP_WORD };
	handleTX(pkt_tx_diagnostic_request, 2);
}

void sendQuerySIDMap() {
	Serial.printf("Querying SID map\n");
	pkt_tx_diagnostic_details_t pkt;
	memset(&pkt, 0, sizeof(pkt));

	pkt.cmd = SPI_TX_DIAGNOSTIC_DETAILS_REQUEST;
	pkt.subtype = SPI_DIAGNOSTIC_DETAILS_UPDATE_SIDMAP;
	pkt.stop = SPI_STOP_WORD;

	handleTX((uint8_t*)&pkt, sizeof(pkt));
}

void sendQueryDiagnosticDetails(uint8_t sid, uint8_t subsubtype) {
	Serial.printf(" Querying remote status for SID %d subsubtype %d\n", sid, subsubtype);

	pkt_tx_diagnostic_details_2arg_t pkt;
	memset(&pkt, 0, sizeof(pkt));

	pkt.cmd = SPI_TX_DIAGNOSTIC_DETAILS_REQUEST;
	pkt.subtype = SPI_DIAGNOSTIC_DETAILS_REMOTE_STATUS;
	pkt.arg1 = sid;
	pkt.arg2 = subsubtype;
	pkt.stop = SPI_STOP_WORD;

	handleTX((uint8_t*)&pkt, sizeof(pkt));
}

void loopTX() {
	if ((millis() - lastTxAt > TX_MESSAGE_DELAY) && txQueue[txQueueSendPos][0] != 0x00) {
		// Actually send message to module (SPI via UART)
		sendSPIMessage(txQueue[txQueueSendPos]);

		// Clear sent message from queue
		memset(txQueue[txQueueSendPos++], 0, TXBUF_SIZE);
		txQueueSendPos %= TX_QUEUE_SIZE;

		lastTxAt = millis();
	}
}

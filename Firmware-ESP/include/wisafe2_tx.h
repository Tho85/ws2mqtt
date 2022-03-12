#ifndef WISAFE2_TX_H
#define WISAFE2_TX_H 1

// Maximum length of a single SPI message
#define TXBUF_SIZE 32

// Number of SPI messages to enqueue
#define TX_QUEUE_SIZE 32

// Minimum delay between sending messages
#define TX_MESSAGE_DELAY 500

void handleTX(uint8_t* msg, int length);
void enqueueSPIMessage(uint8_t* msg);

void sendWelcomeMsg();
void sendTestButtonMsg();
void sendDiagnosticRequest();
void sendQuerySIDMap();
void sendQueryDiagnosticDetails(uint8_t sid, uint8_t subsubtype);

void loopTX();
#endif

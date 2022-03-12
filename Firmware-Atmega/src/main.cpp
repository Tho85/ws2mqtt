#include "Arduino.h"
#include "SPI.h"

#define LED_BUILTIN 13
#define ENABLE_MODULE_PIN 8
#define HANDSHAKE_PIN 9
// #define SPI_DEBUG

unsigned long lastReceivedAt = 0;

#define STOP_WORD 0x7E
#define RECEIVE_TIMEOUT_US 10000

#define SPI_BUFFER_SIZE 32
uint8_t recvbuf[SPI_BUFFER_SIZE];
volatile byte recvbufPos;

uint8_t sendbuf[SPI_BUFFER_SIZE];
volatile byte sendbufPos;

#define SERIAL_BUFFER_SIZE 128
uint8_t serialbuf[SERIAL_BUFFER_SIZE];
byte serialbufPos;

char nibble1;

volatile boolean processReceiveFinished;
volatile boolean processReceived;
volatile boolean processSent;
volatile boolean sendData;

bool binModeStarted = false;
unsigned int binModeExpectedLength = 0;

void send(uint8_t *data, int length) {
	Serial.print("> ");
	for(int i = 0; i < length; i++) {
		if (data[i] < 0x10) Serial.print("0");
		Serial.print (data[i], HEX);
		Serial.print(" ");
	}
	Serial.println("");

	// Initialize sending register
	memcpy(sendbuf, data, length);
	sendbufPos = 0;

	// Put first byte into SPI register
	SPDR = sendbuf[0];
#ifdef SPI_DEBUG
	Serial.print(">. ");
	Serial.println(sendbuf[0], HEX);
#endif

	sendData = true;
	digitalWrite(HANDSHAKE_PIN, HIGH);
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	pinMode(ENABLE_MODULE_PIN, OUTPUT);
	digitalWrite(ENABLE_MODULE_PIN, HIGH);

	pinMode(HANDSHAKE_PIN, OUTPUT);

	Serial.begin(19200);
	pinMode(MISO, OUTPUT);

	SPCR |= _BV(SPE);

	// Initialize SPI buffers
	recvbufPos = 0;
	memset(recvbuf, 0x00, SPI_BUFFER_SIZE);

	sendbufPos = 0;
	memset(sendbuf, 0x00, SPI_BUFFER_SIZE);

	serialbufPos = 0;
	memset(serialbuf, 0x00, SERIAL_BUFFER_SIZE);

	processReceiveFinished = false;
	processReceived = false;
	processSent = false;
	sendData = false;

	SPI.attachInterrupt();
}

ISR (SPI_STC_vect) {
	digitalWrite(HANDSHAKE_PIN, LOW);

	// Read received char into receiving buffer
	byte c = SPDR;
	recvbuf[recvbufPos] = c;

	if (sendData) {
		processSent = true;
	} else {
		processReceived = true;
	}
}

void loopSPI() {
	if (processReceived) {
#ifdef SPI_DEBUG
		Serial.print("<. ");
		Serial.println(SPDR, HEX);
#endif
		recvbufPos += 1;
		recvbufPos %= SPI_BUFFER_SIZE;

		lastReceivedAt = micros();

		// Signal transaction received
		digitalWrite(HANDSHAKE_PIN, HIGH);
		digitalWrite(HANDSHAKE_PIN, LOW);

		// End of message received, stop processing
		if (SPDR == STOP_WORD)
			processReceiveFinished = true;

		processReceived = false;
	}

	if (processReceiveFinished) {
		Serial.print("< ");
		for(int i = 0; i < recvbufPos; i++) {
			if (recvbuf[i] < 0x10) Serial.print("0");
			Serial.print (recvbuf[i], HEX);
			Serial.print(" ");
		}
		Serial.println("");

		Serial.print("{");
		Serial.print((char) recvbufPos);
		for(int i = 0; i < recvbufPos; i++) {
			Serial.print ((char)recvbuf[i]);
		}
		Serial.println("");

		recvbufPos = 0;
		processReceiveFinished = false;
		lastReceivedAt = 0;
	}

	if (processSent) {
		if (sendbuf[sendbufPos] == STOP_WORD) {
			// Finished sending
			sendData = false;
			processSent = false;
		} else {
			// Advance send buffer
			sendbufPos += 1;
			sendbufPos %= SPI_BUFFER_SIZE;

			// Set new char
			SPDR = sendbuf[sendbufPos];
#ifdef SPI_DEBUG
			Serial.print(">. ");
			Serial.println(sendbuf[sendbufPos], HEX);
#endif

			// Trigger send
			digitalWrite(HANDSHAKE_PIN, HIGH);

			processSent = false;
		}
	}

	if (lastReceivedAt > 0 && micros() - lastReceivedAt > RECEIVE_TIMEOUT_US) {
		processReceiveFinished = true;
		processReceived = false;
	}
}

char nibble2c(char c)
{
   if ((c>='0') && (c<='9'))
      return c-'0' ;
   if ((c>='A') && (c<='F'))
      return c+10-'A' ;
   if ((c>='a') && (c<='f'))
      return c+10-'a' ;
   return -1 ;
}

char hex2c(char c1, char c2)
{
   if(nibble2c(c2) >= 0)
     return nibble2c(c1)*16+nibble2c(c2) ;
   return nibble2c(c1) ;
}

String hex2str(char *data)
{
   String result = "" ;
   for (int i=0 ; data[i] == ' ' || nibble2c(data[i])>=0 ; i++)
   {
      if (data[i] == ' ') continue;
      result += hex2c(data[i],data[i+1]) ;
      if(nibble2c(data[i+1])>=0)
        i++ ;
   }
   return result;
}

void loop() {
	loopSPI();

	while (Serial.available()) {
		unsigned char c = Serial.read();
		if (binModeStarted) {
			if (binModeExpectedLength == 0) {
				serialbufPos = 0;
				memset(serialbuf, 0x00, SERIAL_BUFFER_SIZE);
				binModeExpectedLength = c;
			} else {
				// Receive char, end bin mode if length reached
				serialbuf[serialbufPos++] = c;
				if (serialbufPos == binModeExpectedLength) {
					send(serialbuf, serialbufPos);
					serialbufPos = 0;
					memset(serialbuf, 0x00, SERIAL_BUFFER_SIZE);
					binModeStarted = false;
					binModeExpectedLength = 0;
				}
			}
		} else {
			switch(c) {
				case ' ':
					continue;
				case '}':
					binModeStarted = true;
					break;
				case '\r':
					send(serialbuf, serialbufPos);

					nibble1 = 0x00;
					serialbufPos = 0;
					memset(serialbuf, 0x00, SERIAL_BUFFER_SIZE);
					break;

				default:
					if (nibble2c(c) != -1) {
						if (!nibble1)
							nibble1 = c;
						else {
							serialbuf[serialbufPos++] = hex2c(nibble1, c);
							nibble1 = 0x00;
						}
					}
					break;
			}
		}
	}
}

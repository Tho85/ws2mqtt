#include "Arduino.h"

#include "config.h"
#include "homeassistant.h"
#include "main.h"
#include "wifi.h"
#include "spi_uart.h"
#include "wisafe2_rx.h"
#include "wisafe2_tx.h"
#include "device_db.h"

unsigned long lastSIDMapRequestedAt = 0;

void setup() {
	Serial.begin(19200);
	delay(500);
	setupDeviceDB();
	setupSPI();
	setupWifi();
	setupMQTT();

	sendDiagnosticRequest();
	sendQuerySIDMap();
}

void loopUpdateSIDMap() {
	if (millis() - lastSIDMapRequestedAt > QUERY_NEW_DEVICES_INTERVAL * 1000) {
		lastSIDMapRequestedAt = millis();

		sendDiagnosticRequest();
		sendQuerySIDMap();
	}
}

void loop() {
	receiveSPIMessage(&handleRX);
	loopMQTT();

	loopUpdateSIDMap();
	loopTX();
}

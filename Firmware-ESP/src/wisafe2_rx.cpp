#include "Arduino.h"

#include "wisafe2_packets.h"
#include "wifi_inc.h"
#include "spi_uart.h"
#include "wisafe2_escape.h"
#include "wisafe2_rx.h"
#include "wisafe2_tx.h"
#include "device_db.h"

extern char ownSid;
extern PubSubClient mqttClient;

void handleAlarmEvent(uint8_t* msg, int length) {
	pkt_rx_event_alarm_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	Serial.printf("Alarm event received from device %d cmd %02x type %02x\n", (unsigned long)pkt.device_id, pkt.cmd, pkt.device_type);

	sendMQTTState(pkt.device_id, (char *)"alarm", true);
	sendMQTTBridgeEvent();
}

void handleAlarmOffEvent(uint8_t* msg, int length) {
	pkt_rx_event_alarm_off_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	Serial.printf("Alarm off event received from device %d cmd %02x type %02x\n", (unsigned long)pkt.device_id, pkt.cmd, pkt.device_type);

	sendMQTTState(pkt.device_id, (char *)"alarm", false);
}

void handleButtonEvent(uint8_t* msg, int length) {
	pkt_rx_event_button_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	Serial.printf("Button press received from device %d cmd %02x\n", (unsigned long)pkt.device_id, pkt.cmd);

	sendMQTTButtonEvent(pkt.device_id);
	sendMQTTBridgeEvent();
}

void handleError(uint8_t* msg, int length) {
	pkt_rx_error_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	Serial.printf("Error received from device %d cmd %02x error %02x\n", (unsigned long)pkt.device_id, pkt.cmd, pkt.error_flags);

	device_state_t ds;
	ds.sid = pkt.sid;
	ds.device_id = pkt.device_id;
	ds.generic_error = (pkt.error_flags & SPI_RX_ERROR_FLAG_GENERIC);
	ds.docked = (pkt.error_flags & SPI_RX_ERROR_FLAG_DOCKED);
	ds.sensor_battery = (pkt.error_flags & SPI_RX_ERROR_FLAG_SENSOR_BATTERY);
	ds.radio_module_battery = (pkt.error_flags & SPI_RX_ERROR_FLAG_RADIO_MODULE_BATTERY);

	sendMQTTDeviceState(ds);
	updateDeviceDBState(ds);
}

void handleUpdateSIDMapResponse(uint8_t* msg, int length) {
	pkt_rx_diagnostic_details_update_sidmap_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	uint64_t sidmap = pkt.sidmap;
	Serial.printf("Update SID Map received. SID map: %016x\n", sidmap);

	for (char sid = 0; sid < 64; sid++) {
		if ((sidmap >> sid) & 0x01) {
			Serial.printf(" Found SID %d\n", sid);
			checkIfDeviceKnown(sid);
		}
	}
}

void checkIfDeviceKnown(uint8_t sid) {
	if (sid == ownSid)
		return;

	device_t device;
	memset(&device, 0, sizeof(device));

	// Check if device is in cache
	getDeviceFromDB(sid, &device);
	if (device.device_type == 0x00) {
		Serial.println("Device not in DB, querying");
		sendQueryDiagnosticDetails(sid, SPI_TX_DIAGNOSTIC_DETAILS_ID);
	} else {
		char out[128];
		sprintf(out, "Device found, device_id: %d", device.device_id);
		Serial.println(out);
	}
}

// Response to D3 06 xx 00 7E
// => D4 06 xx ...
void handleUpdateRemoteStatusResponse(uint8_t* msg, int length) {
	pkt_rx_diagnostic_details_remote_status_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	Serial.printf("Received diagnostic response for: %d\n", pkt.device_id);

	// Set initial state for sensor. There is no way to query for a
	// currently ongoing alarm, so always assume alarm is off
	sendMQTTState(pkt.device_id, (char *)"alarm", false);
}

// Response to D3 06 xx 01 7E
void handleDiagnosticDetailsRemoteIdResponse(uint8_t* msg, int length) {
	pkt_rx_diagnostic_details_remote_id_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	Serial.printf("Received remote id for: %d\n", pkt.device_id);

	device_t device;
	device.device_id = pkt.device_id;
	device.model = pkt.model;
	device.device_type = pkt.device_type;
	device.sid = pkt.sid;

	announceMQTTDevice(device);
	insertDeviceDB(device);

	sendQueryDiagnosticDetails(pkt.sid, SPI_TX_DIAGNOSTIC_DETAILS_STATUS);
}

// Automatically sent when new device joins network
// => D4 09 xx ...
void handleNewDeviceResponse(uint8_t* msg, int length) {
	pkt_rx_diagnostic_details_new_device_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	Serial.printf("New device announced: %d sid %d\n", pkt.device_id, pkt.sid);
	// TODO: Better handle sid <-> device_id mismatch / conflict
	checkIfDeviceKnown(pkt.sid);
}

void handleDiagnosticResponse(uint8_t* msg, int length) {
	pkt_rx_diagnostic_response_t pkt;
	memcpy(&pkt, msg, sizeof(pkt));

	Serial.printf("Diagnostic response received\n");

	if (pkt.device_id == 0x00) {
		Serial.println("Module not initialized");
	}

	if (pkt.sid == 0x40) { // 64 -> higher than maximum SID -> device not connected?
		Serial.println("Module not connected");

		if (ownSid != 0x40) {
			// Device was connected before, so deannounce all
			// devices
			getAllDevicesFromDB(removeMQTTDevice);

			// Clear SQLite database
			resetDeviceDB();
		}
	}
	Serial.printf("Setting own sid to %d\n", pkt.sid);
	ownSid = pkt.sid;
}

void handleDiagnosticDetailsResponse(uint8_t* msg, int length) {
	switch(msg[1]) {
	case SPI_DIAGNOSTIC_DETAILS_UPDATE_SIDMAP:
		handleUpdateSIDMapResponse(msg, length);
		break;
	case SPI_DIAGNOSTIC_DETAILS_REMOTE_STATUS:
		handleUpdateRemoteStatusResponse(msg, length);
		break;
	case SPI_DIAGNOSTIC_DETAILS_NEW_DEVICE:
		handleNewDeviceResponse(msg, length);
		break;
	}
}

void handleRX(uint8_t* msg, int length) {
	length = unescapeMessage((uint8_t*)msg, length);

	switch(msg[0]) {
	case SPI_RX_REQUEST_WELCOME:
		sendWelcomeMsg();
		break;
	case SPI_RX_EVENT_ALARM:
		handleAlarmEvent(msg, length);
		break;
	case SPI_RX_EVENT_ALARM_OFF:
		handleAlarmOffEvent(msg, length);
		break;
	case SPI_RX_EVENT_BUTTON:
		handleButtonEvent(msg, length);
		break;
	case SPI_RX_ERROR:
		handleError(msg, length);
		break;
	case SPI_RX_DIAGNOSTIC_RESPONSE:
		handleDiagnosticResponse(msg, length);
		break;
	case SPI_RX_DIAGNOSTIC_DETAILS_RESPONSE:
		handleDiagnosticDetailsResponse(msg, length);
		break;
	case SPI_RX_DIAGNOSTIC_DETAILS_REMOTE_ID:
		handleDiagnosticDetailsRemoteIdResponse(msg, length);
		break;
	}
}

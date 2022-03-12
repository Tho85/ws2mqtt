#ifndef HOMEASSISTANT_H
#define HOMEASSISTANT_H 1

#include "ArduinoJson.h"
#include "wisafe2_packets.h"
#include "device_db.h"

#define HA_AUTODISCOVERY_PREFIX "homeassistant"

void announceAllDevices();
void publishAllCachedDeviceStates();

void announceMQTTBridge();
void announceMQTTBridgeEntities();
void announceMQTTBridgeButtonEntity(char* name, char* command, bool diagnostic);

void addBridgeDescription(JsonObject dev);
void sendMQTTBridgeEvent();

void announceMQTTDevice(device_t device);
void addDeviceDescription(JsonObject dev, device_t device);
void announceMQTTBinarySensor(
	device_t device,
	char* device_class,
	char* name,
	char* short_name,
	bool diagnostic
);
void sendMQTTState(uint32_t deviceid, char* short_name, bool state);
void sendMQTTDeviceState(device_state_t ds);
void announceMQTTButtonEvent(device_t device);
void sendMQTTButtonEvent(uint32_t deviceid);

void removeMQTTDevice(device_t device);
void removeMQTTBinarySensor(device_t device, char* short_name);
void removeMQTTButtonEvent(device_t device);

void handleMQTTCommand(char* command);

char* modelString(uint16_t model_id);
#endif

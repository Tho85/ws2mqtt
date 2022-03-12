#ifndef DEVICE_DB_H
#define DEVICE_DB_H 1

#include "wisafe2_packets.h"

typedef struct {
	uint8_t sid;
	uint32_t device_id:24;
	uint16_t model;
	uint8_t device_type;
} device_t;

typedef struct {
	uint8_t sid;
	uint32_t device_id:24;
	bool generic_error;
	bool docked;
	bool sensor_battery;
	bool radio_module_battery;
} device_state_t;

void resetDeviceDB();
void setupDeviceDB();
void connectDeviceDB();
void migrateDeviceDB();

void insertDeviceDB(device_t device);
void updateDeviceDBState(device_state_t ds);

// Get a single device from the database by sid
int getDeviceFromDB(uint8_t sid, device_t* device);

// Get all devices from database, and call callback function
int getAllDevicesFromDB(void (*callback)(device_t device));

// Get all device states from database, and call callback function
int getAllDeviceStatesFromDB(void (*callback)(device_state_t ds));
#endif

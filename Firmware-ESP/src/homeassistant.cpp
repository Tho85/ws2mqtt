#include "homeassistant.h"
#include "config.h"
#include "wifi.h"
#include "wisafe2_packets.h"
#include "wisafe2_tx.h"

extern PubSubClient mqttClient;

void announceAllDevices() {
	announceMQTTBridge();
	announceMQTTBridgeEntities();

	getAllDevicesFromDB(announceMQTTDevice);
}

void publishAllCachedDeviceStates() {
	getAllDeviceStatesFromDB(sendMQTTDeviceState);
}

void announceMQTTBridge() {
	StaticJsonDocument<500> doc;

	// Create device
	JsonObject dev = doc.createNestedObject("dev");
	addBridgeDescription(dev);

	doc["atype"] = "trigger";
	doc["type"] = "received";
	doc["subtype"] = "alarm";

	char eventtopic[200];
	sprintf(eventtopic, "ws2mqtt/bridge_%08x/event", DEVICE_ID);
	doc["t"] = eventtopic;
	doc["pl"] = "alarm";

	char msg[500];
	serializeJson(doc, msg, 500);

	if (mqttClient.connected()) {
		char topic[200];
		sprintf(topic, HA_AUTODISCOVERY_PREFIX"/device_automation/ws2mqtt_bridge_%08x/alarm/config", DEVICE_ID);
		mqttClient.publish(topic, msg, true);
	}
}

void announceMQTTBridgeEntities() {
	announceMQTTBridgeButtonEntity("WS2MQTT Test devices", "test_devices", false);
}

void announceMQTTBridgeButtonEntity(char* name, char* command, bool diagnostic) {
	StaticJsonDocument<500> doc;

	char avtytopic[200];
	sprintf(avtytopic, "ws2mqtt/bridge_%08x/state", DEVICE_ID);
	doc["avty_t"] = avtytopic;

	JsonObject dev = doc.createNestedObject("dev");
	addBridgeDescription(dev);

	doc["name"] = name;
	if (diagnostic)
		doc["entity_category"] = "diagnostic";
	char cmdtopic[200];
	sprintf(cmdtopic, "ws2mqtt/bridge_%08x/command", DEVICE_ID);
	doc["cmd_t"] = cmdtopic;
	doc["cmd_tpl"] = command;

	char uniq_id[128];
	sprintf(uniq_id, "ws2mqtt_bridge_command_%s", command);
	doc["uniq_id"] = uniq_id;

	char msg[500];
	serializeJson(doc, msg, 500);

	if (mqttClient.connected()) {
		char topic[200];
		sprintf(topic, HA_AUTODISCOVERY_PREFIX"/button/ws2mqtt_bridge_%08x/command_%s/config", DEVICE_ID, command);
		Serial.println(topic);
		mqttClient.publish(topic, msg, true);
	}
}

void addBridgeDescription(JsonObject dev) {
	char devid[32];
	sprintf(devid, "ws2mqtt_bridge_%08x", DEVICE_ID);
	dev["mf"] = "Tho85";
	dev["mdl"] = "WS2MQTT bridge";
	dev["ids"] = devid;

	char devname[128];
	sprintf(devname, "WS2MQTT bridge %08x", DEVICE_ID);
	dev["name"] = devname;
}

void sendMQTTBridgeEvent() {
	char eventtopic[200];
	sprintf(eventtopic, "ws2mqtt/bridge_%08x/event", DEVICE_ID);
	if (mqttClient.connected())
		mqttClient.publish(eventtopic, "alarm");
}

void announceMQTTDevice(device_t device) {
	switch(device.device_type) {
	case DEVICE_TYPE_CO:
		announceMQTTBinarySensor(device, (char *)"carbon_monoxide", (char *)"CO detector", (char *)"alarm", false);
		break;
	case DEVICE_TYPE_SMOKE:
		announceMQTTBinarySensor(device, (char *)"smoke", (char *)"Smoke detector", (char *)"alarm", false);
		break;
	default:
		announceMQTTBinarySensor(device, (char *)"safety", (char *)"Detector", (char *)"alarm", false);
		break;
	}
	announceMQTTBinarySensor(device, (char *)"battery", (char *)"Battery", (char *)"sensor_battery", true);
	announceMQTTBinarySensor(device, (char *)"battery", (char *)"Radio module battery", (char *)"radio_module_battery", true);
	announceMQTTBinarySensor(device, (char *)"plug", (char *)"Docked", (char *)"docked", true);
	announceMQTTBinarySensor(device, (char *)"problem", (char *)"Generic error", (char *)"generic_error", true);

	announceMQTTButtonEvent(device);
}

void addDeviceDescription(JsonObject dev, device_t device) {
	char devid[16];
	sprintf(devid, "ws2mqtt_%d", device.device_id);
	dev["ids"] = devid;

	char* manufacturer = (char *)"FireAngel";
	char* model = modelString(device.model);
	dev["mf"] = manufacturer;
	dev["mdl"] = model;

	char bridgeid[32];
	sprintf(bridgeid, "ws2mqtt_bridge_%08x", DEVICE_ID);
	dev["via_device"] = bridgeid;

	char devname[128];
	sprintf(devname, "%s %s %d", manufacturer, model, device.device_id);
	dev["name"] = devname;
}

void announceMQTTBinarySensor(
	device_t device,
	char* device_class,
	char* name,
	char* short_name,
	bool diagnostic
) {
	StaticJsonDocument<500> doc;

	char avtytopic[200];
	sprintf(avtytopic, "ws2mqtt/bridge_%08x/state", DEVICE_ID);
	doc["avty_t"] = avtytopic;

	JsonObject dev = doc.createNestedObject("dev");
	addDeviceDescription(dev, device);

	char sensor_name[128];
	char* model = modelString(device.model);
	sprintf(sensor_name, "%s %d %s", model, device.device_id, name);
	doc["name"] = sensor_name;

	doc["dev_cla"] = device_class;
	if (diagnostic)
		doc["entity_category"] = "diagnostic";

	char stat_t[128];
	sprintf(stat_t, "ws2mqtt/%d/%s/state", device.device_id, short_name);
	doc["stat_t"] = stat_t;

	char uniq_id[128];
	sprintf(uniq_id, "%d_%s_ws2mqtt", device.device_id, short_name);
	doc["uniq_id"] = uniq_id;

	char msg[500];
	serializeJson(doc, msg, 500);

	if (mqttClient.connected()) {
		char topic[200];
		sprintf(topic, HA_AUTODISCOVERY_PREFIX"/binary_sensor/ws2mqtt_%d/%s/config", device.device_id, short_name);
		Serial.println(topic);
		mqttClient.publish(topic, msg, true);
	}
}

void sendMQTTState(uint32_t device_id, char* short_name, bool state) {
	if (mqttClient.connected()) {
		char topic[200];
		sprintf(topic, "ws2mqtt/%d/%s/state", device_id, short_name);
		mqttClient.publish(topic, state ? "ON" : "OFF");
	}
}

void sendMQTTDeviceState(device_state_t ds) {
	sendMQTTState(ds.device_id, "generic_error", ds.generic_error);
	sendMQTTState(ds.device_id, "docked", ds.docked);
	sendMQTTState(ds.device_id, "sensor_battery", ds.sensor_battery);
	sendMQTTState(ds.device_id, "radio_module_battery", ds.radio_module_battery);
}

void announceMQTTButtonEvent(device_t device) {
	StaticJsonDocument<500> doc;

	// Create device
	JsonObject dev = doc.createNestedObject("dev");
	addDeviceDescription(dev, device);

	doc["atype"] = "trigger";
	doc["type"] = "button_short_press";
	doc["subtype"] = "button_1";

	char t[128];
	sprintf(t, "ws2mqtt/%d/event", device.device_id);
	doc["t"] = t;
	doc["pl"] = "test_button";

	char msg[500];
	serializeJson(doc, msg, 500);

	if (mqttClient.connected()) {
		char topic[200];
		sprintf(topic, HA_AUTODISCOVERY_PREFIX"/device_automation/ws2mqtt_%d/test_button/config", device.device_id);
		mqttClient.publish(topic, msg, true);
	}
}

void sendMQTTButtonEvent(uint32_t device_id) {
	if (mqttClient.connected()) {
		char topic[200];
		sprintf(topic, "ws2mqtt/%d/event", device_id);
		mqttClient.publish(topic, "test_button");
	}
}

void removeMQTTDevice(device_t device) {
	removeMQTTBinarySensor(device, (char *)"alarm");

	removeMQTTBinarySensor(device, (char *)"sensor_battery");
	removeMQTTBinarySensor(device, (char *)"radio_module_battery");
	removeMQTTBinarySensor(device, (char *)"docked");
	removeMQTTBinarySensor(device, (char *)"generic_error");

	removeMQTTButtonEvent(device);
}

void removeMQTTBinarySensor(device_t device, char* short_name) {
	if (mqttClient.connected()) {
		char topic[200];
		sprintf(topic, HA_AUTODISCOVERY_PREFIX"/binary_sensor/ws2mqtt_%d/%s/config", device.device_id, short_name);
		Serial.println(topic);
		mqttClient.publish(topic, NULL, true);
	}
}

void removeMQTTButtonEvent(device_t device) {
	if (mqttClient.connected()) {
		char topic[200];
		sprintf(topic, HA_AUTODISCOVERY_PREFIX"/device_automation/ws2mqtt_%d/test_button/config", device.device_id);
		mqttClient.publish(topic, NULL, true);
	}
}

void handleMQTTCommand(char* command) {
	if (strncmp(command, "test_devices", 12) == 0)
		sendTestButtonMsg();
}

char* modelString(uint16_t model_id) {
	switch(model_id) {
	case MODEL_ST630DE: return (char *)"ST-630-DE";
	case MODEL_FP2620W2: return (char *)"FP2620W2";
	case MODEL_WST630: return (char *)"WST630";
	case MODEL_W2CO10X: return (char *)"W2-CO-10X";
	case MODEL_W2SVP630: return (char *)"W2-SVP-630";
	case MODEL_FP1720W2R: return (char *)"FP1720W2-R";
	default: return (char *)"Unkown";
	}
}

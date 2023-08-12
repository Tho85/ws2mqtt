#include "config.h"
#include "wifi_inc.h"
#include "homeassistant.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned long lastReconnectAttempt = 0;

void setupWifi() {
	Serial.println("Connecting to Wifi");

	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PSK);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	randomSeed(micros());

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void setupMQTT() {
	mqttClient.setBufferSize(1024);
	mqttClient.setServer(MQTT_HOST, 1883);
	mqttClient.setCallback(mqttCallback);

	lastReconnectAttempt = 0;
}

boolean reconnectMQTT() {
	Serial.println("Trying to reconnect to MQTT...");
	char avtytopic[200];
	char cmdtopic[200];

	sprintf(avtytopic, "ws2mqtt/bridge_%08x/state", DEVICE_ID);
	sprintf(cmdtopic, "ws2mqtt/bridge_%08x/command", DEVICE_ID);
	if (mqttClient.connect("ws2mqtt", MQTT_USERNAME, MQTT_PASSWORD, avtytopic, 0, true, "offline")) {
		Serial.println("connected");

		mqttClient.publish(avtytopic, "online", true);
		mqttClient.subscribe(cmdtopic);
		announceAllDevices();
		publishAllCachedDeviceStates();
	} else {
		Serial.print("failed, rc=");
		Serial.print(mqttClient.state());
		Serial.println(" try again in 5 seconds");
	}
	return mqttClient.connected();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
	char payloadChar[length+1] = { 0 };
	char cmdtopic[200];

	memcpy(payloadChar, payload, length);
	sprintf(cmdtopic, "ws2mqtt/bridge_%08x/command", DEVICE_ID);
	if (strncmp(topic, cmdtopic, length) == 0) {
		Serial.printf("Command received: %s\n", payloadChar);
		handleMQTTCommand(payloadChar);
	}
}

void loopMQTT() {
	if (!mqttClient.connected()) {
		unsigned long now = millis();

		if (lastReconnectAttempt == 0 || now - lastReconnectAttempt > 5000) {
			lastReconnectAttempt = now;

			if (reconnectMQTT()) {
				lastReconnectAttempt = 0;
			}
		}
	} else
		mqttClient.loop();
}

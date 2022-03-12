#ifndef WISAFE_2_PACKETS_H
#define WISAFE_2_PACKETS_H 1

#pragma pack(push)
#pragma pack(1)

const uint8_t SPI_STOP_WORD = 0x7E;

const uint8_t DEVICE_TYPE_CO = 0x41;
const uint8_t DEVICE_TYPE_SMOKE = 0x81;
const uint8_t DEVICE_TYPE_ALL = 0xFF;

const uint16_t MODEL_WST630    = 0x0311;
const uint16_t MODEL_W2CO10X   = 0x0378;
const uint16_t MODEL_FP1720W2R = 0x0411;
const uint16_t MODEL_ST630DE   = 0x047C;
const uint16_t MODEL_W2SVP630  = 0x04C3;
const uint16_t MODEL_FP2620W2  = 0x08ED;

const uint8_t SPI_RX_REQUEST_WELCOME = 0x41;

const uint8_t SPI_RX_EVENT_ALARM = 0x50;
typedef struct {
	uint8_t cmd;
	uint32_t device_id:24;
	uint8_t device_type;
	uint8_t _unknown;
	uint8_t sid;
	uint8_t seq;
	uint8_t stop;
} pkt_rx_event_alarm_t;

const uint8_t SPI_RX_EVENT_ALARM_OFF = 0x51;
typedef struct {
	uint8_t cmd;
	uint32_t device_id:24;
	uint8_t device_type;
	uint8_t sid;
	uint8_t seq;
	uint8_t stop;
} pkt_rx_event_alarm_off_t;

// Incoming test message
const uint8_t SPI_RX_EVENT_BUTTON = 0x70;
typedef struct {
	uint8_t cmd;
	uint32_t device_id:24;
	uint8_t device_type;
	uint8_t _unknown;
	uint16_t model;
	uint8_t sid;
	uint8_t seq;
	uint8_t stop;
} pkt_rx_event_button_t;

// Outgoing test message
const uint8_t SPI_TX_EVENT_BUTTON = 0x70;
typedef struct {
	uint8_t cmd;
	uint32_t device_id:24;
	uint8_t device_type;
	uint8_t _unknown;
	uint16_t model;
	uint8_t stop;
} pkt_tx_event_button_t;

const uint8_t SPI_RX_ERROR = 0x71;
typedef struct {
	uint8_t cmd;
	uint32_t device_id:24;
	uint16_t model;
	uint8_t error_flags;
	uint8_t sid;
	uint8_t seq;
	uint8_t stop;
} pkt_rx_error_t;

const uint8_t SPI_RX_ERROR_FLAG_GENERIC = 0x02;
const uint8_t SPI_RX_ERROR_FLAG_DOCKED = 0x04;
const uint8_t SPI_RX_ERROR_FLAG_SENSOR_BATTERY = 0x08;
const uint8_t SPI_RX_ERROR_FLAG_RADIO_MODULE_BATTERY = 0x20;

const uint8_t SPI_TX_RESPONSE_WELCOME = 0x91;
typedef struct {
	uint8_t cmd;
	uint32_t device_id:24;
	uint16_t model;
	uint8_t device_type;
	uint8_t flags;
	uint16_t firmware;
	uint8_t stop;
} pkt_tx_welcome_t;

// Response to D3 06 xx 01 7E
const uint8_t SPI_RX_DIAGNOSTIC_DETAILS_REMOTE_ID = 0xC4;
typedef struct {
	uint8_t cmd;
	uint32_t device_id:24;
	uint8_t device_type;
	uint16_t model;
	uint8_t sid;
	uint8_t seq;
	uint8_t stop;
} pkt_rx_diagnostic_details_remote_id_t;

const uint8_t SPI_TX_DIAGNOSTIC_REQUEST = 0xD1;

const uint8_t SPI_RX_DIAGNOSTIC_RESPONSE = 0xD2;
typedef struct {
	uint8_t cmd;
	uint8_t battery1;
	uint8_t battery2;
	uint8_t _unknown1;
	uint8_t rssi;
	uint8_t firmware_version;
	uint32_t device_id:24;
	uint8_t flags;
	uint8_t radio_fault_count;
	uint8_t sid;
	uint8_t _unknown2;
	uint8_t stop;
} pkt_rx_diagnostic_response_t;

const uint8_t SPI_TX_DIAGNOSTIC_DETAILS_REQUEST = 0xD3;
typedef struct {
	uint8_t cmd;
	uint8_t subtype;
	uint8_t stop;
} pkt_tx_diagnostic_details_t;

typedef struct {
	uint8_t cmd;
	uint8_t subtype;
	uint8_t arg1;
	uint8_t stop;
} pkt_tx_diagnostic_details_1arg_t;

typedef struct {
	uint8_t cmd;
	uint8_t subtype;
	uint8_t arg1;
	uint8_t arg2;
	uint8_t stop;
} pkt_tx_diagnostic_details_2arg_t;

const uint8_t SPI_RX_DIAGNOSTIC_DETAILS_RESPONSE = 0xD4;

const uint8_t SPI_DIAGNOSTIC_DETAILS_UPDATE_SIDMAP = 0x03;
typedef struct {
	uint8_t cmd;
	uint8_t subtype_03;
	uint64_t sidmap;
	uint8_t stop;
} pkt_rx_diagnostic_details_update_sidmap_t;

const uint8_t SPI_DIAGNOSTIC_DETAILS_REMOTE_STATUS = 0x06;
const uint8_t SPI_TX_DIAGNOSTIC_DETAILS_STATUS = 0x00;
const uint8_t SPI_TX_DIAGNOSTIC_DETAILS_ID = 0x01;

// Response to D3 06 xx 00 7E
typedef struct {
	uint8_t cmd;
	uint8_t subtype_06;
	uint8_t sid;
	uint8_t battery1;
	uint8_t battery2;
	uint8_t _unknown1;
	uint8_t rssi;
	uint8_t firmware_version;
	uint32_t device_id:24;
	uint8_t flags;
	uint8_t radio_fault_count;
	uint8_t stop;
} pkt_rx_diagnostic_details_remote_status_t;

const uint8_t SPI_DIAGNOSTIC_DETAILS_NEW_DEVICE = 0x09;
typedef struct {
	uint8_t cmd;
	uint8_t subtype_09;
	uint8_t sid;
	uint32_t device_id:24;
	uint8_t stop;
} pkt_rx_diagnostic_details_new_device_t;

#pragma pack(pop)
#endif

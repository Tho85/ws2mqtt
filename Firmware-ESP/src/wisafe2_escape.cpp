#include "Arduino.h"
#include "wisafe2_escape.h"
#include "wisafe2_tx.h"

unsigned char escapeMessage(uint8_t* msg, unsigned char length) {
	unsigned char outidx = 0;
	uint8_t input[TXBUF_SIZE];
	memcpy(&input, msg, length);

	for (int i = 0; i < length; i++) {
		if (input[i] == 0x7D) {
			msg[outidx++] = 0x7D;
			if (outidx == TXBUF_SIZE) break;
			msg[outidx++] = 0x02;
		} else if (i < length - 1 && input[i] == 0x7E) {
			msg[outidx++] = 0x7D;
			if (outidx == TXBUF_SIZE) break;
			msg[outidx++] = 0x01;
		} else
			msg[outidx++] = input[i];

		if (outidx == TXBUF_SIZE) break;
	}

	return outidx;
}

unsigned char unescapeMessage(uint8_t* msg, unsigned char length) {
	unsigned char outidx = 0;

	for (int i = 0; i < length; i++) {
		uint8_t c = msg[i];
		if (c == 0x7D)
			c = (msg[++i] == 0x01 ? 0x7E : 0x7D);
		msg[outidx++] = c;
	}

	return outidx;
}

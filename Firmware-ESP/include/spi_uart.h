#ifndef SPI_UART_H
#define SPI_UART_H 1

void setupSPI();
void sendSPIMessage(uint8_t* msg);
void sendSPIMessage(uint8_t* msg, int length);
void receiveSPIMessage(void (*rxCallback)(uint8_t*, int));
#endif

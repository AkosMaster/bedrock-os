#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

unsigned char port_byte_in (uint16_t port);
void port_byte_out (uint16_t port, uint8_t data);
unsigned short port_word_in (uint16_t port);
void port_word_out (uint16_t port, uint16_t data);

void port_long_ins (uint16_t port, uint32_t* buffer, uint32_t length);
void port_long_outs (uint16_t port, uint32_t* buffer, uint32_t length);

#endif

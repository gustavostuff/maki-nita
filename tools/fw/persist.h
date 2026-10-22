#ifndef FW_PERSIST_H
#define FW_PERSIST_H

#include <stdint.h>

/*
 * One 16-bit word at the start of the EEPROM image.
 * An erased image reads as 0xFFFF.
 */

uint16_t persist_read(void);
void persist_write(uint16_t value);

#endif

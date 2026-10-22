#include "persist.h"

#include <avr/eeprom.h>

/* Word 0 of the on-chip EEPROM. eeprom_update_word skips a matching cell. */

uint16_t persist_read(void)
{
    return eeprom_read_word((const uint16_t *)0);
}

void persist_write(uint16_t value)
{
    eeprom_update_word((uint16_t *)0, value);
}

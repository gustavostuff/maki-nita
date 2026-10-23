#ifndef FW_WIRE_H
#define FW_WIRE_H

/*
 * EVENT, sampled at 40 kHz. A 0 is a low pulse of about 100 us.
 * A 1 is a low pulse of about 200 us. The line is high for about
 * 100 us between pulses. The byte is least significant bit first.
 * wire_take returns the next byte, or -1 when none is waiting.
 */

#define WIRE_RATE 40000

void wire_reset(void);
void wire_sample(int high);
int wire_take(void);

#endif

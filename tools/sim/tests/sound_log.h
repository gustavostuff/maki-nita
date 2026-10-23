#ifndef TEST_SOUND_LOG_H
#define TEST_SOUND_LOG_H

#include <stdint.h>

void sound_log_reset(void);
int sound_log_count(void);
uint8_t sound_log_at(int i);

#endif

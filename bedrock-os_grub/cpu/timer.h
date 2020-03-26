#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_FREQ 50

void sleep(uint32_t milsec);
void init_timer(uint32_t freq);

#endif

#ifndef RD_HOST_H
#define RD_HOST_H

#include "board.h"

/* Point the shared flush at this board before drivers_init or drivers_flush. */
void sim_bind(Board *board);

#endif

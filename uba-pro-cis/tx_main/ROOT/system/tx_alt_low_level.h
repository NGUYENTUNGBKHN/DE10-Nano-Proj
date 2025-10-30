#ifndef TX_ALT_LOW_LEVEL_H
#define TX_ALT_LOW_LEVEL_H

#include "alt_interrupt.h"
#include "alt_timers.h"

#include "debug.h"
/* use private timer */


void tx_alt_initialize_low_level(void);		/* system initialization */
void tx_alt_cache_purge_all(void);		// L1/L2キャッシュパージ, 20/07/28

#endif

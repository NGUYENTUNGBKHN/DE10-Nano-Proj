/*
 * limit_table.c
 *
 *  Created on: 2018/02/27
 *      Author: suzuki-hiroyuki
 */
#ifndef SIMURATION
#include "kernel.h"
#include "kernel_inc.h"
#else
#define EXT
#include "../common/global.h"
#endif

#include "common.h"
#include "country_custom.h"

#ifndef SIMURATION
#define EXT
#include "com_ram.c"
#else
#include "com_ram.h"
#endif


/*--- COMPARE BASE WIDE TABLE ---**/
const u16 banknote_length_min  = 110 - 10;		/*	LENGTH MIN */
const u16 banknote_length_max  = 180 + 10;	 	/*	LENGTH MAX */

/* EOF */

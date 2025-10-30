/*
 * pl.h
 *
 *  Created on: 2018/02/07
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_INCLUDE_PL_H_
#define SRC_INCLUDE_PL_H_

#include "jcm_typedef.h"
#include "fpga.h"

int initialize_pl(void);
int enable_pl(int enable);
int get_pl_state(void);
u8 _pl_ioex_reset(u8 set);

#endif /* SRC_INCLUDE_PL_H_ */

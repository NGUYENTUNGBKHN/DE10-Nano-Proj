/*
 * crc.h
 *
 *  Created on: 2018/01/31
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_CRC_H_
#define SRC_CRC_H_

#include "typedefine.h"

// Calculate CRC
u16 _calc_crc(u8 *data, u32 length, bool erase);
u16 _calc_crc_initial_value(u8 *data, u32 length, u16 initial);
u32 _calc_crc32(u32 start_adr, u32 end_adr, u32 value);

#endif /* SRC_CRC_H_ */

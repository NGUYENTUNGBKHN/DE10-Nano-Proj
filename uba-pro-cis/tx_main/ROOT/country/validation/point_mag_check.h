#ifndef _POINT_MAG_CHECK_H_
#define _POINT_MAG_CHECK_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	void create_mag_wave(u8 buf_num, u8 way);
	u16 mag_amount_calc(mag_area *pos, u8 *mag_ptr, u16 bill_len);
	u8 mag_amount_chk_invalid_count(u8 buf_num, u32 denomination, u32 way);
	u8 create_mag_wave_test(u8 buf_num, u8 way);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //_POINT_MAG_CHECK_H_

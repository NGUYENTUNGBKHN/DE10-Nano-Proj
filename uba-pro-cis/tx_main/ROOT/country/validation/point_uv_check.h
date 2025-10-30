#ifndef _POINT_UV__CHECK_H_
#define _POINT_UV__CHECK_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	void create_uv_wave(u8 buf_num, u8 way);
	u8 get_mount_data_point_uv(u16 start, u16 end, u8* uv_ptr, u8 division);
	u8 get_point_uv_invalid_count(u8 buf_num, u32 denomination, u32 way);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //_POINT_UV__CHECK_H_
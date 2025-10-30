#ifndef	_METALTHREAD_INCLUDE_H
#define	_METALTHREAD_INCLUDE_H

#define NEW_THREAD	//コメントアウトでVer1.3になる

#ifdef NEW_THREAD
/****************************************************************************/
/**
 * @file  thread.h ver2.4
 * @brief 
 * @date 2022/6/7
 */
/****************************************************************************/

#define THREAD_PARA_COUNT 4		//スレッドパラメータ記録個数
#define THRAED_SUB_MEMO 256		//副走査方向記録個数
#define THRAED_MAIN_MEMO 64		//主走査方向記録個数
#define THREAD_INI 4095			//0x0fff
#define THREAD_MASK_LIMIT 64	//マスク最大値

#define THREAD_MAG_MID 514 

#define THREAD_TYPE_MAG 0
#define THREAD_TYPE_METAL 1

#define THREAD_LOGI_X 4
#define THREAD_LOGI_Y 4

#define THREAD_CAPACITANCE_OK 0x40		//64D	OK
#define THREAD_NOT_CAPACITANCE 0x20		//32D	静電無
#define THREAD_NOT_VERTICAL 0x10		//16D	
#define THREAD_WIDTH_OVER 0x08
#define THREAD_MAG_LESS 0x04
#define THREAD_NOT_ONE 0x02
#define THREAD_NOT_FIND 0x01

#define THREAD_MAG_EFFECT_RANGE_ERR 0x01


typedef struct
{
	u8 type;
	u8 x_step;
	u8 y_step;
	u8 logi_y_step;
	u8 logi_x_step;
	u8 y_margin;
	u8 restart_margin;
	u8 t_plane;

	s16 x_start;
	s16 x_end;

	u8 standard_lack_rate;	//基準レベル：欠損率
	u8 min_lack_rate;		//基準レベル：最小欠損率
	u8 search_tir;
	u8 tir_cheack;

	u8 percent_cheak;
	u8 outlier_count;
	u8 outlier_dev;
	u8 wid_limit;
	float mag_cheak;
	u8 mask_len;

	//結果
	u8 remain_percent;
	u8 thread_num;
	s8 wid;//20211202変更
	float res_mag_max;

	s16 thread_center;
	s16 lx_ave;
	s16 rx_ave;
	s16 result;

	u16 pre_percent;
	s16 err_code;

	s8 judge;
	u8 level;
	u8 test;
	u8 padding1;

}ST_THREAD;

typedef struct
{
	ST_SPOINT spt;
	u16	filter_size_x;
	u16	filter_size_y;
	s16 output;
	u8 max;
	u8 min;
	u8 num;
	u8 end;
    u8 padding1[2];

} ST_THEAD_SEARCH_PARA;

typedef struct
{
	s16 x_start;
	s16 x_end;
	s16 center;
	u16 max_remain;

}ST_THREAD_PARA;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	
	
	s16 thread(u8 buf, ST_THREAD* thread);
	s16	thread_dt_get(ST_SPOINT* spt, s16 xx, s16 yy, u8 buf);
	s16	thread_mag_get(ST_SPOINT* spt, s16 xx, s16 yy, u8 buf);

	s16  thread_search(ST_THREAD* thr, ST_THREAD_PARA* para, s16 x_start, s16 x_end, s16 y_start, s16 y_end, u8 buf);
	s16 thread_detail_search(ST_THREAD* thr, ST_THREAD_PARA* para, s16 y_start, s16 y_end, u8 thread_num, u8 buf);
	s16 thread_mag_cheak(ST_THREAD* thr, ST_THREAD_PARA* para, s16 y_start, s16 y_end, s16 x_start, s16 x_end, u8 buf);
	
	s16 point_vicinity_cal_min(ST_THEAD_SEARCH_PARA *pv, s16 xxx, s16 yyy, u8 buf_num);
	s32 thread_outlier_cheak(s16* xx, s32 sum, s16 len, u8 outlier_count, u16 outlier_dev);
	
	s16 thread_outline_cheak_setup(u8 skip, u8 buf, s16* x_current, s16 x_end, s16* y_start, s16* y_end);
	s16 thread_outline_cheak(ST_SPOINT* spt, u8 buf);

	int thread_compare_int(const void *a, const void *b);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#else	//ver1.36
/****************************************************************************/
/**
 * @file  thread.h ver1.33
 * @brief
 * @date 2020/10/13
 */
 /****************************************************************************/


#define THREAD_DEV 32
#define THREAD_MAGNECTIC_CHECK_POINT 9
#define THREAD_CHECK_SKIP_PULS 4
#define THREAD_CHECK_SKIP_MINUS -4
#define THREAD_TILT_SKIP 4
#define THREAD_MAG_OFFSET_DT 128
#define THREAD_INI 4095	//0x0fff

#define THREAD_TYPE_MAG 0
#define THREAD_TYPE_METAL 1
//#define THREAD_TYPE_OTHER 2

#define THREAD_MAG_SEARCH_FAILURE 0x20
#define THREAD_MAG_LESS_THRESHOLD 0x10
#define THREAD_TIR_LESS_THRESHOLD 0x08
#define THREAD_NOT_STRAIGHT	0x04
#define THREAD_MAGNETIC 0x02
#define THREAD_NOTHR 0x01


typedef struct
{
	u8 type;
	u8 x_step;//
	u8 y_step;//
	u8 y_margin;
	u8 restart_margin;
	s16 x_start;
	s16 x_end;
	s16 diff_plane;

	u8 search_tirthr;
	u8 search_magthr;
	u8 search_diffthr;
	u8 search_lack_dt_thr;
	u8 magnectic_check_dt;
	u8 standard_lack_rate;	//基準レベル：欠損率
	u8 min_lack_rate;		//基準レベル：最小欠損率

	//判定値
	float tir_ave_thr;	//閾値　以下でスレッドあり
	float mag_dev_thr;	//閾値　以上でスレッドあり
	float right_dev_thr;
	float left_dev_thr;

	//結果
	float res_tir_ave;
	float res_mag_dev;
	u16 res_lack_len;
	u8 lack_per_len;
	s8 judge;
	s16 err_code;
	u8 level;

	//
	float res_mag_ave;
	u16 thread_wid;
	s16 thread_center;
	float left_dev;
	float right_dev;

}ST_THREAD;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16 thread(u8 buf, ST_THREAD* thread);

	s16 thread_ysearch(ST_THREAD* thread, u8 buf, s16 xx, s16 y_start, s16 y_end);
	s16 thread_xsearch(ST_THREAD* thread, u8 buf, s16 x_start, s16 x_end, s16* y_point, s16* x_point);
	float thread_xavecheck(s16* ex, s16* ave, u8* count);

	s16 thread_outline_cheak_setup(u8 skip, u8 buf, s16* x_current, s16 x_end, s16* y_start, s16* y_end);
	s16 thread_outline_cheak(ST_SPOINT* spt, u8 buf);
	s16 thread_magnetic_material_check(s16 xx, s16* yy, u8 buf, u8 dt);

	s16	thread_dt_check(ST_SPOINT* spt, s16 xx, s16 yy, u8 buf);
	u8 thread_center_dt_check(ST_SPOINT* spt, s16 xx, s16 yy, u8 buf);
	u8	thread_center_magdt_check(ST_SPOINT* mag, s16 xx, s16 yy, u8 buf);
	s16	thread_center_diffdt_check(ST_SPOINT* spt, ST_SPOINT* diff, s16 xx, s16 yy, u8 buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif // NEW_THREAD

#endif


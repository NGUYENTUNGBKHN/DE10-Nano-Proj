#ifndef	_NEOMAG_INCLUDE_H
#define	_NEOMAG_INCLUDE_H

/****************************************************************************/
/**
 * @file	neomag.h ver1.32
 * @brief 
 * @date	2020/10/12
 */
/****************************************************************************/

#define NEOMAG_MAX_COUNT 5
#define NEOMAG_Y_LINE_BUFF 32

#define NEOMAG_SKIP 3
#define NEOMAG_SPLIT 9
#define NEOMAG_Y_OUT -1
#define NEOMAG_X_OUT 1
#define NEOMAG_X_FINISH 1

//
#define NEOMAG_MAG_LESS_THRESHOLD 0x01
#define NEOMAG_IR1_LESS_THRESHOLD 0x02
#define NEOMAG_IR2_LESS_THRESHOLD 0x04
#define NEOMAG_MAG_ABNORMALY 0x08
#define NEOMAG_IR_ABNORMALY 0x10


//judge
#define NEOMAG_OK 0
#define NEOMAG_NO 1
#define NEOMAG_ERR 2

#define ERR_NEOMAG_LGET_FAILURE 0x0001
#define ERR_NEOMAG_SEARCH_FAILURE 0x0002

typedef struct
{
	float magthr;
	float ir1thr;
	float ir2thr;
	s16 x1;
	s16 y1;
	s16 x2;
	s16 y2;


}ST_NEOMAG_PARA;

typedef struct
{
	u8 step;
	u8 num;
	u8 split_mag_thr;
	u8 stain_ir1_thr;
	u8 stain_ir2_thr;
	u8 split_point_thr;
	u16 stain_raito_thr;

	//Œ‹‰Ê
	float mag_dev[NEOMAG_MAX_COUNT];
	float ir1_ave[NEOMAG_MAX_COUNT];
	float ir2_ave[NEOMAG_MAX_COUNT];
	u16 split_point[NEOMAG_MAX_COUNT];
	s8 result[NEOMAG_MAX_COUNT];
	s8 judge;
	
	u16 ir1_below[NEOMAG_MAX_COUNT];
	u16 ir2_below[NEOMAG_MAX_COUNT];

	u16 count;
	s16 err_code;

	ST_NEOMAG_PARA para[NEOMAG_MAX_COUNT];

}ST_NEOMAG;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16 neomag(u8 buf, ST_NEOMAG* neomag);

	s16 neomag_search(ST_NEOMAG* neomag, u8 buf,s16 xx1,s16 xx2, s16 yy1, s16 yy2, u8 num);
	s16 neomag_outline_cheak_setup(u8 skip, u8 buf, s16* x_current, s16 x_end, s16* y_start, s16* y_end);
	s16 neomag_outline_cheak(ST_SPOINT* spt, u8 buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*  */

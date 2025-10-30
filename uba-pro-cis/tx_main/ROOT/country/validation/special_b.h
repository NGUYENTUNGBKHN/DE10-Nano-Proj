//special_b.h
#ifndef _SPECIAL_B_H
#define _SPECIAL_B_H

#define SPB_0 337
#define SPB_1 431
#define SPB_2 525
#define SPB_3 1093
#define SPB_4 1187
#define SPB_5 1281

#define SPB_OK 0
#define SPB_NG 1
#define SPB_HIS 64
#define SPB_NUM 6
#define SPB_INI -1

#define SPB_LEFT_OVER -41
#define SPB_RIGHT_OVER -42
#define SPB_AREA_IN -43
#define SPB_OUT_PIX -44

typedef struct 
{
	float thr;
	float coef1;
	float coef2;
	float intercept;
	s16 xx[2];

	s8 margin;
	u8 padding;
	s16 judge;

	float total_ave;
	float total_dev;
	float predict;

	s16 ys[SPB_NUM];
	s16 ye[SPB_NUM];

}ST_SPECIAL_B;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16 special_b(u8 buf_num, ST_SPECIAL_B* spb);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SPECIAL_B_H */


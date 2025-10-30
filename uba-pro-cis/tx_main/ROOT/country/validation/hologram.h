#ifndef _HOLOGRAM_H
#define _HOLOGRAM_H

/****************************************************************************/
/**
* @file	hologram.h
* @brief	�z���O�������m�@ver1.3
* @date	2023/01/17
*/
/****************************************************************************/

/*

*/

#define HOLO_NUM_LIMIT 5
#define HOLO_HIS_LIMIT 256

#define HOLO_OK 0
#define HOLO_NG 1

typedef struct
{
	float tirthr;
	u16 heid;
	u16 wid;
	s16 x1;
	s16 y1;
	s16 x2;
	s16 y2;
	s8 diff_plane;
	u8 search_type;
	u8 search_in_tirthr;
	u8 search_out_tirthr;

}ST_HOLOGRAM_PARA;

typedef struct
{
	//para
	ST_HOLOGRAM_PARA para[HOLO_NUM_LIMIT];

	u8 num;
	u8 margin;
	u8 search_area_step;
	u8 sumhisper_thr;

	u8 sumhisnum_thr;
	s8 outline_flag;

	//����
	u16 count;	//���m��
	float tir_ave;	//TIR���ϒl
	s16 result;	//����
	s16 judge;	//����
	u8 sumhiscount_per_totalcount;	//
	u8 level;
	s16 err_code;


}ST_HOLOGRAM;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16 hologram(u8 buf_num, ST_HOLOGRAM* holo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HOLOGRAM_H */

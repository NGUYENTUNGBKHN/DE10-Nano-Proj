/****************************************************************************/
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* �{�\�t�g�E�F�A�Ɋ܂܂��\�[�X�R�[�h�ɂ͓��{���K�@�B������ЌŗL��       */
/* ��Ƌ@�����܂�ł��܂��B                                               */
/* �閧�ێ��_�񖳂��Ƀ\�t�g�E�F�A�Ƃ����Ɋ܂܂����̑S�̂������͈ꕔ��   */
/* ���J���������s���܂���B                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
* MODEL NAME : ���ʋ��L
* @file special.h
* @brief  TEMPLATE�̃w�b�_�t�@�C���ł��B
* @date	2020/12/28
* @author JCM. OSAKA TECHNICAL RESARCH 1 GROUP ELEMENTAL TECHNOLOGY RECERCH DEPT.
*/
/****************************************************************************/

/*
	ver1.0
		����
 */

#ifndef _SPECIAL_A_H
#define _SPECIAL_A_H

#define SP_A0 101
#define SP_A1 195
#define SP_A2 290
#define SP_A3 384
#define SP_A4 479
#define SP_A5 573
#define SP_A6 668
#define SP_A7 762
#define SP_A8 857
#define SP_A9 951
#define SP_A10 1046
#define SP_A11 1140
#define SP_A12 1235
#define SP_A13 1329
#define SP_A14 1424
#define SP_A15 1518

#define SPA_NUM_LIMIT 5
#define SPA_INI_DISTANCE 1000
#define SPA_OK 0	//����킵���̂�SPB�ƍ��킹�� furuta 22/10/18
#define SPA_NG 1

typedef struct
{
	s16 x;
	s16 y;
	s16 thr;
	s16 yobi;

}ST_SPECIAL_A_PARA;

typedef struct 
{
	//���̓p�����^
	ST_SPECIAL_A_PARA para[SPA_NUM_LIMIT];
	u8 num;	//����A�|�C���g�ݒ�
	
	u8 total_spa_thr;	//
	u16 x_dis_thr;	//
	u16 y_dis_thr;	//
	u16 dis_thr;//

	//�o��
	u16 res_sum[SPA_NUM_LIMIT];
	u8 res_count[SPA_NUM_LIMIT];
	s8 judge;
	u16 res_total_sum;
	
	
	
}ST_SPECIAL_A;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16 special_a(u8 buf, ST_SPECIAL_A* spa);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SPECIAL_A_H */


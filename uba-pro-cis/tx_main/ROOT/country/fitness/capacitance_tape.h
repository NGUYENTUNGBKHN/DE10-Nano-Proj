/****************************************************************************/
/*                                                                          */
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
 * @file capacitance_tape.h
 * @brief�@�Ód�e�ʃe�[�v���m
 * @date 2020/06/29 Created.
 */
/****************************************************************************/

#ifndef	_Cap_Tp_H
#define	_Cap_Tp_H

//�R�[�h�ǉ����Ă���
#define RES_ERR						(0x0010)	//�G���[
#define RES_SUB_SCANLINE_SHORAGE	(0x0010)	//�����������̃f�[�^�����s�����Ă���B

#define CAP_TAPE_MAX_ELEMENT_COUNT	(5000)		//�ő�v�f��


enum karnel_type
{
	cap_rbf = 0x0,
	cap_sigmoid,
	cap_polynomial,
	cap_liner,
};

typedef struct
{
	s16 start_x;
	s16 start_y;
	s16 end_x;
	s16 end_y;

} ST_CAPACITANCE_TAPE_REFERENCE_AREA;


typedef struct
{
	//����
	u8  black_correction_s;		                                //���␳�v�Z���C���J�n
	u8	black_correction_e;		                                //���␳�v�Z�I��
	u8  reference_area_num;		                                //�Q�ƃG���A��
	u8	padding;

	float tape_judg_thr;		                                //�e�[�v����臒l���x��
	u16 x_interval;				                                //x���W�̊Ԉ����̊Ԋu
	u16 y_interval;				                                //y���W�̊Ԉ����̊Ԋu
	u32 first_thrs_num;			                                //���臒l�z��̗v�f��
	float *p_first_thrs;								        //���臒l�z��
	ST_CAPACITANCE_TAPE_REFERENCE_AREA *p_reference_area;		//�Q�ƃG���A�\���̂̃I�t�Z�b�g

	//����
	float input_data[CAP_TAPE_MAX_ELEMENT_COUNT];		//���̓f�[�^
	float diff_data[CAP_TAPE_MAX_ELEMENT_COUNT];		//�����f�[�^
	//float thrs_data[CAP_TAPE_MAX_ELEMENT_COUNT];		//�����f�[�^
	float tape_total_area;
	float bc_val[128];			//���␳�l


	//�o��
	u8 level;					//���x��
	u8 result;					//����
	u16 err_code;				//�G���[�R�[�h
	
} ST_CAPACITANCE_TAPE;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	capacitance_tape(u8 buf_num ,ST_CAPACITANCE_TAPE *st);
	//float get_median(u16 ary[], s32 num);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _Cap_Tp_H */

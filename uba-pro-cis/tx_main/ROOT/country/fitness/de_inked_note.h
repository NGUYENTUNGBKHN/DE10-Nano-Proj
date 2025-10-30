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
 * @file de_inkde_note.h
 * @brief�@�E�F���m
 * @date 2019 Created.
 */
/****************************************************************************/

#ifndef	_Di_H
#define	_Di_H

#define DEINK_MAX_AREA 5	
#define DEINK_MAX_PLANE 3	

typedef struct
{
	u8 plane[DEINK_MAX_PLANE];
	u8 plane_num;

	s16 start_x;			//��f�̎�͈͂̍���ƉE���̍��W
	s16 start_y;
	s16 end_x;
	s16 end_y;
	u8 sampling_pich_x;		//�͈͓����T���v�����O����s�b�`
	u8 sampling_pich_y;		//�͈͓����T���v�����O����s�b�`
	u8 padding[2];
	float bias;					//�ؕ�
	float weight[3];			//�d��
	float uf_1_level_num;	//���ʂ�uf�x�N�g���̋���/50�̒l�i���x���P�j
	float st_1_level_num;	//���ʂ�st�x�N�g���̋���/50�̒l�i���x���P�j

} ST_DEINK_EACH_AREA_PARAMS;

typedef struct
{
	//����

	//u16 input_node_num;		//�m�[�h��
	//u16 hidden_node_num;
	//u16 output_node_num;

	//double *phidden_weight;	//�d��
	//double *pout_weight;
	//double *p_weight;			//�d��
	//double bias;					//�ؕ�

	//s16 start_x;			//��f�̎�͈͂̍���ƉE���̍��W
	//s16 start_y;
	//s16 end_x;
	//s16 end_y;
	//s16 sampling_pich;		//�͈͓����T���v�����O����s�b�`

	//ST_DEINK_EACH_AREA_PARAMS params[DEINK_MAX_AREA];
	ST_DEINK_EACH_AREA_PARAMS *params;
	u8 area_num;

	u8 comparison_method;	//���r���@ 0�F�������@1�Fa11
	u8	select_color_mode;				//�F��ԃ��f���̐ݒ� 0�FRGB�@1�FHSV
	u8	padding;

	//���ԏ��
	float res_out_put_val[5];	//�v�Z����
	float total[DEINK_MAX_AREA][DEINK_MAX_PLANE];
	float distance[5];	//�_�ƕ��ʂ̋���
	u8	level[5];		//���x��
	u8 avg_val[5][3];			//RGB�̕��ϒl

	//�o��
	//double fitness_out_put_val;		//�^���̔��Βl
	//double de_ink_out_put_val;	//�U���̔��Βl
	u8 res_judge;			//���茋��
	u8 uf_count;			//uf�ƂƔ��肳�ꂽ�G���A�̐�
	u16 output_level;		//�o�͂��ꂽ���x��



} ST_DEINKED;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	de_inked_note(u8 buf_num ,ST_DEINKED *st);
	u8  deink_level_dicision(float distance ,float level_fit,float level_uf);
	//s16 get_de_ink_histogram(ST_BS *pbs ,ST_NOTE_PARAMETER *pnote_param ,s16 start_x, s16 start_y ,s16 end_x ,s16 end_y ,s16 sampling_pich ,u8* plane, u8 plane_max_num ,double *out_put_val);
	//u8 de_ink_nn(double *histogram , double normaliza_num ,u16 input_num ,u16 hidden_num , u16 output_num, double *phidden_weight ,double *pout_weight ,double *fitness_out_put_val ,double *de_ink_out_put_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _Di_H */

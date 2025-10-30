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
 * @file soiling.h
 * @brief�@���ꌟ�m
 * @date 2019 Created.
 */
/****************************************************************************/

#ifndef	_SOILING_H
#define	_SOILING_H

#define SOILING_MAX_AREA 5	
#define SOILING_MAX_PLANE 3	

typedef struct
{
	u8 plane[SOILING_MAX_PLANE];
	u8 plane_num;

	s16 start_x;			//��f�̎�͈͂̍���ƉE���̍��W
	s16 start_y;
	s16 end_x;
	s16 end_y;
	u8 sampling_pich_x;		//�͈͓����T���v�����O����s�b�`
	u8 sampling_pich_y;		//�͈͓����T���v�����O����s�b�`
	u8 padding[2];
	float bias;				//�ؕ�
	float weight[3];			//�d��
	float uf_1_level_num;	//���ʂ�uf�x�N�g���̋���/50�̒l�i���x���P�j
	float st_1_level_num;	//���ʂ�st�x�N�g���̋���/50�̒l�i���x���P�j


} ST_SOILING_EACH_AREA_PARAMS;

typedef struct
{
	//����

	//u16 input_node_num;		//�m�[�h��
	//u16 hidden_node_num;
	//u16 output_node_num;

	//float *phidden_weight;	//�d��
	//float *pout_weight;
	//float *p_weight;			//�d��
	//float bias;					//�ؕ�

	//s16 start_x;			//��f�̎�͈͂̍���ƉE���̍��W
	//s16 start_y;
	//s16 end_x;
	//s16 end_y;
	//s16 sampling_pich;		//�͈͓����T���v�����O����s�b�`

	//ST_SOILING_EACH_AREA_PARAMS params[SOILING_MAX_AREA];	//�Q�ƃG���A�\����
	ST_SOILING_EACH_AREA_PARAMS *params;	//�Q�ƃG���A�\����
	u8 area_num;											//�Q�ƃG���A��
	//float st_1_level_num;	//fit���Q�ƕ��ʂƂ̋���	
	//float uf_1_level_num;

	u8 comparison_method;	//���r���@ 0�F�������@1�Fall
	u8	select_color_mode;				//�F��ԃ��f���̐ݒ� 0�FRGB�@1�FHSV
	u8 padding;

	//���ԏ��
	float res_out_put_val[5];	//�v�Z����
	float total[SOILING_MAX_AREA][SOILING_MAX_PLANE];
	float distance[5];	//�_�ƕ��ʂ̋���
	u8	level[5];		//���x��

	u8 avg_val[5][3];			//RGB�̕��ϒl
	//�o��
	//float fitness_out_put_val;		//�^���̔��Βl
	//float de_ink_out_put_val;	//�U���̔��Βl
	u8 res_judge;			//���茋��
	u8 uf_count;			//uf�ƂƔ��肳�ꂽ�G���A�̐�
	u16 output_level;		//�o�͂��ꂽ���x��

} ST_SOILING;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	soiling(u8 buf_num ,ST_SOILING *st);
	u8  stain_level_dicision(float distance ,float level_fit,float level_uf);
	//s16 get_soiling_histogram(ST_BS *pbs ,ST_NOTE_PARAMETER *pnote_param ,s16 start_x, s16 start_y ,s16 end_x ,s16 end_y ,s16 pich ,u8 *plane ,u8 plane_max_num ,float *out_put_val);
	//u8 soiling_nn(float *histogram , float normaliza_num ,u16 input_num ,u16 hidden_num , u16 output_num, float *phidden_weight ,float *pout_weight ,float *fitness_out_put_val ,float *de_ink_out_put_val);

	float min3(float x, float y, float z);
	void rgb2hsv(float* h, float* s, float* v);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SOILING_H */

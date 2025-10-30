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
 * @file dog_ear.h
 * @brief �p�܂ꌟ�m
 * @date 2018/06/29 Created.
 */
/****************************************************************************/

#ifndef	_DE_H
#define	_DE_H

#define DOG_EAR_LEARNING 1

//�p�����^
#define MAX_SEARCH_RANGE 80			//�T���|�C���g��
#define REV_MAX_NUM 10				//���̉񐔒��_�ɖ߂��Ă�����T�[�`�ł��؂�
#define VERTEX_POINT_NUM 4			//���_�̐�
#define DOG_EAR_TRI_VERTEX_NUM 2	//�p�܂�O�p�̒��_�̐�
#define SCAN_THR 250				//臒l:�}�̂Ɣw�i�����
#define X_Y 4						//���Ȃ��G���A�̍��W������
#define MAX_NOT_SEEN_AREA_COUNT 5	//���Ȃ��G���A�̍ő����






//�R�[�h�ǉ����Ă���
#define RES_OUT_OF_AREA		        0x0010	//�L�^�����G���A�������G���A�O
#define RES_SLOPE_OP		        0x0020	//���߂��X�����t�@(�j��Ɣ��肵�܂��B)
#define RES_COME_BACK_VERTER        0x0030	//���_�ɖ߂��Ă����񐔂��K��l�𒴂����B
#define RES_SLOPE_IS_ABNORMAL       0x0040	//�Εӂ��ߎ������ɉ����Ă��Ȃ�(�j��Ɣ��f���܂��B)
#define RES_HISTOGRAM_PEAKS_MATCH	0x0050	//�q�X�g�O�����̃s�[�N����v(�j��Ɣ��f���܂��B)
#define RES_POINT_COUNT_LESS		0x0060	//�L�^�ł����Εӂ̍��W��5�|�C���g�ȉ�(�����Ɣ��f����)
#define RES_HISTOGRAM_POINT_LESS	0x0070	//�q�X�g�O���������̍ی���Ȃ��G���A���Q�Ƃ���(�p�܂ꌔ�Ɣ��f����)



enum dog_ear
	{
		upper_left,
		lower_left,
		upper_right,
		lower_right,
};

enum dog_ear_status
    {
		NORMAL = 0x00,	//�p�܂�Ȃ�
		LITTLE_DOG_EAR,	//�p�܂�͂��邪����N���A
        DOG_EAR,		//�p�܂�
        TEAR,			//�j��


};

typedef struct
{
	//���ʁH
	u8 buf_num;						//�������̃o�b�t�@�ԍ����i�[����
	u8 plane;						//�p����v���[��
	u8 jcm_id;

	//����
	u8 comp_flg;					//�ǂ���̊�Ŕ�r���邩	1�F���ӂ̂Ȃ����@2�F�ʐρ{�Z��
	u8 scan_move_x;					//�����W���߂��
	u8 scan_move_x_thr_line;		//���_�{���̐�����臒l
	u8 scan_minimum_points;			//���̃|�C���g�ȉ��Ȃ�Ίp�܂�Ȃ�
	s8 tear_scan_renge;				//�j�ꌟ�m�̍ہA�ǂꂾ���T������Δj��Ɣ��f���邩
	u8 garbage_or_note_range;		//�|�C���g�̎�̍ۂԂ��������̂��}�̂����݂��ǂ����𒲂ׂ�͈�
	u8 scan_move_y;					//���_�����߂�Ƃ���y���W�̈ړ���
	u8 tear_scan_start_x;			//�􂯌��m�̍ہA�Εӂ����������邩
	u8 not_seen_area_count;			//���Ȃ��G���A�̐�
	s16 not_seen_areas[MAX_NOT_SEEN_AREA_COUNT][X_Y];			//����Ȃ��G���A�͈̔�

	u16 tear_scan_point_decide;		//�Ώ̎O�p�`�̃X�L�����G���A�����肷��@4���傫�Ȑ����ɂ��邱��

	u8 qp;								//�q�X�g�O�����ʎq���p�����^
	u8 peak_width;						//�q�X�g�O�����F�s�[�N�K�w��r��
	u8 peak_margin;					//��2�s�[�N��臒l
	u8 padding[3];					//��2�s�[�N��臒l


	float ele_pitch_x;				//�呖���f�q�s�b�`
	float ele_pitch_y;				//�������f�q�s�b�`
	u16 main_effective_range_max;	//�����G���A�͈̔�
	u16 main_effective_range_min;	//�����G���A�͈̔�

	float threshold_short_side_dot;		//�Z��̊�@�P��dot
	float threshold_long_side_dot;		//����̊�@�P��dot
	float threshold_short_side_mm;		//�Z��̊�@�P��mm
	float threshold_long_side_mm;		//����̊�@�P��mm
	u32 threshold_area;					//�ʐς̊

	//�o��
	u32	area[VERTEX_POINT_NUM];			//�p�܂�̖ʐ�
	u32	area_mm[VERTEX_POINT_NUM];		//�p�܂�̖ʐ� �P��mm
	float len_x[VERTEX_POINT_NUM];		//x�ӂ̒���
	float len_y[VERTEX_POINT_NUM];		//y�ӂ̒���
	float len_x_mm[VERTEX_POINT_NUM];	//�ӂ̒����@�P��mm
	float len_y_mm[VERTEX_POINT_NUM];	//�ӂ̒����@�P��mm
	float short_side[VERTEX_POINT_NUM];	//�p�܂�̒���
	float long_side[VERTEX_POINT_NUM];	//�p�܂�̒Z��
	float short_side_mm[VERTEX_POINT_NUM];//�p�܂�̒��� �P��mm
	float long_side_mm[VERTEX_POINT_NUM];	//�p�܂�̒Z�� �P��mm

	s16 triangle_vertex_1[VERTEX_POINT_NUM][DOG_EAR_TRI_VERTEX_NUM];	//�p�܂�O�p�`�̐܂ꏉ�߂̍��W���L�^����
																		//����_���炘���ω����Ă��钸�_����L�^����
	s16 triangle_vertex_2[VERTEX_POINT_NUM][DOG_EAR_TRI_VERTEX_NUM];	//�p�܂�O�p�`�̐܂ꏉ�߂̍��W���L�^����

	
	u8	judge[VERTEX_POINT_NUM];			//�e���_�̔��茋��
	u16 judge_reason[VERTEX_POINT_NUM];		//���ʂ̗��R
	float judge_pix[VERTEX_POINT_NUM];		//���ؗp
	u8	level;
	u8	padding1[3];


} ST_DOG_EAR;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	dog_ear(u8 buf_num ,ST_DOG_EAR *dog);
	u8 left_upper(s16 pver_x, s16 pver_y, s16 threshold, ST_BS *pbs ,ST_DOG_EAR *dog );
	u8 left_lower(s16 pver_x, s16 pver_y, s16 threshold, ST_BS *pbs  ,ST_DOG_EAR *dog);
	u8 right_upper(s16 pver_x, s16 pver_y, s16 threshold, ST_BS *pbs  ,ST_DOG_EAR *dog);
	u8 right_lower(s16 pver_x, s16 pver_y, s16 threshold, ST_BS *pbs  ,ST_DOG_EAR *dog);
	s16 cal_symmetric_center_point(s16 pver_a_x, s16 pver_a_y, s16 pver_b_x, s16 pver_b_y, s16 pver_c_x, s16 pver_ac_y, float slope ,float offset, ST_BS *pbs, u8 location ,ST_DOG_EAR *dog);
	void unit_convert(float ele_pitch_x, float ele_pitch_y ,ST_DOG_EAR *dog);
	s16 judge_dog_ear(float shot_side, float long_side ,u32 *area ,ST_DOG_EAR *dog);
	s16 triangle_inspection(ST_BS *pbs , float tan, float offset ,s16 input_a_x ,s16 input_b_x  ,s8 increment_dir_x ,ST_DOG_EAR *dog );
	void approx_line(s16 x[],s16 y[], s32 n, float *a0, float *a1);
	void ini_dog(ST_DOG_EAR *dog);

	s16	set_vertex(s16 *x ,s16 *y ,ST_SPOINT *spt ,ST_BS *pbs ,u16 vertex ,ST_NOTE_PARAMETER* pnote_param ,ST_DOG_EAR *dog);

	s16 judge_dogear_tear(ST_BS *pbs ,u8 plane ,double sym_trai_cen_x ,double sym_trai_cen_y ,s16 ver_b_x ,s16 ver_c_y
		,s8 increment_y ,s8 increment_x ,double sym_slope1 ,double sym_y_ofs1 ,double sym_slope2 ,double sym_y_ofs2
		,s16 scan_out_a ,s16 scan_out_b ,s16 scan_x_range , s16 scan_x_start ,s16 scan_y_start, s16 scan_y_range ,u8 location ,ST_DOG_EAR *dog);

	s16 dog_ear_level_indicator(ST_DOG_EAR *dog);

	s32 area_calculation(float *x , float *y , u16 point_count ,ST_DOG_EAR *dog);

	u8	dog_level_detect(ST_DOG_EAR *dog);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DE_H */

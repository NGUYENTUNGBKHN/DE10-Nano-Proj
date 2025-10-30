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
 * @file dye_note.h
 * @brief �_�C�m�[�g���m
 * @date 2018/09/14 Created.
 * @update�@2019/9/10
 */
/****************************************************************************/

#ifndef	_DY_H
#define	_DY_H

//�R�[�h�ǉ����Ă���
#define REJ_NEW_DYE_INK	(0x1411)
#define REJ_OLD_DYE_INK	(0x1412)



#define COORDINATE_LIST			4096		//���[�N�C���[�W����擾�������W���L�^����

enum dye_note_status
    {
		DYE_NOTE = 0x00
};

enum dye_note_L_R
    {
		left = -1,
		right = 1

};

enum dye_note_U_D
    {
		up	 = 1,
		down = -1

};

enum dye_note_side
{
	dynote_side_up =0,
	dynote_side_down,
	dynote_side_right,
	dynote_side_left

};


typedef struct
{

	//���͏��

	float thr;			//�C���N��臒l
	float thr2;			//�C���N��臒l
	u8 ratio;			//���
	u8	scan_pitch;	//�X�L������x
	u8	note_corner_margin;	//�����֊s����N�o����



	//���ԏ��

	u8 plane1;			//�p����v���[��
	u8 plane2;			//�p����v���[��
	u8 plane_num;		//�p����v���[���̐�
	u8 singularity_exclusion_flg;	//���m�Ԋu���ׂ����Ƃ��i24dot�ȉ��j�A�}�̃T�C�Y��1/2�ȏ�̓_��1�_�������m�����Ƃ��A���̓_�͌v�Z�Ɋ܂܂�Ȃ��B
	u8 padding;

	s16	note_size_x;	//�����T�C�Y
	s16	note_size_y;	//�����T�C�Y
	u32 note_area;		//�����ʐ�
	float area_ratio;	//��ʐ�

	u16 record_count;		//���m�������[����уC���N�Ƃ̋��E�̐����L�^����B
	u16 record_ink_count;	//���m�������[����уC���N�Ƃ̋��E�̐����L�^����B


	s16 record_x[COORDINATE_LIST];	//���m�������[����уC���N�Ƃ̋��E�̍��W���L�^���܂��B
	s16 record_y[COORDINATE_LIST];


	//s16 note_center_val_horaizontal[COORDINATE_LIST];	//�����̒��������̉�f�l�����炩���ߌv�Z���Ă���
	//s16 note_center_val_vertical[COORDINATE_LIST];		//�����Ă��̌��ʂ��i�[����s��

	//s16 note_center_val_horaizontal_count;	//��̃J�E���g��
	//s16 note_center_val_vertical_count;		//



	//s16	x_split;
	//s16	y_split;


	//s16 work_image_size_x;			//���[�N�C���[�W�̃T�C�Y
	//s16 work_image_size_y;			//���[�N�C���[�W�̃T�C�Y

	//u8 work_image[WORK_IMAGE_MAX_SIZE][WORK_IMAGE_MAX_SIZE];		//���[�N�C���[�W

	//u8 work_image_dpi;				//���[�N�C���[�W��d�s

	
	//u8 side1[INK_PATTERN];			//1�ӂ����ׂĐ��܂��Ă��邩�ǂ����t���O

	//u8 side2[INK_PATTERN];			//2�ӂ����܂��Ă��邩�ǂ����t���O
									//0�F��+��
									//1�F��+�E
									//2�F�E+��
									//3�F��+��

	//�o�͏��
	u32 res_ink_area;				//�����ʐ�
	float res_ink_ratio;			//�C���N����߂銄��
	u16 rej_code;
	u8 judge;						//�L�薳�̔��茋�� 0�� 1����
	u8	level;

	//u16 start_ink_x[INK_PATTERN];	//2�ӂɋy�ԃC���N�̊J�n�ʒu���L�^����B
	//u16 start_ink_y[INK_PATTERN];



	//u16 end_ink_x[INK_PATTERN];		//2�ӂɋy�ԃC���N�̏I���ʒu���L�^����B
	//u16 end_ink_y[INK_PATTERN];

	//float slope_leftup_rightdown;	//�Ίp���̌X��
	//float slope_leftdown_rightup;

	//u8 side4;						//4�ӂɋy�ԃC���N������ꍇ�̃t���O

	//u8 side2_all[INK_PATTERN];		//2�ӂɋy�ԃC���N������ꍇ�̃t���O
	




} ST_DYE_NOTE;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16 dye_note(u8 buf_num ,ST_DYE_NOTE *st);

	void status_ini(ST_DYE_NOTE *dy);

	//s16	search_ink(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param);

	u32 search_ink_2_left(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param ,ST_DYE_NOTE *st);
	u32 search_ink_2_right(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param ,ST_DYE_NOTE *st);
	u32 search_ink_2_up(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param ,ST_DYE_NOTE *st);
	u32 search_ink_2_down(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param ,ST_DYE_NOTE *st);

	s16 search_boundary_x(ST_SPOINT spt1 ,ST_SPOINT spt2 , s16 width ,s8 flg ,ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param ,s16* x ,s16* y ,float thr, s8 left_or_right,ST_DYE_NOTE *st);
	s16 search_boundary_y(ST_SPOINT spt1 ,ST_SPOINT spt2 , s16 width ,s8 flg ,ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param ,s16* x ,s16* y ,float thr, s8 up_or_down,ST_DYE_NOTE *st);

	//void calculation_note_inside_values(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param);

	//float area(s16 *x , s16 *y , u16 point_count ,s16 origin_x ,s16 origin_y);
	u32 area(s16 *x , s16 *y , u16 point_count ,s16 origin_x ,s16 origin_y);

	u8	dye_level_detect(ST_DYE_NOTE *dy);
	void singularity_exclusion(ST_DYE_NOTE* dy, u8* scan_location, u16 start_offset, u8 side);
#ifdef DYE_NOTE_VIEWER_MODE
	void debug_circle_write(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, s32 x, s32 y, s32 limit_max, s32 limit_min, u8 XorY, u8 side, s32 base_coordinate, u32* recode_num);
#else
	void debug_circle_write(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, s32 x, s32 y, s32 limit_max, s32 limit_min, u8 XorY);
#endif



	//void debug_circle_write(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, s32 x, s32 y, s32 limit_max, s32 limit_min, u8 XorY, u8 side, s32 base_coordinate, u16* recode_num);
	/*
	s16	check_dye_ink(void);
	float search_area(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param);
	s16 calc_area(ST_BS* pbs ,ST_NOTE_PARAMETER* pnote_param);
	void get_cutout_img (ST_NOTE_PARAMETER*  pnote_param,  ST_BS* pbs  ,u8 buf_num ,u8 *** matrix (*map)[300] ,s16 posx, s16 posy ,s16 xe, s16 ye ,u8 plane[] ,u8 plane_num ,u8 multi);
	void ocr_test (u8 buf_num);
	void ave_img_create(u8 buf_num ,u8 multi ,u8 plane1 ,u8 plane2 ,u8 plane_num);
	s16 check_side_scan_res(s16 fit_1, u16 fit_2 ,u16 *continuity ,u16 *next_continuity );

	

	float upper_side1_area_calcu(void);
	float right_side1_area_calcu(void);
	float lower_side1_area_calcu(void);
	float left_side1_area_calcu(void);
	
	float left_upper_side2_area_calcu(s32 y_offset_start ,s32 y_offset_end, s32 x_scan_start, s32 x_scan_end, s8 gaba_x, s8 gaba_y, u8 side2_or_side3_4, s32 vertex_1, s32 vertex_2, s32 x_step_scan_start, s32 x_step_scan_end);
	float upper_right_side2_area_calcu(s32 y_offset_start ,s32 y_offset_end, s32 x_scan_start, s32 x_scan_end, s8 gaba_x, s8 gaba_y, u8 side2_or_side3_4, s32 vertex_1, s32 vertex_2, s32 x_step_scan_start, s32 x_step_scan_end);
	float right_lower_side2_area_calcu(s32 y_offset_start ,s32 y_offset_end, s32 x_scan_start, s32 x_scan_end, s8 gaba_x, s8 gaba_y, u8 side2_or_side3_4, s32 vertex_1, s32 vertex_2, s32 x_step_scan_start, s32 x_step_scan_end);
	float lower_left_side2_area_calcu(s32 y_offset_start ,s32 y_offset_end, s32 x_scan_start, s32 x_scan_end, s8 gaba_x, s8 gaba_y, u8 side2_or_side3_4, s32 vertex_1, s32 vertex_2, s32 x_step_scan_start, s32 x_step_scan_end);

	float left_upper_all_side2_area_calcu(void);
	float upper_right_all_side2_area_calcu(void);
	float right_lower_all_side2_area_calcu(void);
	float lower_left_all_side2_area_calcu(void);

	float side3_area_calcu_other_than_left(void);
	float side3_area_calcu_other_than_up(void);
	float side3_area_calcu_other_than_right(void);
	float side3_area_calcu_other_than_down(void);

	float side4_area_calcu(void);

	void cal_hora_and_verti_val(void);
	*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DY_H */

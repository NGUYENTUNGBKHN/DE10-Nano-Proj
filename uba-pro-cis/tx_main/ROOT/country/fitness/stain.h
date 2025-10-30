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
 * @file stain.h
 * @brief�@���݌��m
 * @date 2019 Created.
 */
/****************************************************************************/

#ifndef	_STAIN_H
#define	_STAIN_H

#define ERR_ARRAY_OVER		0x0101	// �|�C���g�������E
#define ERR_WHILE_OVER		0x0102	// while�������~�b�g�l�𒴂���񐔎��s���ꂽ�B


#define RES_THRSHOLD_OVER	0x0001	// 臒l�ȏ�
#define RES_TOTALAREA_OVER	0x0002	// �g�[�^���ʐ�

#define STAIN_MAX_PALNE_NUM	2			// �ő�v���[����
#define STAIN_MAX_NOT_SCAN_AREA_NUM	100	// �ő叜�O�G���A��
#define STAIN_MAX_NOT_SCAN_AREA_DETECT_NUM	300	// �ő叜�O�G���A�̃X�L������

#define STAIN_LEVEL_1 -1
#define STAIN_LEVEL_2 -2
#define STAIN_LEVEL_3 -3

typedef struct
{
	s16		left_up_x;				//���_���゘
	s16		left_up_y;				//���_���゙
	s16		right_down_x;			//���_�E����
	s16		right_down_y;			//���_�E����

} ST_STAIN_NOT_SCAN_AREA;

//���݃X�L�������̐i�s�����̖��O
enum stain_scan_direction_of_travel
{
	stain_right_down = 0,	//�E��
	stain_down,	        //��
	stain_left_down,	    //����
	stain_left,           //��
	stain_left_up,        //����
	stain_up,             //��
	stain_right_up,       //�E��
	stain_right           //�E
};

typedef struct
{
	//����
	u8 plane[STAIN_MAX_PALNE_NUM];					//�v���[��
	u8 plane_num;					//�v���[����
	u8 comparison_method;			//��r���@ 0�ʐ� �P�ʐςƒ��a 2���a

	float divide_val;				//���鐔
	u16 point_num;					//�|�C���g��
	u16	mask_size_x;				//�}�X�N�T�C�Y��

	u16	mask_size_y;				//��
	u16 padding;

	u8*	p_mask;						//�}�X�N�|�C���^
	u8*	thr[2];						//�|�C���g�}�b�v
	float stain_size_thr;			//1�̐��݂̖ʐς�臒l �P�� mm^2
	float total_stain_thr;			//���݂̍��v�ʐς�臒l �P�� mm^2
	float stain_diameter_thr;		//���݂̒��a��臒l �P�� mm
	u16 min_thr_val;				//臒l�̍Œ���l�@����ȉ��̐��l��臒l�̃|�C���g�͏����ΏۂƂ��Ȃ��B
	u8 conter_not_scan_range;		//�O������N�o�̓X�L�����Ώۂ���O��
	u8 repeat_num;					//���݃X�L�����Ɏ��s�����Ƃ�臒l��ύX���Ă�蒼����

	u8 x_split;						//��������
	u8 y_split;						//��
	s16 note_size_x;

	s16 note_size_y;
	s8 thr_adjust_val;				//臒l���ߗp
	u8 padding2;

	//���ԏ��
	u16 note_size_x_one_third;				//�����T�C�Y��1/3�@�X�L�����񐔂�臒l�Ȃǂɗp����
	u16 note_size_y_one_third;
	u16 not_scan_margin_x;					//
	u16 not_scan_margin_y;					//
	float up_and_down_edge_slope;			//�㉺�ӂ̌X��
	float up_edge_offset;					//��ӂ̐ؕ�
	float down_edge_offset;					//���ӂ̐ؕ�
	float left_and_right_edge_slope;		//���E�ӂ̌X��
	float left_edge_offset;					//���ӂ̐ؕ�
	float right_edge_offset;				//�E�ӂ̐ؕ�
	u8 raito;								//200dpi�Ƃ̔䗦
	u8 coef_raito;							//dpi���̌W��
	u8 not_scan_area_count[STAIN_MAX_PALNE_NUM];		//���O�G���A�̐��@
	u8 not_scan_safe_area_count[STAIN_MAX_PALNE_NUM][STAIN_MAX_NOT_SCAN_AREA_NUM];	//���O�G���A�̒��ŗL���ȃG���A�̐�
	ST_STAIN_NOT_SCAN_AREA notscan2[STAIN_MAX_PALNE_NUM][STAIN_MAX_NOT_SCAN_AREA_NUM];			//���O�G���A�̍��W
	ST_STAIN_NOT_SCAN_AREA notscan_in_vali[STAIN_MAX_PALNE_NUM][STAIN_MAX_NOT_SCAN_AREA_NUM][5];	//���O�G���A���̗L���G���A�̍��W

	u16 half_size_x;						//
	u16 half_size_y;						//

	u16 main_scan_max_val;
	u16 main_scan_min_val;
	u16 sub_scan_max_val;
	u16 sub_scan_min_val;

	u8  security_mode;	//�g������Č��肷��@1�FON�iIR1,IR2�j�@0:OFF(R,G,B)

	//�O�`����̕s�g�p�G���A���W
	s16	not_scan_range_x_left_up;
	s16	not_scan_range_y_left_up;
	s16	not_scan_range_x_right_down;
	s16	not_scan_range_y_right_down;

	//�o��
	u8 res_judge;					//���m����
	u8	level;
	u16 res_stain_err;				//�G���[�R�[�h
	float res_max_stain_area;		//���m�������݂ōł��傫�����݂̖ʐ�
	float res_max_stain_diameter;	//���m�������݂ōł��傫�����݂̒��a
	float res_total_stain_area;		//���m�������݂̍��v�ʐ�


} ST_STAIN;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	u8 check_area_(ST_STAIN* sta, ST_SPOINT* spt);
	s16	stain(u8 buf_num ,ST_STAIN *st);
	s16 contor_detect(ST_STAIN *sta ,ST_BS *pbs ,ST_NOTE_PARAMETER* pnote_param , s16 x, s16 y,s16 *conter_x ,s16 *conter_y ,u8 thr,u16 *conter_point_counter, u8 plane_num ,float multi ,s32 *center_x ,s32 *center_y , u16* max_x ,u16* max_y ,u16* min_x ,u16* min_y);
	s8 pix_scan_module(ST_SPOINT spt ,u8 dir[] ,ST_BS *pbs , u16 *origin_x ,u16 *origin_y ,u8 thr, ST_STAIN* sta);
	float stain_area(s16 *x , s16 *y , u16 point_count ,s16 origin_x ,s16 origin_y);
	s16 hole_check(s16 x, s16 y ,u8 buf_num ,u8 plane, ST_STAIN* sta);
	void stain_ini_and_params_set(ST_STAIN *st, ST_BS *pbs , u8 palne);
	u8 note_contor_scan(ST_STAIN *st ,ST_BS *pbs, u16 *x, u16 *y ,u8 location, u16 thr, u8 plane ,u16 finish_x ,s16 *conter_x ,s16 *conter_y ,u16 *conter_point_counter ,u16 *conter_point_counter2);
	s8 not_scan_area_dision(ST_BS *pbs ,ST_STAIN *st,ST_NOTE_PARAMETER* pnote_param, s16 x[], s16 y[]);
	s8 thr_scan_module(u8 dir[] ,s32 *x ,s32 *y ,u8 thr[] , s16 x_split , s16 y_split, u8 tot);
	void stain_quick_sort( s16 list[], s16 left, s16 right );
	s8	not_scan_area_serch(ST_STAIN *sta ,s32 start_x, s32 start_y, s32 end_x, s32 end_y , u8 tot, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2], u8 *area_nu);
	s8	validity_area_serch(ST_STAIN *sta ,s32 start_x, s32 start_y, s32 end_x, s32 end_y , u8 tot, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2], u8 *area_num);

	s8	set_area_info(ST_BS *pbs, ST_STAIN *sta, ST_NOTE_PARAMETER* pnote_param, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2],s16 extra_point_x[], s16 extra_point_y[], u16 num);
	s8	set_safearea_info(ST_BS *pbs, ST_STAIN *sta, ST_NOTE_PARAMETER* pnote_param, u8 plane_num, s16 lu_rd_x[][2], s16 lu_rd_y[][2],s16 extra_point_x[], s16 extra_point_y[], u16 num ,u8 area_c);

	u8	stain_level_detect(ST_STAIN *sta);
	float get_right_direction( float x, float y, float dx, float dy, float tx, float ty );
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _STAIN_H */

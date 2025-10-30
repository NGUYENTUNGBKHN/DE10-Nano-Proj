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
* @file tape.h
* @brief�@�e�[�v���m
* @date 2020 Created.
*/
/****************************************************************************/

#ifndef	_TAPE_H
#define	_TAPE_H

#define EVEN_NUM(IN) ((IN%2) != 0 ? (IN+1) : (IN))	//���͒l�������ɂ���B
#define DOT_SIZE_MM		(127)		/* 1dot = 0.127mm */

#ifndef THHKNS_CH_LENGTH
#define THHKNS_CH_LENGTH (256)
#endif

typedef struct					// CIS���W�n�Ō��݃Z���T���g���[�X������W
{
	u16 roller_a_left_pos;		//1�ڂ̃��[���[�̍��[
	u16 roller_a_right_pos;		//1�ڂ̃��[���[�̉E�[
	u16 roller_b_left_pos;      //2�ڂ̃��[���[�̍��[
	u16 roller_b_right_pos;     //2�ڂ̃��[���[�̉E�[
} ST_THKNS_ROLLER_RANGE;

typedef struct
{
	u32 chdata;
	u16 ave_count;
	u16 padding;
//	u8 ave_count;
} ST_THKNS_CH_DATA_AVE;

typedef struct
{
	//����
	u16* plearn_data;			// �S�ʂ̊�f�[�^�z��ւ̐擪�I�t�Z�b�g
	u16 data_size;				// �S�ʂ̊�f�[�^�̗v�f��
	u8 bill_size_x;				// �����T�C�YX
	u8 bill_size_y;				// �����T�C�YY
	u8 mesh_size_x;				// ���b�V���T�C�YX
	u8 mesh_size_y;				// ���b�V���T�C�YY

	u16 base_value;				// �������ݍŏ��l
	u16 judge_count;			// ����I���|�C���g��
	u16	threshold;				// 臒l�⏕�l�iLSB 0.01�j
	u8 threshold_type;			// 臒l���

	u8 moving_average;			// �ړ����ρi����^���Ȃ��j
	u8 moving_ave_value_a;		// �ړ�����A�l
	u8 divide_ab;				// ����A-B�i����^���Ȃ��j
	u16 divide_line_ab;			// �������C��A-B�l
	u8 moving_ave_value_b;		// �ړ�����B�l
	u8 divide_bc;				// ����B-C�i����^���Ȃ��j
	u16 divide_line_bc;			// �������C��B-C�l
	u8 moving_ave_value_c;		// �ړ�����C�l

	u8 exclude_mesh_top;		// ���O�͈́@��[�@���b�V����
	u8 exclude_mesh_bottom;		// ���O�͈́@��[�@���b�V����
	u8 exclude_mesh_left;		// ���O�͈́@���[�@���b�V����
	u8 exclude_mesh_right;		// ���O�͈́@�E�[�@���b�V����
	
	u8 continuous_judge;		// �A������臒l

	u8 mesh_skip_x;				// 
	u8 mesh_skip_y;				// 

	u8 tc1_tc2_corr;			// TC1-TC2�̕␳�t���O
	u8 black_corr;				// ���␳�t���O

	//�o��
	u16 err_code;						// �G���[�R�[�h
	u8  result;							// ���m����
	u8  level;							// ���x��
	u16 detect_num;						// �e�[�v���m��
	u8	ch_tape_num[THICKNESS_SENSOR_MAX_NUM];	// �e�����Z���T���̃e�[�v����ӏ���

} ST_TAPE;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s16	tape(u8 buf_num, ST_TAPE *st);
	u8 get_thickness_map(ST_BS *pbs, ST_NOTE_PARAMETER * pnote_param, u16* pthkns_map, u16 map_size_x, u16 map_size_y, u8 mesh_size_x, u8 mesh_size_y, u8 tc_corr, u8 bc_mode);
	u16 get_thickness_data(ST_BS* pbs, ST_NOTE_PARAMETER* pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy, float* bc_val, s16 multiplier, u8 tc_corr);

	u8 get_thickness_ch_coordinate(ST_BS * pbs, ST_NOTE_PARAMETER * pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy, s8* pthkns_ch, s16* pthikns_actual_posy);
	u8 new_get_thickness_ch_coordinate(ST_BS * pbs, ST_NOTE_PARAMETER * pnote_param, ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 logical_posx, s16 logical_posy, s8* pthkns_ch, s16* pthikns_actual_posy);
	void calc_map_size(u8 bill_size_x, u8 bill_size_y, u8 mesh_size_x, u8 mesh_size_y, u16* pmap_size_x, u16* pmap_size_y);
	u8 convert_bill_actual_coordinate(ST_BS *pbs, ST_NOTE_PARAMETER * pnote_param, s16 logical_posx, s16 logical_posy, s16* pbill_actual_posx, s16* pbill_actual_posy, s16* ptc_actual_posx, s16* ptc_actual_posy);
	s8 get_thickness_sensor_channel(ST_THKNS_ROLLER_RANGE* proller_range, u8 sensor_count, s16 actual_posx);
	void calc_moving_average(u32 input_data[][THHKNS_CH_LENGTH], u32 out_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, ST_TAPE *st, u16* pbill_top_edge, u16 bill_lengh_y);
	void moving_average_sub(u32 input_data[][THHKNS_CH_LENGTH], u32 out_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, u8 moving_ave_value, int start, int end, u16* pbill_top_edge);
	void convert_thickness_sensor_data(ST_BS *pbs, ST_NOTE_PARAMETER * pnote_param, u16* learn_map_data, u16 map_size_x, u16 map_size_y, ST_TAPE * st, ST_THKNS_ROLLER_RANGE* proller_range, ST_THKNS_CH_DATA_AVE plearn_ch_data[][THHKNS_CH_LENGTH], u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 array_size, u16 height);
	u16 compare_thickness_sensor_data(int array_size, u32 pinput_ch_data[][THHKNS_CH_LENGTH], ST_THKNS_CH_DATA_AVE plearn_ch_data[][THHKNS_CH_LENGTH], u32 compared_ch_data[][THHKNS_CH_LENGTH], u16 threshold, u8 continuous_judge);

	u8 get_thkns_data(ST_BS * pbs, u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 height, u8 bc_mode, u8 tc_corr_mode);

	u8 get_roller_range(ST_BS * pbs, ST_THKNS_ROLLER_RANGE** proller_range);
	u16 pre_judge_sensor_data(u32 thkns_ch_data[][THHKNS_CH_LENGTH], u16 ch_num, u16 data_count, u16 threshold);
	u8 calculate_billedge(ST_BS* pbs, ST_THKNS_ROLLER_RANGE* proller_range, u16 ch_num, u16* pbill_top_edge, u16* pbill_lengh_y);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TAPE_H */

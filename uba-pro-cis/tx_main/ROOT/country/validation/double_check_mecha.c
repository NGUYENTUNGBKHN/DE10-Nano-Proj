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
* @file double_check_mecha.c
* @brief �d�����m�i���J���j
* @date 2020/12/ Created.
* @updata
*/
/****************************************************************************/
#include <stdio.h>
#include <string.h>

#define EXT
#include "../common/global.h"
#include "double_check_mecha.h"

#define THICKNESS_SENSOR_NUM (32)

//���J���d�����m�̃��C���֐�
s16	double_check_mecha(u8 buf_num, ST_DBL_CHK_MECHA *st)
{
	u8 ret = 0;

	u16 double_check_threshold = st->double_check_threshold;	// �d�����o����臒l
	u8  double_area_ratio = st->double_area_ratio;				// �ʐϔ䗦臒l(%)
	u16 bill_check_threshold = st->bill_check_threshold;		// �������o����臒l
	u8  exclude_length = st->exclude_length;					// �擪���菜�O�͈�(mm)

	u8  result = 0;
	u8  double_check_ratio = 0;
	u16 double_check_count = 0;
	u16 bill_check_count = 0;
	u8  double_check_point[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	u8  bill_check_point[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	u8  bill_top_point[THICKNESS_SENSOR_MAX_NUM] = { 0 };
	u32 bill_integrated_value = 0;
	u16 bill_thickness_average = 0;

	ST_BS* pbs = work[buf_num].pbs;
	u8 sub_sampling_pitch = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].sub_sampling_pitch;
	u16 height = pbs->block_count / (sub_sampling_pitch / pbs->Blocksize);		// �����������ő�f�[�^��
	u16 sensor_count = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_all_pix - pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min;	// +1;

	u16 thkns_ch_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };

	s16 bill_top_edge = 0;
	s16 bill_end_edge = 0;
	u8 exlude_count = 0;
	u8 continuous_flg = 0;
	// �擪���O�͈͕��̃|�C���g���Z�o
	u8 top_edge_exclude_offset = (u8)((25.4 / (pbs->Subblock_dpi / sub_sampling_pitch) ) * exclude_length);

	// ������[�p�A��[�p�̎擾
	s8 sub_offset = pbs->PlaneInfo[UP_TC1].sub_offset;
	s16 left_up_y = pbs->left_up_y;			//���_���゙
	s16 left_down_y = pbs->left_down_y;		//���_������
	s16 right_up_y = pbs->right_up_y;		//���_�E�゙
	s16 right_down_y = pbs->right_down_y;	//���_�E����

	s32 ch = 0;
	s32 idx = 0;
	s32 j = 0;
	s32 i = 0;

#ifndef _RM_
	s32 array_length = THICKNESS_SENSOR_MAX_NUM * THHKNS_CH_LENGTH;
	s32 x;

	u32 raw_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };
	u32 bill_data[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };
	u32 bill_top_point_callback[THICKNESS_SENSOR_MAX_NUM][THHKNS_CH_LENGTH] = { 0 };
#endif

	if (left_up_y > right_up_y)			// ������[�p
	{
		bill_top_edge = (right_up_y + sub_offset)/ sub_sampling_pitch;
	}
	else
	{
		bill_top_edge = (left_up_y + sub_offset) / sub_sampling_pitch;
	}

	if (bill_top_edge < 0)
	{
		bill_top_edge = 0;
	}

	if (left_down_y > right_down_y)		// ������[�p
	{
		bill_end_edge = (left_down_y + sub_offset) / sub_sampling_pitch;
	}
	else
	{
		bill_end_edge = (right_down_y + sub_offset) / sub_sampling_pitch;
	}
	if (bill_end_edge > height)
	{
		bill_end_edge = height;
	}

	// ���݃Z���T�f�[�^(TC1 - TC2)�擾 �� thkns_ch_data
	ret = get_double_check_thkns_data(pbs, thkns_ch_data, height, sensor_count);

	if (ret == 0)
	{
		// �eCH���ɏ���
		for (ch = 0; ch < sensor_count; ch++)
		{
			exlude_count = 1;
			// ������[�p������A������[�p�܂ő���
			for (idx = bill_top_edge; idx < bill_end_edge+1; idx++)
			{
				u16 thkns_value = thkns_ch_data[ch][idx];
				if (thkns_value > bill_check_threshold)			// ��������臒l�𒴂���
				{
					// �ŏ��Ɏ�������臒l�𒴂����|�C���g
					if (bill_top_point[ch] == 0)
					{
						continuous_flg = 1;
						// ���O�͈͕��A�����Ď�������臒l�𒴂��Ă��邩�m�F����
						for (j = 0; j < top_edge_exclude_offset+1; j++)
						{
							//���m�����ʒu���珜�O�͈͂̊Ԃ̉�f�l������臒l�Ɣ�r
							if (thkns_ch_data[ch][idx + j] <= bill_check_threshold)
							{
								//1�x�ł������΂��̈ʒu����ēx�����X�L�������J�n����悤�Ƀt���O��ݒ肷��B
								continuous_flg = 0;
								idx += j;
								break;
							}
						}
						if (continuous_flg == 1)			//��[�Ɣ��肳�ꂽ�ꍇ
						{
							bill_top_point[ch] = (u8)idx;	//��[���W���L�^����B
						}
						else
						{
							break;	//����������[���m�X�L�������s���B
						}
					}
					// �擪���O�͈͂���� 10��ȏ㎆���L��������Ȃ��ƁA�Z���T�Ɣ�����Ƃ͌����Ȃ��B
					if (top_edge_exclude_offset < exlude_count)
					{
						bill_check_count += 1;						// �S�������o�|�C���g��
						bill_check_point[ch] += 1;					// CH���������o�|�C���g��
						bill_integrated_value += thkns_value;		// ���ݐώZ�l
						if (thkns_value > double_check_threshold)	// �d������臒l�𒴂���
						{
							double_check_count += 1;				// �S�d�����o�|�C���g��
							double_check_point[ch] += 1;			// CH���d�����o�|�C���g��
						}
					}
					exlude_count += 1;	//����Ch�̎������X�L�����ł������C����
				}
			}
		}

		if (bill_check_count == 0)		// ��������ӏ��Ȃ�
		{
			// �ʐϔ䗦�Z�o
			double_check_ratio = 0;
			// ���茋��
			result = 2;
			// ��������ӏ��̌��ݕ��ϒl
			bill_thickness_average = 0;
		}
		else
		{
			// �ʐϔ䗦�Z�o
			double_check_ratio = (u8)((double_check_count * 100) / bill_check_count);
			// ���茋��
			if (double_check_ratio >= double_area_ratio)
			{
				result = 1;
			}
			// ��������ӏ��̌��ݕ��ϒl
			bill_thickness_average = (u16)(bill_integrated_value / bill_check_count);
		}
	}

	// �o��
	// ���ʃu���b�N
	st->result = result;										// ���茋��
	st->double_check_ratio = double_check_ratio;				// �d���ʐϔ䗦
	st->double_check_count = double_check_count;				// �d������ӏ��|�C���g��
	st->bill_check_count = bill_check_count;					// ��������ӏ��|�C���g��

	// ���ԏ��
	st->bill_thickness_average = bill_thickness_average;		// ��������ӏ��̌��ݕ��ϒl
	for (ch = 0; ch < THICKNESS_SENSOR_MAX_NUM; ch++)
	{
		st->double_check_point[ch] = double_check_point[ch]; 	// �d������ӏ����@�i�e�����Z���T���@�ő�16�Z���T���j
		st->bill_check_point[ch] = bill_check_point[ch];		// ��������ӏ����@�i�e�����Z���T���@�ő�16�Z���T���j
		st->bill_top_point[ch] = bill_top_point[ch];			// ������[����ӏ��@�i�e�����Z���T���@�ő�16�Z���T���j
	}
	st->exclude_point = top_edge_exclude_offset;				// �擪���菜�O�͈�(point)
	st->sensor_num = (u8)sensor_count;							// �����Z���TCH��
	st->check_point_top = (u8)bill_top_edge;					// ���m�Ώ۔͈͐�[
	if (bill_end_edge < 256)
	{
		st->check_point_end = (u8)bill_end_edge;				// ���m�Ώ۔͈͌�[
	}
	else
	{
		st->check_point_end = 0xFF;
	}

#ifndef _RM_
	// �eCH���ɏ���
	for (ch = 0; ch < sensor_count; ch++)
	{
		bill_top_point_callback[ch][0] = bill_top_point[ch];
		exlude_count = 0;
		// ������[�p������A������[�p�܂ő���
		for (idx = bill_top_edge, x=0; idx < bill_end_edge+1; idx++,x++)
		{
			u16 thkns_value = thkns_ch_data[ch][idx];
			bill_data[ch][x] = thkns_value;
		}
		for (i = 0; i < THHKNS_CH_LENGTH; i++)
		{
			raw_data[ch][i] = thkns_ch_data[ch][i];
		}
	}
	//�R�[���o�b�N�֐��@�f�o�b�O���
	callback_tape_len((u32*)raw_data, array_length, (u32*)bill_data, array_length, (u32*)bill_top_point_callback, array_length, THHKNS_CH_LENGTH, bill_end_edge+1, sensor_count, top_edge_exclude_offset, 0);
#endif

	return 0;

}

/****************************************************************/
/**
* @brief		TC1�|TC2���v�Z���āAthkns_ch_data�Ɋi�[����B
*@param[in]		pbs				�T���v�����O�f�[�^
*				height			Y�����̗L����f�͈�
*				sensor_count	Ch�̐�
*
*@param[out]	thkns_ch_data	TC1-TC2�̌���
*
* @return		ret�@			0�F�Œ�
*/
/****************************************************************/
u8 get_double_check_thkns_data(ST_BS * pbs, u16 thkns_ch_data[][THHKNS_CH_LENGTH], u16 height, u16 sensor_count)
{
	u8 ret = 0;
	u16 sens_dt_tc1 = 0;
	u16 sens_dt_tc2 = 0;
	u32 period = 0;
	u32 offset_tc1 = 0;
	u32 offset_tc2 = 0;
	u16 y = 0;
	u16 x = 0;
	u8 data_type = 0;
	u16 sensor_ofs = 0;
	u16 tc1_idx_x = 0;

	sensor_ofs = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].main_effective_range_min;
	period = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].Address_Period;
	offset_tc1 = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].Address_Offset;
	offset_tc2 = pbs->PlaneInfo[THICKNESS_SENSOR_NUM + 1].Address_Offset;
	data_type = pbs->PlaneInfo[THICKNESS_SENSOR_NUM].data_type;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < sensor_count; x++)
		{
			tc1_idx_x = x + sensor_ofs;
			if (data_type > 8)
			{
				/*2�o�C�g���̃f�[�^�����o���܂�*/
				memcpy(&sens_dt_tc1, &pbs->sens_dt[y * period + (tc1_idx_x * 2) + offset_tc1], 2);
				memcpy(&sens_dt_tc2, &pbs->sens_dt[y * period + (x * 2) + offset_tc2], 2);
			}
			//�ʏ�
			else
			{
				sens_dt_tc1 = (s16)pbs->sens_dt[y * period + tc1_idx_x + offset_tc1];
				sens_dt_tc2 = (s16)pbs->sens_dt[y * period + x + offset_tc2];
			}

			if (sens_dt_tc1 > sens_dt_tc2)
			{
				thkns_ch_data[x][y] = (sens_dt_tc1 - sens_dt_tc2);
			}
		}
	}

	return ret;
}


//End of File

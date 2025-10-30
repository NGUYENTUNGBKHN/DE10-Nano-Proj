
#include <stdio.h>

#define EXT
#define DEBUG_RBA

#include "../common/global.h"
#include "convert_data.h"
 
#include "mcir_check.h"
#include <stdlib.h>
//demomi�ƕ�����NN�̏o�̓m�[�h�ƑΉ������������ǂ��H
int get_mcir_invalid_count(u8 buf_num ,ST_MCIR *mcir)
{

	u32 color_sum = 0;		//�p���F�O�F�̍��v
	s32 val1;				//�p���F�|�C���g�l
	s32 val2;	
	s32 val3;
	s32 res;				//�p���F�v�Z����
	u16 point_cnt = 0;		//�p���F�J�����g�|�C���g
	u8 plane1=0;			//�p���F�v���[���ԍ�
	u8 plane2=0;
	u8 plane3=0;
	u16 mode_cnt = 0;			//�p���F���[�h
	u16 mode = 0;			//�p���F���[�h
	u16 plane_side;			//�p���F�v���[���̃T�C�h
	u16 invalid_count = 0;	//�o�́F�����J�E���g
	s32 level = 100; // �Z�o���x���i�[

	for(mode_cnt = 0; mode_cnt < CIR_3COLOR_CHECK_COUNT; ++mode_cnt)
	{
		//���[�h��-1�Ȃ�I���
		if(mcir->cal_mode[mode_cnt] == -1)
		{
			break;
		}

		//���[�h�̐ݒ�
		mode = mcir->cal_mode[mode_cnt];

		plane_side = mode % 2;

		//���[�h�ɉ����ăe�[�u������v���[�����擾
		//�������\�@�����
		plane1 = mcir_plane_tbl_ir1[plane_side];
		plane2 = mcir_plane_tbl_red[plane_side];
		plane3 = mcir_plane_tbl_grn[plane_side];

		//�v�Z����@0,0,0,0�̃G���h�}�[�N������܂ŌJ��Ԃ�
		while(!(mcir->ppoint[point_cnt].x == -1 && mcir->ppoint[point_cnt].y == -1 &&
			mcir->ppoint[point_cnt].limit_min == -1 && mcir->ppoint[point_cnt].limit_max == -1))
		{
			//�|�C���g�f�[�^�̎�
			val1 = get_mesh_data(mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y, plane1 ,mcir->ir1_mask_ptn_diameter_x ,mcir->ir1_mask_ptn_diameter_y ,mcir->pir1_mask_ptn ,buf_num ,mcir->ir1_mask_ptn_divide_num);
			val2 = get_mesh_data(mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y, plane2 ,mcir->red_mask_ptn_diameter_x ,mcir->red_mask_ptn_diameter_y ,mcir->pred_mask_ptn ,buf_num ,mcir->red_mask_ptn_divide_num);
			val3 = get_mesh_data(mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y, plane3 ,mcir->grn_mask_ptn_diameter_x ,mcir->grn_mask_ptn_diameter_y ,mcir->pgrn_mask_ptn ,buf_num ,mcir->grn_mask_ptn_divide_num);

			//�v�Z
			color_sum = val1 + val2 + val3;

			if (color_sum < 1) 
			{
				color_sum = 1;
			}
			//fprintf(test, "%d, %d,", mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y);

			if(mode == 0 || mode == 1)
			{
				res = (s32)(abs(val1 * 2 - val2) * MCIR_RATIO_MULTIPLIER / color_sum);
			}
			else if(mode == 2 || mode == 3)
			{
				res = (s32)(abs(val1 * 2 - val3) * MCIR_RATIO_MULTIPLIER / color_sum);
			}
			else
			{
				res = (s32)(abs(val3 * 2 - val2) * MCIR_RATIO_MULTIPLIER / color_sum);	//���ԈႦ�Ă����@190920�@furuta 
			}


			//臒l��r
			if (res < mcir->ppoint[point_cnt].limit_min - NOMAL_THERSHOLD || res > mcir->ppoint[point_cnt].limit_max + NOMAL_THERSHOLD) 
			{
				//�����|�C���g�J�E���g
				invalid_count++;
			}
			// ���b�V���|�C���g�m�F�p
			//fprintf(test, "%d, %d, %d, %d, %d, %d, %d,\n", mode, mcir->ppoint[point_cnt].x, mcir->ppoint[point_cnt].y, val1, val2, val3, invalid_count);
			//���̃|�C���g��
			point_cnt++;

		}//while

		//���̃|�C���g��
		point_cnt++;

	}//for
	
	level = MCIR_STANDARD_LEVEL - (MCIR_CONSTANT * invalid_count);

	if (level > 100)
	{
		level = 100;
	}
	else if (level <= 0)
	{
		level = 1;
	}
	mcir->level = (u8)level;
	return invalid_count;
}

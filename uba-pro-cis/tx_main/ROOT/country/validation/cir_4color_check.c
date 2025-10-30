#define EXT
#include "../common/global.h"
#include "convert_data.h"
#include "cir_4color_check.h"


int get_cir_4color_invalid_count(u8 buf_num ,ST_4CIR *cir4)
{

	u32 color_sum = 0;		//�p���F�O�F�̍��v
	u32 val1;				//�p���F�|�C���g�l
	u32 val2;	
	u32 val3;
	u32 val4;

	u16 res;				//�p���F�v�Z����
	u16 point_cnt = 0;		//�p���F�J�����g�|�C���g
	u8 plane1=0;			//�p���F�v���[���ԍ�
	u8 plane2=0;
	u8 plane3=0;
	u8 plane4=0;
	u16 plane_side;			//�p���F�v���[���̃T�C�h

	u16 mode_cnt = 0;			//�p���F���[�h
	u16 mode = 0;			//�p���F���[�h

	u16 invalid_count = 0;	//�o�́F�����J�E���g

	for(mode_cnt = 0; mode_cnt < CIR_4COLOR_CHECK_COUNT; ++mode_cnt)
	{
		//���[�h��-1�Ȃ�I���
		if(cir4->cal_mode[mode_cnt] == -1)
		{
			break;
		}

		//���[�h�̐ݒ�
		mode = cir4->cal_mode[mode_cnt];
		plane_side = mode % 2;
		//point_cnt = 0;

		//���[�h�ɉ����ăe�[�u������v���[�����Q�b�g
		plane1 = cir3_plane_tbl_ir[plane_side];
		plane2 = cir4_plane_tbl_ir2[plane_side];
		plane3 = cir3_plane_tbl_red[plane_side];
		plane4 = cir3_plane_tbl_grn[plane_side];


		//�v�Z����@0,0,0,0�̃G���h�}�[�N������܂ŌJ��Ԃ�
		while(!(cir4->ppoint[point_cnt].x  == -1 && cir4->ppoint[point_cnt].y  == -1 &&
			  cir4->ppoint[point_cnt].limit_min  == -1 && cir4->ppoint[point_cnt].limit_max == -1))
		{
			//�|�C���g�f�[�^�̎�
			val1 = get_mesh_data(cir4->ppoint[point_cnt].x, cir4->ppoint[point_cnt].y, plane1 ,cir4->ir1_mask_ptn_diameter_x ,cir4->ir1_mask_ptn_diameter_y ,cir4->pir1_mask_ptn ,buf_num ,cir4->ir1_mask_ptn_divide_num);
			val2 = get_mesh_data(cir4->ppoint[point_cnt].x, cir4->ppoint[point_cnt].y, plane2 ,cir4->ir2_mask_ptn_diameter_x ,cir4->ir2_mask_ptn_diameter_y ,cir4->pir2_mask_ptn ,buf_num ,cir4->ir2_mask_ptn_divide_num);
			val3 = get_mesh_data(cir4->ppoint[point_cnt].x, cir4->ppoint[point_cnt].y, plane3 ,cir4->red_mask_ptn_diameter_x ,cir4->red_mask_ptn_diameter_y ,cir4->pred_mask_ptn ,buf_num ,cir4->red_mask_ptn_divide_num);
			val4 = get_mesh_data(cir4->ppoint[point_cnt].x, cir4->ppoint[point_cnt].y, plane4 ,cir4->grn_mask_ptn_diameter_x ,cir4->grn_mask_ptn_diameter_y ,cir4->pgrn_mask_ptn ,buf_num ,cir4->grn_mask_ptn_divide_num);

			//�v�Z
			color_sum = val1 + val2 + val3 + val4; 

			if (color_sum < 1) 
			{
				color_sum = 1;
			}

			if(mode == 0 || mode == 1)
			{
				res = (u16)(val1 * CIR_RATIO_MULTIPLIER / color_sum);
			}
			else if(mode == 2 || mode == 3)
			{
				res = (u16)(val2 * CIR_RATIO_MULTIPLIER / color_sum);
			}
			else if(mode == 4 || mode == 5)
			{
				res = (u16)(val3 * CIR_RATIO_MULTIPLIER / color_sum);
			}
			else
			{
				res = (u16)(val4 * CIR_RATIO_MULTIPLIER / color_sum);
			}


			//臒l��r
			if (res < cir4->ppoint[point_cnt].limit_min - NOMAL_THERSHOLD || res > cir4->ppoint[point_cnt].limit_max + NOMAL_THERSHOLD) 
			{
				//�����|�C���g�J�E���g
				invalid_count++;
			}

			//���̃|�C���g��
			point_cnt++;

		}//while
		
		//���̃|�C���g��
		point_cnt++;

	}//for

	return invalid_count;
}


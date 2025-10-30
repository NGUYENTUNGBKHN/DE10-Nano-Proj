#define EXT
#include "../common/global.h"
#include "ir_check.h"


int get_ir_check_invalid_count(u8 buf_num ,ST_IR_CHECK *ir_check)
{

	//u8 base_i = 0;			//�p��:�x�[�X�f�[�^�쐬for���p

	//u32 color_sum = 0;		//�p���F���v
	u32 val;				//�p���F�|�C���g�l
	u16 res;				//�p���F�v�Z����
	u16 point_cnt = 0;		//�p���F�J�����g�|�C���g
	u8 plane=0;				//�p���F�v���[���ԍ�
	u32 ir_base_value;		//�p���F�x�[�X�f�[�^

	u16 mode_cnt = 0;			//�p���F���[�h
	u16 mode = 0;			//�p���F���[�h

	u16 invalid_count = 0;	//�o�́F�����J�E���g

	for(mode_cnt = 0; mode_cnt < IR_CHECK_COUNT; ++mode_cnt)
	{
		//���[�h��-1�Ȃ�I���
		if(ir_check->cal_mode[mode_cnt] == -1)
		{
			break;
		}

		if(mode_cnt > 0)
		{
			//�A�h���X���X�V����
			point_cnt++;
			ir_check->pmode = (ST_IR_MODES *)(&ir_check->ppoint[point_cnt]);
			ir_check->ppoint = (ST_LIM_AND_POINTS *)((u8*)ir_check->pmode + sizeof(ST_IR_MODES));
		}

		mode = ir_check->cal_mode[mode_cnt];
		plane = ir_plane_tbl[mode];
		ir_base_value = 0;
		point_cnt = 0;

		//�x�[�X�f�[�^�̎�
		ir_base_value += get_mesh_data(ir_check->pmode->base_point_x1,ir_check->pmode->base_point_y1 , plane,ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);
		ir_base_value += get_mesh_data(ir_check->pmode->base_point_x2,ir_check->pmode->base_point_y2 , plane,ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);
		ir_base_value += get_mesh_data(ir_check->pmode->base_point_x3,ir_check->pmode->base_point_y3 , plane,ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);
		ir_base_value += get_mesh_data(ir_check->pmode->base_point_x4,ir_check->pmode->base_point_y4 , plane,ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);

		if (ir_base_value == 0)
		{
			ir_base_value = 1;
		}

		//�v�Z����@0,0,0,0�̃G���h�}�[�N������܂ŌJ��Ԃ�
		while(!(ir_check->ppoint[point_cnt].x == -1 && ir_check->ppoint[point_cnt].y == -1 &&
			ir_check->ppoint[point_cnt].limit_min == -1 && ir_check->ppoint[point_cnt].limit_max == -1))
		{
			//�|�C���g�f�[�^�̎�
			val = get_mesh_data(ir_check->ppoint[point_cnt].x, ir_check->ppoint[point_cnt].y, plane, ir_check->ir1_mask_ptn_diameter_x ,ir_check->ir1_mask_ptn_diameter_y ,ir_check->pir1_mask_ptn ,buf_num ,ir_check->ir1_mask_ptn_divide_num);
			//�v�Z
			res = (u16)(val * CIR_RATIO_MULTIPLIER / ir_base_value);

			//臒l��r
			if (res < ir_check->ppoint[point_cnt].limit_min - NOMAL_THERSHOLD || res > ir_check->ppoint[point_cnt].limit_max + NOMAL_THERSHOLD)
			{
				//�����|�C���g�J�E���g
				invalid_count++;
			}

			//���̃|�C���g��
			point_cnt++;

		}//while

	}//for

	return invalid_count;
}



#include	<math.h>
#include    <string.h>
#include    <float.h>
#include	<stdlib.h>
#define EXT
#include "../common/global.h"
#include "cir_2color_check.h"


//demomi�ƕ�����NN�̏o�̓m�[�h�ƑΉ������������ǂ��H
int get_cir_2color_invalid_count()
{

	//u32 val1;				//�p���F�|�C���g�l
	//u32 val2;	
	//s32 res;				//�p���F�v�Z����
	//u16 point_cnt = 0;		//�p���F�J�����g�|�C���g
	//u8 plane1=0;			//�p���F�v���[���ԍ�
	//u8 plane2=0;
	//u16 mode_cnt = 0;		//�p���F���[�h
	//u16 mode = 0;			//�p���F���[�h

	u16 invalid_count = 0;	//�o�́F�����J�E���g

	//u8 way = 0;				//���́F����
	//u16 denomi = 0;			//���́F����

	//for(mode_cnt = 0; mode_cnt < CIR_2COLOR_CHECK_COUNT; ++mode_cnt)
	//{
	//	//���[�h��-1�Ȃ�I���
	//	if(cir2.cal_mode[mode_cnt] == -1)
	//	{
	//		break;
	//	}

	//	//���[�h�̐ݒ�
	//	mode = cir2.cal_mode[mode_cnt];

	//	//���[�h�ɉ����ăe�[�u������v���[�����Q�b�g
	//	plane1 = cir2_plane_tbl1[mode];
	//	plane2 = cir2_plane_tbl2[mode];


	//	//�v�Z����@0,0,0,0�̃G���h�}�[�N������܂ŌJ��Ԃ�
	//	while(cir2.mode[denomi][way][mode].point[point_cnt].x != 0 && cir2.mode[denomi][way][mode].point[point_cnt].y != 0 &&
	//		cir2.mode[denomi][way][mode].point[point_cnt].limit_min != 0 && cir2.mode[denomi][way][mode].point[point_cnt].limit_max != 0)
	//	{
	//		//�|�C���g�f�[�^�̎�
	//		val1 = get_mesh_data2(cir2.mode[denomi][way][mode].point[point_cnt].x, cir2.mode[denomi][way][mode].point[point_cnt].y, plane1 ,buf_num);
	//		val2 = get_mesh_data2(cir2.mode[denomi][way][mode].point[point_cnt].x, cir2.mode[denomi][way][mode].point[point_cnt].y, plane2 ,buf_num);

	//		//�v�Z
	//		res = val1 - val2;

	//		//臒l��r
	//		if (res < cir2.mode[denomi][way][mode].point[point_cnt].limit_min - CIR2_THERSHOLD || res > cir2.mode[denomi][way][mode].point[point_cnt].limit_max + CIR2_THERSHOLD) 
	//		{
	//			//�����|�C���g�J�E���g
	//			invalid_count++;
	//		}

	//		//���̃|�C���g��
	//		point_cnt++;

	//	}//while

	//}//for

	return invalid_count;
}

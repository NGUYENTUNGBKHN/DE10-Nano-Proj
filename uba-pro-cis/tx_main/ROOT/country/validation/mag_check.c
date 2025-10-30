#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define EXT

#include "../common/global.h"
#include "mag_check.h"
#ifdef VS_DEBUG
EXTERN int debug_logi_view;	//�g���[�X���邩���Ȃ���
#endif
/*
2021/12/2
	�f�o�b�O�p��CSV�o�͂ł���悤��

2022/2/22
	�J�E���g����0.8�{����悤�ɕύX

2022/6/21
	���C�ߑ����p�����^�ɕύX

2022/7/15
	�ϐ����ύX�@�s�v�����폜
	���ԏ��ƌ��ʃu���b�N��ύX

*/

// �}�X�N�p�^�[���Œ�
u8 mag_maskpat[] = {
				1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
				1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
				1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
				1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
};


// 20210329 ichijo ���C�ߑ��p
int Global_mag_bw[MAG_MAX_STRUCT] = {0};
int Global_mag_counter[MAG_MAX_STRUCT] = {0};
int Global_point_count = 0;
int Global_mg_cnt = 0;

// Mag�p��P_Getdt
s16 P_Getdt_8bit_only_MagUv(ST_SPOINT *__restrict spt, ST_BS *__restrict pbs)
{
	if (spt->y < 0)
	{
		return -1;
	}
	if (pbs->PlaneInfo[spt->p_plane_tbl].data_type > 8)
	{
		memcpy(&spt->sensdt, &pbs->sens_dt[pbs->PlaneInfo[spt->p_plane_tbl].Address_Period *spt->y + (spt->x * 2) + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset], 2);
	}
	else
	{
		spt->period = pbs->PlaneInfo[spt->p_plane_tbl].Address_Period;
		spt->p = &pbs->sens_dt[spt->period * spt->y + spt->x + pbs->PlaneInfo[spt->p_plane_tbl].Address_Offset];
		spt->sensdt = *spt->p;

	}
#ifdef VS_DEBUG_ITOOL	//UI�\���p�@���@�̎��͏���
	new_P2L_Coordinate(spt, pbs);
	return 0;
#endif

#ifdef VS_DEBUG	//UI�\���p�@���@�̎��͏���

	if (debug_logi_view == 1)
	{
		deb_para[0] = 1;		// function code
		deb_para[1] = spt->x;	//
		deb_para[2] = spt->y;	//
		deb_para[3] = 1;		// plane
		callback(deb_para);		// debugaa
	}
#endif

	return 0;
}

// Mag�p��L2P_Coordinate
s16 new_L2P_Coordinate_MagUv(ST_SPOINT * __restrict spt, ST_BS * __restrict pbs, ST_NOTE_PARAMETER * __restrict pnote_param)
{
	//�������̎������

	float current_sin_x = 0;
	float current_cos_x = 0;
	float current_sin_y = 0;
	float current_cos_y = 0;
	float current_res_x = 0;
	float current_res_y = 0;
	s8 plane;
	float midst_res_x;
	float midst_res_y;
	float constant_x, constant_y;
	float input_x, input_y;

	float sin_x, cos_x, sin_y, cos_y;

	//���˃v���[����}�������ɉ�����
	//�\�������ւ�
	spt->p_plane_tbl = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];
	plane = (u8)pnote_param->pplane_tbl[spt->l_plane_tbl];

	sin_x = pnote_param->sin_x[plane];

	cos_x = pnote_param->cos_x[plane];

	sin_y = pnote_param->sin_y[plane];

	cos_y = pnote_param->cos_y[plane];

	if (pbs->LEorSE == LE)	//���W�ϊ��@+�@��f�Q�Ɓ@LE�̏ꍇ
	{
		input_x = spt->x * pnote_param->insart_multi_x[plane];
		input_y = (spt->y/* * pnote_param->transport_flg*/) * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x_non_ofs[plane];
		constant_y = pnote_param->coordinate_param_y[plane];

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x = (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_x = constant_x - midst_res_x;
		current_res_y = (midst_res_y + constant_y);

	}

	else  //���W�ϊ��@+�@��f�Q��
	{
		input_x = (spt->x/* * pnote_param->transport_flg*/) * pnote_param->insart_multi_x[plane];
		input_y = spt->y * pnote_param->insart_multi_y[plane];

		constant_x = pnote_param->coordinate_param_x[plane];
		constant_y = pnote_param->coordinate_param_y_non_ofs[plane];

		current_cos_y = input_y * cos_y;
		current_sin_x = input_y * sin_x;

		current_cos_x = input_x * cos_x;
		current_sin_y = input_x * sin_y;

		midst_res_x = (current_cos_x + current_sin_x);
		midst_res_y = -(current_cos_y - current_sin_y);

		current_res_y = constant_x - midst_res_x;
		current_res_x = (midst_res_y + constant_y);

	}

	spt->x = (s32)current_res_x;
	spt->y = (s32)current_res_y;

	return 0;

}
// Mag�p��point_vicinity_cal
s32 point_vicinity_cal_Mag(ST_POINT_VICINITY *__restrict pv, u8 buf_num, float std_para1, float std_para2)
{
	ST_SPOINT spt; /*�s�N�Z���֐��ɓ����*/
	ST_BS* pbs = work[buf_num].pbs;

	/*�t�B���^�n�[�t�T�C�Y*/
	u16 filter_half_size_x = pv->filter_size_x >> 1;
	u16 filter_half_size_y = pv->filter_size_y >> 3;

	s16 current_y = 0;
	s16 current_x = 0;
	u8 mlt = 0;
	
	s32 err_code;
	s16 new_sensdt[25][25] = { 0 };// �V�����f�[�^�l������
	s16 next_sensdt[25][25] = { 0 };// �V�����f�[�^�l������
	s32 n_cnt = 0;// �V�����f�[�^�l������Ƃ��̔z��ԍ�

	s32 valid_count = 0;
	u16 over_value_count = 0;
	/*���͘_���v���[����ݒ�*/
	spt.p_plane_tbl = (enum P_Plane_tbl)pv->plane;
	spt.trace_flg = 1;

	//���o�G���A�̒��S�_�𒆐S���W�ɕϊ�
	pv->x = pv->x - filter_half_size_x; //�����
	pv->y = pv->y - filter_half_size_y; //
	over_value_count = 0;
	for (current_x = 0; current_x <= filter_half_size_x; current_x++)
	{
		current_y = 0;
		mlt = 1;
		/*�t�B���^�[���ʒu����*/
		spt.x = pv->x + current_x;
		spt.y = (pv->y + current_y);										// 	���̍s��loop�̊O�Ɏ����Ă�����
		err_code = P_Getdt_8bit_only_MagUv(&spt, pbs);//��f�l�擾 
		new_sensdt[current_x][current_y] = (spt.sensdt * mlt) >> 2;/*�Z���T�[�f�[�^�Ɨv�f��*/
		if (err_code < 0)
		{
			return err_code;
		}
		for (current_y = 2; current_y <= pv->filter_size_y; current_y++)
		{
			if (over_value_count > filter_half_size_y)
			{
				break;
			}
			/*�t�B���^�[���ʒu����*/
			spt.x = pv->x + current_x;
			spt.y = pv->y + current_y;										// 	���̍s��loop�̊O�Ɏ����Ă�����

			//�X���␳�Ȃ�
			// 2�s�N�Z���ڎ擾
			err_code = P_Getdt_8bit_only_MagUv(&spt, pbs);//��f�l�擾 
			if (err_code < 0)
			{
				return err_code;
			}
			new_sensdt[current_x][current_y] = (spt.sensdt * mlt) >> 2;/*�Z���T�[�f�[�^�Ɨv�f��*/

		    // �f�[�^�l��MAG_BLACK�����܂���MAG_WHITE�ȏ�̂Ƃ��A���C���߃G���[�̃J�E���^���グ��
			if (new_sensdt[current_x][current_y] < MAG_BLACK || new_sensdt[current_x][current_y] > MAG_WHITE)
			{
				Global_mag_bw[Global_point_count]++;
			}

			// ������1�s�N�Z���ڂɓ����
			if (new_sensdt[current_x][current_y] >= 255 || new_sensdt[current_x][current_y] < 0) // �΍s�␳���s���Ă�����̂�Ǎ���ł��܂��̂�h���B���F�Q�T�T���v�Z�l�ɓ���Ă��܂��B
			{
				new_sensdt[current_x][current_y] = 0;
				over_value_count++;
				current_y++;
				continue;
				//break;
			}
			else if (new_sensdt[current_x][current_y - 2] <= 0/* || new_sensdt[current_x][current_y - 2]>= 255*/)
			{
				current_y++;
				continue;
			}
			next_sensdt[current_x][current_y] = (new_sensdt[current_x][current_y] - new_sensdt[current_x][current_y - 2]);
			n_cnt++;

			if (std_para1 < next_sensdt[current_x][current_y] || next_sensdt[current_x][current_y] < std_para2)
			{
				valid_count++;
			}
			current_y++;
			Global_mg_cnt++;
		}//for y
		if (over_value_count > 0)
		{
			break;
		}
	}// for x

	return valid_count;
}
// Mag�p��get_mesh_data
s32 get_mesh_data_Mag(s32 start_x, s32 start_y, s32 end_x, s32 end_y, s32 plane, s32 diameter_x, s32 diameter_y, u8* mask, u8 buf_num, float divide_num, float std_param1, float std_param2)
{
	ST_POINT_VICINITY pv;
	s32 x = 0;
	s32 y = 0;
	s32 valid_count = 0;
	s32 err_code = 0;
	for (y = start_y; y <= end_y; y++)
	{
		for (x = start_x; x <= end_x; x++)
		{
			//s32 i = 0;
			ST_SPOINT spt;
			ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
			ST_BS* pbs = work[buf_num].pbs;

			//L2P�p�̃p�����^�Z�b�g
			spt.l_plane_tbl = (s8)plane;


			/*�_�����W���畨���ɕϊ�*/
			//diameter��xy�Ƃ���200dpi�P�ʂ���Z����
			spt.x = (x * diameter_x);		//se��le�Ŏ����قȂ�@�C���K�v�@190920�@furuta 
			spt.y = (y * diameter_x);
			new_L2P_Coordinate_MagUv(&spt, pbs, pnote_param);
			//new_P2L_Coordinate(&spt, pbs);
			pv.x = (s16)spt.x;
			pv.y = (s16)spt.y;

			//�_�����Wplane���e�[�u������
			pv.plane = (u8)plane;

			//�}�X�N�T�C�Y�ƃ|�C���^�̕ύX�����肢�v���܂�
			pv.filter_size_x = (u16)diameter_x;
			pv.filter_size_y = (u16)diameter_y;
			pv.pfilter_pat = (u8*)mask;

			//divide_val���\�ߌv�Z���Ĉ����œn�����Ƃɂ��܂��B
			//���̓n���l�͋t���Ƃ���B�@19/02/08 furuta
			pv.divide_val = divide_num;

			err_code = point_vicinity_cal_Mag(&pv, buf_num, std_param1, std_param2);//Mag�͂�����
			if (err_code < 0)
			{
				return -1;
			}
			else
			{
				valid_count += err_code;
			}
		}
	}
	return valid_count;
}
// Mag���m�̖{��
s32 get_mag_valid_count(u8 buf_num, ST_MAG* mag)
{
	u8 ii = 0;
	u32 iii = 0;
	u8 point_number = 0;
	u16 valid_count[MAG_MAX_STRUCT] = { 0 };
	s32 tmp_count = 0;
	s32 tmp = 0;
	s32 adjust = 25;
	s32 s_area = 0;		// ���C�̐�߂銄�����v�Z���邽�߂Ɏg�p
	float output = 0.0;		// ���x���v�Z�p

	s16 coordinate[4] = { 0 };
	ST_SPOINT spt;
	float percentage[MAG_MAX_STRUCT] = { 0.0 };  // ���C�̈�ō��i�O�j������������̂��������v�Z���邽�߂Ɏg�p����
	
	float excess_mag_limit = 0;

#ifdef VS_DEBUG
	FILE *fp;
#endif

	u32 mm[MAG_MAX_STRUCT] = { 0 };
	u8 stopcnt = 0;

	spt.way = work[buf_num].pbs->insertion_direction;		//�����ݒ�

	// 20211203 add by furuta
	Global_mg_cnt = 0;
	memset(Global_mag_bw, 0, MAG_MAX_STRUCT * sizeof(Global_mag_bw[0]));
	memset(Global_mag_counter, 0, MAG_MAX_STRUCT * sizeof(Global_mag_counter[0]));
	Global_point_count = 0;
	
	//20220621 ���C�ߑ��p�����^�ݒ�
	for (iii = 0; iii < MAG_MAX_STRUCT; iii++)
	{
		if (mag->ppoint[iii].x_s == 2 &&
			mag->ppoint[iii].y_s == 0 &&
			mag->ppoint[iii].x_e == 0 &&
			mag->ppoint[iii].y_e == 0)
		{
			excess_mag_limit = mag->ppoint[iii].threshold1;
			break;
		}
	}

	
	for(point_number = 0; point_number < mag->point_number; point_number++)
	{
		// �p�����^�ɋL�ڂ��Ă���ꍇ�Ƃ����łȂ��ꍇ������BEUR�ACNY�͋L�ڍ݂�B����ȊO�͂܂��Ȃ̂ŋL�ڂ���K�v������Bwhile�̏����ł��ׂĔ������邪���ꂵ�����B
		if (mag->ppoint[point_number].x_s == 2 &&
			mag->ppoint[point_number].y_s == 0 &&
			mag->ppoint[point_number].x_e == 0 &&
			mag->ppoint[point_number].y_e == 0)
		{
			stopcnt = point_number;
			break;
		}

		if (mag->ppoint[point_number].x_s > mag->ppoint[point_number].x_e)
		{
			coordinate[2] = mag->ppoint[point_number].x_s;
			coordinate[0] = mag->ppoint[point_number].x_e;
		}
		else
		{
			coordinate[0] = mag->ppoint[point_number].x_s;
			coordinate[2] = mag->ppoint[point_number].x_e;
		}
		if (mag->ppoint[point_number].y_s > mag->ppoint[point_number].y_e)
		{
			coordinate[3] = mag->ppoint[point_number].y_s;
			coordinate[1] = mag->ppoint[point_number].y_e;
		}
		else
		{
			coordinate[1] = mag->ppoint[point_number].y_s;
			coordinate[3] = mag->ppoint[point_number].y_e;
		}
		
		tmp = get_mesh_data_Mag(coordinate[0], coordinate[1], coordinate[2], coordinate[3], 
			UP_MAG, adjust, adjust, mag_maskpat, buf_num, mag->mag_mask_ptn_divide_num, 
			mag->ppoint[point_number].threshold1, mag->ppoint[point_number].threshold3);

		if (work[buf_num].pbs->LEorSE == 0)
		{
			s_area = (s32)(abs(coordinate[0] - coordinate[2]) * abs(coordinate[1] - coordinate[3]) * 0.2f);
		}
		else
		{
			s_area = 10;
		}

		// 20210329 ichijo �S�̂̃J�E���g���̕ۑ��Ǝ��C�ʃJ�E���^�̃C���N�������g
		Global_mg_cnt = Global_mg_cnt * 0.8f; //20220222
		Global_mag_counter[point_number] = Global_mg_cnt;
		percentage[point_number] = 100.0f * ((float)Global_mag_bw[point_number] / (float)Global_mg_cnt); // ���C����������Ƃ���̂O�̌��p�[�Z���e�[�W�̌v�Z
		mm[point_number] = Global_mg_cnt;

		if (tmp > s_area)
		{
			valid_count[point_number]++; // ���C������Ƃ����Ӗ���valid
		}
		else if (tmp < 0)
		{
			mag->level = (u8)1;
			mag->result = (u8)4; 
			return tmp;
		}

		mag->average[point_number] = (float)tmp/(float)Global_mg_cnt;
		tmp = 0;

		Global_point_count++;

		// �S�̂̃J�E���g���̏�����
		Global_mg_cnt = 0;

	}// while


	// �擾�������C�ʂ̂����ł��Ⴂ���̂��o�͂��邽�߂Ɍv�Z���� = ���C�����������Ȃ�ɂ����}�̂ɗL��
	for (ii = 0; ii < stopcnt; ii++)
	{	
		if (ii == 0)
		{
			output = mag->average[ii];
			mag->ave_number = ii;
		}
		else if (mag->average[ii] < output)
		{
			output = mag->average[ii];
			mag->ave_number = ii;
		}
	}

	output = (STANDARD_LEVEL + ((output + output) * 100.0f - MAG_AREA_PERCENTAGE) * CONSTANT_VALUE);
	
	if (output > 100)
	{
		mag->level = 100;
	}
	else if (output <= 0)
	{
		mag->level = 1;
	}
	else
	{
		mag->level = (u8)output;
	}

	if (mag->level == 0)
	{
		mag->level = 1;
	}

	if (mag->level > MAG_STANDARD_LEVEL)
	{
		mag->result = (u8)0; // ���C���聁���Ȃ��i�^������j
	}
	else
	{
		mag->result = (u8)1; // ���C�Ȃ�����肠��i�U��������j
	}

	// ���C��������������ꍇ�A�����I�Ƀ��x����1�ɂ��A�G���[���Ƃ���
	for (ii = 0; ii < stopcnt; ii++)
	{
		// 0�`10�A245�`255�̒l�������ӏ����S�̂�5���ȏ�̂Ƃ��A�G���[�ƂȂ�
		if (percentage[ii] >= excess_mag_limit)
		{
			mag->per_number = ii;
			mag->result = (u8)3; // ���C�Ȃ�����肠��i�U��������j
			mag->level = 1;
		}

		mag->percent[ii] = percentage[ii];
	}

#ifdef VS_DEBUG

	if (work[buf_num].pbs->blank3 != 0)
	{
		fp = fopen("mag_check.csv", "a");
		fprintf(fp, "%s,%x,%d,%s,", work[buf_num].pbs->blank4, work[buf_num].pbs->mid_res_nn.result_jcm_id, work[buf_num].pbs->insertion_direction, work[buf_num].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf_num].pbs->blank0[20], work[buf_num].pbs->ser_num1, work[buf_num].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf_num].pbs->spec_code.model_calibration, work[buf_num].pbs->spec_code.sensor_conf, work[buf_num].pbs->spec_code.mode);

		for (ii = 0; ii < MAG_MAX_STRUCT; ii++)
		{
			fprintf(fp, "%f,%d,%d,", mag->percent[ii], Global_mag_bw[ii], mm[ii]);
			fprintf(fp, "%f,", mag->average[ii]);
			Global_mag_bw[ii] = 0;
		}

		fprintf(fp, "%f,", output);
		fprintf(fp, "%d,", mag->level);
		fprintf(fp, "\n");
		fclose(fp);
	}

#endif

	return mag->result;
}

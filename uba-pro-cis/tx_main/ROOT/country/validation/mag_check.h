#ifndef _MAG_CHECK_H_
#define _MAG_CHECK_H_

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

#define OUT_OF_RANGE1 40
#define OUT_OF_RANGE2 -40
#define STANDARD_LEVEL 100.0f
#define MAG_STANDARD_LEVEL 40.0f
#define MAG_AREA_PERCENTAGE 20.0f
//#define EXCESS_MAG_PERCENTAGE 50.0f//3.29f 2021/07/28 CNY�G���[���
#define MAG_WHITE 245
#define MAG_BLACK 10
#define CONSTANT_VALUE 5.0f // ���C���m�̈����4%�̎��C�����鎞�Ƀ��x���̌v�Z���ʂ�40�ƂȂ�萔

#define MAG_MAX_STRUCT 10 //MAG�\���̐�
//MAG�p�̔z��
typedef struct
{
	ST_IMUF_POITNS	*ppoint;		// ���W�p�����[�^�z��
	
	u16 mag_mask_ptn_diameter_x;		// IR�}�X�N�p�^�[���̒��a
	u16 mag_mask_ptn_diameter_y;		// IR�}�X�N�p�^�[���̒��a
	float mag_mask_ptn_divide_num;	// IR�}�X�N�p�^�[���̊���l
	u8* pmag_mask_ptn;				// IR�}�X�N�p�^�[���̃|�C���^
	float average[MAG_MAX_STRUCT];	// ����

	u8 point_number; // �p�����[�^�Ŏw�肵���̈�̐�
	u8 result; // ����
	u8 level; // ���x��
	u8 padding;

	u8 ave_number;
	u8 per_number;
	float percent[MAG_MAX_STRUCT];

} ST_MAG;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s32 get_mag_valid_count(u8 buf_num, ST_MAG *st);
	s16 new_L2P_Coordinate_MagUv(ST_SPOINT * __restrict spt, ST_BS * __restrict pbs, ST_NOTE_PARAMETER * __restrict pnote_param);
	s16 P_Getdt_8bit_only_MagUv(ST_SPOINT *__restrict spt, ST_BS *__restrict pbs);
	s32 point_vicinity_cal_Mag(ST_POINT_VICINITY* __restrict pv, u8 buf_num, float std_para1, float std_para2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

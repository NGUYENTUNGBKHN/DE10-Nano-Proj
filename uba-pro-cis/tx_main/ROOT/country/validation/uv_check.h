#ifndef _UV_CHECK_H_
#define _UV_CHECK_H_

/*
2021/12/2
�f�o�b�O�p��CSV�o�͂ł���悤��

2022/6/7
UV_Fitness���x���o�͕��@�ύX

2022/7/15
���ԏ��ƌ��ʃu���b�N�ύX

*/


#define UV_STANDARD_LEVEL 40 // ���x���v�Z�p�@
#define UV_MULTI_LEVEL 10 // ���x���v�Z�p�A
#define UV_DIFFERENCE -2
#define OUT_OF_RANGE -30
static enum P_Plane_tbl uv_plane_tbl[] = { UP_R_FL, DOWN_R_FL };

//UV�p�̔z��
// �����͑S�́E�ꕔ���m�̗�����ST_UV_VALIDATE�z��ɕς��BST_UV�̕��͕ύX�O�Ȃ̂Ŏc���Ă���
typedef struct
{
	ST_UV_POINTS	*ppoint;		// ���W�p�����[�^�z��
	
	u16 uv_mask_ptn_diameter_x;		// UV�}�X�N�p�^�[���̒��a�i�Œ�j
	u16 uv_mask_ptn_diameter_y;		// UV�}�X�N�p�^�[���̒��a�i�Œ�j
	float uv_mask_ptn_divide_num;	// UV�}�X�N�p�^�[���̊���l�i�Œ�j
	u8* puv_mask_ptn;				// UV�}�X�N�p�^�[���̃|�C���^�i�Œ�j

	float average[IMUF_POINT_NUMBER];	// ����

	u8 point_number; // �p�����[�^�Ŏw�肵���̈�̐�
	u8 result; // ����
	u8 level; // ���x��
	u8 padding;
} ST_UV_VALIDATE;

//�����ĂȂ�
//typedef struct
//{
//	ST_IMUF_POITNS	*ppoint;		// ���W�p�����[�^�z��
//	
//	u16 uv_mask_ptn_diameter_x;		// UV�}�X�N�p�^�[���̒��a�i�Œ�j
//	u16 uv_mask_ptn_diameter_y;		// UV�}�X�N�p�^�[���̒��a�i�Œ�j
//	float uv_mask_ptn_divide_num;	// UV�}�X�N�p�^�[���̊���l�i�Œ�j
//	u8* puv_mask_ptn;				// UV�}�X�N�p�^�[���̃|�C���^�i�Œ�j
//
//	float average[IMUF_POINT_NUMBER];	// ����
//
//	u8 point_number; // �p�����[�^�Ŏw�肵���̈�̐�
//	u8 result; // ����
//	u8 level; // ���x��
//	u8 padding;
//} ST_UV;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	//s32 get_uv_invalid_count(u8 buf_num, ST_UV *st); // ���֐�
	s32 get_uv_fitness(u8 buf_num, ST_UV_VALIDATE* uv); // �u�����m�i�S�́j������
	s32 get_uv_validate(u8 buf_num, ST_UV_VALIDATE* uv);// �u�����m�i�ꕔ�j���ӕ�
	u8 calc_uv_level(float level1, float level2, u8 point_number, ST_UV_VALIDATE* uv, u16 flag); // �u�����m�̃��x���v�Z
	u8 calc_uv_fitness_level(float level1, float level2, u8 point_number, ST_UV_VALIDATE* uv, u16 flag);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

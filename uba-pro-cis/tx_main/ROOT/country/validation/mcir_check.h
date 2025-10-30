#ifndef _MCIR_CHECK_H_
#define _MCIR_CHECK_H_

static enum P_Plane_tbl mcir_plane_tbl_ir1[] = {UP_R_IR1, DOWN_R_IR1};
static enum P_Plane_tbl mcir_plane_tbl_red[] = {UP_R_R,   DOWN_R_R};
static enum P_Plane_tbl mcir_plane_tbl_grn[] = {UP_R_G,   DOWN_R_G};

/*
static enum P_Plane_tbl mcir_plane_tbl1[] = {UP_R_IR1, DOWN_R_IR1,UP_R_IR1, DOWN_R_IR1, UP_R_G,   DOWN_R_G };
static enum P_Plane_tbl mcir_plane_tbl2[] = {UP_R_R,   DOWN_R_R,  UP_R_G,   DOWN_R_G,   UP_R_R,   DOWN_R_R };
static enum P_Plane_tbl mcir_plane_tbl3[] = {UP_R_G,   DOWN_R_G,  UP_R_R,   DOWN_R_R,   UP_R_IR1, DOWN_R_IR1, };

*/

#define MCIR_RATIO_MULTIPLIER	 1000
#define MCIR_MAX_POINT			 120	//�ő�|�C���g��
#define MCIR_MAX_MODE			 6	//���[�h��
#define MCIR_CONSTANT			13 // ���x���v�Z�p�̒萔 = invalid_cnt��3�̂Ƃ��Ƀ��x����60���x�ɂȂ�悤�ɂ��邽�߂̂���
#define MCIR_STANDARD_LEVEL 100 // �ő僌�x���i�������猸�_���Ă����j

enum MCIR_CHECK
{
	IR_MINUS_R_UP = 0,
	IR_MINUS_R_DOWN,
	IR_MINUS_G_UP,
	IR_MINUS_G_DOWN,
	G_MINUS_R_UP,
	G_MINUS_R_DOWN,
};

//MCIR�p�̔z��
typedef struct
{
	s8 cal_mode[MCIR_MAX_MODE];			//�v�Z���[�h
	u8 padding[2];
	ST_LIM_AND_POINTS *ppoint;					//�e���[�h�̔z��

	u16 red_mask_ptn_diameter_x;			// �ԐF�}�X�N�p�^�[���̒��ax
	u16 red_mask_ptn_diameter_y;			// �ԐF�}�X�N�p�^�[���̒��ay
	float red_mask_ptn_divide_num;			// �ԐF�}�X�N�p�^�[���̊��鐔
	u8*	pred_mask_ptn;						// �ԐF�}�X�N�p�^�[��1�̃|�C���^

	u16 grn_mask_ptn_diameter_x;			// �ΐF�}�X�N�p�^�[���̒��ax
	u16 grn_mask_ptn_diameter_y;			// �ΐF�}�X�N�p�^�[���̒��ay
	float grn_mask_ptn_divide_num;			// �ΐF�}�X�N�p�^�[���̊��鐔
	u8*	pgrn_mask_ptn;						// �ΐF�}�X�N�p�^�[��1�̃|�C���^

	u16 ir1_mask_ptn_diameter_x;			// IR1�F�}�X�N�p�^�[���̒��ax
	u16 ir1_mask_ptn_diameter_y;			// IR1�F�}�X�N�p�^�[���̒��ay
	float ir1_mask_ptn_divide_num;			// IR1�F�}�X�N�p�^�[���̊��鐔
	u8*	pir1_mask_ptn;						// IR1�F�}�X�N�p�^�[��1�̃|�C���^
	u8 level; // ���x��
	u8 padding2[3];

} ST_MCIR;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	int get_mcir_invalid_count(u8 buf_num, ST_MCIR *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

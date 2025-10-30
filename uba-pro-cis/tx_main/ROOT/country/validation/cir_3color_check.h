#ifndef _CIR_3COLOR_CHECK_H_
#define _CIR_3COLOR_CHECK_H_


static enum P_Plane_tbl cir3_plane_tbl_ir[] = {UP_R_IR1, DOWN_R_IR1};

static enum P_Plane_tbl cir3_plane_tbl_red[] = {UP_R_R,   DOWN_R_R };

static enum P_Plane_tbl cir3_plane_tbl_grn[] = {UP_R_G,   DOWN_R_G };

/*
static enum P_Plane_tbl cir3_plane_tbl1[] = {UP_R_IR1, DOWN_R_IR1, UP_R_R,   DOWN_R_R,   UP_R_G,   DOWN_R_G };

static enum P_Plane_tbl cir3_plane_tbl2[] = {UP_R_R,   DOWN_R_R,   UP_R_G,   DOWN_R_G,   UP_R_IR1, DOWN_R_IR1 };

static enum P_Plane_tbl cir3_plane_tbl3[] = {UP_R_G,   DOWN_R_G,   UP_R_IR1, DOWN_R_IR1, UP_R_R,   DOWN_R_R };
*/

#define CIR_3COLOR_CHECK_COUNT 6		//���[�h��
#define CIR_3COLOR_CHECK_MAX_POINT 120	//�ő�|�C���g��


enum CIR_3COLOR_CHECK
{
	CIR_3COLOR_IR_UP = 0,
	CIR_3COLOR_IR_DOWN,
	CIR_3COLOR_RED_UP,
	CIR_3COLOR_RED_DOWN,
	CIR_3COLOR_GREEN_UP,
	CIR_3COLOR_GREEN_DOWN
};


//3CIR�p�̔z��
typedef struct
{
	s8 cal_mode[CIR_3COLOR_CHECK_COUNT];						//�v�Z���[�h
	u8 blank[2];
	//ST_LIM_AND_POINTS	*ppoint[CIR_3COLOR_CHECK_COUNT][CIR_3COLOR_CHECK_MAX_POINT];	//�e���[�h�̔z��
	ST_LIM_AND_POINTS	*ppoint;	//�e���[�h�̔z��

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



} ST_3CIR;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_cir_3color_invalid_count(u8 buf_num ,ST_3CIR *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

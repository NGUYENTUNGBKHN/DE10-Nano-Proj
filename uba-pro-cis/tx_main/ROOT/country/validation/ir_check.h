#ifndef _IR_CHECK_H_
#define _IR_CHECK_H_

static enum P_Plane_tbl ir_plane_tbl[] = {UP_R_IR1, DOWN_R_IR1};

#define IR_CHECK_COUNT 2

#define IR_BASE_COUNT 4
#define IR_CHECK_MAX_POINT 120	//�ő�|�C���g��

enum IR_CHECK
{
	IR_CHECK_UP = 0,
	IR_CHECK_DOWN,
};

//IR�`�F�b�N�p�z��
typedef struct
{	
	s8  base_point_x1;	//�x�[�X�f�[�^�쐬�p
	s8  base_point_y1;	//�x�[�X�f�[�^�쐬�p
	s8  base_point_x2;	//�x�[�X�f�[�^�쐬�p
	s8  base_point_y2;	//�x�[�X�f�[�^�쐬�p
	s8  base_point_x3;	//�x�[�X�f�[�^�쐬�p
	s8  base_point_y3;	//�x�[�X�f�[�^�쐬�p
	s8  base_point_x4;	//�x�[�X�f�[�^�쐬�p
	s8  base_point_y4;	//�x�[�X�f�[�^�쐬�p
	
} ST_IR_MODES;

typedef struct
{

	s8 cal_mode[IR_CHECK_COUNT];            //�v�Z���[�h
	u8 blank[2];
	ST_IR_MODES	*pmode;						//�x�[�X�f�[�^�̃p�����^
	ST_LIM_AND_POINTS*	ppoint;				//�|�C���g�f�[�^�̃p�����^

	u16 ir1_mask_ptn_diameter_x;			// IR1�F�}�X�N�p�^�[���̒��ax
	u16 ir1_mask_ptn_diameter_y;			// IR1�F�}�X�N�p�^�[���̒��ay
	float ir1_mask_ptn_divide_num;			// IR1�F�}�X�N�p�^�[���̊��鐔
	u8*	pir1_mask_ptn;						// IR1�F�}�X�N�p�^�[��1�̃|�C���^
	
} ST_IR_CHECK;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_ir_check_invalid_count(u8 buf_num ,ST_IR_CHECK *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

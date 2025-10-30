#ifndef _FOLDING_CHECK_H_
#define _FOLDING_CHECK_H_

static enum P_Plane_tbl tir1_plane_tbl_tir1[] = { UP_T_IR1, DOWN_T_IR1 };

#define HOLOGRAM_THRESHOLD 210 // �w�i���̖��邷����ӏ��𖳎����邽�߂̂������l
#define WINDOW_THRESHOLD 33 // �Â�����Ƃ���𖳎����邽�߂̂������l �z���O�����Ȃǁi���O���t�j
#define EDGE_NUMBER 4 // �G�b�W�i�[�j�̐�
#define FOLDED_LEVEL 2// �܏􃌃x���v�Z�p�̒萔
#define DECREASE_THRESH 0.6f // �܏􃌃x���v�Z�p�̒萔
#define CONSTANT_FOR_FOLDING 5 // �܏􃌃x���v�Z�p�̒萔�@�e�����ʂ̂��߁A��拑�ۓ��̐ݒ�̓��x���ݒ�őΉ����邱��
#define CONSTANT_FOR_MUTILATION 10 // �������x���v�Z�p�̒萔�@�e�����ʂ̂��߁A��拑�ۓ��̐ݒ�̓��x���ݒ�őΉ����邱��
#define FOLDING_LEVEL 60 // �܏�̕W�����x��
#define MUTILATION_LEVEL 80 // �����̕W�����x��
//�܂��݌��m�p�̔z��
typedef struct
{
	ST_IMUF_POITNS *ppoint;			// ���W�p�����[�^�z��
	u16 tir1_mask_ptn_diameter_x;	// T_IR�}�X�N�p�^�[���̒��a
	u16 tir1_mask_ptn_diameter_y;	// T_IR�}�X�N�p�^�[���̒��a
	float tir1_mask_ptn_divide_num;	// T_IR�}�X�N�p�^�[���̊���l
	u8* ptir1_mask_ptn;				// T_IR�}�X�N�p�^�[���̃|�C���^
	u8 folded[EDGE_NUMBER];
	u8 folded_level[EDGE_NUMBER];
	u8 err_code;					// �G���[�R�[�h �O�F��A�P�F���A�Q�F���A�R�F�E
	u8 padding[3];

	float tir_diff[EDGE_NUMBER];

}ST_FOLDING;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	u8 size_level(u16 size);
	s32 get_folding_invalid_count(u8 buf_num, ST_FOLDING *st, u16 x_sz, u16 y_sz);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

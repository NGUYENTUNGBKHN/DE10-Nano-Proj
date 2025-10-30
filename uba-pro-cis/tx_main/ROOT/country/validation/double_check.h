#ifndef _DOUBLE_CHECK_H_
#define _DOUBLE_CHECK_H_

#define DOUBLE_CHECK_THERSHOLD 10	//臒l �d��

//����R�[�h
#define REJ_DOUBLE_NOTE			(0x1813)	//�d������

// ichijo 2020.08.31
#define DOUBLE_UNFIT 40 // unfit���x��
#define DOUBLE_LEVEL 100 // ���̒l�ƌv�Z���ʂ̍����Ń��x�������肷�� ���_����
//#define NEW_DOUBLE_CHECK // �V�����d���A���S���g�p����ꍇ�ɗL���ɂ���B���ꎞ�I�ȑΉ��ŁA���ׂĂ̍��ōĊw�K���ł�����ߋ��̃\�[�X�͔r������B
//�d���`�F�b�N
typedef struct
{
	u16 max_x;	//�������ő�
	u16 max_y;	
	u16 blank;
	u16 threshold;		//臒l

	s16 note_size_x;
	s16 note_size_y;

	u16 red_mask_ptn_diameter_x;			// �ԐF�}�X�N�p�^�[���̒��ax
	u16 red_mask_ptn_diameter_y;			// �ԐF�}�X�N�p�^�[���̒��ay
	float red_mask_ptn_divide_num;			// �ԐF�}�X�N�p�^�[���̊��鐔
	u8*	pred_mask_ptn;						// �ԐF�}�X�N�p�^�[��1�̃|�C���^

	u16 tred_mask_ptn_diameter_x;			// ���ߐԃ}�X�N�p�^�[���̒��ax
	u16 tred_mask_ptn_diameter_y;			// ���ߐԃ}�X�N�p�^�[���̒��ay
	float tred_mask_ptn_divide_num;			// ���ߐԃ}�X�N�p�^�[���̊��鐔
	u8*	ptred_mask_ptn;						// ���ߐԃ}�X�N�p�^�[��1�̃|�C���^
	u8 level; // ���x��
	u8 padding[3]; 

} ST_DOUBLE_CHECK;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_double_check_error(u8 buf_num ,ST_DOUBLE_CHECK *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif 

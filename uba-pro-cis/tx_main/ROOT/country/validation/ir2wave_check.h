#ifndef _IR2WAVE_CHECK_H_
#define _IR2WAVE_CHECK_H_


/*
2021/12/2
IR2�Ȃ���1�Ԗڂ̂ݔ��肵�Ă����̂��C��
�f�o�b�O�p��CSV�o�͂ł���悤��

2022/07/14
IR2�g��������ύX
*/

//IR2wave�p�̔z��
typedef struct
{
	ST_IMUF_POITNS	*ppoint;		// ���W�p�����[�^�z��

	u16 ir1_mask_ptn_diameter_x;	// IR�}�X�N�p�^�[���̒��a�iIR1/2���ʁj
	u16 ir1_mask_ptn_diameter_y;	// IR�}�X�N�p�^�[���̒��a�iIR1/2���ʁj
	float ir1_mask_ptn_divide_num;	// IR�}�X�N�p�^�[���̊���l�iIR1/2���ʁj
	u8* pir1_mask_ptn;				// IR�}�X�N�p�^�[���̃|�C���^�iIR1/2���ʁj
	
	u8 point_number;

	//
	u8 ir1peak[IMUF_POINT_NUMBER];
	u8 ir2peak[IMUF_POINT_NUMBER];
	u32 ir1freq[IMUF_POINT_NUMBER];
	u32 ir2freq[IMUF_POINT_NUMBER];
	float ir1pro[IMUF_POINT_NUMBER];
	float ir2pro[IMUF_POINT_NUMBER];
	u8 feature[IMUF_POINT_NUMBER];

	//output
	u8 number;
	u8 output[IMUF_POINT_NUMBER];

	u8 err_code;
	u8 result;
	u8 level;

} ST_IR2WAVE;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	s32 get_ir2wave_invalid_count(u8 buf_num, ST_IR2WAVE *st);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

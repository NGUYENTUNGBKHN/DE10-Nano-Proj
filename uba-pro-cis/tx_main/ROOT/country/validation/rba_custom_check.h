#ifndef _RBA_CUSTOM_CHECK_H_
#define _RBA_CUSTOM_CHECK_H_

//�d���`�F�b�N
typedef struct
{
	u16 max_x;	//�������ő�
	u16 max_y;	
	u16 blank;
	u16 threshold;		//臒l

	s16 note_size_x;
	s16 note_size_y;

	u16 ir_mask_ptn_diameter_x;				// ����IR1�F�}�X�N�p�^�[���̒��ax
	u16 ir_mask_ptn_diameter_y;				// ����IR1�F�}�X�N�p�^�[���̒��ay
	float ir_mask_ptn_divide_num;			// ����IR1�F�}�X�N�p�^�[���̊��鐔
	u8*	pir_mask_ptn;						// ����IR1�F�}�X�N�p�^�[��1�̃|�C���^

	u16 tir_mask_ptn_diameter_x;			// ����IR1�}�X�N�p�^�[���̒��ax
	u16 tir_mask_ptn_diameter_y;			// ����IR1�}�X�N�p�^�[���̒��ay
	float tir_mask_ptn_divide_num;			// ����IR1�}�X�N�p�^�[���̊��鐔
	u8*	ptir_mask_ptn;						// ����IR1�}�X�N�p�^�[��1�̃|�C���^
	
	s32 rba_custom_check_min;
	s32 rba_custom_check_total_value;

	s32 rba_custom_check_value;
} ST_RBA_CUSTOM_CHECK;

typedef struct
{
	u16 threshold;		//臒l
	u16 blank;

	s16 note_size_x;
	s16 note_size_y;

	u16 ir2_mask_ptn_diameter_x;			// ����IR2�F�}�X�N�p�^�[���̒��ax
	u16 ir2_mask_ptn_diameter_y;			// ����IR2�F�}�X�N�p�^�[���̒��ay
	float ir2_mask_ptn_divide_num;			// ����IR2�F�}�X�N�p�^�[���̊��鐔
	u8*	pir2_mask_ptn;						// ����IR2�F�}�X�N�p�^�[��1�̃|�C���^

	s32 point_check_value;
} ST_POINT_CHECK;

typedef struct
{
	u16 threshold;		//臒l
	u16 blank;

	s16 note_size_x;
	s16 note_size_y;

	s32 ref_check_min;
	s32 ref_check_value;
} ST_REF_CHECK;

typedef struct
{
	u16 threshold;		//臒l
	u16 blank;

	s16 note_size_x;
	s16 note_size_y;

	s32 pen_check_value;
} ST_PEN_CHECK;


enum{
	CUSTOM_CHECK_RESULT_GENUINE_NOTE,
	CUSTOM_CHECK_RESULT_DBL_NOTE,
	CUSTOM_CHECK_RESULT_FAKE_NOTE,
};


struct rba_cs_check
{
	s32 check;
	s32 index;
	s32 x_min;
	s32 x_max;
	s32 y_min;
	s32 y_max;
	s32 min_limit;
	s32 max_limit;
};
typedef struct rba_cs_check RBA_CS_CHECK;

struct point_check
{
	s32 check;
	s32 index;
	s32 x;
	s32 y;
	u8  plane1;
	u8  plane2;
	u8  max_limit;
	u8  blank;
};

typedef struct point_check PT_CHECK;

struct ref_check
{
	s32 check;
	s32 index;
	s32 x;
	s32 y;
	u8  plane1;
	u8  plane2;
	u8  max_limit;
	u8  blank;
};

typedef struct ref_check RF_CHECK;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

s32 get_mesh_data_custom(s32 x, s32 y, s32 plane, s32 diameter_x , s32 diameter_y, u8* mask, u8 buf_num, float divide_num);
s32 get_rba_custom_check_error(u8 buf_num ,ST_RBA_CUSTOM_CHECK *st, s32 denomination, s32 way);
s32 get_point_check_error(u8 buf_num ,ST_POINT_CHECK *st, s32 denomination, s32 way);
s32 get_ref_check_error(u8 buf_num ,ST_REF_CHECK* ref_check, s32 denomination, s32 way);
s32 get_pen_check_error(u8 buf_num ,ST_PEN_CHECK* pen_check, s32 denomination, s32 way);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_CUSTOM_CHECK_H_

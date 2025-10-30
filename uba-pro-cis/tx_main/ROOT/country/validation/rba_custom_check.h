#ifndef _RBA_CUSTOM_CHECK_H_
#define _RBA_CUSTOM_CHECK_H_

//重券チェック
typedef struct
{
	u16 max_x;	//等分割最大
	u16 max_y;	
	u16 blank;
	u16 threshold;		//閾値

	s16 note_size_x;
	s16 note_size_y;

	u16 ir_mask_ptn_diameter_x;				// 反射IR1色マスクパターンの直径x
	u16 ir_mask_ptn_diameter_y;				// 反射IR1色マスクパターンの直径y
	float ir_mask_ptn_divide_num;			// 反射IR1色マスクパターンの割る数
	u8*	pir_mask_ptn;						// 反射IR1色マスクパターン1のポインタ

	u16 tir_mask_ptn_diameter_x;			// 透過IR1マスクパターンの直径x
	u16 tir_mask_ptn_diameter_y;			// 透過IR1マスクパターンの直径y
	float tir_mask_ptn_divide_num;			// 透過IR1マスクパターンの割る数
	u8*	ptir_mask_ptn;						// 透過IR1マスクパターン1のポインタ
	
	s32 rba_custom_check_min;
	s32 rba_custom_check_total_value;

	s32 rba_custom_check_value;
} ST_RBA_CUSTOM_CHECK;

typedef struct
{
	u16 threshold;		//閾値
	u16 blank;

	s16 note_size_x;
	s16 note_size_y;

	u16 ir2_mask_ptn_diameter_x;			// 反射IR2色マスクパターンの直径x
	u16 ir2_mask_ptn_diameter_y;			// 反射IR2色マスクパターンの直径y
	float ir2_mask_ptn_divide_num;			// 反射IR2色マスクパターンの割る数
	u8*	pir2_mask_ptn;						// 反射IR2色マスクパターン1のポインタ

	s32 point_check_value;
} ST_POINT_CHECK;

typedef struct
{
	u16 threshold;		//閾値
	u16 blank;

	s16 note_size_x;
	s16 note_size_y;

	s32 ref_check_min;
	s32 ref_check_value;
} ST_REF_CHECK;

typedef struct
{
	u16 threshold;		//閾値
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

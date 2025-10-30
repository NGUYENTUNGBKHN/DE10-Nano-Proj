#ifndef _DATA_CONVERT_H_
#define _DATA_CONVERT_H_

#define VALI_DIR 4				//4方向
#define MAX_DENOMI 100		//最大金種
#define NOMAL_THERSHOLD 200	//閾値 
#define CIR_RATIO_MULTIPLIER 1000		//定数
#define CIR_CHECK_POINT_COUNT 121	//最大ポイント数



//cirシリーズ用 座標リミットデータ
typedef struct
{
	s16  x;
	s16  y;
	s16 limit_min;	//閾値
	s16 limit_max;	//閾値
} ST_LIM_AND_POINTS;

typedef struct
{
	u8 point_num;		//要素数
	u8 blank[3];
	ST_LIM_AND_POINTS	*ppoint[CIR_CHECK_POINT_COUNT];//ポイントデータのパラメタ
	
} ST_CIR_MODES;

//鑑別nn座標パラメタ構造体
typedef struct
{
	s8  x;		//x
	s8  y;		//y
	u16 side;	//表裏
	
} ST_NN_POINTS;

// 新鑑別・新フィットネス用パラメタ構造体..ichijo
typedef struct
{
	s16 x_s;//始点x
	s16 y_s;//始点y
	s16 x_e;//終点x
	s16 y_e;//終点y
	float threshold1;//しきい値1
	float threshold2;//しきい値2
	float threshold3;//しきい値3
	float threshold4;//しきい値4
} ST_IMUF_POITNS;// IrMagUvFolding->IMUF

typedef struct
{
	s16 x_s;//始点x
	s16 y_s;//始点y
	s16 x_e;//終点x
	s16 y_e;//終点y
	float threshold;//しきい値1
	u8 side;			//表裏フラグ：０＝表、１＝裏
	u8 padding[3];//パディング
	float reserved1;//予備1
	float reserved2;//予備2
}ST_UV_POINTS;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_note_x_boundary(void *p);
int get_note_y_boundary(void *p);
int get_mesh_x_boundary(int diameter, void* p);
int get_mesh_y_boundary(int diameter, void* p);
unsigned char get_pixel_value(int x, int y, unsigned char plane, void* p);
s32 get_mesh_data(s32 x, s32 y, s32 plane, s32 diameter_x , s32 diameter_y, u8* mask, u8 buf_num, float divide_num);
int get_mesh_data2(int x, int y, int plane, int buf_num);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

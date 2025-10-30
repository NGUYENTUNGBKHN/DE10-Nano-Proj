#define EXT
#include "../common/global.h"


//探索用のマスクパターン　2㎜×2mm
EXTERN u8 vali_mask_ptn[] = 
	{
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,

	};


int get_note_x_boundary(void *p) 
{

	//ST_NOTE_PARAM param;
	//DLL_get_note_param(p, &param);
	return (work[0].pbs->note_x_size - 1) / 2;
}

int get_note_y_boundary(void* p) 
{
	//ST_NOTE_PARAM param;
	//DLL_get_note_param(p, &param);
	return (work[0].pbs->note_y_size - 1) / 2;
}

int get_mesh_x_boundary(int diameter, void* p) 
{
	return (get_note_x_boundary(p) - diameter / 2) / diameter;
}

int get_mesh_y_boundary(int diameter, void* p)
{
	return (get_note_y_boundary(p) - diameter / 2) / diameter;
}

//unsigned char get_pixel_value(int x, int y, unsigned char plane, void *p)
//{
//	ST_SPOINT spt;
//	spt.way = W0;
//	spt.l_plane_tbl = (enum L_Plane_tbl)plane;
//	spt.x = x;
//	spt.y = y;
//
//	return (unsigned char)new_L_Getdt(&spt, 0);
//}


//diameter_yを追加しました。　19/02/07 furuta
s32 get_mesh_data(s32 x, s32 y, s32 plane, s32 diameter_x , s32 diameter_y, u8* mask, u8 buf_num, float divide_num)
{
	//s32 i = 0;
	ST_POINT_VICINITY pv;
	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	ST_BS* pbs = work[buf_num].pbs;

	//L2P用のパラメタセット
	spt.l_plane_tbl = (u8)plane;

	/*論理座標から物理に変換*/
	//diameterはxyともに200dpi単位を乗算する
	spt.x = (x * diameter_x);		//seとleで式が異なる　修正必要　190920　furuta 
	spt.y = (y * diameter_x);
#ifdef VS_DEBUG_ITOOL
	for_iTool_trace_callback(&spt, pbs, pnote_param, diameter_x, diameter_y, mask,0);
	spt.trace_flg = 0;
#endif
	new_L2P_Coordinate(&spt ,pbs ,pnote_param);
	//new_P2L_Coordinate(&spt, pbs);
	pv.x = (s16)spt.x;
	pv.y = (s16)spt.y;

	//論理座標planeをテーブル引き
	pv.plane = (u8)plane;
	
	//マスクサイズとポインタの変更をお願い致します
	pv.filter_size_x = (u16)diameter_x;
	pv.filter_size_y = (u16)diameter_y;
	pv.pfilter_pat = (u8*)mask;

	//divide_valも予め計算して引数で渡すことにします。
	//その渡す値は逆数とする。　19/02/08 furuta
	pv.divide_val = divide_num;

	point_vicinity_cal(&pv, buf_num);	//物理モード


	return pv.output;

}


int get_mesh_data2(int x, int y, int plane, int buf_num) 
{
	return	0;//data_extraction.pitch_mode_out_put[buf_num][plane][y][x];
}

//float get_divide_num(u8 * mask_ptn ,u32 diameter_x ,u32 diameter_y )
//{
//	u32 i;
//	float sum = 0;
//
//	for (i = 0; i < diameter_x * diameter_y; i++) 
//	{
//		sum += mask_ptn[i];
//	}
//
//	return 1 / sum;
//}


//int get_mesh_soft_cash_check_and_regist(int x, int y, int plane, int buf_num ,int data) 
//{
//	//引数とマッチするか？
//
//	u32 i = 0;
//	u32 much = 0;
//	u32 no = 0;
//
//
//	for(i = 0; i < soft_cash[buf_num].regist_count; ++i)
//	{
//		if(soft_cash[buf_num].plane[i] == plane && soft_cash[buf_num].x[i] == x &&	soft_cash[buf_num].y[i] == y)
//		{
//			much = 1;
//			break;
//		}
//	}
//
//	//ミスヒット場合、登録する 
//	if(much == 0)
//	{
//		no = soft_cash[buf_num].regist_count;
//		soft_cash[buf_num].plane[no] = plane;
//		soft_cash[buf_num].x[no] = x;
//		soft_cash[buf_num].y[no] = y;
//		soft_cash[buf_num].data[no] = data;
//
//		soft_cash[buf_num].regist_count++;
//
//		return 0;
//	}
//	else if(much == 1)	//ヒットした場合
//	{
//		return soft_cash[buf_num].data[i];
//	}
//
//	return 1;
//}

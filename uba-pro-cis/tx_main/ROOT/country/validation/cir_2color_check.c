
#include	<math.h>
#include    <string.h>
#include    <float.h>
#include	<stdlib.h>
#define EXT
#include "../common/global.h"
#include "cir_2color_check.h"


//demomiと方向をNNの出力ノードと対応させた方が良い？
int get_cir_2color_invalid_count()
{

	//u32 val1;				//パラ：ポイント値
	//u32 val2;	
	//s32 res;				//パラ：計算結果
	//u16 point_cnt = 0;		//パラ：カレントポイント
	//u8 plane1=0;			//パラ：プレーン番号
	//u8 plane2=0;
	//u16 mode_cnt = 0;		//パラ：モード
	//u16 mode = 0;			//パラ：モード

	u16 invalid_count = 0;	//出力：無効カウント

	//u8 way = 0;				//入力：方向
	//u16 denomi = 0;			//入力：金種

	//for(mode_cnt = 0; mode_cnt < CIR_2COLOR_CHECK_COUNT; ++mode_cnt)
	//{
	//	//モードが-1なら終わり
	//	if(cir2.cal_mode[mode_cnt] == -1)
	//	{
	//		break;
	//	}

	//	//モードの設定
	//	mode = cir2.cal_mode[mode_cnt];

	//	//モードに応じてテーブルからプレーンをゲット
	//	plane1 = cir2_plane_tbl1[mode];
	//	plane2 = cir2_plane_tbl2[mode];


	//	//計算する　0,0,0,0のエンドマークが来るまで繰り返す
	//	while(cir2.mode[denomi][way][mode].point[point_cnt].x != 0 && cir2.mode[denomi][way][mode].point[point_cnt].y != 0 &&
	//		cir2.mode[denomi][way][mode].point[point_cnt].limit_min != 0 && cir2.mode[denomi][way][mode].point[point_cnt].limit_max != 0)
	//	{
	//		//ポイントデータ採取
	//		val1 = get_mesh_data2(cir2.mode[denomi][way][mode].point[point_cnt].x, cir2.mode[denomi][way][mode].point[point_cnt].y, plane1 ,buf_num);
	//		val2 = get_mesh_data2(cir2.mode[denomi][way][mode].point[point_cnt].x, cir2.mode[denomi][way][mode].point[point_cnt].y, plane2 ,buf_num);

	//		//計算
	//		res = val1 - val2;

	//		//閾値比較
	//		if (res < cir2.mode[denomi][way][mode].point[point_cnt].limit_min - CIR2_THERSHOLD || res > cir2.mode[denomi][way][mode].point[point_cnt].limit_max + CIR2_THERSHOLD) 
	//		{
	//			//無効ポイントカウント
	//			invalid_count++;
	//		}

	//		//次のポイントへ
	//		point_cnt++;

	//	}//while

	//}//for

	return invalid_count;
}

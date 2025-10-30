#include <stdio.h>
#include <string.h>
#include <math.h>

#define EXT
#include "../common/global.h"
#include "rba_custom_check.h"
#include "../src/data_rba_custom_check.h"



//custom check専用　add 2020-11-19 yuji
s32 get_mesh_data_custom(s32 x, s32 y, s32 plane, s32 diameter_x , s32 diameter_y, u8* mask, u8 buf_num, float divide_num)
{
	//s32 i = 0;
	ST_POINT_VICINITY pv;
	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	ST_BS* pbs = work[buf_num].pbs;

	//L2P用のパラメタセット
	spt.l_plane_tbl = (u8)plane;

	spt.x = -(x * diameter_x);
	spt.y = -(y * diameter_y);

	new_L2P_Coordinate(&spt ,pbs ,pnote_param);
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


s32 get_rba_custom_check_error(u8 buf_num ,ST_RBA_CUSTOM_CHECK* custom_check, s32 denomination, s32 way)
{
	s32 x = 0;
	s32 y = 0;

	int result = CUSTOM_CHECK_RESULT_GENUINE_NOTE;
	int total_value = 0;
	int total_count = 0;
	int ReflectiveUpValue = 0;
	int ReflectiveDownValue = 0;
	int TransparentValue = 0;

	//CString WriteStr = _T("");
	int min = 0xffff;

	RBA_CS_CHECK *cs_data = (RBA_CS_CHECK *)&CS_CHECK_PARAM[denomination][way];

	for (y = cs_data->y_min; y <= cs_data->y_max; y ++)
	{
#if VS_DEBUG
		WriteStr = _T("");
#endif
		for (x = cs_data->x_min; x <= cs_data->x_max; x++)
		{
			//get_mesh_data_customのほうを使用（旧関数でパラメタを作成したため）
			ReflectiveUpValue = get_mesh_data_custom(x, y, UP_R_IR1 ,    custom_check->ir_mask_ptn_diameter_x ,custom_check->ir_mask_ptn_diameter_y ,custom_check->pir_mask_ptn ,buf_num ,custom_check->ir_mask_ptn_divide_num);
			ReflectiveDownValue = get_mesh_data_custom(x, y, DOWN_R_IR1, custom_check->ir_mask_ptn_diameter_x ,custom_check->ir_mask_ptn_diameter_y ,custom_check->pir_mask_ptn ,buf_num ,custom_check->ir_mask_ptn_divide_num);
			TransparentValue = get_mesh_data_custom(x, y, DOWN_T_IR1 ,   custom_check->tir_mask_ptn_diameter_x ,custom_check->tir_mask_ptn_diameter_y ,custom_check->ptir_mask_ptn ,buf_num ,custom_check->tir_mask_ptn_divide_num);

			if (ReflectiveUpValue < 1)
			{
				ReflectiveUpValue = 1;
			}
			if (ReflectiveDownValue < 1)
			{
				ReflectiveDownValue = 1;
			}
			if (TransparentValue < 1)
			{
				TransparentValue = 1;
			}
#if VS_DEBUG
			WriteStr.AppendFormat(_T("%d,"), TransparentValue);
#endif
			total_value += (TransparentValue * 4 - ReflectiveUpValue - ReflectiveDownValue + 300) / 2;
			total_count++;

			if(min > TransparentValue)
			{
				min = TransparentValue;
			}
		}
#if VS_DEBUG
		WriteStr += _T("\n");
		CStdioFile sfSaveFile;
		CFileException e;
			
		CString SaveFilename = _T(".\\Output.csv");
		if (!sfSaveFile.Open(SaveFilename, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeReadWrite | CFile::typeText, &e))
		{
			_T("err");
			return 0x7FFF;
		}
		sfSaveFile.SeekToEnd();
		sfSaveFile.WriteString(WriteStr);
		sfSaveFile.Close();
#endif
	}
	if (total_count == 0)
	{
		total_count = 1;
	}

	total_value /= total_count;
	
#if VS_DEBUG
	return min;
	//return total_value;
#else

	if(min > cs_data->max_limit)//fake note check
	{
		result = CUSTOM_CHECK_RESULT_FAKE_NOTE;
	}
	else if(total_value < cs_data->min_limit)//dbl note check
	{
		result = CUSTOM_CHECK_RESULT_DBL_NOTE;
	}
	else
	{
		result = CUSTOM_CHECK_RESULT_GENUINE_NOTE;
	}

	custom_check->rba_custom_check_min = min;
	custom_check->rba_custom_check_total_value = total_value;

	return result;//0:genuine    1:double    2:fake
#endif
}

s32 get_point_check_error(u8 buf_num ,ST_POINT_CHECK* point_check, s32 denomination, s32 way)
{
	s32 invalid_count = 0;
	s32 x = 0;
	s32 y = 0;
	s32 ref = 0;
	s32 base_ref_ir2[3];
	u16 res1 = 0;
	u16 res2 = 0;
	u16 res3 = 0;
	s32 plane = 0;			//パラ：プレーン番号
	u16 mask_ptn_diameter_x;			// マスクパターンの直径x
	u16 mask_ptn_diameter_y;			// マスクパターンの直径y
	float mask_ptn_divide_num;			// マスクパターンの割る数
	u8*	p_mask_ptn;						// マスクパターン1のポインタ


	s32 ratio_x = 1;
	s32 ratio_y = 1;
	
	s32 max = 0x00000000;
	int index = 0;
	PT_CHECK *point_data = (PT_CHECK *)&PT_CHECK_PARAM[denomination][index];
#if VS_DEBUG
	CString WriteStr1 = _T("");
	CString WriteStr2 = _T("");
	CString WriteStr3 = _T("");
	CString WriteStr4 = _T("");
	CString SaveFilename = _T("");
	CStdioFile sfSaveFile;
	CFileException e;
#endif
	//パラメータ4方向統一化
	switch(way)
	{
	case W0:
		ratio_x = 1;
		ratio_y = 1;
		break;
	case W1:
		ratio_x = -1;
		ratio_y = -1;
		break;
	case W2:
		ratio_x = 1;
		ratio_y = -1;
		break;
	case W3:
		ratio_x = -1;
		ratio_y = 1;
		break;
	}
	//チェックするものが無い場合即終了
	if(point_data->check == 0)
	{
		return 0;
	}

	while(point_data->check)
	{
		if((way == W0) || (way == W1))
		{
			plane = point_data->plane1;

		}
		else
		{
			plane = point_data->plane2;
		}
		switch(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch)
		{
		case PLANE_PITCH_200DPI:
			mask_ptn_diameter_x = DIAMETER_200DPI_X;
			mask_ptn_diameter_y = DIAMETER_200DPI_Y;
			mask_ptn_divide_num = DIVIDE_200DPI;
			p_mask_ptn = MASK_PAT_200DPI;
			break;
		case PLANE_PITCH_100DPI:
			mask_ptn_diameter_x = DIAMETER_100DPI_X;
			mask_ptn_diameter_y = DIAMETER_100DPI_Y;
			mask_ptn_divide_num = DIVIDE_100DPI;
			p_mask_ptn = MASK_PAT_100DPI;
			break;
		case PLANE_PITCH_50DPI:
			mask_ptn_diameter_x = DIAMETER_50DPI_X;
			mask_ptn_diameter_y = DIAMETER_50DPI_Y;
			mask_ptn_divide_num = DIVIDE_50DPI;
			p_mask_ptn = MASK_PAT_50DPI;
			break;
		default:
			mask_ptn_diameter_x = DIAMETER_200DPI_X;
			mask_ptn_diameter_y = DIAMETER_200DPI_Y;
			mask_ptn_divide_num = DIVIDE_200DPI;
			p_mask_ptn = MASK_PAT_200DPI;
			break;
		}
		//方向別パラメータ変換
		x = point_data->x * ratio_x;
		y = point_data->y * ratio_y;

		//get_mesh_dataのほうを使用
		ref = get_mesh_data(x, y, plane, mask_ptn_diameter_x ,mask_ptn_diameter_y ,p_mask_ptn ,buf_num ,mask_ptn_divide_num);
		base_ref_ir2[0] = get_mesh_data(3*ratio_x, 0*ratio_y, plane, point_check->ir2_mask_ptn_diameter_x ,point_check->ir2_mask_ptn_diameter_y ,point_check->pir2_mask_ptn ,buf_num ,mask_ptn_divide_num);
		base_ref_ir2[1] = get_mesh_data(3*ratio_x, 2*ratio_y, plane, point_check->ir2_mask_ptn_diameter_x ,point_check->ir2_mask_ptn_diameter_y ,point_check->pir2_mask_ptn ,buf_num ,mask_ptn_divide_num);
		base_ref_ir2[2] = get_mesh_data(3*ratio_x, -2*ratio_y, plane, point_check->ir2_mask_ptn_diameter_x ,point_check->ir2_mask_ptn_diameter_y ,point_check->pir2_mask_ptn ,buf_num ,mask_ptn_divide_num);

		res1 = (s32)(abs(base_ref_ir2[0] - ref));
		res2 = (s32)(abs(base_ref_ir2[1] - ref));
		res3 = (s32)(abs(base_ref_ir2[2] - ref));

		if(max < res1)
		{
			max = res1;
		}
		if(max < res2)
		{
			max = res2;
		}
		if(max < res3)
		{
			max = res3;
		}

#if VS_DEBUG
		//WriteStr1.AppendFormat(_T("%d,"), res1);
		//WriteStr2.AppendFormat(_T("%d,"), res2);
		//WriteStr3.AppendFormat(_T("%d,"), res3);
		WriteStr4.AppendFormat(_T("%d,"), max);
#endif
		if(max > point_data->max_limit)
		{
			//無効ポイントカウント
			invalid_count++;
		}

		point_data++;// = (PT_CHECK *)&PT_CHECK_PARAM[denomination][index++];
	}
#if VS_DEBUG
#if 0
	WriteStr1 += _T("\n");
	SaveFilename = _T(".\\Output_res1.csv");
	if(!sfSaveFile.Open(SaveFilename, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeReadWrite | CFile::typeText, &e))
	{
		_T("err");
		return -1;
	}
	sfSaveFile.SeekToEnd();
	sfSaveFile.WriteString(WriteStr1);
	sfSaveFile.Close();

	WriteStr2 += _T("\n");
	SaveFilename = _T(".\\Output_res2.csv");
	if(!sfSaveFile.Open(SaveFilename, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeReadWrite | CFile::typeText, &e))
	{
		_T("err");
		return -1;
	}
	sfSaveFile.SeekToEnd();
	sfSaveFile.WriteString(WriteStr2);
	sfSaveFile.Close();

	WriteStr3 += _T("\n");
	SaveFilename = _T(".\\Output_res3.csv");
	if(!sfSaveFile.Open(SaveFilename, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeReadWrite | CFile::typeText, &e))
	{
		_T("err");
		return -1;
	}
	sfSaveFile.SeekToEnd();
	sfSaveFile.WriteString(WriteStr3);
	sfSaveFile.Close();
#endif
	WriteStr4 += _T("\n");
	SaveFilename = _T(".\\Output_res4.csv");
	if(!sfSaveFile.Open(SaveFilename, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeReadWrite | CFile::typeText, &e))
	{
		_T("err");
		return -1;
	}
	sfSaveFile.SeekToEnd();
	sfSaveFile.WriteString(WriteStr4);
	sfSaveFile.Close();
#endif
	return invalid_count;
}
#define VS_DEBUG 0
s32 get_ref_check_error(u8 buf_num ,ST_REF_CHECK* ref_check, s32 denomination, s32 way)
{
	s32 invalid_count = 0;
	s32 x = 0;
	s32 y = 0;
	s32 ref = 0;
	s32 plane = 0;			//パラ：プレーン番号
	u16 mask_ptn_diameter_x;			// マスクパターンの直径x
	u16 mask_ptn_diameter_y;			// マスクパターンの直径y
	float mask_ptn_divide_num;			// マスクパターンの割る数
	u8*	p_mask_ptn;						// マスクパターン1のポインタ

	s32 ratio_x = 1;
	s32 ratio_y = 1;
	
	s32 min = 0xffff;
	int index = 0;
	RF_CHECK *ref_data = (RF_CHECK *)&REF_CHECK_PARAM[denomination][index];
#if VS_DEBUG
	CString WriteStr = _T("");
	CString SaveFilename = _T("");
	CStdioFile sfSaveFile;
	CFileException e;
#endif
	//パラメータ4方向統一化
	switch(way)
	{
	case W0:
		ratio_x = 1;
		ratio_y = 1;
		break;
	case W1:
		ratio_x = -1;
		ratio_y = -1;
		break;
	case W2:
		ratio_x = 1;
		ratio_y = -1;
		break;
	case W3:
		ratio_x = -1;
		ratio_y = 1;
		break;
	}
	//チェックするものが無い場合即終了
	if(ref_data->check == 0)
	{
		return 0;
	}

	while(ref_data->check)
	{
		if((way == W0) || (way == W1))
		{
			plane = ref_data->plane1;
		}
		else
		{
			plane = ref_data->plane2;
		}

		switch(work[buf_num].pbs->PlaneInfo[plane].sub_sampling_pitch)
		{
		case PLANE_PITCH_200DPI:
			mask_ptn_diameter_x = DIAMETER_200DPI_X;
			mask_ptn_diameter_y = DIAMETER_200DPI_Y;
			mask_ptn_divide_num = DIVIDE_200DPI;
			p_mask_ptn = MASK_PAT_200DPI;
			break;
		case PLANE_PITCH_100DPI:
			mask_ptn_diameter_x = DIAMETER_100DPI_X;
			mask_ptn_diameter_y = DIAMETER_100DPI_Y;
			mask_ptn_divide_num = DIVIDE_100DPI;
			p_mask_ptn = MASK_PAT_100DPI;
			break;
		case PLANE_PITCH_50DPI:
			mask_ptn_diameter_x = DIAMETER_50DPI_X;
			mask_ptn_diameter_y = DIAMETER_50DPI_Y;
			mask_ptn_divide_num = DIVIDE_50DPI;
			p_mask_ptn = MASK_PAT_50DPI;
			break;
		default:
			mask_ptn_diameter_x = DIAMETER_200DPI_X;
			mask_ptn_diameter_y = DIAMETER_200DPI_Y;
			mask_ptn_divide_num = DIVIDE_200DPI;
			p_mask_ptn = MASK_PAT_200DPI;
			break;
		}

		//方向別パラメータ変換
		x = ref_data->x * ratio_x;
		y = ref_data->y * ratio_y;

		//get_mesh_dataのほうを使用
		ref = get_mesh_data(x, y, plane, mask_ptn_diameter_x ,mask_ptn_diameter_y ,p_mask_ptn ,buf_num ,mask_ptn_divide_num);
		if(ref < min)
		{
			min = ref;
		}

		ref_data++;// = (RF_CHECK *)&REF_CHECK_PARAM[denomination][index++];
	}

#if VS_DEBUG
	WriteStr.AppendFormat(_T("%d,"), min);
#endif
	if(min > REF_CHECK_THRESHOLD)
	{
		invalid_count++;
	}

#if VS_DEBUG
	WriteStr += _T("\n");
	SaveFilename = _T(".\\Output_res.csv");
	if(!sfSaveFile.Open(SaveFilename, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeReadWrite | CFile::typeText, &e))
	{
		_T("err");
		return -1;
	}
	sfSaveFile.SeekToEnd();
	sfSaveFile.WriteString(WriteStr);
	sfSaveFile.Close();
#endif
	ref_check->ref_check_min = min;
	return invalid_count;
}

s32 get_pen_check_error(u8 buf_num ,ST_PEN_CHECK* pen_check, s32 denomination, s32 way)
{
	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	ST_BS* pbs = work[buf_num].pbs;
	s32 invalid_count = 0;
	s32 pen = 0;
	s32 check_count = 0;
	s32 x = 0;
	s32 y = 0;
	s32 plane;	//パラ：プレーン番号
	s32 x_start;
	s32 x_end;
	s32 y_point;
	s32 x_boudary = 0;
	s32 y_boudary = 0;
	u8 flag = 0;

	plane = DOWN_T_G;
	//SE x<->y
	switch(way)
	{
	case W0:
	case W2:
		x_start = pen_check->note_size_y * 0.45f;
		x_end = pen_check->note_size_y * 0.45f - 150;//クリアウィンドウ対策
		break;
	case W1:
	case W3:
		x_start = pen_check->note_size_y * 0.45f - 150;//クリアウィンドウ対策
		x_end = pen_check->note_size_y * 0.45f;
		break;
	}
	y_point = pen_check->note_size_x * 0.40f;
	//4分割（3本検索）
	x_boudary = (pen_check->note_size_y * 0.25f);
	y_boudary = (pen_check->note_size_x * 0.25f);

	//水平検索
	for(y = -y_boudary; y <= y_boudary; y += y_boudary)
	{
		flag = 0;
		check_count = 0;
		for(x = -x_start; x <= x_end; x++)
		{
			//メッシュサイズ1にしてピクセルデータ取得
			pen = get_mesh_data(x, y, plane, 1 ,1 ,MASK_PAT_PIXEL ,buf_num ,1);
			if(pen > 250)
			{
				check_count++;
				flag = 1;
				if(check_count > pen_check->threshold)
				{
//					TRACE(_T("count 5 [%03d %03d]\n"), x, y);
					invalid_count++;
					break;
				}
			}
			else
			{
				//チャタリング対策
				if(flag)
				{
					flag = 0;
				}
				else
				{
					check_count = 0;
				}
			}
		}
	}
	//垂直検索
	for(x = -x_boudary; x <= x_boudary; x += x_boudary)
	{
		flag = 0;
		check_count = 0;
		for(y = -y_point; y <= y_point; y++)
		{
			//メッシュサイズ1にしてピクセルデータ取得
			pen = get_mesh_data(x, y, plane, 1 ,1 ,MASK_PAT_PIXEL ,buf_num ,1);
			if(pen > 250)
			{
				check_count++;
				flag = 1;
				if(check_count > pen_check->threshold)
				{
					invalid_count++;
					break;
				}
			}
			else
			{
				//チャタリング対策
				if(flag)
				{
					flag = 0;
				}
				else
				{
					check_count = 0;
				}
			}
		}
	}

	return invalid_count;
}
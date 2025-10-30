


#define EXT
#include "../common/global.h"

EXTERN int debug_logi_view;	//トレースするかしないか


void add_grid(  unsigned char *src, u16 h, u16 w, u16 row, u16 col,
                unsigned char *dst, u16 num_block_h, u16 num_block_w) {
	u16 dst_h = h * num_block_h;
	u16 dst_w = w * num_block_w;
    unsigned char *p1 = dst + row * h * dst_w + col * w;
    unsigned char *p2 = src;
	u16 i, j;

    for (i = 0; i < h; ++i) {
        for (j = 0; j < w; ++j) {
            p1[j] = p2[j];
        }
        p1 += dst_w;
        p2 += w;
    }
}

u8 get_pixel(ST_NOTE_PARAMETER* pnote_param, ST_BS* pbs, u8 buf_num, s16 x_coordinates, s16 y_coordinates, u8 plane[])
{
	u8 matrix;
	s16 x = 0;
	s16 y = 0;
	s16 p = 0;
	//	ST_BS* pbs = (ST_BS*)work[buf_num].pbs;
	//	ST_NOTE_PARAMETER* pnote_param =  &work[buf_num].note_param;
	float sin_x, cos_x, sin_y, cos_y;
	u32 period;
	u32 offset; // Huong add
	u8 pro_plane;
	float constant_x, constant_y;
	float input_x, input_y;
	float midst_res_x;
	float midst_res_y;
	float current_sin_x = 0;
	float current_cos_x = 0;
	float current_sin_y = 0;
	float current_cos_y = 0;
	float current_res_x = 0;
	float current_res_y = 0;
	float insart_multi_x = 0;
	float insart_multi_y = 0;
	float temp_x_interval = 0;
	float temp_y_interval = 0;

	float GETDT_ROUNDING = 0.5;

	u32 i = 0;

	u32 index;
#ifdef VS_DEBUG
	u32 debugx = 0;
	u32 debugy = 0;
#endif
	//buf_num = buf_num;
	//プレーン情報とは関係のないパラメタ
	//s32 start_x = start_x * pnote_param->insart_multi_x[(u8)pnote_param->pplane_tbl[plane[0]]];
	//s32 start_y = start_y * pnote_param->insart_multi_y[(u8)pnote_param->pplane_tbl[plane[0]]];
	s32 start_x = 0;
	s32 start_y = 0;


	if (pbs->LEorSE == LE)	//座標変換　+　画素参照　LEの場合
	//if (1)	//座標変換　+　画素参照　LEの場合
	{
		for (p = 0; p < 1; ++p)
		{
			//プレーン情報ごとのパラメタ
			pro_plane = (u8)pnote_param->pplane_tbl[plane[0]];
			constant_x = pnote_param->coordinate_param_x_non_ofs[pro_plane];
			constant_y = pnote_param->coordinate_param_y[pro_plane];
			period = pbs->PlaneInfo[pro_plane].Address_Period;
			offset = pbs->PlaneInfo[pro_plane].Address_Offset;// add
			sin_x = pnote_param->sin_x[pro_plane];
			cos_x = pnote_param->cos_x[pro_plane];
			sin_y = pnote_param->sin_y[pro_plane];
			cos_y = pnote_param->cos_y[pro_plane];
			insart_multi_x = pnote_param->insart_multi_x[pro_plane];
			insart_multi_y = pnote_param->insart_multi_y[pro_plane];


			input_x = x_coordinates * insart_multi_x;
			input_y = y_coordinates * insart_multi_y;

			temp_x_interval = insart_multi_x;
			temp_y_interval = insart_multi_y;


			for (y = 0; y < (1); ++y)
			{
				current_cos_y = input_y * cos_y;
				current_sin_x = input_y * sin_x;

				for (x = 0; x < (1); ++x)
				{
					current_cos_x = input_x * cos_x;
					current_sin_y = input_x * sin_y;

					midst_res_x = (current_cos_x + current_sin_x);
					midst_res_y = -(current_cos_y - current_sin_y);

					current_res_x = constant_x - midst_res_x;
					current_res_y = (midst_res_y + constant_y);


					index = (u32)(current_res_x + GETDT_ROUNDING) + (u32)(current_res_y + GETDT_ROUNDING) * period + offset;

					///* 取得したセンサデータを格納 */
					//matrix[p][y][x] = pbs->sens_dt[index];

#ifdef VS_DEBUG
					//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
					if (pbs->LEorSE == LE)  //SEならば
					{
						if (debug_logi_view == 1)
						{
							debugx = start_x + x + pnote_param->main_eff_range / 2;	//
							debugy = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (start_y - y);	//

							deb_para[0] = 1;		// function code
							deb_para[1] = debugx;
							deb_para[2] = debugy;//(((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (debugy -y);	//
							deb_para[3] = 1;		// plane
							callback(deb_para);		// debug
						}
					}
#endif


					matrix = pbs->sens_dt[index];
				}
			}
		}

	}
	else  //座標変換　+　画素参照
	{
		for (p = 0; p < 1; ++p)
		{
			//プレーン情報ごとのパラメタ
			pro_plane = (u8)pnote_param->pplane_tbl[plane[p]];
			constant_x = pnote_param->coordinate_param_x[pro_plane];
			constant_y = pnote_param->coordinate_param_y_non_ofs[pro_plane];
			period = pbs->PlaneInfo[pro_plane].Address_Period;
			offset = pbs->PlaneInfo[pro_plane].Address_Offset;// Huong add
			sin_x = pnote_param->sin_x[pro_plane];
			cos_x = pnote_param->cos_x[pro_plane];
			sin_y = pnote_param->sin_y[pro_plane];
			cos_y = pnote_param->cos_y[pro_plane];
			insart_multi_x = pnote_param->insart_multi_x[pro_plane];
			insart_multi_y = pnote_param->insart_multi_y[pro_plane];


			input_x = x_coordinates * insart_multi_x;
			input_y = y_coordinates * insart_multi_y;

			//temp_x_interval = x_interval * insart_multi_x;
			//temp_y_interval = y_interval * insart_multi_y;

			temp_x_interval = insart_multi_x;
			temp_y_interval = insart_multi_y;

			for (y = 0; y < (1); ++y)
			{
				current_cos_y = input_y * cos_y;
				current_sin_x = input_y * sin_x;

				for (x = 0; x < (1); ++x)
				{
					current_cos_x = input_x * cos_x;
					current_sin_y = input_x * sin_y;

					midst_res_x = (current_cos_x + current_sin_x);
					midst_res_y = -(current_cos_y - current_sin_y);

					current_res_x = constant_x - midst_res_x;
					current_res_y = (midst_res_y + constant_y);

					//index = (u32)current_res_y + (u32)current_res_x * period;
					index = (u32)(current_res_y + GETDT_ROUNDING) + (u32)(current_res_x + GETDT_ROUNDING) * period + offset;//new_L_Getdt関数と同じです。

#ifdef VS_DEBUG
					//x座標とｙ座標入れ替え　+　Y座標が反転するので修正
					if (pbs->LEorSE == SE)  //SEならば
					{
						if (debug_logi_view == 1)
						{
							debugy = start_y + y + pnote_param->main_eff_range / 2;	//
							debugx = (((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (start_x - x);	//

							deb_para[0] = 1;		// function code
							deb_para[1] = debugx;
							deb_para[2] = debugy;//(((pbs->Blocksize / pbs->PlaneInfo[0].sub_sampling_pitch) * pbs->block_count)) / 2 - (debugy -y);	//
							deb_para[3] = 1;		// plane
							callback(deb_para);		// debug
						}
					}
#endif
					//index = (u32)current_res_x + (u32)current_res_y * period;

					///* 取得したセンサデータを格納 */
					//matrix[p][y][x] = pbs->sens_dt[index];

					matrix = pbs->sens_dt[index];
				}
			}

		}
	}
	return matrix;
}


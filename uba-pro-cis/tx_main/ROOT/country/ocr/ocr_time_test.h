#ifndef	_OCR_TIME_TEST_H
#define	_OCR_TIME_TEST_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	//void get_cutout_img (ST_NOTE_PARAMETER *pnote_param,  ST_BS* pbs ,u8 buf_num ,u8 *** matrix ,s16 posx, s16 posy ,s16 xe, s16 ye ,u8 plane[] ,u8 plane_num);
	//void ocr_get_cutout_img (ST_NOTE_PARAMETER* pnote_param,  ST_BS* pbs ,u8 buf_num ,u8 *** matrix ,s16 start_x, s16 start_y ,s16 width, s16 height ,u8 plane[] ,u8 plane_num);//デバック用パラメタ設定関数
	void ocr_get_cutout_img(ST_NOTE_PARAMETER* pnote_param, ST_BS* pbs, u8 buf_num, u8* matrix, s16 start_x, s16 start_y, s16 width, s16 height, u8 plane[], u8 plane_num, u8 x_interval, u8 y_interval);//デバック用パラメタ設定関数
//void ocr_test (u8 buf_num , char* result);
	void ocr_test_1(u8 buf_num, char* result, float* result_prob, void* st_cod);
	//void ocr_test_2 (u8 buf_num , char* result ,void* st);
	void ocr_color(u8 buf_num, char* result, char* color_detect, void* st_cod);

	//void visualize_bgr1(u8 buf_num, char* result, void* st_cod, u8 num_sn);
	//void visualize_bgr2(u8 buf_num, char* result, void* st_cod, u8 num_sn);
	//void visualize_bgr2(u8 buf_num, char* result, void* st_cod);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OCR_TIME_TEST_H */

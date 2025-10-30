#include "../cod_config.h"

//1箇所目
#define CNY_SERIES_LENGTH 10

static /*const*/ char CNY_character_types[][CNY_SERIES_LENGTH] = {
	{2, 0, 0, 0, 1, 1, 1, 1, 1, 1, }, //CNY1.2   00 0
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY1.3   0c 1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY5.2   02 2
	{2, 0, 0, 1, 1, 1, 1, 1, 1, 1, }, //CNY5.3   01 3
	//{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY5.3   01 3
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY10.2  04 4
	{2, 0, 0, 1, 1, 1, 1, 1, 1, 1, }, //CNY10.3  03 5
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY10.5  0d 6
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY20.1  06 7
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY20.2  05 8
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY20.3  0e 9
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY50.1  08 10
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY50.3  07 11
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY50.5  10 12
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY100.1 09 13
	{2, 0, 0, 0, 1, 1, 1, 1, 1, 1, }, //CNY100.3 0a 14
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY100.4 0b 15
};

static /*const*/ COD_Parameters CNY_COD_Parameters[] =
{
	{-496, -87,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[0],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY1.2
	{-500, -115, 260, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[1],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY1.3
	//{-500, -75,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[2],  110, 45, 40, 20, 20, 0, 0, 0, 200},//CNY5.2
	{-500, -75,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[2],  110, 48, 40, 20, 20, 0, 0, 0, 200}, // CNY5.2//45=>48
	{-500, -70,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[3],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY5.3
	{-530, -90,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[4],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY10.2
	{-522, -95,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[5],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY10.3
	//{-530, -92,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[5],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY10.3
	{-530, -100, 270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[6],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY10.5
	{-543, -105,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[7],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY20.1
	{-546, -100,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[8],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY20.2
	//{-546, -105,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[8],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY20.2
	{-546, -105, 280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[9],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY20.3
	{-555, -96,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[10], 110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY50.1
	{-560, -95,  270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[11], 110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY50.3
	{-570, -106, 270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[12], 110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY50.5
	{-580, -120, 270, 80, 'U', 'R', CNY_SERIES_LENGTH, CNY_character_types[13], 110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY100.1
	{-583, -120, 270, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[14], 110, 47, 40, 20, 20, 0, 0, 0, 200}, // CNY100.3
	//{-591, -125, 280, 80, 'U', 'R', CNY_SERIES_LENGTH, CNY_character_types[15], 110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY100.4
	{-591, -125, 280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types[15], 110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY100.4

};

//2箇所目

//文字列タイプ
//0：すべて　1：数字　2：アルファベット
static /*const*/ char CNY_character_types2[][CNY_SERIES_LENGTH] = {
	{2, 0, 0, 0, 1, 1, 1, 1, 1, 1, }, //CNY1.2
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY1.3
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY5.2
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY5.3
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY10.2
	{2, 0, 0, 1, 1, 1, 1, 1, 1, 1, }, //CNY10.3
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY10.5
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY20.1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY20.2
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY20.3
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY50.1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY50.3
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY50.5
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY100.1
	{2, 0, 0, 0, 1, 1, 1, 1, 1, 1, }, //CNY100.3
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, }, //CNY100.4
};



//座標　サイズ　UP/DOWN　色　長さ　タイプ	平均ピクセル値　2値化の閾値　ｙ座標用閾値　x座標用　絞り込み閾値　反転ｘ　反転ｙ　回転　dpi
static /*const*/ COD_Parameters CNY_COD_Parameters2[] =
{
	{   0, -95,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[0],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY1.2
	{   0, -105, 280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[1],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY1.3

	{   0, -75,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[2],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY5.2
	{   0, -70,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[3],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY5.3

	{   0, -90,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[4],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY10.2
	{   0, -85,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[5],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY10.3
	{ 435, 120,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[6],  110, 45, 40, 20, 20, 1, 0, 1, 200}, // CNY10.5

	{   0, -95,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[7],  110, 45, 40, 20, 20, 0, 0, 1, 200}, // CNY20.1
	{   0, -95,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[8],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY20.2
	{ 470, 120,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[9],  110, 45, 40, 20, 20, 1, 0, 1, 200}, // CNY20.3

	//{   0, -96,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[10], 110, 45, 40, 20, 20, 0, 0, 1, 200}, // CNY50.1
	{ 470, 80,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[10], 110, 45, 40, 20, 20, 1, 0, 1, 200}, // CNY50.1 ket qua khong tot
	{   0, -85,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[11], 110, 45, 40, 20, 20, 0, 0, 0, 200}, // CNY50.3
	{ 480, 120,  280, 80, 'U', 'G', CNY_SERIES_LENGTH, CNY_character_types2[12], 110, 45, 40, 20, 20, 1, 0, 1, 200}, // CNY50.5

	{ 490,   60, 280, 80, 'U', 'R', CNY_SERIES_LENGTH, CNY_character_types2[13], 110, 45, 40, 20, 20, 1, 0, 1, 200}, // CNY100.1
	{   0, -120, 280, 80, 'U', 'R', CNY_SERIES_LENGTH, CNY_character_types2[14], 110, 47, 40, 20, 20, 0, 0, 0, 200}, // CNY100.3
	{ 505,   100, 280, 80, 'U', 'R', CNY_SERIES_LENGTH, CNY_character_types2[15], 110, 45, 40, 20, 20, 1, 0, 1, 200}, // CNY100.4
};

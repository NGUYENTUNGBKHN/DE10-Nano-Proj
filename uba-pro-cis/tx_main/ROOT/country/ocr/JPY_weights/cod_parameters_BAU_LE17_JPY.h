
#define JPY_SERIES_LENGTH_1_type1 8
#define JPY_SERIES_LENGTH_1_type2 9
//文字列タイプ
//0：すべて　1：数字　2：アルファベット
static const char JPY_character_types_1[][JPY_SERIES_LENGTH_1_type1] = {
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY1000.4A	0
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY1000.4B	1
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY1000.4C	2
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY5000.2B	3
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY5000.3A	4
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY5000.3B	5
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY10000.2A	6
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY10000.2B	7
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY10000.3A	8
	{2, 1, 1, 1, 1, 1, 1, 2, }, //JPY10000.3B	9
};

static const char JPY_character_types_2[][JPY_SERIES_LENGTH_1_type2] = {
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY1000.4A		0
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY1000.4B		1
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY1000.4C		2
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY5000.2B		3
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY5000.3A		4
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY5000.3B		5
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY10000.2A	6
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY10000.2B	7
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY10000.3A	8
	{2, 2, 1, 1, 1, 1, 1, 1, 2, }, //JPY10000.3B	9
};
//1箇所目
static const COD_Parameters JPY_COD_Parameters[] =
{
	//座標　    サイズ　 UP/DOWN 色 長さ　                     タイプ	         平均ピクセル値　2値化の閾値　ｙ座標用閾値　x座標用　絞り込み閾値　反転ｘ　反転ｙ　回転　dpi
	{-397, 202, 230,  60, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[0],  110, 38, 40, 20, 20, 0, 0, 0, 200, 0}, // JPY1000.4Aoo   0 OK0
	{-397, 202, 230,  60, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[1],  110, 39, 40, 20, 20, 0, 0, 0, 200, 0}, // JPY1000.4Boo   1 OK0
	{-397, 202, 230,  62, 'U', 'R', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[2],  110, 36, 40, 20, 20, 0, 0, 0, 200, 0}, // JPY1000.4Coo   2 OK0
	{-540, 210, 260,  60, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[3],  110, 45, 40, 20, 20, 0, 0, 0, 200, 1}, // JPY5000.2Boo   3 
	{-442, 183, 230,  57, 'U', 'R', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[4],  110, 42, 40, 20, 20, 0, 0, 0, 200, 1}, // JPY5000.3Aoo   4
	{-442, 183, 230,  60, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[5],  110, 42, 40, 20, 20, 0, 0, 0, 200, 1}, // JPY5000.3Boo   5
	{-437, 188, 230,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[6],  110, 43, 40, 20, 20, 0, 0, 0, 200, 2}, // JPY10000.2Aoo  6
	{-437, 188, 233,  60, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[7],  110, 42, 40, 20, 20, 0, 0, 0, 200, 2}, // JPY10000.2Boo  7 
	{-517, 178, 240,  60, 'U', 'R', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[8],  110, 45, 40, 20, 20, 0, 0, 0, 200, 2}, // JPY10000.3Aoo  8	
	{-512, 180, 235,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[9],  110, 45, 40, 20, 20, 0, 0, 0, 200, 2}, // JPY10000.3Boo  9	
};


//2箇所目

static const COD_Parameters JPY_COD_Parameters_2[] =
{
	//座標　    サイズ　 UP/DOWN 色 長さ　                     タイプ	         平均ピクセル値　2値化の閾値　ｙ座標用閾値　x座標用　絞り込み閾値　反転ｘ　反転ｙ　回転　dpi
	{ 137, -212, 230,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[0],  110, 38, 40, 20, 20, 0, 0, 0, 200}, // JPY1000.4Aoo	0 OK0
	{ 137, -210, 230,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[1],  110, 39, 40, 20, 20, 0, 0, 0, 200}, // JPY1000.4Boo	1 OK0
	{ 137, -210, 230,  60, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[2],  110, 36, 40, 20, 20, 0, 0, 0, 200}, // JPY1000.4Coo	2 
	{ 160, -182, 250,  50, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[3],  110, 29, 40, 20, 20, 0, 0, 0, 200}, // JPY5000.2Boo	3
	{ 170, -200, 235,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[4],  110, 42, 40, 20, 20, 0, 0, 0, 200}, // JPY5000.3Aoo	4 
	{ 172, -200, 230,  60, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[5],  110, 45, 40, 20, 20, 0, 0, 0, 200}, // JPY5000.3Boo	5
	{ 188, -155, 230,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[6],  110, 37, 40, 20, 20, 0, 0, 0, 200}, // JPY10000.2Aoo	6
	{ 188, -155, 230,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[7],  110, 38, 40, 20, 20, 0, 0, 0, 200}, // JPY10000.2Boo	7 
	{ 175, -203, 240,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[8],  110, 44, 40, 20, 20, 0, 0, 0, 200}, // JPY10000.3Aoo	8	
	{ 175, -205, 240,  55, 'U', 'G', JPY_SERIES_LENGTH_1_type2, JPY_character_types_2[9],  110, 44, 40, 20, 20, 0, 0, 0, 200}, // JPY10000.3Boo	9
};

static const COD_Parameters JPY_COD_Parameters_color[] =
{
	//座標　    サイズ　 UP/DOWN 色 使用する文字数 記番号の文字の長さ　                     タイプ	         平均ピクセル値　2値化の閾値　ｙ座標用閾値　x座標用　絞り込み閾値　反転ｘ　反転ｙ　回転　dpi
	{-397, 202, 230,  60, 'U', 'G', 5, JPY_character_types_2[0],  110, 45, 40, 20, 20, 0, 0, 0, 200, 0}, // JPY1000.4Aoo   0 
	{-397, 202, 230,  60, 'U', 'G', 5, JPY_character_types_2[1],  110, 45, 40, 20, 20, 0, 0, 0, 200, 0}, // JPY1000.4Boo   1 
	{-397, 202, 230,  60, 'U', 'G', 5, JPY_character_types_2[2],  110, 45, 40, 20, 20, 0, 0, 0, 200, 0}, // JPY1000.4Coo   2 
	{-540, 210, 260,  60, 'U', 'G', 5, JPY_character_types_2[3],  110, 45, 40, 20, 20, 0, 0, 0, 200, 1}, // JPY5000.2Boo   3 
	{-442, 183, 230,  60, 'U', 'G', 5, JPY_character_types_2[4],  110, 45, 40, 20, 20, 0, 0, 0, 200, 1}, // JPY5000.3Aoo   4
	{-442, 183, 230,  60, 'U', 'G', 5, JPY_character_types_2[5],  110, 45, 40, 20, 20, 0, 0, 0, 200, 1}, // JPY5000.3Boo   5
	{-437, 188, 233,  60, 'U', 'G', 5, JPY_character_types_2[6],  110, 45, 40, 20, 20, 0, 0, 0, 200, 2}, // JPY10000.2Aoo  6
	{-437, 188, 233,  60, 'U', 'G', 5, JPY_character_types_2[7],  110, 45, 40, 20, 20, 0, 0, 0, 200, 2}, // JPY10000.2Boo  7 
	{-517, 180, 240,  60, 'U', 'G', 5, JPY_character_types_2[8],  110, 45, 40, 20, 20, 0, 0, 0, 200, 2}, // JPY10000.3Aoo  8	
	{-517, 180, 240,  60, 'U', 'G', 5, JPY_character_types_2[9],  110, 45, 40, 20, 20, 0, 0, 0, 200, 2}, // JPY10000.3Boo  9	
};

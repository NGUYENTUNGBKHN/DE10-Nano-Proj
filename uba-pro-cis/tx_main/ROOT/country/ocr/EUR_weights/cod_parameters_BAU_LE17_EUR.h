#define EUR_SERIES_LENGTH 12

static char EUR_character_types[][EUR_SERIES_LENGTH] = {
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR5.1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR5.2
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR10.1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR10.2
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR20.1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR20.2
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR50.1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR50.2
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR100.1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR100.2
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR200.1
	{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR200.2
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR500.1
};

static COD_Parameters EUR_COD_Parameters[] = 
{
	{-377, 204, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[0], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR5.1
	{-404, 201, 280, 80, 'D', 'R', EUR_SERIES_LENGTH, EUR_character_types[1], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR5.2
	{-376, 223, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[2], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR10.1
	{-421, 226, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[3], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR10.2
	{-393, 243, 292, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[4], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR20.1
	{-453, 245, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[5], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR20.2
	{-421, 264, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[6], 110, 30, 40, 20, 20, 1, 0, 0, 200}, // EUR50.1
	{-484, 267, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[7], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR50.2
	{-420, 283, 280, 80, 'D', 'R', EUR_SERIES_LENGTH, EUR_character_types[8], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR100.1
	{-505, 265, 280, 80, 'D', 'R', EUR_SERIES_LENGTH, EUR_character_types[9], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR100.2
	{-418, 283, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[10], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR200.1
	{-531, 265, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[11], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR200.2
	{-461, 286, 280, 80, 'D', 'G', EUR_SERIES_LENGTH, EUR_character_types[12], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR500.1
};


//2箇所目
#define EUR_SERIES_LENGTH_2_type1 12
#define EUR_SERIES_LENGTH_2_type2 6


//文字列タイプ
//0：すべて　1：数字　2：アルファベット
static char EUR_character_types_1[][EUR_SERIES_LENGTH_2_type1] = {
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR5.1
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR10.1
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR20.1
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR50.1
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR100.1
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR200.1
	{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, }, //EUR500.1
};

static char EUR_character_types_2[][EUR_SERIES_LENGTH_2_type2] = {
	{1, 1, 1, 1, 1, 1, }, //EUR5.2
	{1, 1, 1, 1, 1, 1, }, //EUR10.2
	{1, 1, 1, 1, 1, 1, }, //EUR20.2
	{1, 1, 1, 1, 1, 1, }, //EUR50.2
	{1, 1, 1, 1, 1, 1, }, //EUR100.2
	{1, 1, 1, 1, 1, 1, }, //EUR200.2
};

//座標　サイズ　UP/DOWN　色　長さ　タイプ	平均ピクセル値　2値化の閾値　ｙ座標用閾値　x座標用　絞り込み閾値　反転ｘ　反転ｙ　回転　dpi
static COD_Parameters EUR_COD_Parameters_2[] =
{
	//座標　    サイズ　 UP/DOWN 色 長さ　                     タイプ	         平均ピクセル値　2値化の閾値　ｙ座標用閾値　x座標用　絞り込み閾値　反転ｘ　反転ｙ　回転　dpi
	{113, -65, 280,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type1, EUR_character_types_1[0], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR5.1o   0
	{ 41, -55, 170,  80, 'D', 'R', EUR_SERIES_LENGTH_2_type2, EUR_character_types_2[0], 110, 40, 40, 20, 20, 1, 1, 1, 200}, // EUR5.2oo  1
	{118, -78, 280,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type1, EUR_character_types_1[1], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR10.1o  2
	{ 55, -75, 170,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type2, EUR_character_types_2[1], 110, 40, 40, 20, 20, 1, 1, 1, 200}, // EUR10.2oo 3
	{121, -85, 292,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type1, EUR_character_types_1[2], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR20.1o  4
	{ 23, -81, 170,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type2, EUR_character_types_2[2], 110, 40, 40, 20, 20, 1, 1, 1, 200}, // EUR20.2o  5
	{ 97,-121, 280,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type1, EUR_character_types_1[3], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR50.1o  6
	{ 46, -89, 170,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type2, EUR_character_types_2[3], 110, 40, 40, 20, 20, 1, 1, 1, 200}, // EUR50.2o  7
	{ 77,-138, 280,  80, 'D', 'R', EUR_SERIES_LENGTH_2_type1, EUR_character_types_1[4], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR100.1o 8
	{ 43,-101, 170,  80, 'D', 'R', EUR_SERIES_LENGTH_2_type2, EUR_character_types_2[4], 110, 40, 40, 20, 20, 1, 1, 1, 200}, // EUR100.2o 9
	{ 93,-121, 280,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type1, EUR_character_types_1[5], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR200.1o 10
	{ 48,-100, 170,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type2, EUR_character_types_2[5], 110, 40, 40, 20, 20, 1, 1, 1, 200}, // EUR200.2o 11
	{148,-128, 280,  80, 'D', 'G', EUR_SERIES_LENGTH_2_type1, EUR_character_types_1[6], 110, 40, 40, 20, 20, 1, 0, 0, 200}, // EUR500.1o 12
};

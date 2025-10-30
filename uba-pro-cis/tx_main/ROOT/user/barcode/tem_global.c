/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_global.h
* Contents: common source for all files
*
*
*******************************************************************************/
#include "set_enable.h"

#include "kernel.h"
#if defined(EXT)
	#define EXTERN extern
#else
	#undef EXTERN
	#define EXTERN
#endif

#ifndef TEM_GLOBAL_H
#define TEM_GLOBAL_H


/*******************************************************************************
* Common Defines
*
*******************************************************************************/
//#define FALSE		0
//#define TRUE		1

#define DIRECTION_COUNT			4
#define DENOMINATION_COUNT		100
#define PATTERN_COUNT			(DIRECTION_COUNT * DENOMINATION_COUNT)

#define PIXEL_VALUE_COUNT		256 //8bit
#define PIXEL_MAX_VALUE			255

#define DEGREES_PER_RADIAN		(float)57.295780

//#define PITCH_MAIN_200DPI			0.127
//#define PITCH_SUB_204DPI			0.1245
//#define PITCH_SUB_200DPI			0.127
#define PITCH_200DPI				0.127
#define PITCH_100DPI				0.254
#define PITCH_50DPI					0.508

#define RATIO_200DPI	1
#define RATIO_100DPI	2
#define RATIO_50DPI		4


enum bill_direction
{
	A_DIRECTION = 0,
	B_DIRECTION,
	C_DIRECTION,
	D_DIRECTION,
	UNKNOWN_DIRECTION,
};

#define CIS_SIDE_COUNT 				3
#define CIS_REFLECTIVE_SIDE_COUNT	2

enum cis_sensor_types
{
	SENSOR_1COLOR = 0,
	SENSOR_2COLOR_DIFF,
	SENSOR_3COLOR_RATIO,
	SENSOR_2COLOR_CROSS,
	SENSOR_BARCODE,
	SENSOR_DISABLED,
};


struct sensor_information
{
	int side;
	int color;
	int x_pixels;
	int y_pixels;
	int deskew_center_x;
	int deskew_center_y;
	int deskew_align_x;
	int deskew_align_y;
	float pitch;
};
typedef struct sensor_information SENSOR_INFORMATION;
typedef struct sensor_information* P_SENSOR_INFORMATION;


struct sensor_dimension
{
	int x_pixels;
	int y_pixels;
	float pitch;
};
typedef struct sensor_dimension SENSOR_DIMENSION;


struct sensor_deskew
{
	int deskew_center_x;
	int deskew_center_y;
	int deskew_align_x;
	int deskew_align_y;
};
typedef struct sensor_deskew SENSOR_DESKEW;


struct sensor_enable
{
	int identification;
	int mesh;
	int custom;
	int cf_1color;
	int cf_2color;
	int dye;
	int fitness;
};
typedef struct sensor_enable SENSOR_ENABLE;


#define CIS_SIDE_COUNT 				3
#define CIS_REFLECTIVE_SIDE_COUNT	2

enum cis_side
{
	CIS_SIDE_UP = 0,
	CIS_SIDE_DOWN,
	CIS_SIDE_TRANSPARENT,
};

#define CIS_COLOR_COUNT		5

enum cis_color
{
	CIS_COLOR_BLUE = 0,
	CIS_COLOR_GREEN,
	CIS_COLOR_RED,
	CIS_COLOR_IR,
	CIS_COLOR_FIR,
	CIS_COLOR_BARCODE,
};


/*******************************************************************************
* Model Specific Defines
*
*******************************************************************************/


	#include <user/barcode/tem_global_iVIZION2.h>



/*******************************************************************************
* Edge Detection Settings
*
*******************************************************************************/
#define REFLECTIVE_BILL_EDGE_LIMIT		30

#define EDGE_PIXEL_SKIP					15
#define EDGE_SEARCH_START_X				45
#define EDGE_SEARCH_START_Y				0

//skip the calibration square
#define HEIGHT_SEARCH_SKIP_BAR_START	154
#define HEIGHT_SEARCH_SKIP_BAR_END		184

#define MIN_POINT_SLOPE_X		(float)(-0.8)
#define MAX_POINT_SLOPE_X		(float)(0.8)
#define MIN_POINT_SLOPE_Y		(float)(-0.8)
#define MAX_POINT_SLOPE_Y		(float)(0.8)

#define LEFT_EDGE_LIMIT				70

#define CORNER_REMOVE_PERCENT_X		15
#define CORNER_REMOVE_PERCENT_Y		30

#define OUTLIER_REMOVE_PERCENT		50

#define MIN_BILL_TICKET_WIDTH		55.0		//absolute minimum width of bill or ticket
#define MIN_BILL_TICKET_LENGTH		105.0		//absolute minimum length of bill or ticket


/*******************************************************************************
* Edge Detection Limits
*
*******************************************************************************/
#define MAX_BILL_SKEW				(float)30.0
#define MIN_BILL_OFFSET_MM			(float)-20.0
#define MAX_BILL_OFFSET_MM			(float)20.0

#define EDGE_MAX_SLOPE_COUNT		200
#define EDGE_SLOPE_SKIP_PERCENT		5


/*******************************************************************************
* Transform Settings
*
*******************************************************************************/
#define BILL_EDGE_BUFFER			2


/*******************************************************************************
* Neural Network Settings
*
*******************************************************************************/
#define NETWORK_AREA_COUNT_X		16
#define NETWORK_AREA_COUNT_Y		8

#define NETWORK_AREAS_PER_SENSOR	(NETWORK_AREA_COUNT_X * NETWORK_AREA_COUNT_X)
#define TOTAL_NETWORK_AREA_COUNT	(CIS_BILL_SENSOR_COUNT * NETWORK_AREAS_PER_SENSOR)

#define NETWORK_OUTPUT_NOT_USED		-1
#define NETWORK_INPUT_DIVIDER		(float)256.0


/*******************************************************************************
* Mesh Area Settings
*
*******************************************************************************/
#define MESH_CIRCLE_DIAMETER		15
#define MESH_SQUARE_DIAMETER		15

#define MESH_X_PIXEL_START			80
#define MESH_X_PIXEL_END			(156*8-80)
#define MESH_AREA_COUNT_X			((MESH_X_PIXEL_END - MESH_X_PIXEL_START + 1) / MESH_CIRCLE_DIAMETER) //44
#define MESH_AREA_MIDDLE_X			(MESH_AREA_COUNT_X / 2)

#define MESH_Y_PIXEL_START			70
#define MESH_Y_PIXEL_END			649
#define MESH_AREA_COUNT_Y			((MESH_Y_PIXEL_END - MESH_Y_PIXEL_START + 1) / MESH_CIRCLE_DIAMETER) //38
#define MESH_AREA_MIDDLE_Y			(MESH_AREA_COUNT_Y / 2)

#define TOTAL_MESH_AREA_COUNT		(CIS_BILL_SENSOR_COUNT * MESH_AREA_COUNT_X * MESH_AREA_COUNT_Y)

#define MESH_SERIES_END		255

#define TABLE_END_FLAG		-1


/*******************************************************************************
* Validation Test Settings
*
*******************************************************************************/
enum bill_security
{
	BILL_SECURITY_NORMAL = 0,
	BILL_SECURITY_LOW,
	BILL_SECURITY_HIGH,
};

enum validation_modes
{
	VALIDATION_MODE_PARTIAL = 0,
	VALIDATION_MODE_FULL,
};

enum test_result
{
	RESULT_PASS = 0,
	RESULT_FAIL,
};

#define VALIDATION_TEST_COUNT	15

enum validation_test
{
	CALIBRATION_CHECK = 0,
	EDGE_CHECK,
	NETWORK_MARGIN_CHECK,
	LENGTH_CHECK,
	WIDTH_CHECK,
	MESH_2COLOR_CHECK,
	MESH_3COLOR_CHECK,
	MESH_CROSS_CHECK,
	DOUBLE_CHECK,
	UV_CHECK,
	NETWORK_CF_1COLOR_LAYER1_CHECK,
	NETWORK_CF_1COLOR_LAYER2_CHECK,
	NETWORK_CF_2COLOR_LAYER1_CHECK,
	NETWORK_CF_2COLOR_LAYER2_CHECK,
	CUSTOM_CHECK,
};

enum bill_validation_reject_codes
{
	BILL_RESULT_NO_ERROR = 0,
	BILL_RESULT_CALIBRATION_ERROR = 6,
	BILL_RESULT_EDGE_ERROR = 6,
	BILL_RESULT_NETWORK_MARGIN_ERROR = 6,
	BILL_RESULT_LENGTH_ERROR = 13,
	BILL_RESULT_WIDTH_ERROR = 13,
	BILL_RESULT_MESH_2COLOR_ERROR = 7,
	BILL_RESULT_MESH_3COLOR_ERROR = 7,
	BILL_RESULT_MESH_CROSS_ERROR = 14,
	BILL_RESULT_DOUBLE_ERROR = 8,
	BILL_RESULT_UV_ERROR = 2,
	BILL_RESULT_NET_CF_1COLOR_LAYER1_ERROR = 15,
	BILL_RESULT_NET_CF_1COLOR_LAYER2_ERROR = 15,
	BILL_RESULT_NET_CF_2COLOR_LAYER1_ERROR = 15,
	BILL_RESULT_NET_CF_2COLOR_LAYER2_ERROR = 15,
	BILL_RESULT_CUSTOM_ERROR = 15,
};

enum bill_validation_reject_sub_codes
{
	BILL_RESULT_SUB_NO_ERROR = 0,
	BILL_RESULT_SUB_CALIBRATION_ERROR = 1,
	BILL_RESULT_SUB_EDGE_ERROR = 2,
	BILL_RESULT_SUB_NETWORK_MARGIN_ERROR = 3,
	BILL_RESULT_SUB_LENGTH_ERROR = 1,
	BILL_RESULT_SUB_WIDTH_ERROR = 2,
	BILL_RESULT_SUB_MESH_2COLOR_ERROR = 1,
	BILL_RESULT_SUB_MESH_3COLOR_ERROR = 2,
	BILL_RESULT_SUB_MESH_CROSS_ERROR = 1,
	BILL_RESULT_SUB_DOUBLE_ERROR = 1,
	BILL_RESULT_SUB_UV_ERROR = 1,
	BILL_RESULT_SUB_NET_CF_1COLOR_LAYER1_ERROR = 1,
	BILL_RESULT_SUB_NET_CF_1COLOR_LAYER2_ERROR = 2,
	BILL_RESULT_SUB_NET_CF_2COLOR_LAYER1_ERROR = 3,
	BILL_RESULT_SUB_NET_CF_2COLOR_LAYER2_ERROR = 4,
	BILL_RESULT_SUB_CUSTOM_ERROR = 5,
};

enum error_class
{
	ERROR_CLASS_MAIN = 0,
	ERROR_CLASS_LS_VALIDATION = 1,
	ERROR_CLASS_CIS_VALIDATION = 2,
	ERROR_CLASS_1D_BARCODE = 1,
	ERROR_CLASS_2D_BARCODE = 2,
};


/*******************************************************************************
* Fitness Test Settings
*
*******************************************************************************/
#define FITNESS_HOLE_EDGE_SKIP_MM			5.0
#define	FITNESS_HOLE_TRANSPARENT_LEVEL		254

#define DYE_EDGE_SKIP_MM			1.5

#define DYE_AREA_COUNT_X			20
#define DYE_AREA_COUNT_Y			10

#define DYE_AREAS_PER_SENSOR		(DYE_AREA_COUNT_X * 2 + DYE_AREA_COUNT_Y * 2)
#define TOTAL_DYE_AREA_COUNT 		(CIS_BILL_SENSOR_COUNT * DYE_AREAS_PER_SENSOR)


#define FITNESS_TEST_COUNT		5

enum fitness_test
{
	DYE_INK_CHECK = 0,
	FITNESS_LENGTH_CHECK,
	FITNESS_WIDTH_CHECK,
	FITNESS_HOLE_CHECK,
	FITNESS_MESH_CHECK,
};

enum bill_fitness_reject_codes
{
	FITNESS_RESULT_NO_ERROR = 0,
	FITNESS_RESULT_DYE_ERROR = 4,
	FITNESS_RESULT_LENGTH_ERROR = 4,
	FITNESS_RESULT_WIDTH_ERROR = 4,
	FITNESS_RESULT_HOLE_ERROR = 4,
	FITNESS_RESULT_FITNESS_MESH_ERROR = 4,

};

enum bill_fitness_sub_reject_codes
{
	FITNESS_RESULT_SUB_NO_ERROR = 0,
	FITNESS_RESULT_SUB_DYE_ERROR = 1,
	FITNESS_RESULT_SUB_LENGTH_ERROR = 2,
	FITNESS_RESULT_SUB_WIDTH_ERROR = 3,
	FITNESS_RESULT_SUB_HOLE_ERROR = 4,
	FITNESS_RESULT_SUB_FITNESS_MESH_ERROR = 5,
};


/*******************************************************************************
* Serial OCR Settings
*
*******************************************************************************/
#define SERIAL_MAX_AREA_HEIGHT		60
#define SERIAL_MAX_AREA_WIDTH		190
#define SERIAL_MAX_CHARACTERS		12

#define SERIAL_GLYPH_WIDTH			16
#define SERIAL_GLYPH_HEIGHT			24
#define SERIAL_NETWORK_INPUT_COUNT	(SERIAL_GLYPH_WIDTH * SERIAL_GLYPH_HEIGHT)

#define SERIAL_IMAGE_OBJECT_COUNT	20

#define SERIAL_NUMBER_VALUE_COUNT	11
#define SERIAL_LETTER_VALUE_COUNT	29

#define SENSOR_CONTRAST_FACTOR			128
#define SERIAL_CONTRAST_SKIP_PERCENT	2
#define SERIAL_CONTRAST_USE_PERCENT		10

#define SERIAL_FRAME_EDGE_LEVEL			5
#define SERIAL_CHARACTER_EDGE_LEVEL		1

#define SERIAL_SPACING_MIN_RATIO_HORIZONTAL		0.45
#define SERIAL_SPACING_MIN_RATIO_VERTICAL		1.2

#define SERIAL_GLYPH_MAX_SIZE_RATIO		1.2
#define SERIAL_GLYPH_MIN_SIZE_RATIO		0.0

#define SERIAL_GLYPH_MIN_PIXEL_WIDTH	1
#define SERIAL_GLYPH_MAX_PIXEL_WIDTH	25

#define SERIAL_GLYPH_MIN_PIXEL_HEIGHT	5
#define SERIAL_GLYPH_MAX_PIXEL_HEIGHT	40

#define SERIAL_GLYPH_MIN_PIXEL_AREA		5

#define SERIAL_NARROW_RESIZE_RATIO		1.1

enum serial_orientation_types
{
	SERIAL_HORIZONTAL = 0,
	SERIAL_VERTICAL_DOWN,
	SERIAL_VERTICAL_LEFT,
	SERIAL_VERTICAL_RIGHT,
};

#define SERIAL_UNKNOWN_CHARACTER 		'?'
#define SERIAL_EXCLUDE_CHARACTER		'-'

#define SERIAL_CHARACTER_TYPE_COUNT		2

enum serial_character_types
{
	NUMBER = 0,
	LETTER,
	BOTH,
	SPECIAL,
};

#define SERIAL_MAX_FONT_COUNT		10

enum serial_font_indicies
{
	FONT_INDEX_01 = 0,
	FONT_INDEX_02,
	FONT_INDEX_03,
	FONT_INDEX_04,
	FONT_INDEX_05,
	FONT_INDEX_06,
	FONT_INDEX_07,
	FONT_INDEX_08,
	FONT_INDEX_09,
	FONT_INDEX_10,
};

enum serial_reject_codes
{
	SERIAL_RESULT_PASS = 0,
	SERIAL_RESULT_DISABLED,
	SERIAL_RESULT_OBJECT_ERROR,
	SERIAL_RESULT_CHARACTER_COUNT_ERROR,
	SERIAL_RESULT_DECODE_ERROR,
	SERIAL_RESULT_CHARACTER_MISMATCH,
};

#define SERIAL_NUMBER_COUNT		2

enum serial_indicies
{
	SERIAL_NUMBER_INDEX_1 = 0,
	SERIAL_NUMBER_INDEX_2,
};

enum serial_post_process_types
{
	SERIAL_POST_PROCESS_DEFAULT = 0,
	SERIAL_POST_PROCESS_IND,
	SERIAL_POST_PROCESS_CHN,
	SERIAL_POST_PROCESS_EUR,
};

enum serial_filter_math
{
	FILTER_1COLOR_INV = 0,
	FILTER_2COLOR_SUM_INV,
	FILTER_2COLOR_DIFF,
};


/*******************************************************************************
* Category Settings
*
*******************************************************************************/
#define CATEGORY_COUNT	5

enum category_types
{
	CATEGORY_UNKNOWN = 0,
	CATEGORY_1,
	CATEGORY_2,
	CATEGORY_3,
	CATEGORY_4A,
	CATEGORY_4B,
};


/*******************************************************************************
* Barcode Settings
*
*******************************************************************************/
#define BARCODE_MAXIMUM_CHARACTERS			450
#define ASCII_NUMBER_OFFSET					0x30

#define BARCODE_MAX_SEARCH_POINTS			1000
//#define BARCODE_MAX_SEARCH_POINTS			250

//ir check values
#define BARCODE_IR_LIMIT					165

//double ticket check values
//#define DOUBLE_TICKET_OFFSET_COUNT			4
//#define DOUBLE_TICKET_OFFSET_COUNT			8
#define DOUBLE_TICKET_OFFSET_COUNT			16
#define DOUBLE_TICKET_EDGE_SKIP				4
//#define THERMAL_TICKET_DOUBLE_LIMIT			80	// memo: ivizion limit is 80
//#define PROMO_TICKET_DOUBLE_LIMIT			40
#define THERMAL_TICKET_DOUBLE_LIMIT			80	// memo: ivizion limit is 80
#define PROMO_TICKET_DOUBLE_LIMIT			80

#define BARCODE_SIMULATION_RESULT_MISREAD	15	//simulator error code only

enum barcode_search_type
{
	BARCODE_SEARCH_UP_ONLY = 0,
	BARCODE_SEARCH_DOWN_ONLY,
	BARCODE_SEARCH_UP_AND_DOWN,
	BARCODE_TEST,
};

#define TICKET_TYPE_COUNT		7

enum ticket_type
{
	TICKET_TYPE_TITO = 0,
	TICKET_TYPE_C128,
	TICKET_TYPE_MATRIX,
	TICKET_TYPE_TITO2D,
	TICKET_TYPE_CITO,
	TICKET_TYPE_QR,
	TICKET_TYPE_PDF,
};

#define BARCODE_TICKET_MIN_WIDTH			58.0
#define BARCODE_TICKET_MAX_WIDTH			68.0
#define BARCODE_TICKET_MIN_LENGTH			105.0
#define BARCODE_TICKET_MAX_LENGTH			165.0

#define INDEXMARK_TOTAL_SEARCH_AREA			110

#define TICKET_INDEXMARK_LIMIT				65
#define TICKET_INDEXMARK_SEARCH_AREA_MM		10.0

#define INDEXMARK_WHITE_AREA1_WIDTH_MM		8.0
#define INDEXMARK_BLACK_AREA_WIDTH_MM		5.0
#define INDEXMARK_WHITE_AREA2_WIDTH_MM		8.0


/*******************************************************************************
* 1 Dimensional Barcode Settings
*
*******************************************************************************/
#define BARCODE_1D_SIDE_AREA_MM				(float)1.5
#define BARCODE_1D_WHITE_EDGE_AREA_MM		(float)3.0
#define BARCODE_1D_MINIMUM_WIDTH_MM			(float)10.0
#define BARCODE_1D_EDGE_LEVEL				110

#define BARCODE_1D_OFFSET_COUNT				9	// VERY SLOW
//#define BARCODE_1D_OFFSET_COUNT				1	// VERY FAST


#define BARCODE_1D_MINIMUM_BAR_AREA			2
#define BARCODE_1D_MAXIMUM_BAR_AREA			3000

#define BARCODE_1D_MAXIMUM_BAR_COUNT 		167	//same as 32 digit itf barcode

enum barcode_1d_orientation_type
{
	BARCODE_1D_ORIENATION_HORIZONTAL = 0,
	BARCODE_1D_ORIENATION_VERTICAL,
};

enum barcode_1d_direction_type
{
	BARCODE_1D_FORWARD_DIRECTION = 0,
	BARCODE_1D_REVERSE_DIRECTION,
};


/*******************************************************************************
* Interleaved 2 of 5 Settings
*
*******************************************************************************/
#define ITF_BARCODE_START_CODE_LENGTH		4
#define ITF_BARCODE_STOP_CODE_LENGTH		3

#define ITF_BARCODE_AREA_RATIO_MIN			(float)0.8
#define ITF_BARCODE_AREA_RATIO_MAX			(float)6.0

#define ITF_BARCODE_MINIMUM_DIGITS			2
#define ITF_BARCODE_MAXIMUM_DIGITS			32
#define ITF_BARCODE_BARS_PER_DIGIT			5
#define ITF_BARCODE_DIGIT_COUNT				10

#define ITF_BARCODE_MINIMUM_BAR_COUNT		((ITF_BARCODE_MINIMUM_DIGITS * ITF_BARCODE_BARS_PER_DIGIT) + ITF_BARCODE_START_CODE_LENGTH + ITF_BARCODE_STOP_CODE_LENGTH) //2 digit barcode
#define ITF_BARCODE_MAXIMUM_BAR_COUNT		((ITF_BARCODE_MAXIMUM_DIGITS * ITF_BARCODE_BARS_PER_DIGIT) + ITF_BARCODE_START_CODE_LENGTH + ITF_BARCODE_STOP_CODE_LENGTH) //32 digit barcode

#define ITF_BAR_RATIO_NORMALIZE				(float)0.45 //normalize narrow bars to a 0.5 average

#define ITF_BARCODE_NET_MARGIN				(float)0.98 //smaller values increase acceptance but also increase misreads

#define	ITF_BARCODE_UNKNOWN_DIGIT			'?'

enum bar_width_type
{
	NARROW_BAR = 0,
	WIDE_BAR,
	UNKNOWN_WIDTH,
};

enum bar_color_type
{
	BLACK_BAR = 0,
	WHITE_BAR,
};

enum itf_barcode_reject_codes
{
	ITF_BARCODE_RESULT_EDGE_ERROR = 1,
	ITF_BARCODE_RESULT_AREA_ERROR = 1,
	ITF_BARCODE_RESULT_4WAY_ERROR = 1,
	ITF_BARCODE_RESULT_WIDTH_ERROR = 2,
	ITF_BARCODE_RESULT_COUNT_ERROR = 2,
	ITF_BARCODE_RESULT_START_STOP_CODE_ERROR = 4,
	ITF_BARCODE_RESULT_UNKNOWN_DIGITS_ERROR = 5,
	ITF_BARCODE_RESULT_ICB_ATTRIBUTE_ERROR = 6,
	ITF_BARCODE_RESULT_ICB_CHECK_ERROR = 6,
	ITF_BARCODE_RESULT_DOUBLE = 8,
	ITF_BARCODE_RESULT_UP_SIDE_DOWN_ERROR = 11,
	ITF_BARCODE_RESULT_SIZE_ERROR = 13,
};


/*******************************************************************************
* TITO Ticket Settings
*
*******************************************************************************/
#define TITO_TICKET_MINIMUM_DIGIT_COUNT		10
#define TITO_TICKET_MAXIMUM_DIGIT_COUNT		28

/*******************************************************************************
* 2 Dimensional Barcode Settings
*
*******************************************************************************/
#define BARCODE_2D_EDGE_LEVEL					80
#define BARCODE_2D_WHITE_EDGE_AREA_MM			(float)2.5	//white area required around the 2d barcode ticket

#define BARCODE_2D_EDGE_SEARCH_WIDTH_MM			(float)3.0
#define BARCODE_2D_EDGE_SEARCH_PIXEL_SKIP_MM	(float)0.5
#define BARCODE_2D_CELL_EDGE_SKIP				40	//skip 40% of pixels around the edge of cells
//#define BARCODE_2D_MAXIMUM_PIXEL_SEARCH_MM		(float)100.0
#define BARCODE_2D_MAXIMUM_PIXEL_SEARCH_MM		(float)66.0

#define XX									-1 //cell does not contain regular data

#define BARCODE_2D_MINIMUM_DIGIT_COUNT		1
#define BARCODE_2D_MAXIMUM_DIGIT_COUNT		128

#define BARCODE_2D_MAX_CELLS_X				50
#define BARCODE_2D_MAX_CELLS_Y				50

enum barcode_2d_orientation_type
{
	BARCODE_2D_ORIENTATION_1 = 0,	//rotated 0 degrees
	BARCODE_2D_ORIENTATION_2,		//rotated 90 degrees
	BARCODE_2D_ORIENTATION_3,		//rotated 180 degrees
	BARCODE_2D_ORIENTATION_4,		//rotated 270 degrees
	BARCODE_2D_ORIENTATION_UNKNOWN,	//unable to detect
};

enum barcode_2d_area_type
{
	BARCODE_2D_BLACK_AREA = 0,
	BARCODE_2D_WHITE_AREA,
};

#define BARCODE_2D_CELL_READ_MODE_COUNT		2

enum barcode_2d_cell_read_mode
{
	CELL_READ_NORMAL = 0,
	CELL_READ_ENHANCED,
};


/*******************************************************************************
* Reed Solomon Error Checking Settings
*
*******************************************************************************/
#define POLYNOMIAL_MAX_SIZE				250

/*******************************************************************************
* QR Code Settings
*
*******************************************************************************/
#define QR_BARCODE_MIN_CELLS				21
#define QR_BARCODE_MAX_CELLS				29

#define QR_BARCODE_MIN_CELL_SIZE_MM			(float)0.5
#define QR_BARCODE_MAX_CELL_SIZE_MM			(float)2.0

#define QR_BARCODE_BLACK_WHITE_AVERAGE		90	//90% of calculate black white average

#define QR_BARCODE_FINDER_AREA_SIZE			8
// 2020-03-18 change limit , countermeasure qr_barcode_search_border() failed
#define QR_BARCODE_FINDER_BLACK_LIMIT		60
// 2024-12-06 yuji QR Code 受け取り改善（チケットの白部分が汚れていてもQRCodeのサーチに失敗せず受け取るように修正）
#define QR_BARCODE_FINDER_WHITE_LIMIT		90//130

#define	QR_BARCODE_FINDER_SIZE				15

#define QR_BARCODE_FORMAT_MASK				0x5412
#define QR_BARCODE_FORMAT_GENERATOR			0x0537

#define QR_BARCODE_CELLS_PER_CODEWORD					8

#define QR_BARCODE_MODE_SIZE_IN_BITS					4

#define QR_BARCODE_NUMERIC_CHARACTER_SIZE_IN_BITS		10
#define QR_BARCODE_NUMERIC_GROUP_SIZE_IN_BITS			10
#define QR_BARCODE_NUMERIC_CHARACTERS_PER_GROUP			3

#define QR_BARCODE_ALPHANUMERIC_CHARACTER_SIZE_IN_BITS	9
#define QR_BARCODE_ALPHANUMERIC_GROUP_SIZE_IN_BITS		11
#define QR_BARCODE_ALPHANUMERIC_CHARACTERS_PER_GROUP	2

#define	QR_BARCODE_ALPHANUMERIC_TABLE_SIZE				45

enum qr_error_correction_levels
{
	QR_BARCODE_ERROR_CORRECTION_LOW = 1,
	QR_BARCODE_ERROR_CORRECTION_MEDIUM = 0,
	QR_BARCODE_ERROR_CORRECTION_QUALITY = 3,
	QR_BARCODE_ERROR_CORRECTION_HIGH = 2,
};

enum qr_barcode_mask_types
{
	QR_BARCODE_MASK_0 = 0,
	QR_BARCODE_MASK_1,
	QR_BARCODE_MASK_2,
	QR_BARCODE_MASK_3,
	QR_BARCODE_MASK_4,
	QR_BARCODE_MASK_5,
	QR_BARCODE_MASK_6,
	QR_BARCODE_MASK_7,
};

enum qr_barcode_mode_types
{
	QR_BARCODE_MODE_NONE = 0,
	QR_BARCODE_MODE_NUMERIC = 1,
	QR_BARCODE_MODE_ALPHANUMERIC = 2,
	QR_BARCODE_MODE_BYTE = 4,
	QR_BARCODE_MODE_KANJI = 8,
	QR_BARCODE_MODE_ECI = 7,
};

enum qr_validation_reject_codes
{
	QR_BARCODE_RESULT_EDGE_ERROR = 1,
	QR_BARCODE_RESULT_ORIENTATION_ERROR = 1,
	QR_BARCODE_RESULT_4WAY_ERROR = 1,
	QR_BARCODE_RESULT_CELL_COUNT_ERROR = 2,
	QR_BARCODE_RESULT_CELL_SIZE_ERROR = 2,
	QR_BARCODE_RESULT_FORMAT_ERROR = 4,
	QR_BARCODE_RESULT_CORRECTION_ERROR = 4,
	QR_BARCODE_RESULT_DECODE_ERROR = 5,
	QR_BARCODE_RESULT_UNSUPPORTED_DIGITS_ERROR = 5,
	QR_BARCODE_RESULT_DOUBLE = 8,
	QR_BARCODE_RESULT_SIZE_ERROR = 13,
};

//error correction values
#define QR_BARCODE_FIRST_CONSECUTIVE_ROOT	0

/*******************************************************************************
* Data Structures
*
*******************************************************************************/
struct cis_coordinate
{
	int x;
	int y;
};
typedef struct cis_coordinate CIS_COORDINATE;
typedef struct cis_coordinate* P_CIS_COORDINATE;


struct cis_rectangle
{
	CIS_COORDINATE start;
	CIS_COORDINATE end;
};
typedef struct cis_rectangle CIS_RECTANGLE;
typedef struct cis_rectangle* P_CIS_RECTANGLE;


struct sensor_configuration
{
	int side;
	int color;

	int x_pixels;
	int y_pixels;
	float pitch;

	int deskew_center_x;
	int deskew_center_y;
	int deskew_align_x;
	int deskew_align_y;
};
typedef struct sensor_configuration SENSOR_CONFIGURATION;
typedef struct sensor_configuration* P_SENSOR_CONFIGURATION;

struct dye_check
{
	unsigned short int line_index;
	unsigned char sensor;
	int min_limit;
};
typedef struct dye_check DYE_CHECK;
typedef struct dye_check* P_DYE_CHECK;


struct check_limits
{
	int sensor;
	int min;
	int max;
};
typedef struct check_limits CHECK_LIMITS;
typedef struct check_limits* P_CHECK_LIMITS;

struct polynomial
{
	int value[POLYNOMIAL_MAX_SIZE];
	int length;
};
typedef struct polynomial POLYNOMIAL;
typedef struct polynomial* P_POLYNOMIAL;


struct serial_filter
{
	int math;
	int color1;
	int color2;
	int color3;
};
typedef struct serial_filter SERIAL_FILTER;
typedef struct serial_filter* P_SERIAL_FILTER;


struct barcode_1d_result
{
	int sensor;
	int x_point;
	int y_point;
	int orientation; //vertical or horizontal
	int wave_length;
	int wave_point;

	int left_edge;
	int right_edge;
	int black_white_average;

	int try_count;
	int digits_decoded;

	int direction;
	int error_code;

	int attribute;

	int character_length;
	int characters[BARCODE_MAXIMUM_CHARACTERS];
};
typedef struct barcode_1d_result BARCODE_1D_RESULT;
typedef struct barcode_1d_result* P_BARCODE_1D_RESULT;


struct barcode_2d_result
{
	int sensor;
	int x_point;
	int y_point;
	int orientation; //0, 90, 180, 270 degrees rotated
	int x_length;
	int y_length;

	int left_edge;
	int right_edge;
	int top_edge;
	int bottom_edge;
	int black_white_average;

	int pixel_width;
	int pixel_height;

	float x_shear;
	float y_shear;
	int x_shear_offset;
	int y_shear_offset;

	int size_version;
	int rotated_pixel_width;
	int rotated_pixel_height;
	int rotated_width_cell_count;
	int rotated_height_cell_count;
	float rotated_pixels_per_cell_width;
	float rotated_pixels_per_cell_height;

	int cell_start_x[BARCODE_2D_MAX_CELLS_X];
	int cell_start_y[BARCODE_2D_MAX_CELLS_Y];

	//qr barcode only
	int error_correction_level;
	int mask_pattern;

	int try_count;
	int codeword_error_count;
	int correction_codeword_count;
	int rs_block_count;
	POLYNOMIAL codewords_work;
	POLYNOMIAL block_message;

	int direction;
	int error_code;

	POLYNOMIAL codewords;

	int jcm_header_order;

	int character_length;
	int characters[BARCODE_MAXIMUM_CHARACTERS];
};
typedef struct barcode_2d_result BARCODE_2D_RESULT;
typedef struct barcode_2d_result* P_BARCODE_2D_RESULT;


struct image_object
{
	int bottom_edge;
	int top_edge;
	int left_edge;
	int right_edge;

	unsigned char glyph[SERIAL_GLYPH_WIDTH][SERIAL_GLYPH_HEIGHT];

	float network_margin[SERIAL_CHARACTER_TYPE_COUNT];
	int character[SERIAL_CHARACTER_TYPE_COUNT];
	int second_character[SERIAL_CHARACTER_TYPE_COUNT];
};
typedef struct image_object IMAGE_OBJECT;
typedef struct image_object* P_IMAGE_OBJECT;


struct serial_result
{
	int side;
	int orientation;

	int character_size_variable;

	int font_index;
	int filter_type;
	int filter_level;

	int center_x;
	int center_y;
	int left_edge;
	int right_edge;
	int bottom_edge;
	int top_edge;
	int width;
	int height;

	int frame_left_edge;
	int frame_right_edge;
	int frame_bottom_edge;
	int frame_top_edge;
	int frame_width;
	int frame_height;
	int frame_center_x;
	int frame_center_y;

	unsigned char raw_image[SERIAL_SENSOR_COUNT][SERIAL_MAX_AREA_WIDTH][SERIAL_MAX_AREA_HEIGHT];
	unsigned char filter_image[SERIAL_MAX_AREA_WIDTH][SERIAL_MAX_AREA_HEIGHT];

	int filter_min;
	int filter_max;
	int filter_average;
	int filter_contrast;

	IMAGE_OBJECT image_objects[SERIAL_IMAGE_OBJECT_COUNT];
	int image_object_count;

	int character_type[SERIAL_MAX_CHARACTERS];

	float network_margins[SERIAL_MAX_CHARACTERS];

	int characters[SERIAL_MAX_CHARACTERS];
	int character_length;
	int unknown_character_count;

	int partial_pass;
	int error_code;
};
typedef struct serial_result SERIAL_RESULT;
typedef struct serial_result* P_SERIAL_RESULT;


struct bill_information
{
	//validator specific sensor information
	SENSOR_INFORMATION sensor_info[CIS_SENSOR_COUNT];

	int validation_mode;

	int area_average[CIS_BILL_SENSOR_COUNT][MESH_AREA_COUNT_X][MESH_AREA_COUNT_Y];

	int top_edge[EDGE_PIXEL_COUNT_X];
	int bottom_edge[EDGE_PIXEL_COUNT_X];
	int left_edge[EDGE_PIXEL_COUNT_Y];
	int right_edge[EDGE_PIXEL_COUNT_Y];

	int calibration_up_white_x_start;
	int calibration_up_white_x_end;
	int calibration_up_white_y_start;
	int calibration_up_white_y_end;

	int calibration_up_black_x_start;
	int calibration_up_black_x_end;
	int calibration_up_black_y_start;
	int calibration_up_black_y_end;

	int calibration_down_white_x_start;
	int calibration_down_white_x_end;
	int calibration_down_white_y_start;
	int calibration_down_white_y_end;

	int calibration_down_black_x_start;
	int calibration_down_black_x_end;
	int calibration_down_black_y_start;
	int calibration_down_black_y_end;

	int calibration_difference[CALIBRATION_SENSOR_COUNT];
	int calibration_value[CALIBRATION_SENSOR_COUNT];
	int calibration_adjust[CALIBRATION_SENSOR_COUNT];

	float top_slope;
	float top_y_intercept;
	float bottom_slope;
	float bottom_y_intercept;
	float left_slope;
	float left_x_intercept;
	float right_slope;
	float right_x_intercept;

	float final_slope;

	CIS_COORDINATE raw_top_left_corner;
	CIS_COORDINATE raw_top_right_corner;
	CIS_COORDINATE raw_bottom_left_corner;
	CIS_COORDINATE raw_bottom_right_corner;

	CIS_COORDINATE raw_center;

	int center_offset_x;
	int center_offset_y;
	float bill_offset_mm;

	CIS_COORDINATE deskewed_top_left_corner;
	CIS_COORDINATE deskewed_top_right_corner;
	CIS_COORDINATE deskewed_bottom_left_corner;
	CIS_COORDINATE deskewed_bottom_right_corner;

	float skew_degree_x;
	float skew_radian_x;
	float skew_degree_y;
	float skew_radian_y;

	float top_skew_sine_correction_x;
	float top_skew_cosine_correction_x;
	float bottom_skew_sine_correction_x;
	float bottom_skew_cosine_correction_x;

	int deskewed_left_edge;
	int deskewed_right_edge;
	int deskewed_top_edge;
	int deskewed_bottom_edge;

	int width_pixel;
	int length_pixel;
	float width_mm;
	float length_mm;

	int denomination_direction;

	int mesh_start_area_x;
	int mesh_end_area_x;
	int mesh_start_area_y;
	int mesh_end_area_y;

	SENSOR_ENABLE sensor_enable_table[CIS_SENSOR_COUNT];

	int possible_ticket_detected;

	CIS_COORDINATE barcode_search_points[BARCODE_MAX_SEARCH_POINTS];
	int barcode_search_point_count;
	int ticket_direction;

	//1D barcode values
	unsigned char barcode_1d_wave[CIS_MAX_PIXEL_COUNT_X];
	int barcode_1d_areas[BARCODE_1D_MAXIMUM_BAR_COUNT];
	int barcode_1d_widths[BARCODE_1D_MAXIMUM_BAR_COUNT];
	float barcode_1d_ratios[BARCODE_1D_MAXIMUM_BAR_COUNT];
	int barcode_1d_bar_count;

	//interleaved 2 of 5 barcode values
	float itf_barcode_narrow_area_average;
	float itf_barcode_wide_area_average;

	int itf_barcode_narrow_bar_count;
	int itf_barcode_wide_bar_count;

	int itf_barcode_has_ir;

	BARCODE_1D_RESULT itf_barcode_result_temp;
	BARCODE_1D_RESULT itf_barcode_result;

	BARCODE_1D_RESULT tito_ticket_result;

	//reed solomon error correction values
	int rs_correction_first_consecutive_root;
	int rs_correction_codeword_error_found;
	int rs_correction_error_count;

	POLYNOMIAL rs_correction_syndromes;
	POLYNOMIAL rs_correction_error_locator;
	POLYNOMIAL rs_correction_error_position;
	POLYNOMIAL rs_correction_coefficient_position;
	POLYNOMIAL rs_correction_error_evaluator;

	//QR barcode values
	BARCODE_2D_RESULT qr_barcode_result_temp;
	BARCODE_2D_RESULT qr_barcode_result;

	BARCODE_2D_RESULT qr_ticket_result;
};
typedef struct bill_information BILL_INFORMATION;
typedef struct bill_information* P_BILL_INFORMATION;

/*******************************************************************************
* Ticket Validation
*
*******************************************************************************/
EXTERN BILL_INFORMATION _bill_info;
EXTERN P_BILL_INFORMATION bill_info;
EXTERN int previous_ticket_type;
EXTERN int checked_ticket_types[TICKET_TYPE_COUNT];
extern const int calibration_level[CALIBRATION_SENSOR_COUNT];
extern const int sensor_normalize_table[CIS_SENSOR_COUNT][DIRECTION_COUNT];
#endif

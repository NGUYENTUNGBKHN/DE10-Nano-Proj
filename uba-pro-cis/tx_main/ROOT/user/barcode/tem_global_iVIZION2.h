/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_global_RBA40C.h
* Contents: common source for all files for the RBA40C product
*******************************************************************************/

#include "architecture.h"
#include "struct.h"
#include "set_enable.h"
#include "common.h"

#ifndef TEM_GLOBAL_RBA40C_H
#define TEM_GLOBAL_RBA40C_H

/*******************************************************************************
* Sensor Settings
*
*******************************************************************************/
#define CIS_SENSOR_COUNT			27
#define CIS_BILL_SENSOR_COUNT		27
#define CIS_BARCODE_SENSOR_COUNT	0
#define CIS_CUSTOM_SENSOR_COUNT		0
enum cis_sensor_type
{
	CIS_RED_UP = 0,
	CIS_GREEN_UP,
	CIS_BLU_UP,
	CIS_IR1_UP,
	CIS_IR2_UP,
	BLANK1,
	BLANK2,
	BLANK3,
	CIS_RED_DOWN,
	CIS_GREEN_DOWN,
	CIS_BLU_DOWN,
	CIS_IR1_DOWN,
	CIS_IR2_DOWN,
	BLANK4,
	BLANK5,
	BLANK6,
	CIS_RED_TRANSPARENT_UP,
	CIS_GREEN_TRANSPARENT_UP,
	CIS_BLU_TRANSPARENT_UP,
	CIS_IR1_TRANSPARENT_UP,
	CIS_IR2_TRANSPARENT_UP,
	BLANK7,
	BLANK8,
	CIS_RED_TRANSPARENT_DOWN,
	CIS_GREEN_TRANSPARENT_DOWN,
	CIS_BLU_TRANSPARENT_DOWN,
	CIS_IR1_TRANSPARENT_DOWN,
	CIS_IR2_TRANSPARENT_DOWN,
};

#define CIS_BILL_PIXEL_ALIGN_X				0
#define CIS_BILL_PIXEL_ALIGN_Y				0
#define CIS_BARCODE_UP CIS_RED_UP
#define CIS_BARCODE_DOWN CIS_RED_DOWN



/*******************************************************************************
* Serial OCR Settings
*
*******************************************************************************/
#define SERIAL_SENSOR_COUNT			3

enum serial_sensors
{
	SERIAL_SENSOR_GREEN = 0,
	SERIAL_SENSOR_RED,
	SERIAL_SENSOR_IR,
};

//pixel counts for barcode sensors
#define CIS_BARCODE_PIXEL_COUNT_X			(SCAN_BLOCK_SIZE * 4)
#define CIS_BARCODE_PIXEL_COUNT_Y			720
#define CIS_MAX_PIXEL_COUNT_X	CIS_BARCODE_PIXEL_COUNT_X
#define CIS_MAX_PIXEL_COUNT_Y	CIS_BARCODE_PIXEL_COUNT_Y

#define CIS_BARCODE_UP_PIXEL_ALIGN_X		0
#define CIS_BARCODE_UP_PIXEL_ALIGN_Y		0
#define CIS_BARCODE_DOWN_PIXEL_ALIGN_Y		0
#define CIS_BARCODE_DOWN_PIXEL_ALIGN_X		0

/*******************************************************************************
* Edge Detection Settings
*
*******************************************************************************/
#define EDGE_DETECTION_SENSOR			CIS_RED_UP
#define EDGE_PIXEL_COUNT_X				CIS_BARCODE_PIXEL_COUNT_X
#define EDGE_PIXEL_COUNT_Y				CIS_BARCODE_PIXEL_COUNT_Y


/*******************************************************************************
* Calibration Settings
*
*******************************************************************************/
#define CALIBRATION_SENSOR_COUNT			11

#define CALIBRATION_UP_X_START				10
#define CALIBRATION_UP_X_END				15
#define CALIBRATION_UP_WHITE_Y_START		164
#define CALIBRATION_UP_WHITE_Y_END			172
#define CALIBRATION_UP_BLACK_Y_START		189
#define CALIBRATION_UP_BLACK_Y_END			197

#define CALIBRATION_DOWN_AREA_1_X_BUFFER	25
#define CALIBRATION_DOWN_AREA_1_X_WIDTH		9

#define CALIBRATION_DOWN_WHITE_Y_START		19
#define CALIBRATION_DOWN_WHITE_Y_END		27
#define CALIBRATION_DOWN_BLACK_Y_START		44
#define CALIBRATION_DOWN_BLACK_Y_END		52

#define CALIBRATION_DIFFERENCE_MIN_LIMIT	-30
#define CALIBRATION_DIFFERENCE_MAX_LIMIT	30

#define CALIBRATION_RATIO_MULTIPLIER		128




#endif

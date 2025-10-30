/*******************************************************************************
* Project: CIS Bill Acceptor
* File: set_enable.h
* Contents: validation test options
*
*
*******************************************************************************/
#ifndef SET_ENABLE_H
#define SET_ENABLE_H


/*******************************************************************************
* Enable Ticket Validation
*
*******************************************************************************/
//#define ENABLE_SETTING_TICKET
#define ENABLE_TITO_TICKET
#define ENABLE_QR_TICKET

/*******************************************************************************
* Validation Algorithm Options
*
*******************************************************************************/
//#define OPTION_ENABLE_CALIBRATION			1
#define OPTION_ENABLE_CALIBRATION			0
#define OPTION_DYE_CHECK_SINGLE_SIDE		0


/*******************************************************************************
* Enable and Disable Validation Tests
*
*******************************************************************************/
#define CHECK_CALIBRATION_ERROR				0 //enable to sort data only
#define CHECK_EDGE_ERROR					1
#define CHECK_NETWORK_MARGIN_ERROR			1
#define CHECK_LENGTH_ERROR					1
#define CHECK_WIDTH_ERROR					1
#define CHECK_MESH_2COLOR_ERROR				1
#define CHECK_MESH_CROSS_ERROR				1
#define CHECK_MESH_3COLOR_ERROR				1
#define CHECK_DOUBLE_ERROR					1
#define CHECK_UV_ERROR						1
#define CHECK_NET_CF_1COLOR_LAYER1_ERROR	1
#define CHECK_NET_CF_1COLOR_LAYER2_ERROR	1
#define CHECK_NET_CF_2COLOR_LAYER1_ERROR	1
#define CHECK_NET_CF_2COLOR_LAYER2_ERROR	1
#define CHECK_CUSTOM_ERROR					1


/*******************************************************************************
* Enable and Disable Dye Stain Tests
*
*******************************************************************************/
#define CHECK_DYE_ERROR						0


/*******************************************************************************
* Enable and Disable Category Fitness Tests
*
*******************************************************************************/
#define CHECK_CATEGORY_FITNESS				0

#define CHECK_FITNESS_LENGTH_ERROR			0
#define CHECK_FITNESS_WIDTH_ERROR			0
#define CHECK_FITNESS_HOLE_ERROR			0
#define CHECK_FITNESS_MESH_ERROR			0


/*******************************************************************************
* Enable and Disable Serial Number OCR
*
*******************************************************************************/
#define ENABLE_SERIAL_OCR					1

#define SERIAL1_RESULT_ONLY					0
#define SERIAL2_RESULT_ONLY					0
#define SERIAL_RESULT_BOTH					1

#endif

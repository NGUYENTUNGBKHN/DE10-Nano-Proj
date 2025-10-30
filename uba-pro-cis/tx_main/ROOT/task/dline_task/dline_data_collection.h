/******************************************************************************/
/*! @addtogroup Group1
    @file       usb_data_collection.h
    @brief      usb data collection header file
    @date       2018/6/22
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan Cash Machine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/6/22 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

#ifndef _SRC_INCLUDE_DATA_COLLECTION_H_
#define _SRC_INCLUDE_DATA_COLLECTION_H_

#define DATA_COLLECTION_HEADER_SIZE 	128
#define DATA_COLLECTION_PARAMETER_SIZE 	7
enum DATA_COLLECTION_PHASE_NUMBER
{
	PHASE_DATA_COLLECTION			= 0x01,
	PHASE_DATA_COLLECTION_STATUS	= 0x02
};

/************************** PRIVATE FUNCTIONS *************************/

void send_data_collection_header(void);
void send_data_collection_information(void);
u8 send_data_collection_data(void);
void send_data_collection_status(void);
void front_usb_data_collection_request(void);

#endif/* _SRC_INCLUDE_DATA_COLLECTION_H_ */
/*--- End of File ---*/

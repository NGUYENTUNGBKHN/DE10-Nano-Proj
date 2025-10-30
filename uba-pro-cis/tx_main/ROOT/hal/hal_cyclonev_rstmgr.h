/******************************************************************************/
/*! @addtogroup Main
    @file       hal_bezel.c
    @brief      Bezel LED driver.
    @brief      ベゼルLED発光制御ドライバファイル。
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_bezel.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#pragma once

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

void RstmgrInit(void);
void RstmgrWdt0Reset(void);
void RstmgrWdt1Reset(void);
void RstmgrI2C1Reset(void);
void RstmgrI2C0Reset(void);
void RstmgrI2C3Reset(void);
void RstmgrUSB0Reset(void);
void RstmgrUSB1Reset(void);
void RstmgrUart1Reset(void);
void RstmgrSdmmcReset(void);
void RstmgrS2fReset(void);
void RstmgrFpgamgrReset(void);
void RstmgrBridgeReset(void);
/* EOF */

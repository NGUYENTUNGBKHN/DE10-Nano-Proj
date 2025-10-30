/******************************************************************************/
/*! @addtogroup Main
    @file       otg_task.c
    @brief      control status led task function
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "js_oswapi.h"
#include "common.h"
#include "perid.h"
#include "peri_prm.h"
#include "peri_sts.h"
#include "peri_hal.h"
#include "hal_usb.h"

#define EXT
#include "com_ram.c"

/************************** Function Prototypes ******************************/
LOCAL void OTG_Mode_Change(void);

/************************** External functions *******************************/
EXTERN void usb_host_dev_test_init(void);
/************************** Variable declaration *****************************/

/* Device or Host */
#define GRP_MODE_HOST			(0)
#define GRP_MODE_DEVICE			(1)

/************************** EXTERN FUNCTIONS *************************/

/*******************************
        otg_task
 *******************************/
void otg_task(VP_INT exinf)
{
	/* モード切替テスト */
	OTG_Mode_Change();
}

/************************************************************************************************/
/* FUNCTION   : OTG_Mode_Change                                                                 */
/*                                                                                              */
/* DESCRIPTION: モード切り替えタスク	                                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none									                                        */
/*                                                                                              */
/************************************************************************************************/
void OTG_Mode_Change(void)
{
	/* モード切り替え処理初期化 */
	reset_usb0();

	usb_host_dev_test_init();
}


/* EOF */

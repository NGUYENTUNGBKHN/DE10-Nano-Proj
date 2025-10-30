/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2019 Grape Systems, Inc.                             */
/*                                     All Rights Reserved.                                     */
/*                                                                                              */
/* This software is furnished under a license and may be used and copied only in accordance     */
/* with the terms of such license and with the inclusion of the above copyright notice.         */
/* No title to and ownership of the software is transferred. Grape Systems Inc. makes no        */
/* representation or warranties with respect to the performance of this computer program, and   */
/* specifically disclaims any responsibility for any damages, special or consequential,         */
/* connected with the use of this program.                                                      */
/*                                                                                              */
/************************************************************************************************/
/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      grp_cyclonevh.h                                                         1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# CYCLONEV Controller common function header File                            */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kaneko       2019/04/26  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                              (based on GR-USB/HOST# for STM32Fx(OTG_HS) V1.01)               */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CYCLONEVH_H_
#define _GRP_CYCLONEVH_H_

#include "grp_std_tools.h"
#include "grp_vos.h"
#include "grp_std_types.h"
#include "grp_cmem.h"
#include "grp_usbd.h"
#include "grp_hcd.h"
#include "grp_hcd_index.h"
#include "grp_cyclonev_tg.h"
#include "grp_cyclonev_cmod.h"
#include "grp_cyclonev_macro.h"
#include "grp_cyclonev_bit_val.h"
#include "grp_cyclonev_reg.h"
#include "grp_cyclonevh_debug.h"
#include "grp_cyclonevh_config.h"

/*** INTERNAL DATA DEFINES **********************************************************************/
/* cancel flag */
#define GRP_CYCLONEVH_CANCEL_FLAG           ((grp_u8)0x80)
/* default pipe number */
#define GRP_CYCLONEVH_PIPE0                 (0)

/* internal management information structure */
typedef struct grp_cyclonevh_mng_info_tag
{
    grp_u8                              ucStatus;               /* Pipe status                  */
    grp_u8                              ucChIdx;                /* Index number                 */
    grp_u8                              ucDPID;
    grp_u8                              aucPad[1];              /* padding                      */
    grp_u32                             ulTmpBufLen;            /* Tmporary buffer length       */
    grp_u32                             ulHcChar;               /* Host channel characteristics */
    grp_u32                             ulDMABufSize;           /* DMA buffer size              */

} grp_cyclonevh_mng_info;

/* Pipe information structure */
typedef struct grp_cyclonevh_pipe_info_tag
{
    grp_u8                              ucStatus;               /* Pipe status                  */
    grp_u8                              ucCount;
    grp_u8                              ucTxMode;
    grp_u8                              ucTxDir;
    grp_hcdi_tr_request                 *ptTrReq;
    
} grp_cyclonevh_pipe_info;

/* Pipe status */
#define GRP_CYCLONEVH_PS_NO_USE             (0)
    /* use bulk, interrupt, isochonous transfer */
#define GRP_CYCLONEVH_PS_USE                (1)
    /* use control transfer */
#define GRP_CYCLONEVH_PS_SETUP_OUT          (2)
#define GRP_CYCLONEVH_PS_SETUP_IN           (3)
#define GRP_CYCLONEVH_PS_DATA_OUT           (4)
#define GRP_CYCLONEVH_PS_DATA_IN            (5)
#define GRP_CYCLONEVH_PS_DATA_0_OUT         (6)
#define GRP_CYCLONEVH_PS_DATA_0_IN          (7)
#define GRP_CYCLONEVH_PS_END                (8)
#define GRP_CYCLONEVH_PS_END_0_IN           (9)
#define GRP_CYCLONEVH_PS_ERROR              (10)
    /* Halt status */
#define GRP_CYCLONEVH_PS_PIPE_HALT          (0x80)
/* PID */
#define GRP_CYCLONEVH_PID_DATA0             (0)
#define GRP_CYCLONEVH_PID_DATA1             (1)

/* Pipe number */
#define GRP_CYCLONEVH_CTRL_SETUP            (0)
#define GRP_CYCLONEVH_CTRL_DATAIN           (1)
#define GRP_CYCLONEVH_CTRL_DATAOUT          (2)
#define GRP_CYCLONEVH_CTRL_PIPE             GRP_CYCLONEVH_CTRL_DATAOUT

/* Vbus Power flag */
#define GRP_CYCLONEVH_VBUS_POWER_OFF        (0)
#define GRP_CYCLONEVH_VBUS_POWER_ON         (1)

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
extern grp_s32  grp_cyclonevh_HcInit(void);
extern grp_s32  grp_cyclonevh_DisableAllChannel(void);

extern grp_s32  grp_cyclonevh_CtrInit(void);
extern grp_s32  grp_cyclonevh_CtrTrRun(grp_hcdi_tr_request *ptTrReq);
extern void     grp_cyclonevh_Interrupt(void);
extern grp_s32  grp_cyclonevh_CtrCmpPipe(grp_u16 usIntrFactor, grp_u16 usPktData, grp_u8 ucPipeNum);

extern grp_s32  grp_cyclonevh_RhubInit(void);
extern void     grp_cyclonevh_RhEnable(grp_u32 ulFlag);
extern grp_s32  grp_cyclonevh_RhubPortPowerOn(grp_u32 ulIndex, grp_u8 ucPortNum);
extern grp_s32  grp_cyclonevh_RhubPortPowerOff(grp_u32 ulIndex, grp_u8 ucPortNum);
extern grp_s32  grp_cyclonevh_RhubEnumeration(grp_u8 ucPortNum);

extern grp_s32  grp_cyclonevh_MngInit(void);
extern grp_s32  grp_cyclonevh_MngGetPipe(grp_hcdi_endpoint *ptHcdiEp, grp_cyclonevh_mng_info **pptPipeInfo);
extern grp_s32  grp_cyclonevh_MngReleasePipe(grp_hcdi_endpoint *ptHcdiEp);
extern grp_s32  grp_cyclonevh_MngGetPipeFromNum(grp_u8 ucPipeNum, grp_hcdi_tr_request **pptTrReq);
extern grp_s32  grp_cyclonevh_MngSetRequest(grp_hcdi_tr_request *ptTrReq);
extern grp_s32  grp_cyclonevh_MngGetRequest(grp_hcdi_tr_request **pptTrReq);
extern grp_s32  grp_cyclonevh_MngReleaseRequest(grp_hcdi_tr_request *ptTrReq, grp_hcdi_tr_request **pptNextReq);
extern grp_s32  grp_cyclonevh_MngCheckRequest(grp_hcdi_tr_request *ptTrReq);
extern grp_s32  grp_cyclonevh_MngCancelAllReq(grp_u8 ucIdx, grp_hcdi_endpoint *ptHcdiEp);

extern grp_s32  grp_cyclonevh_CB_Process(grp_hcdi_tr_request *ptTrReq, grp_u8 ucIdx);
extern grp_s32  grp_cyclonevh_MngNotCmpReqAbort(void);
extern grp_s32  grp_cyclonevh_MngDelRequest(grp_hcdi_tr_request *ptTrReq);

extern grp_s32  grp_cyclonevh_SetVbusPower(grp_s32 lPower);

/*** INTERNAL MACRO FUNCTION ********************************************************************/
/*                                                                                              */
extern grp_vos_t_semaphore              *g_ptListLock;
#define GRP_CYCLONEVH_LOCK() {                                                                  \
    if (grp_vos_GetSemaphore(g_ptListLock, GRP_VOS_INFINITE) != GRP_VOS_POS_RESULT) {           \
        return GRP_HCDI_ERROR;                                                                  \
    }                                                                                           \
}
/*                                                                                              */
#define GRP_CYCLONEVH_UNLOCK() {                                                                \
    if (grp_vos_ReleaseSemaphore(g_ptListLock) != GRP_VOS_POS_RESULT) {                         \
        return GRP_HCDI_ERROR;                                                                  \
    }                                                                                           \
}


#endif  /* _GRP_CYCLONEVH_H_ */


/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2015 Grape Systems, Inc.                        */
/*                                       All Rights Reserved                                    */
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
/*      grp_cyclonevd_hal.h                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/DEVICE for CycloneV(OTG) - Hardware Abstraction Layer module header file         */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Kobayashi    2014/12/24  V0.90                                                           */
/*                            - Created beta release version.                                   */
/*   T.Kobayashi    2015/01/21  V1.00                                                           */
/*                            - 1st release version.                                            */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CYCLONEVD_HAL_H_
#define _GRP_CYCLONEVD_HAL_H_

/**** INCLUDE FILES *****************************************************************************/
#include    "grusbtyp.h"
#include    "grp_cyclonev_cfg.h"


/**** EXTERNAL DATA DEFINES *********************************************************************/
#define GRP_CYCLONEVD_EP_NUM                (1 + 15)                /* Number of endpoint       */
#define GRP_CYCLONEVD_BUF_SIZE              (8 * 3)                 /* Setup packet buffer size */

/* CONTROL transfer state */
#define GRP_CYCLONEVD_CTLST_IDLE            (0x0000)                /* Idle                     */
#define GRP_CYCLONEVD_CTLST_STAGE_TYPE      (0xFF00)                /* Stage type               */
    #define GRP_CYCLONEVD_CTLST_DATA            (0x0100)                /* Data stage               */
    #define GRP_CYCLONEVD_CTLST_STATUS          (0x1000)                /* Status stage             */
#define GRP_CYCLONEVD_CTLST_TRANS_TYPE      (0x000F)                /* Transfer type            */
    #define GRP_CYCLONEVD_CTLST_IN              (0x0001)                /* CONTROL IN               */
    #define GRP_CYCLONEVD_CTLST_OUT             (0x0002)                /* CONTROL OUT              */
    #define GRP_CYCLONEVD_CTLST_NODATA          (0x0004)                /* CONTROL NODATA           */
#define GRP_CYCLONEVD_CTLST_OPTION_TYPE     (0x00F0)                /* Transfer option type     */
    #define GRP_CYCLONEVD_CTLST_TRANS_CONT      (0x0010)                /* Transfer continue        */
    #define GRP_CYCLONEVD_CTLST_0LEN_PKT        (0x0020)                /* Send zero length packet  */
    #define GRP_CYCLONEVD_CTLST_NO_BUF_WAIT     (0x0040)                /* No buffer wait           */
    #define GRP_CYCLONEVD_CTLST_STALL           (0x0080)                /* STALL condition          */
    
/* Packet type */
#define GRP_CYCLONEVD_PKT_TYPE_SETUP        (0x00)                  /* SETUP packet */
#define GRP_CYCLONEVD_PKT_TYPE_DATA         (0x01)                  /* DATA packet  */

/* Receive transfer result */
#define GRP_CYCLONEVD_RX_INIT               (0x00)                  /* Initialization completion    */
#define GRP_CYCLONEVD_RX_FIRST              (0x01)                  /* First receive transfer       */
#define GRP_CYCLONEVD_RX_CONT               (0x02)                  /* Next data is exist           */
#define GRP_CYCLONEVD_RX_END                (0x03)                  /* End of transfer to buffer    */
#define GRP_CYCLONEVD_RX_NOBUF              (0x04)                  /* No buffer to receive data    */

/* Test mode selectors */
#define GRP_CYCLONEVD_TM_DISABLED           (0x0000)                /* Test mode disabled   */
#define GRP_CYCLONEVD_TM_TEST_J             (0x0001)                /* Test_J mode          */
#define GRP_CYCLONEVD_TM_TEST_K             (0x0002)                /* Test_K mode          */
#define GRP_CYCLONEVD_TM_TEST_SE0_NAK       (0x0003)                /* Test_SE0_NAK mode    */
#define GRP_CYCLONEVD_TM_TEST_PACKET        (0x0004)                /* Test_Packet mode     */
#define GRP_CYCLONEVD_TM_TEST_FORCE_ENA     (0x0005)                /* Test_Force_Enable    */

/* Threshold length */
#define GRP_CYCLONEVD_RX_THR_LEN            (64)                    /* Receive threshold length     */
#define GRP_CYCLONEVD_TX_THR_LEN            (64)                    /* Transmit threshold length    */


/**** STRUCTURES ********************************************************************************/
/* Receive info structure */
typedef struct grp_cyclonevd_rcv_info_tag
{
    BOOLEAN         bRcvFlag;                       /* This flag checks whether it is receiving */
    UINT32          ulRcvStat;                      /* Data receiving status                    */
} grp_cyclonevd_rcv_info;


/**** EXTERNAL FUNCTION PROTOTYPES **************************************************************/
VOID    grp_cyclonevd_hal_EnumerationCompletion( VOID );
VOID    grp_cyclonevd_hal_BusResetIntr( VOID );
VOID    grp_cyclonevd_hal_SuspendIntr( VOID );
VOID    grp_cyclonevd_hal_ResumeIntr( VOID );
VOID    grp_cyclonevd_hal_CtrlSetupIntr( VOID );
VOID    grp_cyclonevd_hal_CtrlOutTransactionIntr( VOID );
VOID    grp_cyclonevd_hal_CtrlInTransactionIntr( VOID );
VOID    grp_cyclonevd_hal_DataOutTransactionIntr( GRUSB_EndPointInfo* );
VOID    grp_cyclonevd_hal_DataInTransactionIntr( GRUSB_EndPointInfo* );
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
VOID    grp_cyclonevd_hal_RxFifoNonEmptyIntr( VOID );
VOID    grp_cyclonevd_hal_TxFifoEmptyIntr( GRUSB_EndPointInfo* );
#endif
VOID    grp_cyclonevd_hal_GlobalOutNakEffectiveIntr( VOID );
VOID    grp_cyclonevd_hal_InEndpointNakEffectiveIntr( INT );
VOID    grp_cyclonevd_hal_OutEndpointDisabledIntr( GRUSB_EndPointInfo* );
VOID    grp_cyclonevd_hal_InEndpointDisabledIntr( GRUSB_EndPointInfo* );
VOID    grp_cyclonevd_hal_RecvEnable( INT, UINT8 );
VOID    grp_cyclonevd_hal_RecvDisable( INT );
VOID    grp_cyclonevd_hal_SendEnable( INT );
VOID    grp_cyclonevd_hal_SendDisable( INT );
BOOLEAN grp_cyclonevd_hal_ChackVBusStatus( VOID );


VOID    grp_cyclonevd_hal_EnumerationCompletion2( VOID );
VOID    grp_cyclonevd_hal_BusResetIntr2( VOID );
VOID    grp_cyclonevd_hal_SuspendIntr2( VOID );
VOID    grp_cyclonevd_hal_ResumeIntr2( VOID );
VOID    grp_cyclonevd_hal_CtrlSetupIntr2( VOID );
VOID    grp_cyclonevd_hal_CtrlOutTransactionIntr2( VOID );
VOID    grp_cyclonevd_hal_CtrlInTransactionIntr2( VOID );
VOID    grp_cyclonevd_hal_DataOutTransactionIntr2( GRUSB_EndPointInfo* );
VOID    grp_cyclonevd_hal_DataInTransactionIntr2( GRUSB_EndPointInfo* );
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
VOID    grp_cyclonevd_hal_RxFifoNonEmptyIntr2( VOID );
VOID    grp_cyclonevd_hal_TxFifoEmptyIntr2( GRUSB_EndPointInfo* );
#endif
VOID    grp_cyclonevd_hal_GlobalOutNakEffectiveIntr2( VOID );
VOID    grp_cyclonevd_hal_InEndpointNakEffectiveIntr2( INT );
VOID    grp_cyclonevd_hal_OutEndpointDisabledIntr2( GRUSB_EndPointInfo* );
VOID    grp_cyclonevd_hal_InEndpointDisabledIntr2( GRUSB_EndPointInfo* );
VOID    grp_cyclonevd_hal_RecvEnable2( INT, UINT8 );
VOID    grp_cyclonevd_hal_RecvDisable2( INT );
VOID    grp_cyclonevd_hal_SendEnable2( INT );
VOID    grp_cyclonevd_hal_SendDisable2( INT );
BOOLEAN grp_cyclonevd_hal_ChackVBusStatus2( VOID );


#endif  /* _GRP_CYCLONEVD_HAL_H_ */

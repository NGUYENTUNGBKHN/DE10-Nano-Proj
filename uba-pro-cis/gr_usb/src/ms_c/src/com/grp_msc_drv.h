/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2008 Grape Systems, Inc.                          */
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
/*      grp_msc_drv.h                                                           1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Header.                                          */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created new policy initial version                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Correction by member change in structure.                       */
/*                              - grp_msc_reg                                                   */
/*                                  pfnCallback -> pfnMscEvCallback                             */
/*                              - _grp_msc_staff                                                */
/*                                  pfnCallback -> pfnMscSfCallback                             */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_MSC_DRV_H_
#define _GRP_MSC_DRV_H_

/***** INCLUDE FILES ****************************************************************************/

#include "grp_msc_config.h"


/***** CONSTANT DEFINITIONS *********************************************************************/
/* Function Result Codes */
#define GRP_MSC_OK                      (grp_s32)0

#define GRP_MSC_RET_ERROR_BASE          GRP_RET_ERROR_BASE(GRP_MSC_MDL_ID)

#define GRP_MSC_ERROR                   (GRP_MSC_RET_ERROR_BASE-1)  /* Other error              */
#define GRP_MSC_NOSUPPORT               (GRP_MSC_RET_ERROR_BASE-2)  /* Not support              */
#define GRP_MSC_BUSY                    (GRP_MSC_RET_ERROR_BASE-3)  /* Busy, please retry       */
#define GRP_MSC_UNINIT_ERROR            (GRP_MSC_RET_ERROR_BASE-4)  /* Uninit                   */
#define GRP_MSC_REQUEST_ERROR           (GRP_MSC_RET_ERROR_BASE-5)  /* Request error            */
#define GRP_MSC_RESOURCE_ERROR          (GRP_MSC_RET_ERROR_BASE-6)  /* Resource too short       */
#define GRP_MSC_VOS_ERROR               (GRP_MSC_RET_ERROR_BASE-7)  /* Virtual OS error         */
#define GRP_MSC_CMEM_ERROR              (GRP_MSC_RET_ERROR_BASE-8)  /* CMEM module error        */

/* Function Complete Codes */
#define GRP_MSC_NOT_PROCESS             (grp_s32)0
#define GRP_MSC_NO_FAIL                 (grp_s32)1
#define GRP_MSC_CANCEL                  (grp_s32)2

#define GRP_MSC_TR_FAIL_BASE            GRP_TR_FAIL_BASE(GRP_MSC_MDL_ID)

#define GRP_MSC_ILLEGAL_FAIL            (GRP_MSC_TR_FAIL_BASE-1)
#define GRP_MSC_TIMEOUT                 (GRP_MSC_TR_FAIL_BASE-2)
#define GRP_MSC_CHECK_CONDITION         (GRP_MSC_TR_FAIL_BASE-3)
#define GRP_MSC_STALL                   (GRP_MSC_TR_FAIL_BASE-4)

/* Device Identifier */
#define GRP_MSC_DEFAULT_DEVID           (0x8000)

/* Device Condition Codes */
#define GRP_MSC_ATTACHED                GRP_CNFSFT_DEVICE_ATTACHED
#define GRP_MSC_DETACHED                GRP_CNFSFT_DEVICE_DETACHED

/* Device Status Code */
#define GRP_MSC_BLANK                   (0)                         /* No connect and close     */
#define GRP_MSC_CONNECT                 (1)                         /* Attach but not open      */
#define GRP_MSC_DISCONNECT              (2)                         /* Detach but not close     */
#define GRP_MSC_OPEN                    (3)                         /* Attach and open          */
#define GRP_MSC_CLOSE                   (4)                         /* Temporary close          */
#define GRP_MSC_HIBERNATE               (5)                         /* Suspend when connect     */
#define GRP_MSC_SUSPEND                 (6)                         /* Suspend when open        */
#define GRP_MSC_UNKNOWN                 (9)                         /* Unknown                  */

/* MSC class code */
#define GRP_MSC_CLASS_CODE              0x08
#define GRP_MSC_VENDOR_CODE             0xff                    /* Use subclass and/or protocol */

/* MSC sub class code */
#define GRP_MSC_ATAPI_CODE              0x02
#define GRP_MSC_SFF8070I_CODE           0x05
#define GRP_MSC_SCSI_CODE               0x06
#define GRP_MSC_UFI_CODE                0x04

/* MSC protocol code */
#define GRP_MSC_BOT_CODE                0x50
#define GRP_MSC_CBI_CODE                0x00
#define GRP_MSC_CB_CODE                 0x01

/* MSC number of pipes */
#define GRP_MSC_BOT_PIPES               2
#define GRP_MSC_CBI_PIPES               3

/* Reset mode */
#define GRP_MSC_RESET_MASS              1           /* Mass strage reset or Command block reset */
#define GRP_MSC_RESET_ENUM              2           /* Port reset and Enumeration               */

/* Number of Pipes for Mass Storage Class */
#ifdef GRP_MSC_USE_CBI
#define GRP_MSC_PIPE_MAX                GRP_MSC_CBI_PIPES
#else
#define GRP_MSC_PIPE_MAX                GRP_MSC_BOT_PIPES
#endif

/* Transfer mode between interfaces */
#define GRP_MSC_XFER_STYLE_CONCURRENT   (grp_msc_set *)(1)
#define GRP_MSC_XFER_STYLE_EXCLUSIVE    (grp_msc_set *)((grp_s32)-1)

/* Register mode */
#define GRP_MSC_REG_VENDOR              (0)
#define GRP_MSC_REG_PROTOCOL            (4)

/* Transfer phase */
#define GRP_MSC_PHASE_NON               (0)
#define GRP_MSC_PHASE_MSRESET           (1)
#define GRP_MSC_PHASE_GETMAXLUN         (2)
#define GRP_MSC_PHASE_COMMAND           (3)
#define GRP_MSC_PHASE_DOUT              (4)
#define GRP_MSC_PHASE_DIN               (5)
#define GRP_MSC_PHASE_STATUS            (6)
#define GRP_MSC_PHASE_UNKNOWN           (9)

/* Sequence */
#define GRP_MSC_SEQ_NORMAL              0                       /* Normal sequence              */
#define GRP_MSC_SEQ_MSRESET             1                       /* Mass storage reset           */
#define GRP_MSC_SEQ_CLRPIPE             2                       /* Clear pipe use clear feature */
#define GRP_MSC_SEQ_TERM                3                       /* Terminate. Maybe callback    */
#define GRP_MSC_SEQ_NOP                 9                       /* No operation                 */

/* MSC internal status */
#define GRP_MSC_FREE                    (0)
#define GRP_MSC_USE                     (1)

/* MSC wrapper parameter */
/* for BOT */
#define GRP_MSC_CBW_SIGNATURE           0x43425355
#define GRP_MSC_CBW_LENGTH              (31)
#define GRP_MSC_CBWCB_LENGTH            (16)
#define GRP_MSC_CSW_SIGNATURE           0x53425355
#define GRP_MSC_CSW_LENGTH              (13)
#define GRP_MSC_FIRST_TAG_NUM           0x00000001

/* for CBI */
#define GRP_MSC_IDB_LENGTH              (2)                     /* Interrupt Data Block Length  */

/* for Common */
#define GRP_MSC_CMD_LENGTH              GRP_MSC_CBWCB_LENGTH /* Maximum length in all protocols */
#define GRP_MSC_STS_LENGTH              GRP_MSC_CSW_LENGTH   /* Maximum length in all protocols */

/* MSC semaphore name */
#define GRP_MSC_SEM_NAME                (grp_u8 *)"sMSC"

/***** SETTING DEPEND ON CONFIGURATIONS *********************************************************/
#ifdef GRP_MSC_EXCLUSIVE_TRANSFER
#define GRP_MSC_XFER_STYLE              GRP_MSC_XFER_STYLE_EXCLUSIVE
#else   /* GRP_MSC_EXCLUSIVE_TRANSFER */
#define GRP_MSC_XFER_STYLE              GRP_MSC_XFER_STYLE_CONCURRENT
#endif  /* GRP_MSC_EXCLUSIVE_TRANSFER */


/***** STRUCTURE TYPEDEFS ***********************************************************************/
typedef struct _grp_msc_set             grp_msc_set;
typedef struct _grp_msc_staff           grp_msc_staff;
typedef struct _grp_msc_cmd             grp_msc_cmd;

typedef grp_msc_set                     *grp_msc_hdr;

typedef struct _grp_msc_pipes
{
    grp_usbdi_pipe                      *ptBulkIn;
    grp_usbdi_pipe                      *ptBulkOut;
    grp_usbdi_pipe                      *ptIntrIn;
}grp_msc_pipes;

typedef struct _grp_msc_cbw
{
    grp_u32                             dCBWSignature;
    grp_u32                             dCBWTag;
    grp_u32                             dCBWDataTransferLength;
    grp_u8                              bmCBWFlags;
    grp_u8                              bmCBWLUN;
    grp_u8                              bmCBWCBLength;
    grp_u8                              bmCBWCB[GRP_MSC_CBWCB_LENGTH];
}grp_msc_cbw;

typedef struct _grp_msc_csw
{
    grp_u32                             dCSWSignature;
    grp_u32                             dCSWTag;
    grp_u32                             dCSWDataResidue;
    grp_u8                              bCSWStatus;
}grp_msc_csw;

/* Interrupt data block for CBI status */
typedef struct _grp_msc_idb
{
    grp_u8                              bType;
    grp_u8                              bValue;
}grp_msc_idb;

typedef struct _grp_msc_notify
{
    grp_si                              iEvent;
    grp_msc_hdr                         hMscHdr;
    grp_u8                              ucSubClass;
    grp_u8                              ucProtocol;
    void                                *pvUserRef;
}grp_msc_notify;
typedef grp_s32 (*grp_msc_notify_callback)(grp_msc_notify *);

typedef struct _grp_msc_reg
{
    grp_u8                              ucSubClass;
    grp_u8                              ucProtocol;
    grp_u8                              ucInputIfNum;    /* Use vendor specified register only   */
    grp_u8                              ucMode;
    grp_msc_notify_callback             pfnMscEvCallback;
    void                                *pvUserRef;
    grp_u16                             usVendorId;
    grp_u16                             usProductId;
    grp_u32                             ulStatus;
}grp_msc_reg;

typedef grp_s32 (*grp_msc_request_callback)(void *);
struct _grp_msc_staff
{
    grp_msc_cmd                         *ptNextCmd;
    grp_u32                             ulThisTag;
    grp_msc_request_callback            pfnMscSfCallback;
    void                                *pvParam;
    grp_s32                             lStatus;
    grp_u32                             ulRetry;
    void                                *pvExtraRef;
};

typedef grp_s32 (*grp_msc_func)(grp_msc_cmd *);
struct _grp_msc_cmd
{
    grp_msc_hdr                         hMscHdr;
    grp_u8                              ucLun;
    grp_u8                              ucDir;
    grp_u8                              ucPadding[2];
    grp_s32                             lStatus;
    grp_u8                              *pucReqBuffer;
    grp_u32                             ulReqLength;
    grp_u32                             ulActualLength;
    grp_msc_func                        pfnCallback;
    void                                *pvUserRef;
    grp_u32                             ulCmdLength;
    grp_u8                              aucCmdContent[GRP_MSC_CMD_LENGTH];
    grp_msc_staff                       tStaff;
};

typedef struct _grp_msc_seq
{
    grp_u32                             ulStatus;

    grp_msc_func                        pfnOpen;
    grp_msc_func                        pfnClose;
    grp_msc_func                        pfnCmdReq;
    grp_msc_func                        pfnCancel;
    grp_msc_func                        pfnAbort;
    grp_msc_func                        pfnReset;

    grp_msc_func                        pfnRun;
    grp_msc_func                        pfnDel;
}grp_msc_seq;

struct _grp_msc_set
{
    grp_msc_reg                         *ptMscReg;
    grp_u8                              ucSubClass;
    grp_u8                              ucInterfaceNum;
    grp_u8                              ucAlternateNum;
    grp_u8                              ucPadding;
    grp_u16                             usUsbDevId;
    grp_u16                             usPhase;
    grp_u32                             ulStatus;
    grp_usbdi_pipe                      atPipe[GRP_MSC_PIPE_MAX];
    grp_msc_pipes                       tPipes;
    grp_u8                              *pucConfigDesc;
    grp_u8                              *pucInterfaseDesc;
    grp_msc_cmd                         *ptCmd;
    grp_msc_seq                         *pfnProtocolSeq;

    grp_usbdi_st_device_request         tStdReq;
    grp_usbdi_device_request            tDevReq;
    grp_usbdi_request                   tNmlReq;

    /* The following buffer in addition to the NON CACHE domain. */
    grp_u8                              *pucCmd;
    grp_u8                              *pucSts;
};


/***** API FUNCTION PROTOTYPE *******************************************************************/
EXTERN grp_s32 grp_msc_Init( void *pvRsv );
EXTERN grp_s32 grp_msc_Register( grp_msc_reg *ptUsrReg );

EXTERN grp_s32 grp_msc_Open( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_Close( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_ReqCmd( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_Cancel( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_Abort( grp_msc_set *ptMscSet );
EXTERN grp_s32 grp_msc_Reset( grp_msc_cmd *ptCmd, grp_u32 ulMode );
EXTERN grp_s32 grp_msc_GetMaxLun( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_GetMaxLunCancel( grp_msc_cmd *ptCmd );

EXTERN grp_s32 grp_msc_GetDeviceId( grp_msc_hdr hMscHdr, grp_u16 *usDevId );
EXTERN grp_s32 grp_msc_GetSubClass( grp_msc_hdr hMschdr, grp_u8 *ucSubClass );

EXTERN grp_s32 grp_msc_ReadSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize );
EXTERN grp_s32 grp_msc_WriteSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize );
EXTERN grp_s32 grp_msc_Inquiry( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_ReadFormatCapacity( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_ReadCapacity( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_ModeSense( grp_msc_cmd *ptCmd, grp_u8 ucPage );
EXTERN grp_s32 grp_msc_TestUnitReady( grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_RequestSense( grp_msc_cmd *ptCmd );

/***** CLASS INTERNAL PROTOTYPE *****************************************************************/
EXTERN grp_s32 grp_msc_ClearPipe( grp_msc_cmd *ptCmd, grp_u8 ucEpNum );
EXTERN grp_s32 grp_msc_ClearAllPipes( grp_msc_pipes *ptPipes );
EXTERN grp_s32 grp_msc_LinkCmd( grp_msc_set *ptMscSet, grp_msc_cmd *ptNewCmd );
EXTERN grp_s32 grp_msc_UnlinkCmd( grp_msc_set *ptMscSet, grp_msc_cmd *ptCmd );
EXTERN grp_s32 grp_msc_Lock_Sem( void );
EXTERN grp_s32 grp_msc_Unlock_Sem( void );

#endif /* _GRP_MSC_DRV_H_ */

/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2015 Grape Systems, Inc.                          */
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
/*      grp_fsif_local.h                                                        1.03            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      This file composes the File System Interface between a file system                      */
/*      and GR-USB/HOST# Mass storage class modules.                                            */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2006/11/27  V0.01                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2012/01/12  V1.02                                                           */
/*                            - The definition of struct _grp_fsif_list_tag is corrected.       */
/*   K.Kaneko       2015/12/07  V1.03                                                           */
/*                            - Added a member(tMscCmd) to the _grp_fsif_unit_info structure.   */
/*                                                                                              */
/************************************************************************************************/

#ifndef _GRP_FSIF_LOCAL_H_
#define _GRP_FSIF_LOCAL_H_

/**** INCLUDE FILES *****************************************************************************/
#include    "grp_fsif_config.h"
#include    "grp_fsif_debug.h"

/**** INTERNAL DATA DEFINES *********************************************************************/

/* Use/Unuse flag */
#define GRP_FSIF_NO_USE                 ((grp_s32)0x00000000)
#define GRP_FSIF_USED                   ((grp_s32)0x00000001)

/* Unit status */
#define GRP_FSIF_US_NOMEDIA             ((grp_u8)0x00)
#define GRP_FSIF_US_ATTACHED            ((grp_u8)0x80)      /* bit7 is indicate media attached  */
#define GRP_FSIF_US_WR_PROTECT          ((grp_u8)0x01)
#define GRP_FSIF_US_UNKNOWN             ((grp_u8)0x02)

/* Notify status */
#define GRP_FSIF_NS_DISCONNECT          ((grp_u8)0x00)      /* Notify disconnect to application */
#define GRP_FSIF_NS_CONNECT             ((grp_u8)0x01)      /* Notigy connect to application    */

/* Removable media information */
#define GRP_FSIF_NOT_RMB                ((grp_u8)0x00)
#define GRP_FSIF_RMB                    ((grp_u8)0x01)

/* Data size */
#define GRP_FSIF_INQUIRY_DATA_SZ        ((grp_u32)36)   /* ((grp_u32)36) */
#define GRP_FSIF_READ_CAPACITY_DATA_SZ  ((grp_u32)8)
#define GRP_FSIF_MODE_SENSE_DATA_SZ     ((grp_u32)36)   /* ((grp_u32)48) */
#define GRP_FSIF_REQUEST_SENSE_DATA_SZ  ((grp_u32)18)

/* Multi-Media flag */
#define GRP_FSIF_MM_FLAG                ((grp_u8)0x05)      /* Peripheral Qualifier   = 000b    */
                                                            /* Peripheral Device Type = 00101b  */

/* Peripheral device type */
#define GRP_FSIF_PDT_NORMAL             ((grp_u8)0x00)
#define GRP_FSIF_PDT_MUILT              ((grp_u8)0x01)
#define GRP_FSIF_PDT_OTHER              ((grp_u8)0x02)

/* priority flag's information */
#define GRP_FSIF_PF_ALREADY_USE         ((grp_u8)0x01)
#define GRP_FSIF_PF_NO_USE              ((grp_u8)0x00)

/* Write protect bit (MODE SENSE) */
#define GRP_FSIF_MS_WP_BIT              ((grp_u8)0x80)

/* Removable medium bit (INQUIRY) */
#define GRP_FSIF_INQ_RM_BIT             ((grp_u8)0x80)

/* Pege code for MODE SENSE command */
#define GRP_FSIF_MS_IEC_PC              ((grp_u8)0x1C)
#define GRP_FSIF_MS_ALL_PC              ((grp_u8)0x3F)


/* Sense key */
#define GRP_FSIF_NO_SENSE               ((grp_u8)0x00)
#define GRP_FSIF_RECOVERED_ERROR        ((grp_u8)0x01)
#define GRP_FSIF_NOT_READY              ((grp_u8)0x02)
#define GRP_FSIF_MEDIUM_ERROR           ((grp_u8)0x03)
#define GRP_FSIF_HARDWARE_ERROR         ((grp_u8)0x04)
#define GRP_FSIF_ILLEGAL_REQUEST        ((grp_u8)0x05)
#define GRP_FSIF_UNIT_ATTANTION         ((grp_u8)0x06)
#define GRP_FSIF_DATA_PROTECT           ((grp_u8)0x07)

/* ASC */
#define GRP_FSIF_ASC_NOT_READY          ((grp_u8)0x04)
#define GRP_FSIF_ASC_NOT_PRESENT        ((grp_u8)0x3A)
#define GRP_FSIF_ASC_MEDIA_CHANGE       ((grp_u8)0x28)

/* ASCQ */
#define GRP_FSIF_ASCQ_BECOMING_READY    ((grp_u8)0x01)
#define GRP_FSIF_ASCQ_MEDIA_CHANGE      ((grp_u8)0x00)


/* Error code */
#define GRP_FSIF_EC_NO_ERR              ((grp_s32)( 0))
#define GRP_FSIF_EC_NOT_RDY             ((grp_s32)(-1))
#define GRP_FSIF_EC_HW_ERR              ((grp_s32)(-2))
#define GRP_FSIF_EC_ILLEGL_REQ          ((grp_s32)(-3))
#define GRP_FSIF_EC_OTHER_ERR           ((grp_s32)(-4))
#define GRP_FSIF_EC_WR_PROTECT          ((grp_s32)(-5))

/* several index number */
#define GRP_FSIF_BOT_IDX                ((grp_u32)0)
#define GRP_FSIF_CBI_IDX                ((grp_u32)1)
#define GRP_FSIF_CB_IDX                 ((grp_u32)2)
#define GRP_FSIF_SCSI_IDX               ((grp_u32)0)
#define GRP_FSIF_ATAPI_IDX              ((grp_u32)1)
#define GRP_FSIF_SFF8070I_IDX           ((grp_u32)2)
#define GRP_FSIF_UFI_IDX                ((grp_u32)3)

#define GRP_FSIF_MAX_CLASS              ((grp_u32)3)
#define GRP_FSIF_MAX_SUBCLASS           ((grp_u32)4)


typedef struct _grp_fsif_list_tag   _grp_fsif_list;

struct _grp_fsif_list_tag                                                           /* V1.02    */
{
    _grp_fsif_list              *ptPrev;
    _grp_fsif_list              *ptNext;
    
};


typedef struct _grp_fsif_unit_info_tag
{
    _grp_fsif_list              *ptPrev;
    _grp_fsif_list              *ptNext;
    grp_u8                      ucUnitStatus;               /* 0x00: no media                   */
                                                            /* 0x01: media is exist             */
                                                            /* 0x03: write protect              */
                                                            /* 0x04: unknown                    */
    grp_u8                      ucNotifyStat;               /* 0x00: disconnected               */
                                                            /* 0x01: connected                  */
    grp_u8                      ucRmb;                      /* 0x00: medhia is not removable    */
                                                            /* 0x01: medhia is removable        */
    grp_u8                      ucMaxLun;
    grp_u8                      ucOwnLun;
    grp_u8                      ucPeriDevType;
    grp_u8                      ucPriorityFlag;
    grp_u8                      ucIndex;
    grp_u32                     ulSectorSize;
    grp_u32                     ulSectorNum;
    grp_msc_hdr                 ptMscHdr;
    grp_msc_cmd                 tMscCmd;
    grp_u8                      *pucCmemArea;
#ifndef GRP_FSIF_LOW_MEMORY
    grp_u8                      aucInquiryData[GRP_FSIF_INQUIRY_DATA_SZ+4];
#endif  /* GRP_FSIF_LOW_MEMORY */
} _grp_fsif_unit_info;


typedef struct _grp_fsif_notice_tag
{
    _grp_fsif_list              *ptPrev;
    _grp_fsif_list              *ptNext;
    grp_s32                     lFlag;
    grp_msc_notify              tMscNotify;

} _grp_fsif_notice;


typedef struct _grp_fsif_local_vos_table_tag
{
    grp_vos_t_flag              *ptNoticeFlg;

    grp_vos_t_semaphore         *ptNoticeSem;
    grp_vos_t_semaphore         *ptListSem;
    grp_vos_t_semaphore         *ptLockTaskSem;
    grp_vos_t_semaphore         *ptUnitInfoSem;

    grp_vos_t_semaphore         *ptBufSem;

    grp_vos_t_task              *ptMainTsk;
    grp_vos_t_task              *ptPollTsk;

    grp_vos_t_queue             *ptMainTskQue;
    grp_vos_t_queue             *ptPollTskQue;

} _grp_fsif_local_vos_table;


typedef struct _grp_fsif_access_queue_tag
{
    grp_s32                     lFlag;
    grp_vos_t_queue             *ptAccessQue;

} _grp_fsif_access_queue;

typedef struct _grp_fsif_access_mng_tag
{
    grp_vos_t_semaphore         *ptAccessCntSem;
    _grp_fsif_access_queue      tAccessQue[GRP_FSIF_MAX_ACCESS];

} _grp_fsif_access_mng;


/* VOS */
#define GRP_FSIF_MAIN_TASK_PRI          GRP_VOS_PRI_NORMAL
#define GRP_FSIF_POLL_TASK_PRI          GRP_VOS_PRI_NORMAL
#define GRP_FSIF_MAIN_QUE_SIZE          ((grp_u32)4)
#define GRP_FSIF_MAIN_QUE_CNT           ((grp_u32)1)
#define GRP_FSIF_POLL_QUE_SIZE          GRP_FSIF_MAIN_QUE_SIZE
#define GRP_FSIF_POLL_QUE_CNT           GRP_FSIF_MAIN_QUE_CNT
#define GRP_FSIF_ACCESS_QUE_SIZE        GRP_FSIF_MAIN_QUE_SIZE
#define GRP_FSIF_ACCESS_QUE_CNT         GRP_FSIF_MAX_ACCESS


#define GRP_FSIF_EVENT                  ((grp_u32)1)
#define GRP_FSIF_EVENT_CLEAR            ((grp_u32)0)
#define GRP_FSIF_RCV                    ((grp_u32)1)

#endif  /* _GRP_FSIF_LOCAL_H_ */


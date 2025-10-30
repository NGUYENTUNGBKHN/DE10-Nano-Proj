/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2015 Grape Systems, Inc.                        */
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
/*      grp_cyclonev_cfg.h                                                      1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV user configuration file                   */
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
#ifndef _GRP_CYCLONEV_CFG_H_
#define _GRP_CYCLONEV_CFG_H_

/**** INCLUDE FILES *****************************************************************************/


/**** EXTERNAL DATA DEFINES *********************************************************************/
/* USB OTG core */
//#define GRP_CYCLONEV_USB0_OTG               (0)                             /* USB0 */
//#define GRP_CYCLONEV_USB1_OTG               (1)                             /* USB1 */

/* Operation mode */
#define GRP_CYCLONEV_MODE_DEVICE            (0)                             /* Device mode  */
#define GRP_CYCLONEV_MODE_HOST              (1)                             /* Host mode    */

/* OTG FIFO access */
#define GRP_CYCLONEV_FIFO_PIO               (0)                             /* FIFO access PIO  */
#define GRP_CYCLONEV_FIFO_DMA               (1)                             /* FIFO access DMA  */

/* System data cache type */
#define GRP_CYCLONEV_DCACHE_INVALID         (0)                             /* Data cache invalid               */
#define GRP_CYCLONEV_DCACHE_WRITE_THROUGH   (1)                             /* Data cache valid (write-through) */

/************************************************************************************************/
/* Select use USB OTG core                                                                      */
/************************************************************************************************/
////#define GRP_CYCLONEV_USB_OTG_CORE           GRP_CYCLONEV_USB1_OTG
//#define GRP_CYCLONEV_USB_OTG_CORE           GRP_CYCLONEV_USB0_OTG

/************************************************************************************************/
/* Select use OTG FIFO access mode                                                              */
/************************************************************************************************/
#define GRP_CYCLONEV_FIFO_ACCESS_MODE       GRP_CYCLONEV_FIFO_DMA

/************************************************************************************************/
/* Select use system data cache type                                                            */
/************************************************************************************************/
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
 #define GRP_CYCLONEV_USE_DCACHE_TYPE       GRP_CYCLONEV_DCACHE_INVALID
 //#define GRP_CYCLONEV_USE_DCACHE_TYPE       GRP_CYCLONEV_DCACHE_WRITE_THROUGH
#endif

#endif /* _GRP_CYCLONEV_CFG_H_ */

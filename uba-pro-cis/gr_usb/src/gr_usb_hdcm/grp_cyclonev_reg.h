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
/*      grp_cyclonev_reg.h                                                      1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV OTG register address definition file      */
/*       (for CycloneV internal controller)                                                     */
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
#ifndef _GRP_CYCLONEV_OTG_REG_H_
#define _GRP_CYCLONEV_OTG_REG_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_cyclonev_cfg.h"

/************************************************************************************************/
/*  Register address definitions                                                                */
/************************************************************************************************/
/*----------------------------------------------------------------------------------------------*/
/*  USB on-the-go high-speed (OTG)                                                              */
/*----------------------------------------------------------------------------------------------*/
/*** USB OTG register base address ***/
#define CYCLONEV_USB0_OTG_BASE_ADDR      (0xFFB00000)
#define CYCLONEV_USB1_OTG_BASE_ADDR      (0xFFB40000)

/*** Register address acquisition macro ***/
//#if (GRP_CYCLONEV_USB_OTG_CORE == GRP_CYCLONEV_USB0_OTG)
//    #define CYCLONEV_OTG_GET_ADDR(offset)   (CYCLONEV_USB0_OTG_BASE_ADDR + (offset))
//#else
//    #define CYCLONEV_OTG_GET_ADDR(offset)   (CYCLONEV_USB1_OTG_BASE_ADDR + (offset))
//#endif
//#ifdef SWITCH_USB0_USB1
//#define CYCLONEV_OTG_GET_ADDR(offset)    (CYCLONEV_USB1_OTG_BASE_ADDR + (offset))
//#define CYCLONEV_OTG_GET_ADDR2(offset)   (CYCLONEV_USB0_OTG_BASE_ADDR + (offset))
//#else
#define CYCLONEV_OTG_GET_ADDR(offset)    (CYCLONEV_USB0_OTG_BASE_ADDR + (offset))
#define CYCLONEV_OTG_GET_ADDR2(offset)   (CYCLONEV_USB1_OTG_BASE_ADDR + (offset))
//#endif

/*----------------------------------------------------------------------------------------------*/
/*  Core global control and status registers                                                    */
/*----------------------------------------------------------------------------------------------*/
/* OTG control and status register (GOTGCTL) */
#define CYCLONEV_O32_OTG_GOTGCTL        0x0000
#define CYCLONEV_A32_OTG_GOTGCTL        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GOTGCTL)
#define CYCLONEV_A32_OTG_GOTGCTL2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GOTGCTL)

/* OTG interrupt register (GOTGINT) */
#define CYCLONEV_O32_OTG_GOTGINT        0x0004
#define CYCLONEV_A32_OTG_GOTGINT        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GOTGINT)
#define CYCLONEV_A32_OTG_GOTGINT2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GOTGINT)

/* OTG AHB configuration register (GAHBCFG) */
#define CYCLONEV_O32_OTG_GAHBCFG        0x0008
#define CYCLONEV_A32_OTG_GAHBCFG        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GAHBCFG)
#define CYCLONEV_A32_OTG_GAHBCFG2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GAHBCFG)

/* OTG USB configuration register (GUSBCFG) */
#define CYCLONEV_O32_OTG_GUSBCFG        0x000C
#define CYCLONEV_A32_OTG_GUSBCFG        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GUSBCFG)
#define CYCLONEV_A32_OTG_GUSBCFG2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GUSBCFG)

/* OTG reset register (GRSTCTL) */
#define CYCLONEV_O32_OTG_GRSTCTL        0x0010
#define CYCLONEV_A32_OTG_GRSTCTL        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GRSTCTL)
#define CYCLONEV_A32_OTG_GRSTCTL2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GRSTCTL)

/* OTG core interrupt register (GINTSTS) */
#define CYCLONEV_O32_OTG_GINTSTS        0x0014
#define CYCLONEV_A32_OTG_GINTSTS        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GINTSTS)
#define CYCLONEV_A32_OTG_GINTSTS2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GINTSTS)

/* OTG interrupt mask register (GINTMSK) */
#define CYCLONEV_O32_OTG_GINTMSK        0x0018
#define CYCLONEV_A32_OTG_GINTMSK        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GINTMSK)
#define CYCLONEV_A32_OTG_GINTMSK2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GINTMSK)

/* OTG receive status debug read register (GRXSTSR) */
#define CYCLONEV_O32_OTG_GRXSTSR        0x001C
#define CYCLONEV_A32_OTG_GRXSTSR        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GRXSTSR)
#define CYCLONEV_A32_OTG_GRXSTSR2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GRXSTSR)

/* OTG status read and pop register (GRXSTSP) */
#define CYCLONEV_O32_OTG_GRXSTSP        0x0020
#define CYCLONEV_A32_OTG_GRXSTSP        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GRXSTSP)
#define CYCLONEV_A32_OTG_GRXSTSP2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GRXSTSP)

/* OTG Receive FIFO size register (GRXFSIZ) */
#define CYCLONEV_O32_OTG_GRXFSIZ        0x0024
#define CYCLONEV_A32_OTG_GRXFSIZ        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GRXFSIZ)
#define CYCLONEV_A32_OTG_GRXFSIZ2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GRXFSIZ)

/* OTG non-periodic transmit FIFO size register (GNPTXFSIZ) */
#define CYCLONEV_O32_OTG_GNPTXFSIZ      0x0028
#define CYCLONEV_A32_OTG_GNPTXFSIZ      CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GNPTXFSIZ)
#define CYCLONEV_A32_OTG_GNPTXFSIZ2     CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GNPTXFSIZ)

/* OTG non-periodic transmit FIFO/queue status register (GNPTXSTS) */
#define CYCLONEV_O32_OTG_GNPTXSTS       0x002C
#define CYCLONEV_A32_OTG_GNPTXSTS       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GNPTXSTS)
#define CYCLONEV_A32_OTG_GNPTXSTS2      CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GNPTXSTS)

/* OTG PHY vendor control register (GPVNDCTL) */
#define CYCLONEV_O32_OTG_GPVNDCTL       0x0034
#define CYCLONEV_A32_OTG_GPVNDCTL       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GPVNDCTL)
#define CYCLONEV_A32_OTG_GPVNDCTL2      CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GPVNDCTL)

/* OTG general purpose input output register (GGPIO) */
#define CYCLONEV_O32_OTG_GGPIO          0x0038
#define CYCLONEV_A32_OTG_GGPIO          CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GGPIO)
#define CYCLONEV_A32_OTG_GGPIO2         CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GGPIO)

/* OTG user ID register (GUID) */
#define CYCLONEV_O32_OTG_GUID           0x003C
#define CYCLONEV_A32_OTG_GUID           CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GUID)
#define CYCLONEV_A32_OTG_GUID2          CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GUID)

/* OTG synopsys ID register (GSNPSID) */
#define CYCLONEV_O32_OTG_GSNPSID        0x0040
#define CYCLONEV_A32_OTG_GSNPSID        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GSNPSID)
#define CYCLONEV_A32_OTG_GSNPSID2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GSNPSID)

/* OTG user HW config1 register (GHWCFG1) */
#define CYCLONEV_O32_OTG_GHWCFG1        0x0044
#define CYCLONEV_A32_OTG_GHWCFG1        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GHWCFG1)
#define CYCLONEV_A32_OTG_GHWCFG12       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GHWCFG1)

/* OTG user HW config2 register (GHWCFG2) */
#define CYCLONEV_O32_OTG_GHWCFG2        0x0048
#define CYCLONEV_A32_OTG_GHWCFG2        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GHWCFG2)
#define CYCLONEV_A32_OTG_GHWCFG22       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GHWCFG2)

/* OTG user HW config3 register (GHWCFG3) */
#define CYCLONEV_O32_OTG_GHWCFG3        0x004C
#define CYCLONEV_A32_OTG_GHWCFG3        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GHWCFG3)
#define CYCLONEV_A32_OTG_GHWCFG32       CYCLONEV_OTG_GET_ADDR2CYCLONEV_O32_OTG_GHWCFG3)

/* OTG user HW config4 register (GHWCFG4) */
#define CYCLONEV_O32_OTG_GHWCFG4        0x0050
#define CYCLONEV_A32_OTG_GHWCFG4        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GHWCFG4)
#define CYCLONEV_A32_OTG_GHWCFG42       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GHWCFG4)

/* OTG DFIFO software config register (GDFIFOCFG) */
#define CYCLONEV_O32_OTG_GDFIFOCFG      0x005C
#define CYCLONEV_A32_OTG_GDFIFOCFG      CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_GDFIFOCFG)
#define CYCLONEV_A32_OTG_GDFIFOCFG2     CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_GDFIFOCFG)

/* OTG host periodic transmit FIFO size register (HPTXFSIZ) */
#define CYCLONEV_O32_OTG_HPTXFSIZ       0x0100
#define CYCLONEV_A32_OTG_HPTXFSIZ       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HPTXFSIZ)
#define CYCLONEV_A32_OTG_HPTXFSIZ2      CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_HPTXFSIZ)

/* OTG device IN endpoint transmit FIFO size register (DIEPTXFx) (x = 1..15, where x = FIFO_number) */
#define CYCLONEV_O32_OTG_DIEPTXFx       0x0104
#define CYCLONEV_A32_OTG_DIEPTXF(x)     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DIEPTXFx + (((x) - 1) * 0x04))
#define CYCLONEV_A32_OTG_DIEPTXF2(x)    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DIEPTXFx + (((x) - 1) * 0x04))

/*----------------------------------------------------------------------------------------------*/
/*  Host-mode control and status registers                                                      */
/*----------------------------------------------------------------------------------------------*/
/* OTG Host Configuration Register (HCFG) */
#define CYCLONEV_O32_OTG_HCFG           0x0400
#define CYCLONEV_A32_OTG_HCFG           CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HCFG)

/* OTG Host Frame Interval Register (HFIR) */
#define CYCLONEV_O32_OTG_HFIR           0x0404
#define CYCLONEV_A32_OTG_HFIR           CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HFIR)

/* OTG Host Frame Number Frame Time Remaining Register (HFNUM) */
#define CYCLONEV_O32_OTG_HFNUM          0x0408
#define CYCLONEV_A32_OTG_HFNUM          CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HFNUM)

/* OTG Host Periodic Transmit FIFO Queue Status Register (HPTXSTS) */
#define CYCLONEV_O32_OTG_HPTXSTS        0x0410
#define CYCLONEV_A32_OTG_HPTXSTS        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HPTXSTS)

/* OTG Host All Channels Interrupt Register (HAINT) */
#define CYCLONEV_O32_OTG_HAINT          0x0414
#define CYCLONEV_A32_OTG_HAINT          CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HAINT)

/* OTG Host All Channels Interrupt Mask Register (HAINTMSK) */
#define CYCLONEV_O32_OTG_HAINTMSK       0x0418
#define CYCLONEV_A32_OTG_HAINTMSK       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HAINTMSK)

/* OTG Host Frame List Base Address Register (HFLBADDR) */
#define CYCLONEV_O32_OTG_HFLBADDR       0x041C
#define CYCLONEV_A32_OTG_HFLBADDR       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HFLBADDR)

/* OTG Host Port Control and Status Register (HPRT) */
#define CYCLONEV_O32_OTG_HPRT           0x0440
#define CYCLONEV_A32_OTG_HPRT           CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HPRT)

/* OTG Host Channel x Characteristics Register (HCCHARx) (x = 0..15, where x = Channel number) */
#define CYCLONEV_O32_OTG_HCCHARx        0x0500
#define CYCLONEV_A32_OTG_HCCHAR(x)      CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HCCHARx + ((x) * 0x20))

/* OTG Host Channel x Split Control Register (HCSPLTx) (x = 0..15, where x = Channel number) */
#define CYCLONEV_O32_OTG_HCSPLTx        0x0504
#define CYCLONEV_A32_OTG_HCSPLT(x)      CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HCSPLTx + ((x) * 0x20))

/* OTG Host Channel x Interrupt Register (HCINTx) (x = 0..15, where x = Channel number) */
#define CYCLONEV_O32_OTG_HCINTx         0x0508
#define CYCLONEV_A32_OTG_HCINT(x)       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HCINTx + ((x) * 0x20))

/* OTG Host Channel x Interrupt Mask Register (HCINTMSKx) (x = 0..15, where x = Channel number) */
#define CYCLONEV_O32_OTG_HCINTMSKx      0x050C
#define CYCLONEV_A32_OTG_HCINTMSK(x)    CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HCINTMSKx + ((x) * 0x20))

/* OTG Host Channel x Transfer Size Register (HCTSIZx) (x = 0..15, where x = Channel number) */
#define CYCLONEV_O32_OTG_HCTSIZx        0x0510
#define CYCLONEV_A32_OTG_HCTSIZ(x)      CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HCTSIZx + ((x) * 0x20))

/* OTG Host Channel 0 DMA Address Register (HCDMAx) (x = 0..15, where x = Channel number) */
#define CYCLONEV_O32_OTG_HCDMAx         0x0514
#define CYCLONEV_A32_OTG_HCDMA(x)       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HCDMAx + ((x) * 0x20))

/* OTG Host Channel 0 DMA Buffer Address Register (HCDMABx) (x = 0..15, where x = Channel number) */
#define CYCLONEV_O32_OTG_HCDMABx        0x0518
#define CYCLONEV_A32_OTG_HCDMAB(x)      CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_HCDMABx + ((x) * 0x20))

/*----------------------------------------------------------------------------------------------*/
/*  Device-mode control and status registers                                                    */
/*----------------------------------------------------------------------------------------------*/
/* OTG device configuration register (DCFG) */
#define CYCLONEV_O32_OTG_DCFG           0x0800
#define CYCLONEV_A32_OTG_DCFG           CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DCFG)
#define CYCLONEV_A32_OTG_DCFG2          CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DCFG)

/* OTG device control register (DCTL) */
#define CYCLONEV_O32_OTG_DCTL           0x0804
#define CYCLONEV_A32_OTG_DCTL           CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DCTL)
#define CYCLONEV_A32_OTG_DCTL2          CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DCTL)

/* OTG device status register (DSTS) */
#define CYCLONEV_O32_OTG_DSTS           0x0808
#define CYCLONEV_A32_OTG_DSTS           CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DSTS)
#define CYCLONEV_A32_OTG_DSTS2          CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DSTS)

/* OTG device IN endpoint common interrupt mask register (DIEPMSK) */
#define CYCLONEV_O32_OTG_DIEPMSK        0x0810
#define CYCLONEV_A32_OTG_DIEPMSK        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DIEPMSK)
#define CYCLONEV_A32_OTG_DIEPMSK2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DIEPMSK)

/* OTG device OUT endpoint common interrupt mask register (DOEPMSK) */
#define CYCLONEV_O32_OTG_DOEPMSK        0x0814
#define CYCLONEV_A32_OTG_DOEPMSK        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DOEPMSK)
#define CYCLONEV_A32_OTG_DOEPMSK2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DOEPMSK)

/* OTG device all endpoints interrupt register (DAINT) */
#define CYCLONEV_O32_OTG_DAINT          0x0818
#define CYCLONEV_A32_OTG_DAINT          CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DAINT)
#define CYCLONEV_A32_OTG_DAINT2         CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DAINT)

/* OTG device all endpoints interrupt mask register (DAINTMSK) */
#define CYCLONEV_O32_OTG_DAINTMSK       0x081C
#define CYCLONEV_A32_OTG_DAINTMSK       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DAINTMSK)
#define CYCLONEV_A32_OTG_DAINTMSK2      CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DAINTMSK)

/* OTG device VBUS discharge time register (DVBUSDIS) */
#define CYCLONEV_O32_OTG_DVBUSDIS       0x0828
#define CYCLONEV_A32_OTG_DVBUSDIS       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DVBUSDIS)
#define CYCLONEV_A32_OTG_DVBUSDIS2      CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DVBUSDIS)

/* OTG device VBUS pulsing time register (DVBUSPULSE) */
#define CYCLONEV_O32_OTG_DVBUSPULSE     0x082C
#define CYCLONEV_A32_OTG_DVBUSPULSE     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DVBUSPULSE)
#define CYCLONEV_A32_OTG_DVBUSPULSE2    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DVBUSPULSE)

/* OTG device threshold control register (DTHRCTL) */
#define CYCLONEV_O32_OTG_DTHRCTL        0x0830
#define CYCLONEV_A32_OTG_DTHRCTL       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DTHRCTL)
#define CYCLONEV_A32_OTG_DTHRCTL2      CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DTHRCTL)

/* OTG device IN endpoint FIFO empty interrupt mask register (DIEPEMPMSK) */
#define CYCLONEV_O32_OTG_DIEPEMPMSK     0x0834
#define CYCLONEV_A32_OTG_DIEPEMPMSK     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DIEPEMPMSK)
#define CYCLONEV_A32_OTG_DIEPEMPMSK2    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DIEPEMPMSK)

/* OTG device IN endpoint-x control register (DIEPCTLx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DIEPCTLx       0x0900
#define CYCLONEV_A32_OTG_DIEPCTL(x)     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DIEPCTLx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DIEPCTL2(x)    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DIEPCTLx + ((x) * 0x20))

/* OTG device IN endpoint-x interrupt register (DIEPINTx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DIEPINTx       0x0908
#define CYCLONEV_A32_OTG_DIEPINT(x)     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DIEPINTx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DIEPINT2(x)    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DIEPINTx + ((x) * 0x20))

/* OTG device IN endpoint-x transfer size register (DIEPTSIZx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DIEPTSIZx      0x0910
#define CYCLONEV_A32_OTG_DIEPTSIZ(x)    CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DIEPTSIZx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DIEPTSIZ2(x)   CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DIEPTSIZx + ((x) * 0x20))

/* OTG device IN endpoint-x DMA address register (DIEPDMAx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DIEPDMAx       0x0914
#define CYCLONEV_A32_OTG_DIEPDMA(x)     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DIEPDMAx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DIEPDMA2(x)    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DIEPDMAx + ((x) * 0x20))

/* OTG device IN endpoint-x transmit FIFO status register (DTXFSTSx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DTXFSTSx       0x0918
#define CYCLONEV_A32_OTG_DTXFSTS(x)     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DTXFSTSx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DTXFSTS2(x)    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DTXFSTSx + ((x) * 0x20))

/* OTG device IN endpoint-x DMA buffer address register (DIEPDMABx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DIEPDMABx      0x091C
#define CYCLONEV_A32_OTG_DIEPDMAB(x)    CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DIEPDMABx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DIEPDMAB2(x)   CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DIEPDMABx + ((x) * 0x20))

/* OTG device OUT endpoint-x control register (DOEPCTLx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DOEPCTLx       0x0B00
#define CYCLONEV_A32_OTG_DOEPCTL(x)     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DOEPCTLx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DOEPCTL2(x)    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DOEPCTLx + ((x) * 0x20))

/* OTG device OUT endpoint-x interrupt register (DOEPINTx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DOEPINTx       0x0B08
#define CYCLONEV_A32_OTG_DOEPINT(x)     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DOEPINTx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DOEPINT2(x)    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DOEPINTx + ((x) * 0x20))

/* OTG device OUT endpoint-x transfer size register (DOEPTSIZx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DOEPTSIZx      0x0B10
#define CYCLONEV_A32_OTG_DOEPTSIZ(x)    CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DOEPTSIZx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DOEPTSIZ2(x)   CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DOEPTSIZx + ((x) * 0x20))

/* OTG device OUT endpoint-x DMA address register (DOEPDMAx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DOEPDMAx       0x0B14
#define CYCLONEV_A32_OTG_DOEPDMA(x)     CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DOEPDMAx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DOEPDMA2(x)    CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DOEPDMAx + ((x) * 0x20))

/* OTG device OUT endpoint-x DMA buffer address register (DOEPDMABx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEV_O32_OTG_DOEPDMABx      0x0B1C
#define CYCLONEV_A32_OTG_DOEPDMAB(x)    CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DOEPDMABx + ((x) * 0x20))
#define CYCLONEV_A32_OTG_DOEPDMAB2(x)   CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DOEPDMABx + ((x) * 0x20))

/*----------------------------------------------------------------------------------------------*/
/*  Power and clock gating control and status registers                                         */
/*----------------------------------------------------------------------------------------------*/
/* OTG power and clock gating control register (PCGCCTL) */
#define CYCLONEV_O32_OTG_PCGCCTL        0x0E00
#define CYCLONEV_A32_OTG_PCGCCTL        CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_PCGCCTL)
#define CYCLONEV_A32_OTG_PCGCCTL2       CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_PCGCCTL)

/*----------------------------------------------------------------------------------------------*/
/*  Data FIFO (DFIFO) access register                                                           */
/*----------------------------------------------------------------------------------------------*/
/* x = 0..15, where x = Endpoint_number */
#define CYCLONEV_O32_OTG_DFIFOx         0x1000
#define CYCLONEV_A32_OTG_DFIFO(x)       CYCLONEV_OTG_GET_ADDR(CYCLONEV_O32_OTG_DFIFOx + ((x) * 0x1000))
#define CYCLONEV_A32_OTG_DFIFO2(x)      CYCLONEV_OTG_GET_ADDR2(CYCLONEV_O32_OTG_DFIFOx + ((x) * 0x1000))


#endif  /* _GRP_CYCLONEV_OTG_REG_H_ */

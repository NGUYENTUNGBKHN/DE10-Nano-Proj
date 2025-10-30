/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2019 Grape Systems, Inc.                        */
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
/*      grp_cyclonev_bit_val.h                                                  1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV OTG register bit/value                    */
/*      definition file                                                                         */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Kobayashi    2014/12/24  V0.90                                                           */
/*                            - Created beta release version.                                   */
/*   T.Kobayashi    2015/01/21  V1.00                                                           */
/*                            - 1st release version.                                            */
/*   K.Kaneko       2019/04/26  V1.01                                                           */
/*                            - Added definition on host.                                       */
/*                                                                                              */
/************************************************************************************************/
#ifndef _GRP_CYCLONEV_BIT_VAL_H_
#define _GRP_CYCLONEV_BIT_VAL_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grp_cyclonev_types.h"


/************************************************************************************************/
/*  Register bit/value definitions                                                              */
/************************************************************************************************/
/*----------------------------------------------------------------------------------------------*/
/*  Core global control and status registers                                                    */
/*----------------------------------------------------------------------------------------------*/
/* OTG control and status register (GOTGCTL) */
#define CYCLONEVG_B01_OTGVER            ((grp_u32)0x00100000)           /* OTG version                                  */
#define CYCLONEVG_B01_BSESVLD           ((grp_u32)0x00080000)           /* B-session valid                              */
#define CYCLONEVG_B01_ASESVLD           ((grp_u32)0x00040000)           /* A-session valid                              */
#define CYCLONEVG_B01_DBNCTIME          ((grp_u32)0x00020000)           /* Long/short debounce time                     */
#define CYCLONEVG_B01_CONIDSTS          ((grp_u32)0x00010000)           /* Connector ID status                          */
#define CYCLONEVG_B01_DEVHNPEN          ((grp_u32)0x00000800)           /* Device HNP enabled                           */
#define CYCLONEVG_B01_HSTSETHNPEN       ((grp_u32)0x00000400)           /* Host set HNP enable                          */
#define CYCLONEVG_B01_HNPREQ            ((grp_u32)0x00000200)           /* HNP request                                  */
#define CYCLONEVG_B01_HSTNEGSCS         ((grp_u32)0x00000100)           /* Host negotiation success                     */
#define CYCLONEVG_B01_BVALIDOVVAL       ((grp_u32)0x00000080)           /* B-peripheral session valid override value    */
#define CYCLONEVG_B01_BVALIDOVEN        ((grp_u32)0x00000040)           /* B-peripheral session valid override enable   */
#define CYCLONEVG_B01_AVALIDOVVAL       ((grp_u32)0x00000020)           /* A-peripheral session valid override value    */
#define CYCLONEVG_B01_AVALIDOVEN        ((grp_u32)0x00000010)           /* A-peripheral session valid override enable   */
#define CYCLONEVG_B01_VBVALIDOVVAL      ((grp_u32)0x00000008)           /* VBUS valid override value                    */
#define CYCLONEVG_B01_VBVALIDOVEN       ((grp_u32)0x00000004)           /* VBUS valid override enable                   */
#define CYCLONEVG_B01_SESREQ            ((grp_u32)0x00000002)           /* Session request                              */
#define CYCLONEVG_B01_SESREQSCS         ((grp_u32)0x00000001)           /* Session request success                      */

/* OTG interrupt register (GOTGINT) */
#define CYCLONEVG_B01_DBNCEDONE         ((grp_u32)0x00080000)           /* Debounce done                            */
#define CYCLONEVG_B01_ADEVTOUTCHG       ((grp_u32)0x00040000)           /* A-device timeout change                  */
#define CYCLONEVG_B01_HSTNEGDET         ((grp_u32)0x00020000)           /* Host negotiation detected                */
#define CYCLONEVG_B01_HSTNEGSUCSTSCHNG  ((grp_u32)0x00000200)           /* Host negotiation success status change   */
#define CYCLONEVG_B01_SESREQSUCSTSCHNG  ((grp_u32)0x00000100)           /* Session request success status change    */
#define CYCLONEVG_B01_SESENDDET         ((grp_u32)0x00000004)           /* Session end detected                     */

/* OTG AHB configuration register (GAHBCFG) */
#define CYCLONEVG_B01_NOTIALLDMAWRIT    ((grp_u32)0x00400000)           /* Notify all DMA write transactions    */
#define CYCLONEVG_B01_REMMEMSUPP        ((grp_u32)0x00200000)           /* Remote memory support                */
#define CYCLONEVG_B01_PTXFEMPLVL        ((grp_u32)0x00000100)           /* Periodic TxFIFO empty level          */
#define CYCLONEVG_B01_NPTXFEMPLVL       ((grp_u32)0x00000080)           /* Non-periodic TxFIFO empty level      */
#define CYCLONEVG_B01_DMAEN             ((grp_u32)0x00000020)           /* DMA enable                           */
#define CYCLONEVG_B04_HBSTLEN           ((grp_u32)0x0000001E)           /* Burst length/type                    */
    #define CYCLONEVG_VHBSTLEN_WORD1ORSINGLE    ((grp_u32)0x00000000)           /* 0000: When the burst type is 1 word or Single    */
    #define CYCLONEVG_VHBSTLEN_WORD4ORINCR      ((grp_u32)0x00000002)           /* 0001: When the burst legth is 4 word or INCR     */
    #define CYCLONEVG_VHBSTLEN_WORD8            ((grp_u32)0x00000004)           /* 0010: When the burst legth is 8 word             */
    #define CYCLONEVG_VHBSTLEN_WORD16ORINCR4    ((grp_u32)0x00000006)           /* 0011: When the burst legth is 16 word or INCR4   */
    #define CYCLONEVG_VHBSTLEN_WORD32           ((grp_u32)0x00000008)           /* 0100: When the burst legth is 32 word            */
    #define CYCLONEVG_VHBSTLEN_WORD64ORINCR8    ((grp_u32)0x0000000A)           /* 0101: When the burst legth is 64 word or INCR8   */
    #define CYCLONEVG_VHBSTLEN_WORD128          ((grp_u32)0x0000000C)           /* 0110: When the burst legth is 128 word           */
    #define CYCLONEVG_VHBSTLEN_WORD256ORINCR16  ((grp_u32)0x0000000E)           /* 0111: When the burst legth is 256 word or INCR16 */
    #define CYCLONEVG_VHBSTLEN_WORDX            ((grp_u32)0x00000010)           /* 1000: When the burst legth is others reserved    */
#define CYCLONEVG_B01_GLBLINTRMSK       ((grp_u32)0x00000001)           /* Global interrupt mask                */

/* OTG USB configuration register (GUSBCFG) */
#define CYCLONEVG_B01_CORRUPTTXPKT      ((grp_u32)0x80000000)           /* Corrupt Tx packet                    */
#define CYCLONEVG_B01_FORCEDEVMODE      ((grp_u32)0x40000000)           /* Force device mode                    */
#define CYCLONEVG_B01_FORCEHSTMODE      ((grp_u32)0x20000000)           /* Force host mode                      */
#define CYCLONEVG_B01_TXENDDELAY        ((grp_u32)0x10000000)           /* Tx end delay                         */
#define CYCLONEVG_B01_ULPI              ((grp_u32)0x02000000)           /* ULPI interface protect disable       */
#define CYCLONEVG_B01_INDICATOR         ((grp_u32)0x01000000)           /* Indicator pass through               */
#define CYCLONEVG_B01_COMPLEMENT        ((grp_u32)0x00800000)           /* Indicator complement                 */
#define CYCLONEVG_B01_TERMSELDLPULSE    ((grp_u32)0x00400000)           /* TermSel DLine pulsing selection      */
#define CYCLONEVG_B01_ULPIEXTVBUSINDICATOR  ((grp_u32)0x00200000)       /* ULPI external VBUS indicator         */
#define CYCLONEVG_B01_ULPIEXTVBUSDRV    ((grp_u32)0x00100000)           /* ULPI external VBUS drive             */
#define CYCLONEVG_B01_ULPICLKSUSM       ((grp_u32)0x00080000)           /* ULPI clock suspendM                  */
#define CYCLONEVG_B01_ULPIAUTORES       ((grp_u32)0x00040000)           /* ULPI auto-resume                     */
#define CYCLONEVG_B04_USBTRDTIM         ((grp_u32)0x00003C00)           /* USB turnaround time                  */
    #define CYCLONEVG_VUSBTRDTIM_8BIT_UTMIP     ((grp_u32)0x00002400)           /* 1001: MAC interface is 8-bit UTMI+   */
#define CYCLONEVG_B01_HNPCAP            ((grp_u32)0x00000200)           /* HNP-capable                          */
#define CYCLONEVG_B01_SRPCAP            ((grp_u32)0x00000100)           /* SRP-capable                          */
#define CYCLONEVG_B01_DDRSEL            ((grp_u32)0x00000080)           /* ULPI DDR select                      */
#define CYCLONEVG_B01_PHYSEL            ((grp_u32)0x00000040)           /* USB 2.0 high-speed PHY               */
#define CYCLONEVG_B01_FSINTF            ((grp_u32)0x00000020)           /* Full-speed serial interface select   */
#define CYCLONEVG_B01_ULPI_UTMI_SEL     ((grp_u32)0x00000010)           /* ULPI select                          */
#define CYCLONEVG_B01_PHYIF             ((grp_u32)0x00000008)           /* PHY interfac                         */
#define CYCLONEVG_B03_TOUTCAL           ((grp_u32)0x00000007)           /* HS FS timeout calibration            */

/* OTG reset register (GRSTCTL) */
#define CYCLONEVG_B01_AHBIDLE           ((grp_u32)0x80000000)           /* AHB master idle                  */
#define CYCLONEVG_B01_DMAREQ            ((grp_u32)0x80000000)           /* DMA request signal               */
#define CYCLONEVG_B05_TXFNUM            ((grp_u32)0x000007C0)           /* TxFIFO number (TXF0-TXF15)       */
    #define CYCLONEVG_VTXFNUM_FLASHALL      ((grp_u32)0x00000400)           /* Flash all TxFIFO (TXF16)         */
#define CYCLONEVG_B01_TXFFLSH           ((grp_u32)0x00000020)           /* TxFIFO flush                     */
#define CYCLONEVG_B01_RXFFLSH           ((grp_u32)0x00000010)           /* RxFIFO flush                     */
#define CYCLONEVG_B01_FRMCNTRRST        ((grp_u32)0x00000004)           /* Host frame counter reset         */
#define CYCLONEVG_B01_CSFTRST           ((grp_u32)0x00000001)           /* Core soft reset                  */

/* OTG core interrupt register (GINTSTS) */
#define CYCLONEVG_B01_WKUPINT           ((grp_u32)0x80000000)           /* Resume/remote wakeup detected interrupt          */
#define CYCLONEVG_B01_SESSREQINT        ((grp_u32)0x40000000)           /* Session request/new session detected interrupt   */
#define CYCLONEVG_B01_DISCONNINT        ((grp_u32)0x20000000)           /* Disconnect detected interrupt                    */
#define CYCLONEVG_B01_CONIDSTSCHNG      ((grp_u32)0x10000000)           /* Connector ID status change                       */
#define CYCLONEVG_B01_PTXFEMP           ((grp_u32)0x04000000)           /* Periodic TxFIFO empty                            */
#define CYCLONEVG_B01_HCHINT            ((grp_u32)0x02000000)           /* Host channels interrupt                          */
#define CYCLONEVG_B01_PRTINT            ((grp_u32)0x01000000)           /* Host port interrupt                              */
#define CYCLONEVG_B01_RESETDET          ((grp_u32)0x00800000)           /* Reset detected interrupt                         */
#define CYCLONEVG_B01_FETSUSP           ((grp_u32)0x00400000)           /* Data fetch suspended                             */
#define CYCLONEVG_B01_INCOMPLP          ((grp_u32)0x00200000)           /* Incomplete periodic transfer                     */
#define CYCLONEVG_B01_INCOMPISOOUT      ((grp_u32)0x00200000)           /* Incomplete isochronous OUT transfer              */
#define CYCLONEVG_B01_INCOMPISOIN       ((grp_u32)0x00100000)           /* Incomplete isochronous IN transfer               */
#define CYCLONEVG_B01_OEPINT            ((grp_u32)0x00080000)           /* OUT endpoint interrupt                           */
#define CYCLONEVG_B01_IEPINT            ((grp_u32)0x00040000)           /* IN endpoint interrupt                            */
#define CYCLONEVG_B01_EPMIS             ((grp_u32)0x00020000)           /* Endpoint mismatch interrupt                      */
#define CYCLONEVG_B01_ISOOUTDROP        ((grp_u32)0x00004000)           /* Isochronous OUT packet dropped interrupt         */
#define CYCLONEVG_B01_ENUMDONE          ((grp_u32)0x00002000)           /* Enumeration done                                 */
#define CYCLONEVG_B01_USBRST            ((grp_u32)0x00001000)           /* USB reset                                        */
#define CYCLONEVG_B01_USBSUSP           ((grp_u32)0x00000800)           /* USB suspend                                      */
#define CYCLONEVG_B01_ERLYSUSP          ((grp_u32)0x00000400)           /* Early suspend                                    */
#define CYCLONEVG_B01_GOUTNAKEFF        ((grp_u32)0x00000080)           /* Global OUT NAK effective                         */
#define CYCLONEVG_B01_GINNAKEFF         ((grp_u32)0x00000040)           /* Global IN non-periodic NAK effective             */
#define CYCLONEVG_B01_RXFLVL            ((grp_u32)0x00000010)           /* RxFIFO non-empty                                 */
#define CYCLONEVG_B01_SOF               ((grp_u32)0x00000008)           /* Start of frame                                   */
#define CYCLONEVG_B01_OTGINT            ((grp_u32)0x00000004)           /* OTG interrupt                                    */
#define CYCLONEVG_B01_MODEMIS           ((grp_u32)0x00000002)           /* Mode mismatch interrupt                          */
#define CYCLONEVG_B01_CURMOD            ((grp_u32)0x00000001)           /* Current mode of operation                        */

/* OTG interrupt mask register (GINTMSK) */
#define CYCLONEVG_B01_WKUPINTMSK        ((grp_u32)0x80000000)           /* Resume/remote wakeup detected interrupt mask         */
#define CYCLONEVG_B01_SESSREQINTMSK     ((grp_u32)0x40000000)           /* Session request/new session detected interrupt mask  */
#define CYCLONEVG_B01_DISCONNINTMSK     ((grp_u32)0x20000000)           /* Disconnect detected interrupt mask                   */
#define CYCLONEVG_B01_CONIDSTSCHNGMSK   ((grp_u32)0x10000000)           /* Connector ID status change mask                      */
#define CYCLONEVG_B01_PTXFEMPMS         ((grp_u32)0x04000000)           /* Periodic TxFIFO empty mask                           */
#define CYCLONEVG_B01_HCHINTMSK         ((grp_u32)0x02000000)           /* Host channels interrupt mask                         */
#define CYCLONEVG_B01_PRTINTMSK         ((grp_u32)0x01000000)           /* Host port interrupt mask                             */
#define CYCLONEVG_B01_RESETDETMSK       ((grp_u32)0x00800000)           /* Reset detected interrupt mask                        */
#define CYCLONEVG_B01_FETSUSPMSK        ((grp_u32)0x00400000)           /* Data fetch suspended mask                            */
#define CYCLONEVG_B01_INCOMPLPMSK       ((grp_u32)0x00200000)           /* Incomplete periodic transfer mask                    */
#define CYCLONEVG_B01_INCOMPISOINMSK    ((grp_u32)0x00100000)           /* Incomplete isochronous IN transfer mask              */
#define CYCLONEVG_B01_OEPINTMSK         ((grp_u32)0x00080000)           /* OUT endpoints interrupt mask                         */
#define CYCLONEVG_B01_IEPINTMSK         ((grp_u32)0x00040000)           /* IN endpoints interrupt mask                          */
#define CYCLONEVG_B01_EPMISMSK          ((grp_u32)0x00020000)           /* Endpoint mismatch interrupt mask                     */
#define CYCLONEVG_B01_EOPFMSK           ((grp_u32)0x00008000)           /* End of periodic frame interrupt mask                 */
#define CYCLONEVG_B01_ISOOUTDROPMSK     ((grp_u32)0x00004000)           /* Isochronous OUT packet dropped interrupt mask        */
#define CYCLONEVG_B01_ENUMDONEMSK       ((grp_u32)0x00002000)           /* Enumeration done mask                                */
#define CYCLONEVG_B01_USBRSTMSK         ((grp_u32)0x00001000)           /* USB reset mask                                       */
#define CYCLONEVG_B01_USBSUSPMSK        ((grp_u32)0x00000800)           /* USB suspend mask                                     */
#define CYCLONEVG_B01_ERLYSUSPMSK       ((grp_u32)0x00000400)           /* Early suspend mask                                   */
#define CYCLONEVG_B01_GOUTNAKEFFMSK     ((grp_u32)0x00000080)           /* Global OUT NAK effective mask                        */
#define CYCLONEVG_B01_GINNAKEFFMSK      ((grp_u32)0x00000040)           /* Global non-periodic IN NAK effective mask            */
#define CYCLONEVG_B01_RXFLVLMSK         ((grp_u32)0x00000010)           /* Receive FIFO non-empty mask                          */
#define CYCLONEVG_B01_SOFMSK            ((grp_u32)0x00000008)           /* Start of frame mask                                  */
#define CYCLONEVG_B01_OTGINTMSK         ((grp_u32)0x00000004)           /* OTG interrupt mask                                   */
#define CYCLONEVG_B01_MODEMISMSK        ((grp_u32)0x00000002)           /* Mode mismatch interrupt mask                         */

/* OTG receive status debug read register (GRXSTSR) */
/* OTG status read and pop register (GRXSTSP) */
#define CYCLONEVG_B04_FN                ((grp_u32)0x01E00000)           /* Frame number     */
#define CYCLONEVG_B04_PKTSTS            ((grp_u32)0x001E0000)           /* Packet status    */
    #define CYCLONEVG_VD_PKTSTS_GOUT_NAK    ((grp_u32)0x00020000)           /* 0001: Global OUT NAK                 */
    #define CYCLONEVG_VD_PKTSTS_OD_PKT_RCV  ((grp_u32)0x00040000)           /* 0010: OUT data packet received       */
    #define CYCLONEVG_VD_PKTSTS_OTF_CMPL    ((grp_u32)0x00060000)           /* 0011: OUT transfer completed         */
    #define CYCLONEVG_VD_PKTSTS_STTR_CMPL   ((grp_u32)0x00080000)           /* 0100: SETUP transaction completed    */
    #define CYCLONEVG_VD_PKTSTS_STDT_RCV    ((grp_u32)0x000C0000)           /* 0110: SETUP data packet received     */
#define CYCLONEVG_B02_DPID              ((grp_u32)0x00018000)           /* Data PID         */
    #define CYCLONEVG_V_DPID_DATA0          ((grp_u32)0x00000000)           /* 00: DATA0 */
    #define CYCLONEVG_V_DPID_DATA1          ((grp_u32)0x00010000)           /* 10: DATA1 */
    #define CYCLONEVG_V_DPID_DATA2          ((grp_u32)0x00008000)           /* 01: DATA2 */
    #define CYCLONEVG_V_DPID_MDATA          ((grp_u32)0x00018000)           /* 11: MDATA */
#define CYCLONEVG_B11_BCNT              ((grp_u32)0x00007FF0)           /* Byte count       */
#define CYCLONEVG_B04_CHNUM             ((grp_u32)0x0000000F)           /* Channel number   */

/* OTG receive FIFO size register (GRXFSIZ) */
#define CYCLONEVG_B14_RXFDEP            ((grp_u32)0x00003FFF)           /* RxFIFO depth */

/* OTG non-periodic transmit FIFO size register (GNPTXFSIZ) */
#define CYCLONEVG_B14_NPTXFDEP          ((grp_u32)0x3FFF0000)           /* Non-periodic TxFIFO depth                */
#define CYCLONEVG_B14_NPTXFSTADDR       ((grp_u32)0x00003FFF)           /* Non-periodic transmit RAM start address  */

/* OTG non-periodic transmit FIFO/queue status register (GNPTXSTS) */
#define CYCLONEVG_B07_NPTXQTOP          ((grp_u32)0x7F000000)           /* Top of the non-periodic transmit request queue       */
#define CYCLONEVG_B08_NPTXQSPCAVAIL     ((grp_u32)0x00FF0000)           /* Non-periodic transmit request queue space available  */
#define CYCLONEVG_B16_NPTXFSPCAVAIL     ((grp_u32)0x0000FFFF)           /* Non-periodic TxFIFO space available                  */

/* OTG PHY vendor control register (GPVNDCTL) */
#define CYCLONEVG_B01_DISULPIDRVR       ((grp_u32)0x80000000)           /* Disable ULPI drivers                     */
#define CYCLONEVG_B01_VSTSDONE          ((grp_u32)0x08000000)           /* VStatus done                             */
#define CYCLONEVG_B01_VSTSBSY           ((grp_u32)0x04000000)           /* VStatus busy                             */
#define CYCLONEVG_B01_NEWREGREQ         ((grp_u32)0x02000000)           /* New register request                     */
#define CYCLONEVG_B01_REGWR             ((grp_u32)0x00400000)           /* Register write                           */
#define CYCLONEVG_B06_REGADDR           ((grp_u32)0x003F0000)           /* Register address                         */
#define CYCLONEVG_B08_VCTRL             ((grp_u32)0x0000FF00)           /* UTMI+ vendor control register address    */
#define CYCLONEVG_B08_REGDATA           ((grp_u32)0x000000FF)           /* Register data                            */

/* OTG general purpose input output register (GGPIO) */
#define CYCLONEVG_B16_GPO               ((grp_u32)0xFFFF0000)           /* General purpose output   */
#define CYCLONEVG_B16_GPI               ((grp_u32)0x0000FFFF)           /* General purpose input    */

/* OTG user ID register (GUID) */
#define CYCLONEVG_B32_GUID              ((grp_u32)0xFFFFFFFF)           /* User ID field    */

/* OTG synopsys ID register (GSNPSID) */
#define CYCLONEVG_B32_GSNPSID           ((grp_u32)0xFFFFFFFF)           /* Synopsys ID field    */

/* OTG user HW config1 register (GHWCFG1) */
#define CYCLONEVG_B32_GHWCFG1           ((grp_u32)0xFFFFFFFF)           /* Endpoint direction field */
    #define CYCLONEVG_B02_GHWCFG1_EP(x)     ((grp_u32)(0x00000003 << (x*2)))    /* Endpoint x direction mask */
        #define CYCLONEVG_V_BDIR_EP(x)          ((grp_u32)(0x00000000 << (x*2)))    /* Endpoint x direction BIDIR(IN and OUT)   */
        #define CYCLONEVG_V_INENDPT_EP(x)       ((grp_u32)(0x00000001 << (x*2)))    /* Endpoint x direction IN endpoint         */
        #define CYCLONEVG_V_OUTENDPT_EP(x)      ((grp_u32)(0x00000002 << (x*2)))    /* Endpoint x direction OUT endpoint        */
        #define CYCLONEVG_V_RESERVED_EP(x)      ((grp_u32)(0x00000003 << (x*2)))    /* Endpoint x direction reserved            */

/* OTG user HW config2 register (GHWCFG2) */
#define CYCLONEVG_B05_TKNQDEPTH         ((grp_u32)0x7C000000)           /* Device mode IN token sequence learning queue depth   */
#define CYCLONEVG_B02_PTXQDEPTH         ((grp_u32)0x03000000)           /* Host mode periodic request queue depth               */
#define CYCLONEVG_B02_NPTXQDEPTH        ((grp_u32)0x00C00000)           /* Non periodic request queue depth                     */
#define CYCLONEVG_B01_MULTIPROCINTRPT   ((grp_u32)0x00100000)           /* Multi processor interrupt enabled                    */
#define CYCLONEVG_B01_DYNFIFOSIZING     ((grp_u32)0x00080000)           /* Dynamic FIFO sizing enabled                          */
#define CYCLONEVG_B01_PERIOSUPPORT      ((grp_u32)0x00040000)           /* Periodic OUT channels supported in host mode         */
#define CYCLONEVG_B04_NUMHSTCHNL        ((grp_u32)0x0003C000)           /* Number of host channels                              */
#define CYCLONEVG_B04_NUMDEVEPS         ((grp_u32)0x00003C00)           /* Number of device endpoints                           */
#define CYCLONEVG_B02_FSPHYTYPE         ((grp_u32)0x00000300)           /* Full speed PHY interface type                        */
#define CYCLONEVG_B02_HSPHYTYPE         ((grp_u32)0x000000C0)           /* High speed PHY interface type                        */
#define CYCLONEVG_B01_SINGPNT           ((grp_u32)0x00000020)           /* Point-to-point                                       */
#define CYCLONEVG_B02_OTGARCH           ((grp_u32)0x00000018)           /* Architecture                                         */
#define CYCLONEVG_B03_OTGMODE           ((grp_u32)0x00000007)           /* Mode of operation                                    */

/* OTG user HW config3 register (GHWCFG3) */
#define CYCLONEVG_B16_DFIFODEPTH        ((grp_u32)0xFFFF0000)           /* DFIFO depth                                  */
#define CYCLONEVG_B01_LPMMODE           ((grp_u32)0x00008000)           /* OTG enable LPM                               */
#define CYCLONEVG_B01_BCSUPPORT         ((grp_u32)0x00004000)           /* OTG BC support                               */
#define CYCLONEVG_B01_HSICMODE          ((grp_u32)0x00002000)           /* OTG enable HSIC                              */
#define CYCLONEVG_B01_ADPSUPPORT        ((grp_u32)0x00001000)           /* OTG ADP support                              */
#define CYCLONEVG_B01_RSTTYPE           ((grp_u32)0x00000800)           /* Reset style for clocked always blocks in RTL */
#define CYCLONEVG_B01_OPTFEATURE        ((grp_u32)0x00000400)           /* Optional features removed                    */
#define CYCLONEVG_B01_VNDCTLSUPT        ((grp_u32)0x00000200)           /* Vendor control interface suppor              */
#define CYCLONEVG_B01_I2CINTSEL         ((grp_u32)0x00000100)           /* I2C selection                                */
#define CYCLONEVG_B01_OTGEN             ((grp_u32)0x00000080)           /* OTG function enabled                         */
#define CYCLONEVG_B03_PKTSIZEWIDTH      ((grp_u32)0x00000070)           /* Width of packet size counters                */
#define CYCLONEVG_B04_XFERSIZEWIDTH     ((grp_u32)0x0000000F)           /* Width of transfer size counters              */

/* OTG user HW config4 register (GHWCFG4) */
#define CYCLONEVG_B01_DMA               ((grp_u32)0x80000000)           /* Scatter gather DMA                                                   */
#define CYCLONEVG_B01_DMA_CONFIGURATION ((grp_u32)0x40000000)           /* Scatter gather DMA configuration                                     */
#define CYCLONEVG_B04_INEPS             ((grp_u32)0x3C000000)           /* Number of device mode IN endpoints including control                 */
#define CYCLONEVG_B01_DEDFIFOMODE       ((grp_u32)0x02000000)           /* Enable dedicated transmit FIFO for device IN endpoints               */
#define CYCLONEVG_B01_SESSENDFLTR       ((grp_u32)0x01000000)           /* Session end filter                                                   */
#define CYCLONEVG_B01_BVALIDFLTR        ((grp_u32)0x00800000)           /* b_valid filter                                                       */
#define CYCLONEVG_B01_AVALIDFLTR        ((grp_u32)0x00400000)           /* a_valid filter                                                       */
#define CYCLONEVG_B01_VBUSVALIDFLTR     ((grp_u32)0x00200000)           /* Vbus valid filter enabled                                            */
#define CYCLONEVG_B01_IDDGFLTR          ((grp_u32)0x00100000)           /* Iddig Filter Enable                                                  */
#define CYCLONEVG_B04_NUMCTLEPS         ((grp_u32)0x000F0000)           /* Number of device mode control endpoints in addition to endpoint 0    */
#define CYCLONEVG_B02_PHYDATAWIDTH      ((grp_u32)0x0000C000)           /* UTMI+ PHY/ULPI-to-internal UTMI+ wrapper data width                  */
#define CYCLONEVG_B01_HIBERNATION       ((grp_u32)0x00000040)           /* Enable hibernation                                                   */
#define CYCLONEVG_B01_AHBFREQ           ((grp_u32)0x00000020)           /* Minimum AHB frequency less than 60 MHz                               */
#define CYCLONEVG_B01_PARTIALPWRDN      ((grp_u32)0x00000010)           /* Enable partial power down                                            */
#define CYCLONEVG_B04_NUMDEVPERIOEPS    ((grp_u32)0x0000000F)           /* Number of device mode periodic IN endpoints                          */

/* OTG DFIFO software config register (GDFIFOCFG) */
#define CYCLONEVG_B16_EPINFOBASEADDR    ((grp_u32)0xFFFF0000)           /* Endpoint info base addr  */
#define CYCLONEVG_B16_GDFIFOCFG         ((grp_u32)0x0000FFFF)           /* DFIFO software config    */

/* OTG host periodic transmit FIFO size register (HPTXFSIZ) */
#define CYCLONEVG_B14_PTXFSIZE          ((grp_u32)0x3FFF0000)           /* Host periodic TxFIFO depth           */
#define CYCLONEVG_B15_PTXFSTADDR        ((grp_u32)0x00007FFF)           /* Host periodic TxFIFO start address   */

/* OTG device IN endpoint transmit FIFO size register (DIEPTXFx) (x = 1..15, where x is the FIFO_number) */
#define CYCLONEVG_B14_INEPNTXFDEP       ((grp_u32)0x3FFF0000)           /* IN endpoint TxFIFO depth                     */
#define CYCLONEVG_B16_INEPNTXFSTADDR    ((grp_u32)0x0000FFFF)           /* IN endpoint FIFOx transmit RAM start address */

/*----------------------------------------------------------------------------------------------*/
/*  Host-mode control and status registers                                                      */
/*----------------------------------------------------------------------------------------------*/
/* OTG Host Configuration Register (HCFG) */
#define CYCLONEVH_B01_MODECHTIMEN       ((grp_u32)0x80000000)           /* used to wait for 200 PHY clock cycles    */
    #define CYCLONEVH_VMODECHTIMEN_200      ((grp_u32)0x00000000)           /* wait for 200 PHY clock cycles            */
    #define CYCLONEVH_VMODECHTIMEN_0        ((grp_u32)0x80000000)           /* no wait                                  */
#define CYCLONEVH_B01_PERSCHEDENA       ((grp_u32)0x04000000)           /* Enables periodic scheduling              */
    #define CYCLONEVH_VPERSCHEDENA_DIS      ((grp_u32)0x00000000)           /* Disable                                  */
    #define CYCLONEVH_VPERSCHEDENA_ENA      ((grp_u32)0x04000000)           /* Enable                                   */
#define CYCLONEVH_B02_FRLISTEN          ((grp_u32)0x03000000)           /* number of entries in the Frame list      */
    #define CYCLONEVH_VFRLISTEN_8ENT        ((grp_u32)0x01000000)           /* 8 Entrys                                 */
    #define CYCLONEVH_VFRLISTEN_16ENT       ((grp_u32)0x02000000)           /* 16 Entrys                                */
    #define CYCLONEVH_VFRLISTEN_32ENT       ((grp_u32)0x03000000)           /* 32 Entrys                                */
#define CYCLONEVH_B01_DESCDMA           ((grp_u32)0x00800000)           /* enable the Scatter/Gather DMA            */
    #define CYCLONEVH_VDESCDMA_NOR          ((grp_u32)0x00000000)           /* No Scatter/Gather DMA                    */
    #define CYCLONEVH_VDESCDMA_SCATTER      ((grp_u32)0x00800000)           /* Scatter/Gather DMA selected              */
#define CYCLONEVH_B08_RESVALID          ((grp_u32)0x007F0000)           /* detect a valid resume                    */
#define CYCLONEVH_B01_ENA32KHZS         ((grp_u32)0x00000080)           /* USB 1.1 Full-Speed                       */
    #define CYCLONEVH_VENA32KHZS_NOTSEL     ((grp_u32)0x00000000)           /* USB 1.1 Full-Speed                       */
    #define CYCLONEVH_VENA32KHZS_USB11      ((grp_u32)0x00000080)           /* USB 1.1 Full-Speed                       */
#define CYCLONEVH_B01_FSLSSUPP          ((grp_u32)0x00000004)           /* FS/LS-only                               */
    #define CYCLONEVH_VFSLSSUPP_HS          ((grp_u32)0x00000000)           /* HS/FS/LS                                 */
    #define CYCLONEVH_VFSLSSUPP_FS          ((grp_u32)0x00000004)           /* FS/LS-only                               */
#define CYCLONEVH_B02_FSLSPCLKSEL       ((grp_u32)0x00000003)           /* PHY clock                                */
    #define CYCLONEVH_VFSLSPCLKSEL_30_60MHZ ((grp_u32)0x00000000)           /* PHY clock is running at 30/60 MHz        */
    #define CYCLONEVH_VFSLSPCLKSEL_48MHZ    ((grp_u32)0x00000001)           /* PHY clock is running at 48 MHz           */
    #define CYCLONEVH_VFSLSPCLKSEL_6MHZ     ((grp_u32)0x00000002)           /* PHY clock is running at 6 MHz            */

/* OTG Host Frame Interval Register (HFIR) */
#define CYCLONEVH_B01_HFIRRLDCTRL       ((grp_u32)0x00010000)           /* dynamic reloading of the HFIR register   */
    #define CYCLONEVH_VHFIRRLDCTRL_NO       ((grp_u32)0x00000000)           /* cannot be reloaded dynamically           */
    #define CYCLONEVH_VHFIRRLDCTRL_RELOD    ((grp_u32)0x00010000)           /* can be dynamically reloaded              */
#define CYCLONEVH_B16_FRINT             ((grp_u32)0x0000FFFF)           /* interval between two consecutive SOFs    */

/* OTG Host Frame Number Frame Time Remaining Register (HFNUM) */
#define CYCLONEVH_B16_FRREM             ((grp_u32)0xFFFF0000)           /* amount of time remaining                 */
#define CYCLONEVH_B16_FRNUM             ((grp_u32)0x0000FFFF)           /* increments when a new SOF is transmitted */

/* OTG Host Periodic Transmit FIFO Queue Status Register (HPTXSTS) */
#define CYCLONEVH_B01_ODDEVNMFRAME      ((grp_u32)0x80000000)           /* odd/even micro frame                     */
    #define CYCLONEVH_VODDEVNMFRAME_EVEN    ((grp_u32)0x00000000)           /* Send in even (micro)Frame                */
    #define CYCLONEVH_VODDEVNMFRAME_ODD     ((grp_u32)0x80000000)           /* Send in odd (micro)Frame                 */
#define CYCLONEVH_B04_CHANENDPT         ((grp_u32)0x78000000)           /* channel endpoint number                  */
    #define CYCLONEVH_VCHANENDPT_EP01       ((grp_u32)0x00000000)           /* End point 1                              */
    #define CYCLONEVH_VCHANENDPT_EP02       ((grp_u32)0x08000000)           /* End point 2                              */
    #define CYCLONEVH_VCHANENDPT_EP03       ((grp_u32)0x10000000)           /* End point 3                              */
    #define CYCLONEVH_VCHANENDPT_EP04       ((grp_u32)0x18000000)           /* End point 4                              */
    #define CYCLONEVH_VCHANENDPT_EP05       ((grp_u32)0x20000000)           /* End point 5                              */
    #define CYCLONEVH_VCHANENDPT_EP06       ((grp_u32)0x28000000)           /* End point 6                              */
    #define CYCLONEVH_VCHANENDPT_EP07       ((grp_u32)0x30000000)           /* End point 7                              */
    #define CYCLONEVH_VCHANENDPT_EP08       ((grp_u32)0x38000000)           /* End point 8                              */
    #define CYCLONEVH_VCHANENDPT_EP09       ((grp_u32)0x40000000)           /* End point 9                              */
    #define CYCLONEVH_VCHANENDPT_EP10       ((grp_u32)0x48000000)           /* End point 10                             */
    #define CYCLONEVH_VCHANENDPT_EP11       ((grp_u32)0x50000000)           /* End point 11                             */
    #define CYCLONEVH_VCHANENDPT_EP12       ((grp_u32)0x58000000)           /* End point 12                             */
    #define CYCLONEVH_VCHANENDPT_EP13       ((grp_u32)0x60000000)           /* End point 13                             */
    #define CYCLONEVH_VCHANENDPT_EP14       ((grp_u32)0x68000000)           /* End point 14                             */
    #define CYCLONEVH_VCHANENDPT_EP15       ((grp_u32)0x70000000)           /* End point 15                             */
    #define CYCLONEVH_VCHANENDPT_EP16       ((grp_u32)0x78000000)           /* End point 16                             */
#define CYCLONEVH_B02_TYPE              ((grp_u32)0x06000000)           /* Periodic Tx Request Queue                */
    #define CYCLONEVH_VTYPE_IN_OUT          ((grp_u32)0x00000000)           /* IN/OUT type                              */
    #define CYCLONEVH_VTYPE_ZERO_LEN        ((grp_u32)0x02000000)           /* Zero-length packet type                  */
    #define CYCLONEVH_VTYPE_CSPLIT          ((grp_u32)0x04000000)           /* CSPLIT type                              */
    #define CYCLONEVH_VTYPE_DIS             ((grp_u32)0x06000000)           /* Disable channel command                  */
#define CYCLONEVH_B01_TERM              ((grp_u32)0x01000000)           /* Terminate last entry                     */
    #define CYCLONEVH_VTERM_NO_TERM         ((grp_u32)0x00000000)           /* No termination                           */
    #define CYCLONEVH_VTERM_TERMINATE       ((grp_u32)0x01000000)           /* Terminate last entry                     */
#define CYCLONEVH_B08_PTXQSPCAVAIL      ((grp_u32)0x00FF0000)           /* Periodic Transmit Request Queue          */
    #define CYCLONEVH_VPTXQSPCAVAIL_FULL    ((grp_u32)0x00000000)           /* Periodic Transmit Request Queue is full  */
    #define CYCLONEVH_VPTXQSPCAVAIL_1LOC    ((grp_u32)0x00010000)           /* 1 location available                     */
    #define CYCLONEVH_VPTXQSPCAVAIL_2LOC    ((grp_u32)0x00020000)           /* 2 location available                     */
    #define CYCLONEVH_VPTXQSPCAVAIL_3LOC    ((grp_u32)0x00030000)           /* 3 location available                     */
    #define CYCLONEVH_VPTXQSPCAVAIL_4LOC    ((grp_u32)0x00040000)           /* 4 location available                     */
    #define CYCLONEVH_VPTXQSPCAVAIL_5LOC    ((grp_u32)0x00050000)           /* 5 location available                     */
    #define CYCLONEVH_VPTXQSPCAVAIL_6LOC    ((grp_u32)0x00060000)           /* 6 location available                     */
    #define CYCLONEVH_VPTXQSPCAVAIL_7LOC    ((grp_u32)0x00070000)           /* 7 location available                     */
    #define CYCLONEVH_VPTXQSPCAVAIL_8LOC    ((grp_u32)0x00080000)           /* 8 location available                     */
#define CYCLONEVH_B16_PTXFSPCAVAIL      ((grp_u32)0x0000FFFF)           /* Periodic TxFIFO.                         */

/* OTG Host All Channels Interrupt Register (HAINT) */
#define CYCLONEVH_B16_HAINT             ((grp_u32)0x0000FFFF)           /* One bit per channel: Bit 0 for Channel 0 */
    #define CYCLONEVH_VHAINT_CH(x)          ((grp_u32)(0x00000001 << (x)))  /* Channel x interrupt */

/* OTG Host All Channels Interrupt Mask Register (HAINTMSK) */
#define CYCLONEVH_B16_HAINTMSK          ((grp_u32)0x0000FFFF)           /* One bit per channel: Bit 0 for Channel 0 */
                                                                        /* 0x0 Mask interrupt  0x1 Unmask interrupt */
//    #define CYCLONEVH_VHAINT_CH(x)          ((grp_u32)(0x00000001 << (x)))  /* Channel x interrupt */

/* OTG Host Frame List Base Address Register (HFLBADDR) */
#define CYCLONEVH_B32_HFLBADDR          ((grp_u32)0xFFFFFFFF)           /* Starting address of the Frame list       */

/* OTG Host Port Control and Status Register (HPRT) */
#define CYCLONEVH_B02_PRTSPD            ((grp_u32)0x00060000)           /* speed of the device attached             */
    #define CYCLONEVH_VPRTSPD_HS            ((grp_u32)0x00000000)           /* High speed                               */
    #define CYCLONEVH_VPRTSPD_FS            ((grp_u32)0x00020000)           /* Full speed                               */
    #define CYCLONEVH_VPRTSPD_LS            ((grp_u32)0x00040000)           /* Low speed                                */
#define CYCLONEVH_B04_PRTTSTCTL         ((grp_u32)0x0001E000)           /* Test mode                                */
    #define CYCLONEVH_VPRTTSTCTL_DIS        ((grp_u32)0x00000000)           /* Test mode disable                        */
    #define CYCLONEVH_VPRTTSTCTL_J          ((grp_u32)0x00002000)           /* Test J mode                              */
    #define CYCLONEVH_VPRTTSTCTL_K          ((grp_u32)0x00004000)           /* Test K mode                              */
    #define CYCLONEVH_VPRTTSTCTL_SE0NAK     ((grp_u32)0x00006000)           /* Test SE0 NAK mode                        */
    #define CYCLONEVH_VPRTTSTCTL_PACKET     ((grp_u32)0x00008000)           /* Test Packet mode                         */
    #define CYCLONEVH_VPRTTSTCTL_FORCE      ((grp_u32)0x0000A000)           /* Test Force mode                          */
#define CYCLONEVH_B01_PRTPWR            ((grp_u32)0x00001000)           /* control power to this port               */
    #define CYCLONEVH_VPRTPWR_OFF           ((grp_u32)0x00000000)           /* Power off                                */
    #define CYCLONEVH_VPRTPWR_ON            ((grp_u32)0x00001000)           /* Power on                                 */
#define CYCLONEVH_B02_PRTLNSTS          ((grp_u32)0x00000C00)           /* current logic level USB data lines       */
    #define CYCLONEVH_VPRTLNSTS_DP          ((grp_u32)0x00000400)           /* D+                                       */
    #define CYCLONEVH_VPRTLNSTS_DM          ((grp_u32)0x00000800)           /* D-                                       */
#define CYCLONEVH_B01_PRTRST            ((grp_u32)0x00000100)           /* reset sequence is started on this port   */
    #define CYCLONEVH_VPRTRST_NOR           ((grp_u32)0x00000000)           /* Port not in reset                        */
    #define CYCLONEVH_VPRTRST_RESET         ((grp_u32)0x00000100)           /* Port in reset                            */
#define CYCLONEVH_B01_PRTSUSP           ((grp_u32)0x00000080)           /* put this port in Suspend mode            */
    #define CYCLONEVH_VPRTSUSP_NOR          ((grp_u32)0x00000000)           /* Port not in Suspend mode                 */
    #define CYCLONEVH_VPRTSUSP_SUSPEND      ((grp_u32)0x00000080)           /* Port in Suspend mode                     */
#define CYCLONEVH_B01_PRTRES            ((grp_u32)0x00000040)           /* drive resume signaling on the port       */
    #define CYCLONEVH_VPRTRES_NOR           ((grp_u32)0x00000000)           /* No resume driven                         */
    #define CYCLONEVH_VPRTRES_RESUME        ((grp_u32)0x00000040)           /* Resume driven                            */
#define CYCLONEVH_B01_PRTOVRCURRCHNG    ((grp_u32)0x00000020)           /* PortOvercurrent Active bit               */
    #define CYCLONEVH_VPRTOVRCURRCHNG_NO    ((grp_u32)0x00000000)           /* Status of port overcurrent no change     */
    #define CYCLONEVH_VPRTOVRCURRCHNG_CH    ((grp_u32)0x00000020)           /* Status of port overcurrent changed       */
#define CYCLONEVH_B01_PRTOVRCURRACT     ((grp_u32)0x00000010)           /* overcurrent condition of the port        */
    #define CYCLONEVH_VPRTOVRCURRACT_NOR    ((grp_u32)0x00000000)           /* No overcurrent condition                 */
    #define CYCLONEVH_VPRTOVRCURRACT_OVC    ((grp_u32)0x00000010)           /* Overcurrent condition                    */
#define CYCLONEVH_B01_PRTENCHNG         ((grp_u32)0x00000008)           /* Port Enable bit of this register changes */
    #define CYCLONEVH_VPRTENCHNG_NO         ((grp_u32)0x00000000)           /* Port Enable bit 2 no change              */
    #define CYCLONEVH_VPRTENCHNG_CHG        ((grp_u32)0x00000008)           /* Port Enable bit 2 changed                */
#define CYCLONEVH_B01_PRTENA            ((grp_u32)0x00000004)           /* port is enabled                          */
    #define CYCLONEVH_VPRTENA_DIS           ((grp_u32)0x00000000)           /* Port disable                             */
    #define CYCLONEVH_VPRTENA_ENA           ((grp_u32)0x00000004)           /* Port enable                              */
#define CYCLONEVH_B01_PRTCONNDET        ((grp_u32)0x00000002)           /* device connection is detected            */
    #define CYCLONEVH_VPRTCONNDET_DTCT      ((grp_u32)0x00000000)           /* Device connection detected               */
    #define CYCLONEVH_VPRTCONNDET_NO        ((grp_u32)0x00000002)           /* No device connection detected            */
#define CYCLONEVH_B01_PRTCONNSTS        ((grp_u32)0x00000001)           /* Defines whether port is attached         */
    #define CYCLONEVH_VPRTCONNSTS_NO        ((grp_u32)0x00000000)           /* No device is attached to the port        */
    #define CYCLONEVH_VPRTCONNSTS_ATCH      ((grp_u32)0x00000001)           /* A device is attached to the port         */

/* OTG Host Channel x Characteristics Register (HCCHARx) (x = 0..15, where x = Channel number) */
#define CYCLONEVH_B01_CHENA             ((grp_u32)0x80000000)           /* structure is ready                       */
    #define CYCLONEVH_VCHENA_NOT_READY      ((grp_u32)0x00000000)           /* descriptor structure is not yet ready    */
    #define CYCLONEVH_VCHENA_READY          ((grp_u32)0x80000000)           /* channel can access the descriptor        */
#define CYCLONEVH_B01_CHDIS             ((grp_u32)0x40000000)           /* stop transmitting/receiving data         */
    #define CYCLONEVH_VCHDIS_NO             ((grp_u32)0x00000000)           /* No activity                              */
    #define CYCLONEVH_VCHDIS_STOP           ((grp_u32)0x40000000)           /* Stop transmitting/receiving data         */
#define CYCLONEVH_B07_DEVADDR           ((grp_u32)0x1FC00000)           /* device serving as the data source or sink*/
#define CYCLONEVH_B02_EC                ((grp_u32)0x00300000)           /* executed per microframe                  */
    #define CYCLONEVH_VEC_TR1               ((grp_u32)0x00100000)           /* 1 transaction                            */
    #define CYCLONEVH_VEC_TR2               ((grp_u32)0x00200000)           /* 2 transactions                           */
    #define CYCLONEVH_VEC_TR3               ((grp_u32)0x00300000)           /* 3 transactions                           */
#define CYCLONEVH_B02_EPTYPE            ((grp_u32)0x000C0000)           /* transfer type                            */
    #define CYCLONEVH_VEPTYPE_CTRL          ((grp_u32)0x00000000)           /* Control                                  */
    #define CYCLONEVH_VEPTYPE_ISO           ((grp_u32)0x00040000)           /* Isochronous                              */
    #define CYCLONEVH_VEPTYPE_BULK          ((grp_u32)0x00080000)           /* Bulk                                     */
    #define CYCLONEVH_VEPTYPE_INT           ((grp_u32)0x000C0000)           /* Interrupt                                */
#define CYCLONEVH_B01_LSPDDEV           ((grp_u32)0x00020000)           /* communicating to a low-speed device      */
    #define CYCLONEVH_VLSPDDEV_NOTLS        ((grp_u32)0x00000000)           /* Communicating with non lowspeed          */
    #define CYCLONEVH_VLSPDDEV_LS           ((grp_u32)0x00020000)           /* Communicating with lowspeed              */
#define CYCLONEVH_B01_EPDIR             ((grp_u32)0x00008000)           /* transaction is IN or OUT                 */
    #define CYCLONEVH_VEPDIR_OUT            ((grp_u32)0x00000000)           /* OUT                                      */
    #define CYCLONEVH_VEPDIR_IN             ((grp_u32)0x00008000)           /* IN                                       */
#define CYCLONEVH_B04_EPNUM             ((grp_u32)0x00007800)           /* endpoint number                          */
    #define CYCLONEVH_VEPNUM_EP0            ((grp_u32)0x00000000)           /* End point 0                              */
    #define CYCLONEVH_VEPNUM_EP1            ((grp_u32)0x00000800)           /* End point 1                              */
    #define CYCLONEVH_VEPNUM_EP2            ((grp_u32)0x00001000)           /* End point 2                              */
    #define CYCLONEVH_VEPNUM_EP3            ((grp_u32)0x00001800)           /* End point 3                              */
    #define CYCLONEVH_VEPNUM_EP4            ((grp_u32)0x00002000)           /* End point 4                              */
    #define CYCLONEVH_VEPNUM_EP5            ((grp_u32)0x00002800)           /* End point 5                              */
    #define CYCLONEVH_VEPNUM_EP6            ((grp_u32)0x00003000)           /* End point 6                              */
    #define CYCLONEVH_VEPNUM_EP7            ((grp_u32)0x00003800)           /* End point 7                              */
    #define CYCLONEVH_VEPNUM_EP8            ((grp_u32)0x00004000)           /* End point 8                              */
    #define CYCLONEVH_VEPNUM_EP9            ((grp_u32)0x00004800)           /* End point 9                              */
    #define CYCLONEVH_VEPNUM_EP10           ((grp_u32)0x00005000)           /* End point 10                             */
    #define CYCLONEVH_VEPNUM_EP11           ((grp_u32)0x00005800)           /* End point 11                             */
    #define CYCLONEVH_VEPNUM_EP12           ((grp_u32)0x00006000)           /* End point 12                             */
    #define CYCLONEVH_VEPNUM_EP13           ((grp_u32)0x00006800)           /* End point 13                             */
    #define CYCLONEVH_VEPNUM_EP14           ((grp_u32)0x00007000)           /* End point 14                             */
    #define CYCLONEVH_VEPNUM_EP15           ((grp_u32)0x00007800)           /* End point 15                             */
#define CYCLONEVH_B11_MPS               ((grp_u32)0x000007FF)           /* maximum packet size                      */

/* OTG Host Channel x Split Control Register (HCSPLTx) (x = 0..15, where x = Channel number) */
#define CYCLONEVH_B01_SPLTENA           ((grp_u32)0x80000000)           /* enabled to split transactions            */
    #define CYCLONEVH_VSPLTENA_DIS          ((grp_u32)0x00000000)           /* Split not enabled                        */
    #define CYCLONEVH_VSPLTENA_ENA          ((grp_u32)0x80000000)           /* Split enabled                            */
#define CYCLONEVH_B01_COMPSPLT          ((grp_u32)0x00010000)           /* request to complete split transaction    */
    #define CYCLONEVH_VCOMPSPLT_NO          ((grp_u32)0x00000000)           /* No split transaction                     */
    #define CYCLONEVH_VCOMPSPLT_SPLIT       ((grp_u32)0x00010000)           /* Split transaction                        */
#define CYCLONEVH_B01_XACTPOS           ((grp_u32)0x0000C000)           /* send all,first, middle, or last payloads */
    #define CYCLONEVH_VXACTPOS_MID          ((grp_u32)0x00000000)           /* iddle payload of this transaction        */
    #define CYCLONEVH_VXACTPOS_END          ((grp_u32)0x00004000)           /* last payload of this transaction         */
    #define CYCLONEVH_VXACTPOS_BEGIN        ((grp_u32)0x00008000)           /* first data payload of this transaction   */
    #define CYCLONEVH_VXACTPOS_ALL          ((grp_u32)0x0000C000)           /* entire data payload of this transaction  */
#define CYCLONEVH_B01_HUBADDR           ((grp_u32)0x00003F80)           /* address of transaction translator's hub  */
#define CYCLONEVH_B01_PRTADDR           ((grp_u32)0x0000007F)           /* port number of recipient translator      */

/* OTG Host Channel x Interrupt Register (HCINTx) (x = 0..15, where x = Channel number) */
/* A bit is set when an interrupt occurs. */
#define CYCLONEVH_B01_DESC_LST_ROLLINTR ((grp_u32)0x00002000)           /* Descriptor rollover interrupt            */
#define CYCLONEVH_B01_XCS_XACT_ERR      ((grp_u32)0x00001000)           /* Excessive Transaction Error              */
#define CYCLONEVH_B01_BNAINTR           ((grp_u32)0x00000800)           /* BNA Interrupt                            */
#define CYCLONEVH_B01_DATATGLERR        ((grp_u32)0x00000400)           /* Data Toggle Error interrupt              */
#define CYCLONEVH_B01_FRMOVRUN          ((grp_u32)0x00000200)           /* Frame Overrun interrupt                  */
#define CYCLONEVH_B01_BBLERR            ((grp_u32)0x00000100)           /* Babble Error interrupt                   */
#define CYCLONEVH_B01_XACTERR           ((grp_u32)0x00000080)           /* Transaction Error interrupt              */
#define CYCLONEVH_B01_NYET              ((grp_u32)0x00000040)           /* NYET Response Received Interrupt         */
#define CYCLONEVH_B01_ACK               ((grp_u32)0x00000020)           /* ACK Response Received Transmitted Interru*/
#define CYCLONEVH_B01_NAK               ((grp_u32)0x00000010)           /* NAK Response Received Interrupt          */
#define CYCLONEVH_B01_STALL             ((grp_u32)0x00000008)           /* Stall Interrupt                          */
#define CYCLONEVH_B01_AHBERR            ((grp_u32)0x00000004)           /* AHB error during AHB read/write interrupt*/
#define CYCLONEVH_B01_CHHLTD            ((grp_u32)0x00000002)           /* Channel Halted interrupt                 */
#define CYCLONEVH_B01_XFERCOMPL         ((grp_u32)0x00000001)           /* Transfer completed normally              */

/* OTG Host Channel x Interrupt Mask Register (HCINTMSKx) (x = 0..15, where x = Channel number) */
/* When bit is set, interrupt is enabled. */
#define CYCLONEVH_B01_FRM_LST_ROLLINTRMSK ((grp_u32)0x00002000)           /* Descriptor rollover interrupt            */
#define CYCLONEVH_B01_XCS_XACT_ERRMSK   ((grp_u32)0x00001000)           /* Excessive Transaction Error              */
#define CYCLONEVH_B01_BNAINTRMSK        ((grp_u32)0x00000800)           /* BNA Interrupt                            */
#define CYCLONEVH_B01_DATATGLERRMSK     ((grp_u32)0x00000400)           /* Data Toggle Error interrupt              */
#define CYCLONEVH_B01_FRMOVRUNMSK       ((grp_u32)0x00000200)           /* Frame Overrun interrupt                  */
#define CYCLONEVH_B01_BBLERRMSK         ((grp_u32)0x00000100)           /* Babble Error interrupt                   */
#define CYCLONEVH_B01_XACTERRMSK        ((grp_u32)0x00000080)           /* Transaction Error interrupt              */
#define CYCLONEVH_B01_NYETMSK           ((grp_u32)0x00000040)           /* NYET Response Received Interrupt         */
#define CYCLONEVH_B01_ACKMSK            ((grp_u32)0x00000020)           /* ACK Response Received Transmitted Interru*/
#define CYCLONEVH_B01_NAKMSK            ((grp_u32)0x00000010)           /* NAK Response Received Interrupt          */
#define CYCLONEVH_B01_STALLMSK          ((grp_u32)0x00000008)           /* Stall Interrupt                          */
#define CYCLONEVH_B01_AHBERRMSK         ((grp_u32)0x00000004)           /* AHB error during AHB read/write interrupt*/
#define CYCLONEVH_B01_CHHLTDMSK         ((grp_u32)0x00000002)           /* Channel Halted interrupt                 */
#define CYCLONEVH_B01_XFERCOMPLMSK      ((grp_u32)0x00000001)           /* Transfer completed normally              */

/* OTG Host Channel x Transfer Size Register (HCTSIZx) (x = 0..15, where x = Channel number) */
#define CYCLONEVH_B01_DOPNG             ((grp_u32)0x80000000)           /* do PING protocol                         */
    #define CYCLONEVH_VDOPNG_NOPING         ((grp_u32)0x00000000)           /* No ping protocol                         */
    #define CYCLONEVH_VDOPNG_PING           ((grp_u32)0x80000000)           /* Ping protocol                            */
#define CYCLONEVH_B02_PID               ((grp_u32)0x60000000)           /* type of PID                              */
    #define CYCLONEVH_VPID_DATA0            ((grp_u32)0x00000000)           /* DATA0                                    */
    #define CYCLONEVH_VPID_DATA2            ((grp_u32)0x20000000)           /* DATA2                                    */
    #define CYCLONEVH_VPID_DATA1            ((grp_u32)0x40000000)           /* DATA1                                    */
    #define CYCLONEVH_VPID_MDATA            ((grp_u32)0x60000000)           /* MDATA (non-control)                      */
    #define CYCLONEVH_VPID_SETUP            ((grp_u32)0x60000000)           /* SETUP (control)                          */
#define CYCLONEVH_B10_PKTCNT            ((grp_u32)0x1FF80000)           /* number of packets                        */
#define CYCLONEVH_B19_XFERSIZE          ((grp_u32)0x0007FFFF)           /* number of data bytes                     */

/* OTG Host Channel 0 DMA Address Register (HCDMAx) (x = 0..15, where x = Channel number) */
#define CYCLONEVH_B01_HCDMA             ((grp_u32)0xFFFFFFFF)           /* start address of the 512 bytes page      */

/* OTG Host Channel 0 DMA Buffer Address Register (HCDMABx) (x = 0..15, where x = Channel number) */
#define CYCLONEVH_B01_HCDMAB            ((grp_u32)0xFFFFFFFF)           /* current buffer address                   */

/*----------------------------------------------------------------------------------------------*/
/*  Device-mode control and status registers                                                    */
/*----------------------------------------------------------------------------------------------*/
/* OTG device configuration register (DCFG) */
#define CYCLONEVD_B06_RESVALID          ((grp_u32)0xFC000000)           /* Resume validation period                 */
#define CYCLONEVD_B02_PERSCHINTVL       ((grp_u32)0x03000000)           /* Periodic scheduling interval             */
    #define CYCLONEVD_VPERSCHINTVL_MF25     ((grp_u32)0x00000000)           /* 00: 25% of (micro)frame */
    #define CYCLONEVD_VPERSCHINTVL_MF50     ((grp_u32)0x01000000)           /* 01: 50% of (micro)frame */
    #define CYCLONEVD_VPERSCHINTVL_MF75     ((grp_u32)0x02000000)           /* 10: 75% of (micro)frame */
#define CYCLONEVD_B01_DESCDMA           ((grp_u32)0x00800000)           /* Enable scatter gather DMA in device mode */
#define CYCLONEVD_B01_ENDEVOUTNAK       ((grp_u32)0x00002000)           /* Enable device OUT NA                     */
#define CYCLONEVD_B02_PERFRINT          ((grp_u32)0x00001800)           /* Periodic (micro)frame interval           */
    #define CYCLONEVD_VPERFRINT_EOPF80      ((grp_u32)0x00000000)           /* 00: 80% of the frame interval */
    #define CYCLONEVD_VPERFRINT_EOPF85      ((grp_u32)0x00000800)           /* 01: 85% of the frame interval */
    #define CYCLONEVD_VPERFRINT_EOPF90      ((grp_u32)0x00001000)           /* 10: 90% of the frame interval */
    #define CYCLONEVD_VPERFRINT_EOPF95      ((grp_u32)0x00001800)           /* 11: 95% of the frame interval */
#define CYCLONEVD_B07_DEVADDR           ((grp_u32)0x000007F0)           /* Device address                           */
#define CYCLONEVD_B01_ENA32KHZSUSP      ((grp_u32)0x00000008)           /* Enable 32 KHz suspend mode               */
#define CYCLONEVD_B01_NZSTSOUTHSHK      ((grp_u32)0x00000004)           /* Nonzero-length status OUT handshake      */
#define CYCLONEVD_B02_DEVSPD            ((grp_u32)0x00000003)           /* Device speed                             */
    #define CYCLONEVD_VDEVSPD_USBHS20       ((grp_u32)0x00000000)           /* 00: High speed USB 2.0 PHY       */
    #define CYCLONEVD_VDEVSPD_USBFS20       ((grp_u32)0x00000001)           /* 01: Full speed USB 2.0 PHY       */
    #define CYCLONEVD_VDEVSPD_USBLS116      ((grp_u32)0x00000002)           /* 10: Low speed USB 1.1 TC 6MHz    */
    #define CYCLONEVD_VDEVSPD_USBLS1148     ((grp_u32)0x00000003)           /* 11: Full speed USB 1.1 TC 48MHz  */

/* OTG device control register (DCTL) */
#define CYCLONEVD_B01_NAKONBBLE         ((grp_u32)0x00008000)           /* NAK on babble error                              */
#define CYCLONEVD_B01_IGNRFRMNUM        ((grp_u32)0x00004000)           /* Ignore frame number for isochronous endpoints    */
#define CYCLONEVD_B02_GMC               ((grp_u32)0x00003000)           /* Global multi count                               */
    #define CYCLONEVD_VGMC_NOTVALID         ((grp_u32)0x00000000)           /* 00: Invalid      */
    #define CYCLONEVD_VGMC_ONEPACKET        ((grp_u32)0x00001000)           /* 01: 1 packet     */
    #define CYCLONEVD_VGMC_TWOPACKET        ((grp_u32)0x00002000)           /* 10: 2 packets    */
    #define CYCLONEVD_VGMC_THREEPACKET      ((grp_u32)0x00003000)           /* 11: 3 packets    */
#define CYCLONEVD_B01_PWRONPRGDONE      ((grp_u32)0x00000800)           /* Power-on programming done                        */
#define CYCLONEVD_B01_CGOUTNAK          ((grp_u32)0x00000400)           /* Clear global OUT NAK                             */
#define CYCLONEVD_B01_SGOUTNAK          ((grp_u32)0x00000200)           /* Set global OUT NAK                               */
#define CYCLONEVD_B01_CGNPINNAK         ((grp_u32)0x00000100)           /* Clear global IN NAK                              */
#define CYCLONEVD_B01_SGNPINNAK         ((grp_u32)0x00000080)           /* Set global IN NAK                                */
#define CYCLONEVD_B03_TSTCTL            ((grp_u32)0x00000070)           /* Test control                                     */
    #define CYCLONEVD_VTSTCTL_DISABLE       ((grp_u32)0x00000000)           /* 000: Test mode disabled  */
    #define CYCLONEVD_VTSTCTL_TESTJ         ((grp_u32)0x00000010)           /* 001: Test_J mode         */
    #define CYCLONEVD_VTSTCTL_TESTK         ((grp_u32)0x00000020)           /* 010: Test_K mode         */
    #define CYCLONEVD_VTSTCTL_TESTSN        ((grp_u32)0x00000030)           /* 011: Test_SE0_NAK mode   */
    #define CYCLONEVD_VTSTCTL_TESTPM        ((grp_u32)0x00000040)           /* 100: Test_Packet mode    */
    #define CYCLONEVD_VTSTCTL_TESTFE        ((grp_u32)0x00000050)           /* 101: Test_Force_Enable   */
#define CYCLONEVD_B01_GOUTNAKSTS        ((grp_u32)0x00000008)           /* Global OUT NAK status                            */
#define CYCLONEVD_B01_GNPINNAKSTS       ((grp_u32)0x00000004)           /* Global IN NAK status                             */
#define CYCLONEVD_B01_SFTDISCON         ((grp_u32)0x00000002)           /* Soft disconnect                                  */
#define CYCLONEVD_B01_RMTWKUPSIG        ((grp_u32)0x00000001)           /* Remote wakeup signaling                          */

/* OTG device status register (DSTS) */
#define CYCLONEVD_B14_SOFFN             ((grp_u32)0x003FFF00)           /* Frame number of the received SOF */
#define CYCLONEVD_B01_ERRTICERR         ((grp_u32)0x00000008)           /* Erratic error                    */
#define CYCLONEVD_B02_ENUMSPD           ((grp_u32)0x00000006)           /* Enumerated speed                 */
    #define CYCLONEVD_VENUMSPD_HS3060       ((grp_u32)0x00000000)           /* 00: High speed (PHY clock is running at 30 or 60 MHz)    */
    #define CYCLONEVD_VENUMSPD_FS3060       ((grp_u32)0x00000002)           /* 01: Full speed (PHY clock is running at 30 or 60 MHz)    */
    #define CYCLONEVD_VENUMSPD_LS6          ((grp_u32)0x00000004)           /* 10: Low speed (PHY clock is running at 6 MHz)            */
    #define CYCLONEVD_VENUMSPD_FS48         ((grp_u32)0x00000006)           /* 11: Full speed (PHY clock is running at 48 MHz)          */
#define CYCLONEVD_B01_SUSPSTS           ((grp_u32)0x00000001)           /* Suspend status                   */

/* OTG device IN endpoint common interrupt mask register (DIEPMSK) */
#define CYCLONEVD_B01_NAKMSK            ((grp_u32)0x00002000)           /* NAK interrupt mask                                 */
#define CYCLONEVD_B01_BNAININTRMSK      ((grp_u32)0x00000200)           /* BNA interrupt mask                                 */
#define CYCLONEVD_B01_TXFIFOUNDRNMSK    ((grp_u32)0x00000100)           /* FIFO underrun mask                                 */
#define CYCLONEVD_B01_INEPNAKEFFMSK     ((grp_u32)0x00000040)           /* IN endpoint NAK effective mask                     */
#define CYCLONEVD_B01_INTKNEPMISMSK     ((grp_u32)0x00000020)           /* IN token received with EP mismatch mask            */
#define CYCLONEVD_B01_INTKNTXFEMPMSK    ((grp_u32)0x00000010)           /* IN token received when TxFIFO empty mask           */
#define CYCLONEVD_B01_TIMEOUTMSK        ((grp_u32)0x00000008)           /* Timeout condition mask (Non-isochronous endpoints) */
#define CYCLONEVD_B01_AHBERRMSK         ((grp_u32)0x00000004)           /* AHB error mask                                     */
#define CYCLONEVD_B01_EPDISBLDMSK       ((grp_u32)0x00000002)           /* Endpoint disabled interrupt mask                   */
#define CYCLONEVD_B01_XFERCOMPLMSK      ((grp_u32)0x00000001)           /* Transfer completed interrupt mask                  */

/* OTG device OUT endpoint common interrupt mask register (DOEPMSK) */
#define CYCLONEVD_B01_NYETMSK           ((grp_u32)0x00004000)           /* NYET interrupt mask                              */
//#define CYCLONEVD_B01_NAKMSK          ((grp_u32)0x00002000)           /* NAK interrupt mask                               */
#define CYCLONEVD_B01_BBLEERRMSK        ((grp_u32)0x00001000)           /* Babble Error interrupt Mask                      */
#define CYCLONEVD_B01_BNAOUTINTRMSK     ((grp_u32)0x00000200)           /* BNA interrupt mask                               */
#define CYCLONEVD_B01_OUTPKTERRMSK      ((grp_u32)0x00000100)           /* OUT packet error mask                            */
#define CYCLONEVD_B01_BACK2BACKSETUP    ((grp_u32)0x00000040)           /* Back-to-back SETUP packets received mask         */
#define CYCLONEVD_B01_OUTTKNEPDISMSK    ((grp_u32)0x00000010)           /* OUT token received when endpoint disabled mask   */
#define CYCLONEVD_B01_SETUPMSK          ((grp_u32)0x00000008)           /* SETUP phase done mask                            */
//#define CYCLONEVD_B01_AHBERRMSK       ((grp_u32)0x00000004)           /* AHB error mask                                   */
//#define CYCLONEVD_B01_EPDISBLDMSK     ((grp_u32)0x00000002)           /* Endpoint disabled interrupt mask                 */
//#define CYCLONEVD_B01_XFERCOMPLMSK    ((grp_u32)0x00000001)           /* Transfer completed interrupt mask                */

/* OTG device all endpoints interrupt register (DAINT) */
#define CYCLONEVD_B16_OUTEPINT          ((grp_u32)0xFFFF0000)           /* OUT endpoint interrupt bits  */
    #define CYCLONEVD_VOUTEPINT_EP(x)       ((grp_u32)(0x00010000 << (x)))  /* OUT endpoint x interrupt */
#define CYCLONEVD_B16_INEPINT           ((grp_u32)0x0000FFFF)           /* IN endpoint interrupt bits   */
    #define CYCLONEVD_VINEPINT_EP(x)        ((grp_u32)(0x00000001 << (x)))  /* IN endpoint x interrupt  */

/* OTG device all endpoints interrupt mask register (DAINTMSK) */
#define CYCLONEVD_B16_OUTEPMSK          ((grp_u32)0xFFFF0000)           /* OUT EP interrupt mask bits   */
    #define CYCLONEVD_VOUTEPMSK_EP(x)       ((grp_u32)(0x00010000 << (x)))  /* OUT endpoint x interrupt mask    */
#define CYCLONEVD_B16_INEPMSK           ((grp_u32)0x0000FFFF)           /* IN EP interrupt mask bits    */
    #define CYCLONEVD_VINEPMSK_EP(x)        ((grp_u32)(0x00000001 << (x)))  /* IN endpoint x interrupt mask     */

/* OTG device VBUS discharge time register (DVBUSDIS) */
#define CYCLONEVD_B16_DVBUSDIS          ((grp_u32)0x0000FFFF)           /* Device VBUS discharge time   */

/* OTG device VBUS pulsing time register (DVBUSPULSE) */
#define CYCLONEVD_B12_DVBUSPULSE        ((grp_u32)0x00000FFF)           /* Device VBUS pulsing time */

/* OTG device threshold control register (DTHRCTL) */
#define CYCLONEVD_B01_ARBPRKEN          ((grp_u32)0x08000000)           /* Arbiter parking enable                           */
#define CYCLONEVD_B09_RXTHRLEN          ((grp_u32)0x03FE0000)           /* Receive threshold length                         */
#define CYCLONEVD_B01_RXTHREN           ((grp_u32)0x00010000)           /* Receive threshold enable                         */
#define CYCLONEVD_B02_AHBTHRRATIO       ((grp_u32)0x00001800)           /* AHB threshold ratio                              */
    #define CYCLONEVD_VAHBTHRRATIO_THRESZERO    ((grp_u32)0x00000000)           /* 00: AHB threshold = MAC threshold        */
    #define CYCLONEVD_VAHBTHRRATIO_THRESONE     ((grp_u32)0x00000800)           /* 01: AHB threshold = MAC threshold / 2    */
    #define CYCLONEVD_VAHBTHRRATIO_THRESTWO     ((grp_u32)0x00001000)           /* 10: AHB threshold = MAC threshold / 4    */
    #define CYCLONEVD_VAHBTHRRATIO_THRESTHREE   ((grp_u32)0x00001800)           /* 11: AHB threshold = MAC threshold /      */
#define CYCLONEVD_B09_TXTHRLEN          ((grp_u32)0x000007FC)           /* Transmit threshold length                        */
#define CYCLONEVD_B01_ISOTHREN          ((grp_u32)0x00000002)           /* ISO IN endpoint threshold enable                 */
#define CYCLONEVD_B01_NONISOTHREN       ((grp_u32)0x00000001)           /* Non-isochronous IN endpoints threshold enable    */

/* OTG device IN endpoint FIFO empty interrupt mask register (DIEPEMPMSK) */
#define CYCLONEVD_B16_INEPTXFEMPMSK     ((grp_u32)0x0000FFFF)           /* IN EP TxFIFO empty interrupt mask bits */
    #define CYCLONEVD_VINEPTXFEMPMSK_EP(x)  ((grp_u32)(0x00000001 << (x)))  /* IN endpoint x TxFIFO empty interrupt mask */

/* OTG device IN endpoint-x control register (DIEPCTLx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B01_EPENA             ((grp_u32)0x80000000)           /* Endpoint enable                                      */
#define CYCLONEVD_B01_EPDIS             ((grp_u32)0x40000000)           /* Endpoint disable                                     */
#define CYCLONEVD_B01_SODDFRM           ((grp_u32)0x20000000)           /* Set odd frame (isochronous IN endpoints only)        */
#define CYCLONEVD_B01_SETD1PID          ((grp_u32)0x20000000)           /* Set DATA1 PID (interrupt/bulk IN endpoints only)     */
#define CYCLONEVD_B01_SEVNFRM           ((grp_u32)0x10000000)           /* Set even frame (isochronous IN endpoints only)       */
#define CYCLONEVD_B01_SETD0PID          ((grp_u32)0x10000000)           /* Set DATA0 PID (interrupt/bulk IN endpoints only)     */
#define CYCLONEVD_B01_SNAK              ((grp_u32)0x08000000)           /* Set NAK                                              */
#define CYCLONEVD_B01_CNAK              ((grp_u32)0x04000000)           /* Clear NAK                                            */
#define CYCLONEVD_B04_TXFNUM            ((grp_u32)0x03C00000)           /* TxFIFO number                                        */
#define CYCLONEVD_B01_STALL             ((grp_u32)0x00200000)           /* STALL handshake                                      */
#define CYCLONEVD_B02_EPTYPE            ((grp_u32)0x000C0000)           /* Endpoint type                                        */
    #define CYCLONEVD_VEPTYP_CONTROL        ((grp_u32)0x00000000)           /* 00: Control      */
    #define CYCLONEVD_VEPTYP_ISOCHRONOUS    ((grp_u32)0x00040000)           /* 01: Isochronous  */
    #define CYCLONEVD_VEPTYP_BULK           ((grp_u32)0x00080000)           /* 10: Bulk         */
    #define CYCLONEVD_VEPTYP_INTERRUP       ((grp_u32)0x000C0000)           /* 11: Interrupt    */
#define CYCLONEVD_B01_NAKSTS            ((grp_u32)0x00020000)           /* NAK status                                           */
#define CYCLONEVD_B01_EONUM             ((grp_u32)0x00010000)           /* Even/odd frame (isochronous IN endpoints only)       */
#define CYCLONEVD_B01_DPID              ((grp_u32)0x00010000)           /* Endpoint data PID (interrupt/bulk IN endpoints only) */
#define CYCLONEVD_B01_USBACTEP          ((grp_u32)0x00008000)           /* USB active endpoint                                  */
#define CYCLONEVD_B02_EP0_MPS           ((grp_u32)0x00000003)           /* Maximum packet size (endpoint 0)                     */
    #define CYCLONEVD_VEP0_MPS_BYTES64    ((grp_u32)0x00000000)           /* 00: 64bytes    */
    #define CYCLONEVD_VEP0_MPS_BYTES32    ((grp_u32)0x00000001)           /* 01: 32bytes    */
    #define CYCLONEVD_VEP0_MPS_BYTES16    ((grp_u32)0x00000002)           /* 10: 16bytes    */
    #define CYCLONEVD_VEP0_MPS_BYTES8     ((grp_u32)0x00000003)           /* 11:  8bytes    */
#define CYCLONEVD_B11_MPS               ((grp_u32)0x000007FF)           /* Maximum packet size                                  */

/* OTG device IN endpoint-x interrupt register (DIEPINTx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B01_NYETINTRPT        ((grp_u32)0x00004000)           /* NYET interrupt                                                         */
#define CYCLONEVD_B01_NAKINTRPT         ((grp_u32)0x00002000)           /* NAK interrupt                                                          */
#define CYCLONEVD_B01_BBLEERR           ((grp_u32)0x00001000)           /* Babble error interrupt                                                 */
#define CYCLONEVD_B01_PKTDRPSTS         ((grp_u32)0x00000800)           /* Packet dropped status                                                  */
#define CYCLONEVD_B01_BNAINTR           ((grp_u32)0x00000200)           /* Buffer not available interrupt                                         */
#define CYCLONEVD_B01_TXFIFOUNDRN       ((grp_u32)0x00000100)           /* Transmit FIFO underrun                                                 */
#define CYCLONEVD_B01_TXFEMP            ((grp_u32)0x00000080)           /* Transmit FIFO empty                                                    */
#define CYCLONEVD_B01_INEPNAKEFF        ((grp_u32)0x00000040)           /* IN endpoint NAK effective                                              */
#define CYCLONEVD_B01_INTKNEPMIS        ((grp_u32)0x00000020)           /* IN token received with EP mismatch (nonperiodic IN endpoints only)     */
#define CYCLONEVD_B01_INTKNTXFEMP       ((grp_u32)0x00000010)           /* IN token received when TxFIFO is empty (nonperiodic IN endpoints only) */
#define CYCLONEVD_B01_TIMEOUT           ((grp_u32)0x00000008)           /* Timeout condition (control IN endpoints only)                          */
#define CYCLONEVD_B01_AHBERR            ((grp_u32)0x00000004)           /* AHB error                                                              */
#define CYCLONEVD_B01_EPDISBLD          ((grp_u32)0x00000002)           /* Endpoint disabled interrupt                                            */
#define CYCLONEVD_B01_XFERCOMPL         ((grp_u32)0x00000001)           /* Transfer completed interrupt                                           */

/* OTG device IN endpoint-x transfer size register (DIEPTSIZx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B02_MC                ((grp_u32)0x60000000)           /* Multi count                      */
    #define CYCLONEVD_VMC_PACKETONE         ((grp_u32)0x20000000)           /* 01: 1 packet     */
    #define CYCLONEVD_VMC_PACKETTWO         ((grp_u32)0x40000000)           /* 10: 2 packets    */
    #define CYCLONEVD_VMC_PACKETTHREE       ((grp_u32)0x60000000)           /* 11: 3 packets    */
#define CYCLONEVD_B02_EP0_PKTCNT        ((grp_u32)0x00180000)           /* Packet count (for endpoint 0)    */
    #define CYCLONEVD_VEP0_PKTCNT_1PKT      ((grp_u32)0x00080000)           /* 01: 1 packet     */
#define CYCLONEVD_B10_PKTCNT            ((grp_u32)0x1FF80000)           /* Packet count                     */
    #define CYCLONEVD_VPKTCNT_1PKT          ((grp_u32)0x00080000)           /* 01: 1 packet     */
#define CYCLONEVD_B07_EP0_XFERSIZE      ((grp_u32)0x0000007F)           /* Transfer size (for endpoint 0)   */
#define CYCLONEVD_B19_XFERSIZE          ((grp_u32)0x0007FFFF)           /* Transfer size                    */

/* OTG device IN endpoint-x DMA address register (DIEPDMAx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B32_DIEPDMAx          ((grp_u32)0xFFFFFFFF)           /* DMA address  */

/* OTG device IN endpoint-x transmit FIFO status register (DTXFSTSx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B16_INEPTXFSPCAVAIL   ((grp_u32)0x0000FFFF)           /* IN endpoint TxFIFO space available */

/* OTG device IN endpoint-x DMA buffer address register (DIEPDMABx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B32_DIEPDMABx          ((grp_u32)0xFFFFFFFF)           /* DMA buffer address  */

/* OTG device OUT endpoint-x control register (DOEPCTLx) (x = 0..15, where x = Endpoint_number) */
//#define CYCLONEVD_B01_EPENA           ((grp_u32)0x80000000)           /* Endpoint enable                                          */
//#define CYCLONEVD_B01_EPDIS           ((grp_u32)0x40000000)           /* Endpoint disable                                         */
//#define CYCLONEVD_B01_SODDFRM         ((grp_u32)0x20000000)           /* Set odd frame (isochronous IN endpoints only)            */
//#define CYCLONEVD_B01_SETD1PID        ((grp_u32)0x20000000)           /* Set DATA1 PID (interrupt/bulk IN endpoints only)         */
//#define CYCLONEVD_B01_SEVNFRM         ((grp_u32)0x10000000)           /* Set even frame (isochronous IN endpoints only)           */
//#define CYCLONEVD_B01_SETD0PID        ((grp_u32)0x10000000)           /* Set DATA0 PID (interrupt/bulk IN endpoints only)         */
//#define CYCLONEVD_B01_SNAK            ((grp_u32)0x08000000)           /* Set NAK                                                  */
//#define CYCLONEVD_B01_CNAK            ((grp_u32)0x04000000)           /* Clear NAK                                                */
//#define CYCLONEVD_B01_STALL           ((grp_u32)0x00200000)           /* STALL handshake                                          */
#define CYCLONEVD_B01_SNP               ((grp_u32)0x00100000)           /* Snoop mode                                               */
//#define CYCLONEVD_B02_EPTYPE          ((grp_u32)0x000C0000)           /* Endpoint type                                            */
//    #define CYCLONEVD_VEPTYP_CONTROL      ((grp_u32)0x00000000)           /* 00: Control      */
//    #define CYCLONEVD_VEPTYP_ISOCHRONOUS  ((grp_u32)0x00040000)           /* 01: Isochronous  */
//    #define CYCLONEVD_VEPTYP_BULK         ((grp_u32)0x00080000)           /* 10: Bulk         */
//    #define CYCLONEVD_VEPTYP_INTERRUP     ((grp_u32)0x000C0000)           /* 11: Interrupt    */
//#define CYCLONEVD_B01_NAKSTS          ((grp_u32)0x00020000)           /* NAK status                                               */
//#define CYCLONEVD_B01_EONUM           ((grp_u32)0x00010000)           /* Even/odd frame (isochronous OUT endpoints only)          */
//#define CYCLONEVD_B01_DPID            ((grp_u32)0x00010000)           /* Endpoint data PID (interrupt/bulk OUT endpoints only)    */
//#define CYCLONEVD_B01_USBACTEP        ((grp_u32)0x00008000)           /* USB active endpoint                                      */
//#define CYCLONEVD_B02_EP0_MPS         ((grp_u32)0x00000003)           /* Maximum packet size (endpoint 0)                         */
//    #define CYCLONEVD_VEP0_MPS_BYTES64  ((grp_u32)0x00000000)           /* 00: 64bytes    */
//    #define CYCLONEVD_VEP0_MPS_BYTES32  ((grp_u32)0x00000001)           /* 01: 32bytes    */
//    #define CYCLONEVD_VEP0_MPS_BYTES16  ((grp_u32)0x00000002)           /* 10: 16bytes    */
//    #define CYCLONEVD_VEP0_MPS_BYTES8   ((grp_u32)0x00000003)           /* 11:  8bytes    */
//#define CYCLONEVD_B11_MPS             ((grp_u32)0x000007FF)           /* Maximum packet size                                      */

/* OTG device OUT endpoint-x interrupt register (DOEPINTx) (x = 0..15, where x = Endpoint_number) */
//#define CYCLONEVD_B01_NYETINTRPT      ((grp_u32)0x00004000)           /* NYET interrupt                                                           */
//#define CYCLONEVD_B01_NAKINTRPT       ((grp_u32)0x00002000)           /* NAK interrupt                                                            */
//#define CYCLONEVD_B01_BBLEERR         ((grp_u32)0x00001000)           /* Babble error interrupt                                                   */
//#define CYCLONEVD_B01_PKTDRPSTS       ((grp_u32)0x00000800)           /* Packet dropped status                                                    */
//#define CYCLONEVD_B01_BNAINTR         ((grp_u32)0x00000200)           /* Buffer not available interrupt                                           */
#define CYCLONEVD_B01_OUTPKTERR         ((grp_u32)0x00000100)           /* OUT packet error                                                         */
#define CYCLONEVD_B01_BACK2BACKSETUP    ((grp_u32)0x00000040)           /* Back-to-back SETUP packets received                                      */
#define CYCLONEVD_B01_STSPHSERCVD       ((grp_u32)0x00000020)           /* Status phase received for control write (control OUT endpoints only)     */
#define CYCLONEVD_B01_OUTTKNEPDIS       ((grp_u32)0x00000010)           /* OUT token received when endpoint disabled (control OUT endpoints only)   */
#define CYCLONEVD_B01_SETUP             ((grp_u32)0x00000008)           /* SETUP Phase Done (control OUT endpoints only)                            */
//#define CYCLONEVD_B01_AHBERR          ((grp_u32)0x00000004)           /* AHB error                                                                */
//#define CYCLONEVD_B01_EPDISBLD        ((grp_u32)0x00000002)           /* Endpoint disabled interrupt                                              */
//#define CYCLONEVD_B01_XFERCOMPL       ((grp_u32)0x00000001)           /* Transfer completed interrupt                                             */

/* OTG device OUT endpoint-x transfer size register (DOEPTSIZx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B02_SUPCNT            ((grp_u32)0x60000000)           /* SETUP packet count (for endpoint 0)                  */
    #define CYCLONEVD_VSUPCNT_ONEPACKET     ((grp_u32)0x20000000)           /* 01: 1 packet     */
    #define CYCLONEVD_VSUPCNT_TWOPACKET     ((grp_u32)0x40000000)           /* 10: 2 packets    */
    #define CYCLONEVD_VSUPCNT_THREEPACKET   ((grp_u32)0x60000000)           /* 11: 3 packets    */
#define CYCLONEVD_B02_RXDPID            ((grp_u32)0x60000000)           /* SETUP packet count (isochronous OUT endpoints only)  */
    #define CYCLONEVD_VRXDPID_DATA0         ((grp_u32)0x00000000)           /* 00: DATA0                */
    #define CYCLONEVD_VRXDPID_DATA2PACKET1  ((grp_u32)0x20000000)           /* 01: DATA2 or 1 packet    */
    #define CYCLONEVD_VRXDPID_DATA1PACKET2  ((grp_u32)0x40000000)           /* 10: DATA1 or 2 packets   */
    #define CYCLONEVD_VRXDPID_MDATAPACKET3  ((grp_u32)0x60000000)           /* 11: MDATA or 3 packets   */
#define CYCLONEVD_B01_EP0_PKTCNT        ((grp_u32)0x00080000)           /* Packet count (for endpoint 0)                        */
//#define CYCLONEVD_B10_PKTCNT          ((grp_u32)0x1FF80000)           /* Packet count                                         */
//  #define CYCLONEVD_VPKTCNT_1PKT          ((grp_u32)0x00080000)           /* 01: 1 packet     */
//#define CYCLONEVD_B07_EP0_XFERSIZE    ((grp_u32)0x0000007F)           /* Transfer size (for endpoint 0)                       */
//#define CYCLONEVD_B19_XFERSIZE        ((grp_u32)0x0007FFFF)           /* Transfer size                                        */

/* OTG device OUT endpoint-x DMA address register (DOEPDMAx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B32_DOEPDMAx          ((grp_u32)0xFFFFFFFF)           /* DMA address  */

/* OTG device OUT endpoint-x DMA buffer address register (DOEPDMABx) (x = 0..15, where x = Endpoint_number) */
#define CYCLONEVD_B32_DOEPDMABx          ((grp_u32)0xFFFFFFFF)           /* DMA buffer address  */

/*----------------------------------------------------------------------------------------------*/
/*  Power and clock gating control and status registers                                         */
/*----------------------------------------------------------------------------------------------*/
/* OTG power and clock gating control register (PCGCCTL) */
#define CYCLONEV_B01_L1SUSPENDED        ((grp_u32)0x00000080)           /* Deep sleep               */
#define CYCLONEV_B01_PHYSLEEP           ((grp_u32)0x00000040)           /* PHY in sleep             */
#define CYCLONEV_B01_RSTPDWNMODULE      ((grp_u32)0x00000008)           /* Reset power-down modules */
#define CYCLONEV_B01_STOPPCLK           ((grp_u32)0x00000001)           /* Stop PHY clock (phy_clk) */


#endif  /* _GRP_CYCLONEV_BIT_VAL_H_ */

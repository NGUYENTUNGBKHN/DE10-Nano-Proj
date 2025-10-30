/********************************************************************************/
/*                Copyright (C) 2002-2017 Grape Systems, Inc.                   */
/*                     All Rights Reserved.                                     */
/*                                                                              */
/*  This software is furnished under a license and may be used and copied only  */
/*  in accordance with the terms of such license and with the inclusion of the  */
/*  above copyright notice. No title to and ownership of the software is        */
/*  transferred.                                                                */
/*  Grape Systems Inc. makes no warranties with respect to the performance of   */
/*  this computer program, and specifically disclaims any responsibility for    */
/*  any damages, special or consequential, connected with the use of this       */
/*  program.                                                                    */
/*                                                                              */
/********************************************************************************/

/********************************************************************************/
/*                                                                              */
/* FILE NAME                                                                    */
/*                                                                              */
/*      sil.h                                                                   */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*                                                                              */
/*      System interface layer for ThreadX-uITRON                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*                                                                              */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DEPENDENCIES                                                                 */
/*                                                                              */
/*      NONE                                                                    */
/*                                                                              */
/* HISTORY                                                                      */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/

#ifndef _SIL_H_
#define _SIL_H_

#include <tx_port.h>
#include <itron.h>

#ifdef __cplusplus
extern "C"{
#endif

#define SIL_ENDIAN_LITTLE   0
#define SIL_ENDIAN_BIG      1

#ifndef SIL_ENDIAN
    #define SIL_ENDIAN  SIL_ENDIAN_LITTLE
#endif

/* Interrupt control */
#ifndef SIL_PRE_LOC
    #define SIL_PRE_LOC     TX_INTERRUPT_SAVE_AREA
    #define SIL_LOC_INT()   TX_DISABLE
    #define SIL_UNL_INT()   TX_RESTORE
    #define ISIL_LOC_INT    SIL_LOC_INT
#endif

#ifndef sil_swap16
    #define sil_swap16(x)   ((((x)<<8)&0xff00)|(((x)&0xff00)>>8))
#endif
#ifndef sil_swap32
    #define sil_swap32(x)   ((sil_swap16((x)&0x0000ffffL)<<16)|sil_swap16(((x)&0xffff0000L)>>16))
#endif

/* Small time wait */
#ifndef sil_dly_nse
    #define sil_dly_nse(x)  { volatile unsigned int xx = (x); while(0<xx) xx--; }
#endif

/* 8bit access */
#ifndef sil_reb_mem
    #define     sil_reb_mem(mem)        (*((volatile char*)(mem)))
#endif
#ifndef sil_wrb_mem
    #define     sil_wrb_mem(mem,data)   ((*((volatile char*)(mem)))=(char)(data))
#endif

/* 16bit access */
#ifndef sil_reh_mem
    #define     sil_reh_mem(mem)        (*((volatile short*)(mem)))
#endif
#ifndef sil_reh_lem
    #if SIL_ENDIAN==SIL_ENDIAN_LITTLE
        #define     sil_reh_lem(mem)        (*((volatile short*)(mem)))
    #else
        #define     sil_reh_lem(mem)        sil_swap16(*((volatile short*)(mem)))
    #endif
#endif
#ifndef sil_reh_bem
    #if SIL_ENDIAN==SIL_ENDIAN_LITTLE
        #define     sil_reh_bem(mem)        sil_swap16(*((volatile short*)(mem)))
    #else
        #define     sil_reh_bem(mem)        (*((volatile short*)(mem)))
    #endif
#endif

#ifndef sil_wrh_mem
    #define     sil_wrh_mem(mem,data)   ((*((volatile short*)(mem)))=(short)(data))
#endif
#ifndef sil_wrh_lem
    #if SIL_ENDIAN==SIL_ENDIAN_LITTLE
        #define     sil_wrh_lem(mem,data)   ((*((volatile short*)(mem)))=(short)(data))
    #else
        #define     sil_wrh_lem(mem,data)   ((*((volatile short*)(mem)))=sil_swap16((short)data))
    #endif
#endif
#ifndef sil_wrh_bem
    #if SIL_ENDIAN==SIL_ENDIAN_LITTLE
        #define     sil_wrh_bem(mem,data)   ((*((volatile short*)(mem)))=sil_swap16((short)data))
    #else
        #define     sil_wrh_bem(mem,data)   ((*((volatile short*)(mem)))=(short)(data))
    #endif
#endif

/* 32bit access */
#ifndef sil_rew_mem
    #define     sil_rew_mem(mem)        (*((volatile long*)(mem)))
#endif
#ifndef sil_rew_lem
    #if SIL_ENDIAN==SIL_ENDIAN_LITTLE
        #define     sil_rew_lem(mem)        (*((volatile long*)(mem)))
    #else
        #define     sil_rew_lem(mem)        sil_swap32(*((volatile long*)(mem)))
    #endif
#endif
#ifndef sil_rew_bem
    #if SIL_ENDIAN==SIL_ENDIAN_LITTLE
        #define     sil_rew_bem(mem)        sil_swap32(*((volatile long*)(mem)))
    #else
        #define     sil_rew_bem(mem)        (*((volatile long*)(mem)))
    #endif
#endif

#ifndef sil_wrw_mem
    #define     sil_wrw_mem(mem,data)   ((*((volatile long*)(mem)))=(long)(data))
#endif
#ifndef sil_wrw_lem
    #if SIL_ENDIAN==SIL_ENDIAN_LITTLE
        #define     sil_wrw_lem(mem,data)   ((*((volatile long*)(mem)))=(long)(data))
    #else
        #define     sil_wrw_lem(mem,data)   ((*((volatile long*)(mem)))=sil_swap32((long)data))
    #endif
#endif
#ifndef sil_wrw_bem
    #if SIL_ENDIAN==SIL_ENDIAN_LITTLE
        #define     sil_wrw_bem(mem,data)   ((*((volatile long*)(mem)))=sil_swap32((long)data))
    #else
        #define     sil_wrw_bem(mem,data)   ((*((volatile long*)(mem)))=(long)(data))
    #endif
#endif

#ifdef __cplusplus
}
#endif 

#endif /* _SIL_H_ */


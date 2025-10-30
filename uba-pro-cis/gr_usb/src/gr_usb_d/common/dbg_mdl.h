/****************************************************************************/
/*                                                                          */
/*              Copyright(C) 2002-2006 Grape Systems, Inc.                  */
/*                        All Rights Reserved                               */
/*                                                                          */
/* This software is furnished under a license and may be used and copied    */
/* only in accordance with the terms of such license and with the inclusion */
/* of the above copyright notice. No title to and ownership of the software */
/* is transferred.                                                          */
/* Grape Systems Inc. makes no representation or warranties with respect to */
/* the performance of this computer program, and specifically disclaims any */
/* responsibility for any damages, special or consequential, connected with */
/* the use of this program.                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILE NAME                                                    VERSION     */
/*                                                                          */
/*      dbg_mdl.h                                                 1.10      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      �f�o�b�O�p�̊֐����O�@�\                                            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  K.Takagi    2003/11/26  Created initial version 1.00                    */
/*  K.Handa     2006/12/27  Version was updated to 1.10                     */
/*                                                                          */
/****************************************************************************/
#ifndef _DBG_MDL_H_
#define _DBG_MDL_H_


#define GR_DBG_MDL  /* �R���p�C���I�v�V�������킩��Ȃ����߁A�Ƃ肠���������Œ�`   */
                    /*                                          2003/11/28 K.Takagi */



#ifdef  GR_DBG_MDL
/*----- ���O�@�\��L���ɂ���ꍇ -----*/

/* �g���[�X���̐ݒ� */
#define DBG_MAX_TRACE       8000                     /* Trace Buffer number  */

/* �f�o�b�O�g���[�X�̃��[�h�ݒ�*/
/*�i�ȉ��̒�`�̂���������u1�v�A���́u0�v��ݒ肵�Ă��������j */
#define GR_DBG_MDL_8BIT_MODE    1
#define GR_DBG_MDL_16BIT_MODE   0
#define GR_DBG_MDL_32BIT_MODE   0

/* The Trace information for Debug is set. */
#if GR_DBG_MDL_8BIT_MODE
void    GRDBG_DebugTrace( unsigned char, unsigned char, unsigned char, unsigned char);
#define GRDBG_TRACE(x,y,m,n)    GRDBG_DebugTrace((unsigned char)x,(unsigned char)y,(unsigned char)m,(unsigned char)n)
void    GRDBG_DebugTrace2( unsigned char, unsigned char, unsigned char, unsigned char);
#define GRDBG_TRACE2(x,y,m,n)    GRDBG_DebugTrace2((unsigned char)x,(unsigned char)y,(unsigned char)m,(unsigned char)n)

#elif GR_DBG_MDL_16BIT_MODE
void    GRDBG_DebugTrace( unsigned short, unsigned short, unsigned short, unsigned short);
#define GRDBG_TRACE(x,y,m,n)    GRDBG_DebugTrace((unsigned short)x,(unsigned short)y,(unsigned short)m,(unsigned short)n)
void    GRDBG_DebugTrace2( unsigned short, unsigned short, unsigned short, unsigned short);
#define GRDBG_TRACE2(x,y,m,n)    GRDBG_DebugTrace2((unsigned short)x,(unsigned short)y,(unsigned short)m,(unsigned short)n)

#elif GR_DBG_MDL_32BIT_MODE
void    GRDBG_DebugTrace( unsigned long, unsigned long, unsigned long, unsigned long);
#define GRDBG_TRACE(x,y,m,n)    GRDBG_DebugTrace((unsigned long)x,(unsigned long)y,(unsigned long)m,(unsigned long)n)
void    GRDBG_DebugTrace2( unsigned long, unsigned long, unsigned long, unsigned long);
#define GRDBG_TRACE2(x,y,m,n)    GRDBG_DebugTrace2((unsigned long)x,(unsigned long)y,(unsigned long)m,(unsigned long)n)

#endif

#else   /* GR_DBG_MDL */
/*----- ���O�@�\�𖳌��ɂ���ꍇ -----*/
#define     GRDBG_TRACE(x,y,m,n)
#define     GRDBG_TRACE2(x,y,m,n)

#endif  /* GR_DBG_MDL */

/* GRDBG_TRACE(x,y,m,n)
 * GRDBG_TRACE2(x,y,m,n)
 *
 * x: File(module) number
 * y: Function number
 * m: Sequence number
 * n: marker
 *    ( "STR_FUNC"   is start marker
 *      "END_FUNC"   is end marker
 *       "STED_FUNC" is the marker that indicate start and end is same.
 *      "SEQ_FUNC" is sequence marker
 *    )
 *
 */






/*--------------------*/
/*  Module Code list  */
/*--------------------*/
#define GRDBG_COCL  (0x01)
#define GRDBG_CSM   (0x02)
#define GRDBG_CORS  (0x03)
#define GRDBG_CH    (0x04)
#define GRDBG_HC    (0x05)
#define GRDBG_RH    (0x06)
#define GRDBG_PTTK  (0x07)
#define GRDBG_GPLT  (0x08)
#define GRDBG_PT    (0x09)

#define GRDBG_USBD  (0x10)
#define GRDBG_HUBD  (0x20)
#define GRDBG_HUBDI (0x21)

#define GRDBG_CNFS  (0x30)
#define GRDBG_BOT   (0x40)
#define GRDBG_8070i (0x41)
#define GRDBG_8070s (0x42)
#define GRDBG_FSIF  (0x50)

#define GRDBG_PERI_CTL	(0x60)
#define GRDBG_PERI_CYC	(0x61)
#define GRDBG_PERI_PRM	(0x62)
#define GRDBG_PERI_STS	(0x63)
#define GRDBG_PERI_TRS	(0x64)
#define GRDBG_PERID		(0x65)
#define GRDBG_COMD_CNF	(0x70)
#define GRDBG_COMD		(0x71)

#define END_FUNC    (0xFFFFFFFF)
#define STR_FUNC    (0x00000000)
#define STED_FUNC   (0x80000000)
#define SEQ_FUNC    (0x00000001)
#endif /* _DBG_MDL_H_ */

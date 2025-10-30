#ifndef	_GRP_FS_TRACE_H_
#define	_GRP_FS_TRACE_H_

#ifdef	GRP_FS_TRACE
/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_trace.h												*/
/* VERSION:		1.0															*/
/* DESCRIPTION:																*/
/* 		Trace routine for GR-FILE											*/
/* FUNCTIONS:																*/
/*		grp_fs_trace_init			initialize trace						*/
/*		grp_fs_btrace				trace I/O buffer operation				*/
/*		grp_fs_btraceX				trace I/O buffer operation				*/
/*		grp_fs_iotrace				trace I/O operation						*/
/*		grp_fs_ftrace				trace file operation					*/
/*		grp_fs_dump_trace			dump trace								*/
/*		grp_fs_set_trace_mode		set trace mode							*/
/* DEPENDENCIES:															*/
/*		grp_fs.h															*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/09/01	Created inital version 1.0				*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003 Grape Systems, Inc.,  All Rights Reserved.             */
/*                                                                          */
/* This software is furnished under a license, and may be used only in      */
/* accordance with the terms of such license and with the inclusion of the  */
/* above copyright notice.  No title to and the ownership of the software   */
/* is transferred.                                                          */
/* Grape Systems Inc. makes no representation or warranties with respect to */
/* non-infringement of any intellectual property rights and the performance */
/* of this computer program, and specifically disclaims any responsibility  */
/* for any damages, special or consequential, connected  with the use of    */
/* this program.                                                            */
/****************************************************************************/

#include "grp_fs.h"
/*
 * trace value
 */
#define GRP_FS_BT_ALC	0x0000		/* alloc buffer */
#define GRP_FS_BT_INT	0x0001		/* init buffer */
#define GRP_FS_BT_FND	0x0002		/* found */
#define GRP_FS_BT_RCB	0x0004		/* check again after wait buffer */
#define GRP_FS_BT_RCF	0x0005		/* check again after wait fill */
#define GRP_FS_BT_RCW	0x0006		/* check again after write error */
#define GRP_FS_BT_WTB	0x0010		/* wait buffer */
#define GRP_FS_BT_WTF	0x0011		/* wait fill */
#define GRP_FS_BT_WTM	0x0012		/* wait mod */
#define GRP_FS_BT_WKB	0x0020		/* wakeup buffer */
#define GRP_FS_BT_WKF	0x0021		/* wakeup fill */
#define GRP_FS_BT_WKM	0x0022		/* wakeup mod */
#define GRP_FS_BT_WKW	0x0023		/* wakeup write */
#define GRP_FS_BT_STR	0x0041		/* start read */
#define GRP_FS_BT_ENR	0x0042		/* end read */
#define GRP_FS_BT_ERR	0x0043		/* error read */
#define GRP_FS_BT_ENF	0x0044		/* end buffer fill */
#define GRP_FS_BT_STW	0x0048		/* start write */
#define GRP_FS_BT_ENW	0x0049		/* end write */
#define GRP_FS_BT_ERW	0x004a		/* error write */
#define GRP_FS_BT_OTW	0x004b		/* write by others */
#define GRP_FS_BT_URB	0x0080		/* unref buf */
#define GRP_FS_BT_BMD	0x0081		/* block mod */
#define GRP_FS_BT_UBM	0x0082		/* unblock mod */
#define GRP_FS_BT_INV	0x00ff		/* invalidate */
#define GRP_FS_IT_RFS	0x0100		/* read file start */
#define GRP_FS_IT_RFE	0x0101		/* read file end */
#define GRP_FS_IT_WFS	0x0102		/* write file start */
#define GRP_FS_IT_WFE	0x0103		/* write file end */
#define GRP_FS_FT_CRF	0x0200		/* create file */
#define GRP_FS_FT_RMF	0x0201		/* unlink file */
#define GRP_FS_FT_RNF	0x0202		/* rename file */
#define GRP_FS_FT_FRE	0x0203		/* free directory entry */

#define GRP_FS_TRC_TYPE(x)	((x) & 0xff00)		/* trace type */
#define GRP_FS_TRC_TBUF		0x0000				/* buffer trace */
#define GRP_FS_TRC_TIO		0x0100				/* I/O trace */
#define GRP_FS_TRC_TFOP		0x0200				/* file trace */

/* 
 * trace mode
 */
#define GRP_FS_TRC_BUF	0x0001		/* trace buffer */
#define GRP_FS_TRC_IO	0x0002		/* trace I/O */
#define GRP_FS_TRC_FOP	0x0004		/* trace file op */

/*
 * buffer trace data
 */
typedef struct grp_fs_trc {
	unsigned short	op;				/* op */
	unsigned short	stat;			/* status */
	int				dev;			/* dev */
	grp_uint32_t	blk;			/* block */
	void			*vp;			/* misc pointer */
	int				misc1;			/* misc1 */
	int				misc2;			/* misc2 */
	int				pid;			/* pid */
	grp_uchar_t		data[16];		/* data */
}  grp_fs_trc_t;

#define TRC_CNT			0x4000		/* trace count */

typedef struct grp_fs_trc_ctl {		/* trc control block */
	int				mode;			/* trace mode */
	int				cnt;			/* trace count */
	grp_fs_trc_t	trc[TRC_CNT];	/* trace buffer */
} grp_fs_trc_ctl_t;

void grp_fs_trace_init(void);		/* init trace */
void grp_fs_btrace(grp_fs_buf_t *bp, int op);
void grp_fs_btraceX(int dev, grp_uint32_t blk, int type, int op);
void grp_fs_iotrace(
		int dev, grp_uint32_t blk, grp_uint32_t off, 
		int size, grp_uchar_t *bp, int op);
void grp_fs_ftrace(
		int dev, grp_uint32_t clst, 
		grp_uint32_t dclst, grp_uint32_t doff, 
		grp_fs_file_t *fp, grp_uchar_t *name, int op);
void grp_fs_dump_trace(const char *file);
void grp_fs_set_trace_mode(grp_uchar_t *mode_str);

#else	/* GRP_FS_TRACE */

#define grp_fs_trace_init()
#define grp_fs_btrace(bp, op)
#define grp_fs_btraceX(dev, blk, type, op)
#define grp_fs_iotrace(dev, blk, off, size, bp, op)
#define grp_fs_ftrace(dev, clst, dclst, doff, fp, name, op)
#define grp_fs_dump_trace(file)
#define grp_fs_set_trace_mode(mode_str)

#endif	/* GRP_FS_TRACE */

#endif	/* _GRP_FS_TRACE_H_ */

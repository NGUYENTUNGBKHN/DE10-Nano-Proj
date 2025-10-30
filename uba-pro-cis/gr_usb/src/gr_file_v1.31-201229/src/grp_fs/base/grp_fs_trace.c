/****************************************************************************/
/*																			*/
/* FILE NAME:	grp_fs_trace.c												*/
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
/*		None																*/
/* HISTORY:																	*/
/*		NAME			DATE		REMARKS									*/
/*		T.Imashiki		2003/09/01	Created inital version 1.0				*/
/*		K.Kaneko		2008/05/21	Added include grp_fs_sysdef.h			*/
/*		K.Kaneko		2010/11/16	Fixed include position to				*/
/*									 grp_fs_sysdef.h						*/
/*																			*/
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* Copyright(C) 2003-2010 Grape Systems, Inc.,  All Rights Reserved.        */
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

#include "grp_fs_sysdef.h"
#ifdef	GRP_FS_TRACE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include "grp_fs_trace.h"
#include "../test_cmd/test_cmd.h"

static grp_fs_trc_ctl_t *grp_fs_trc_ctl = NULL;	/* trace control */

/*
 * init trace
 */
void grp_fs_trace_init(void)
{
	char *area;

	area = mmap(NULL, sizeof(grp_fs_trc_ctl_t), PROT_READ|PROT_WRITE, 
				MAP_ANON|MAP_SHARED, -1, 0);
	if (area == MAP_FAILED) {
		test_cmd_printf("cannot allocate shared memory\n");
		exit(1);
	}
	memset(area, 0, sizeof(grp_fs_trc_ctl_t));
	grp_fs_trc_ctl = (grp_fs_trc_ctl_t *)area;
}

/*
 * trace I/O buffer operation
 */
void grp_fs_btrace(grp_fs_buf_t *bp, int op)
{
	grp_fs_trc_t *tp;

	if ((grp_fs_trc_ctl->mode & GRP_FS_TRC_BUF) == 0)
		return;
	tp = &grp_fs_trc_ctl->trc[grp_fs_trc_ctl->cnt++ % TRC_CNT];
	tp->op = op;
	tp->stat = bp->usStatus;
	tp->dev = bp->iDev;
	tp->blk = bp->uiBlk;
	tp->misc1 = bp->iRefCnt;
	tp->misc2 = bp->iSize;
	tp->vp = bp;
	tp->pid = getpid();
	memcpy(tp->data, bp->pucData, sizeof(tp->data));
}

/*
 * trace I/O buffer operation
 */
void grp_fs_btraceX(int dev, grp_uint32_t blk, int type, int op)
{
	grp_fs_trc_t *tp;

	if ((grp_fs_trc_ctl->mode & GRP_FS_TRC_BUF) == 0)
		return;
	tp = &grp_fs_trc_ctl->trc[grp_fs_trc_ctl->cnt++ % TRC_CNT];
	tp->op = op;
	tp->stat = type;
	tp->dev = dev;
	tp->blk = blk;
	tp->misc1 = 0;
	tp->misc2 = 0;
	tp->vp = NULL;
	tp->pid = getpid();
}

/*
 * trace I/O operation
 */
void grp_fs_iotrace(
	int dev, grp_uint32_t blk, grp_uint32_t off, int size, grp_uchar_t *bp, int op)
{
	grp_fs_trc_t *tp;

	if ((grp_fs_trc_ctl->mode & GRP_FS_TRC_IO) == 0)
		return;
	tp = &grp_fs_trc_ctl->trc[grp_fs_trc_ctl->cnt++ % TRC_CNT];
	tp->op = op;
	tp->stat = 0;
	tp->dev = dev;
	tp->blk = blk;
	tp->misc1 = off;
	tp->misc2 = size;
	tp->vp = bp;
	tp->pid = getpid();
	memcpy(tp->data, bp, sizeof(tp->data));
}

/*
 * trace file op
 */
void grp_fs_ftrace(int dev, grp_uint32_t clst, 
				grp_uint32_t dclst, grp_uint32_t doff, 
				grp_fs_file_t *fp, grp_uchar_t *name, int op)
{
	grp_fs_trc_t *tp;

	if ((grp_fs_trc_ctl->mode & GRP_FS_TRC_FOP) == 0)
		return;
	tp = &grp_fs_trc_ctl->trc[grp_fs_trc_ctl->cnt++ % TRC_CNT];
	tp->op = op;
	tp->stat = 0;
	tp->dev = dev;
	tp->blk = clst;
	tp->misc1 = dclst;
	tp->misc2 = doff;
	tp->vp = fp;
	tp->pid = getpid();
	strncpy(tp->data, name, sizeof(tp->data));
}

/*
 * dump trace
 */
void grp_fs_dump_trace(const char *file)
{
	FILE *fp;
	grp_fs_trc_t *tp;
	int count;
	int i, j;
	const char *str;

	if ((fp = fopen(file, "w")) == NULL) {
		test_cmd_printf("cannot create %s\n", file);
		return;
	}
	if (grp_fs_trc_ctl->cnt > TRC_CNT) {
		tp = &grp_fs_trc_ctl->trc[grp_fs_trc_ctl->cnt % TRC_CNT];
		count = TRC_CNT;
	} else {
		tp = &grp_fs_trc_ctl->trc[0];
		count = grp_fs_trc_ctl->cnt;
	}
	for (i = grp_fs_trc_ctl->cnt - count; count > 0; count--, i++) {
		fprintf(fp, "%08d: ", i);
		switch(tp->op) {
		case GRP_FS_BT_ALC:
			str = "ALLOC";
			break;
		case GRP_FS_BT_INT:
			str = "INIT ";
			break;
		case GRP_FS_BT_FND:
			str = "FOUND";
			break;
		case GRP_FS_BT_RCB:
			str = "RCBUF";
			break;
		case GRP_FS_BT_RCF:
			str = "RCFIL";
			break;
		case GRP_FS_BT_RCW:
			str = "RCWT ";
			break;
		case GRP_FS_BT_WTB:
			str = "WTBUF";
			break;
		case GRP_FS_BT_WTF:
			str = "WTFIL";
			break;
		case GRP_FS_BT_WTM:
			str = "WTMOD";
			break;
		case GRP_FS_BT_WKB:
			str = "WKBUF";
			break;
		case GRP_FS_BT_WKF:
			str = "WKFIL";
			break;
		case GRP_FS_BT_WKM:
			str = "WKMOD";
			break;
		case GRP_FS_BT_WKW:
			str = "WKWT ";
			break;
		case GRP_FS_BT_STR:
			str = "ST_RD";
			break;
		case GRP_FS_BT_ENR:
			str = "ED_RD";
			break;
		case GRP_FS_BT_ERR:
			str = "ERR_R";
			break;
		case GRP_FS_BT_ENF:
			str = "ED_FL";
			break;
		case GRP_FS_BT_STW:
			str = "ST_WT";
			break;
		case GRP_FS_BT_ENW:
			str = "ED_WT";
			break;
		case GRP_FS_BT_ERW:
			str = "ERR_W";
			break;
		case GRP_FS_BT_OTW:
			str = "OTH_W";
			break;
		case GRP_FS_BT_URB:
			str = "UNREF";
			break;
		case GRP_FS_BT_BMD:
			str = "BLK_M";
			break;
		case GRP_FS_BT_UBM:
			str = "UNB_M";
			break;
		case GRP_FS_BT_INV:
			str = "INV_B";
			break;
		case GRP_FS_IT_RFS:
			str = "IOR_S";
			break;
		case GRP_FS_IT_RFE:
			str = "IOR_E";
			break;
		case GRP_FS_IT_WFS:
			str = "IOW_S";
			break;
		case GRP_FS_IT_WFE:
			str = "IOW_E";
			break;
		case GRP_FS_FT_CRF:
			str = "FL_CT";
			break;
		case GRP_FS_FT_RMF:
			str = "FL_RM";
			break;
		case GRP_FS_FT_RNF:
			str = "FL_RN";
			break;
		case GRP_FS_FT_FRE:
			str = "FL_FE";
			break;
		default:		  
			str = "?????";
			break;
		}
		switch(GRP_FS_TRC_TYPE(tp->op)) {
		case GRP_FS_TRC_TBUF: /* buffer trace */
			fprintf(fp, "%s(%x,%04x) s:%04x r:%d z:%d b:%08x p:%d ",
					str, tp->dev, tp->blk, tp->stat, tp->misc1, tp->misc2,
					(unsigned int)tp->vp, tp->pid);
			break;
		case GRP_FS_TRC_TIO: /* I/O trace */
			fprintf(fp, "%s(%x,%04x) o:0x%08x z:%d b:%08x p:%d ",
					str, tp->dev, tp->blk, tp->misc1, tp->misc2, 
					(unsigned int)tp->vp, tp->pid);
			break;
		case GRP_FS_TRC_TFOP: /* file op */
			fprintf(fp, "%s(%x,%04x) d:%04x(%d-%d) f:%08x p:%d ",
					str, tp->dev, tp->blk, tp->misc1, (tp->misc2 & 0xffff),
					((tp->misc2 >> 16) & 0xffff), (unsigned int)tp->vp, 
					tp->pid);
		}
		if (tp->vp || GRP_FS_TRC_TYPE(tp->op) == GRP_FS_TRC_TFOP) {
			fprintf(fp, "*");
			for (j = 0; j < sizeof(tp->data); j++)
				fprintf(fp, "%c", (isprint(tp->data[j]))? tp->data[j]: '.');
			fprintf(fp, "*");
		}
		fprintf(fp, "\n");
		tp++;
		if (tp >= &grp_fs_trc_ctl->trc[TRC_CNT])
			tp = grp_fs_trc_ctl->trc;
	}
	fclose(fp);
}

/*
 * change trace mode
 */
void grp_fs_set_trace_mode(grp_uchar_t *mode_str)
{
	int mode = 0;

	for ( ; *mode_str; mode_str++) {
		switch(*mode_str) {
		case 'b':
			mode |= GRP_FS_TRC_BUF;
			continue;
		case 'i':
			mode |= GRP_FS_TRC_IO;
			continue;
		case 'f':
			mode |= GRP_FS_TRC_FOP;
			continue;
		default:
			test_cmd_printf("unknown trace mode '%c'\n", *mode_str);
			return;
		}
	}
	grp_fs_trc_ctl->mode = mode;
}
#endif	/* GRP_FS_TRACE */

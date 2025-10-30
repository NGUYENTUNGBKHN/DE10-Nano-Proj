/*************************************************************************/
/*                                                                       */
/* Copyright(C) 2002-2016 Grape Systems, Inc.                            */
/*                        All Rights Reserved                            */
/*                                                                       */
/* This software is furnished under a license and may be used and copied */
/* only in accordance with the terms of such license and with the        */
/* inclusion of the above copyright notice. No title to and ownership of */
/* the software is transferred.                                          */
/* Grape Systems Inc. makes no representation or warranties with respect */
/* to the performance of this computer program, and specifically         */
/* disclaims any responsibility for any damages, special or              */
/* consequential, connected with the use of this program.                */
/*                                                                       */
/*************************************************************************/
/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      cmd_prog.c                                         2.04          */
/*                                                                       */
/* DESCRIPTION:                                                          */
/*                                                                       */
/*      GR-FILE PROGRAM I/F Sample program for GR-USB/HOST#              */
/*                                                                       */
/* FUNCTIONS:                                                            */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*   NAME         DATE        REMARKS                                    */
/*                                                                       */
/*   K.Kaneko     2008/06/23  Version 0.02                               */
/*   K.Takagi     2009/03/24  Version 2.00                               */
/*                            不要なヘッダファイルを削除                 */
/*   K.Kaneko     2010/11/17  Version 2.03                               */
/*                            Added GRP_FS_MINIMIZE_LEVEL option for     */
/*                            GR-FILE minimize level                     */
/*   K.Kaneko     2016/03/17  Version 2.04                               */
/*                            Supported compile option                   */
/*                            GRP_FS_ENABLE_OVER_2G                      */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/* includes                                                              */
/*************************************************************************/
#include <stdarg.h>
#include <string.h>

#include "grp_vos.h"

#include "fat.h"
#include "grp_fs_if.h"
#include "grp_fs_conv.h"
#include "grp_time.h"
#include "grp_fs_disk_part.h"
#include "grp_fat_format.h"
#include "grp_fs_proc_event.h"
#include "grp_fs_readdir.h"
#include "grp_stdio.h"

#include "cmd.h"
#include "usb_test.h"

/*************************************************************************/
/* FUNCTION   : test_cmd_open                                            */
/*                                                                       */
/* DESCRIPTION: open command                                             */
/*************************************************************************/
int test_cmd_open(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int mode;
    int prot;
    int ret;

    if (ac != 4) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &mode)) {
        grp_fs_printf("%s: invalid mode %s\n", av[0], av[2]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[3], &prot)) {
        grp_fs_printf("%s: invalid prot %s\n", av[0], av[3]);
        return(-1);
    }
    ret = grp_fs_open(av[1], mode, prot);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("open = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("open = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_close                                           */
/*                                                                       */
/* DESCRIPTION: close command                                            */
/*************************************************************************/
int test_cmd_close(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int fd;
    int ret;

    if (ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[1], &fd)) {
        grp_fs_printf("%s: invalid fd %s\n", av[0], av[1]);
        return(-1);
    }
    ret = grp_fs_close(fd);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("close = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("close = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_read                                            */
/*                                                                       */
/* DESCRIPTION: read command                                             */
/*************************************************************************/
int test_cmd_read(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int fd;
    int size;
    int ret = 0;
    int i;
    int iread;
    unsigned char buf[4096];
#if(GRP_FS_MINIMIZE_LEVEL > 0)
	int j;
	unsigned char out[4];
#endif	/* GRP_FS_MINIMIZE_LEVEL > 0 */

    if (ac != 3)
    {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[1], &fd)) {
        grp_fs_printf("%s: invalid fd %s\n", av[0], av[1]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &size)) {
        grp_fs_printf("%s: invalid size %s\n", av[0], av[2]);
        return(-1);
    }

    for (i = 0; i < size; i += ret) {
        iread = sizeof(buf);
        if (size - i < iread)
            iread = size - i;
        ret = grp_fs_read(fd, buf, iread);
        if (ret <= 0)
            break;
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        {
            fwrite(buf, 1, ret, stdout);
            fflush(stdout);
        }
#else	/* GRP_FS_MINIMIZE_LEVEL < 1 */
		for(j = 0; j < ret; j++) {
			out[0] = buf[j];
			out[1] = '\0';
			grp_fs_printf("%s", out);
	}
#endif	/* GRP_FS_MINIMIZE_LEVEL < 1 */
    }
	if (ret < 0) {
#if(GRP_FS_MINIMIZE_LEVEL < 1)
        grp_fs_printf("read error %s\n", grp_fs_err(ret, NULL)); 
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
		grp_fs_printf("read error %d\n", ret); 
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
	}
    grp_fs_printf("read = %d\n", i);
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_write                                           */
/*                                                                       */
/* DESCRIPTION: write command                                            */
/*************************************************************************/
int test_cmd_write(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int fd;
    int size;
    int ret = 0;
    int i;
    int iwrite;
    unsigned char buf[4096];

    int ilen = 4096;
    for(i=0;i<ilen;i++){
        buf[i] = (i % 10) + '0';
        if( (i % 100) == 99 ){
            buf[i-2] = '#';
            buf[i-1] = 0x0d;
            buf[i] = 0x0a;
        }
    }

    if (ac != 3) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[1], &fd)) {
        grp_fs_printf("%s: invalid fd %s\n", av[0], av[1]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &size)) {
        grp_fs_printf("%s: invalid size %s\n", av[0], av[2]);
        return(-1);
    }
    for (i = 0; i < size; i += ret) {
        iwrite = ilen;
        if (size - i < iwrite)
            iwrite = size - i;
        ret = grp_fs_write(fd, buf, iwrite);
        if (ret != iwrite) {
#if(GRP_FS_MINIMIZE_LEVEL < 1)
            grp_fs_printf("%s: write error %s\n", av[0], 
                            grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
			grp_fs_printf("%s: write error %d\n", av[0], ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
            break;
        }
    }
    grp_fs_printf("write = %d\n", i);
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_seek                                            */
/*                                                                       */
/* DESCRIPTION: seek command                                             */
/*************************************************************************/
int test_cmd_seek(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int fd;
    int offset;
    int mode;
    int ret = 0;

    if (ac != 4) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[1], &fd)) {
        grp_fs_printf("%s: invalid fd %s\n", av[0], av[1]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &offset)) {
        grp_fs_printf("%s: invalid offset %s\n", av[0], av[2]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[3], &mode)) {
        grp_fs_printf("%s: invalid mode %s\n", av[0], av[3]);
        return(-1);
    }
    ret = grp_fs_lseek(fd, offset, mode);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("seek = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("seek = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

#ifdef GRP_FS_ENABLE_OVER_2G
/*************************************************************************/
/* FUNCTION   : test_cmd_seek4G                                          */
/*                                                                       */
/* DESCRIPTION: seek4G command                                           */
/*************************************************************************/
int test_cmd_seek4G(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret = 0;
    int fd;
    int mode;
    unsigned int offset;
    unsigned int resultoffset;

    if (ac != 4) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[1], &fd)) {
        grp_fs_printf("%s: invalid fd %s\n", av[0], av[1]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &offset)) {
        grp_fs_printf("%s: invalid offset %s\n", av[0], av[2]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[3], &mode)) {
        grp_fs_printf("%s: invalid mode %s\n", av[0], av[3]);
        return(-1);
    }
    ret = grp_fs_lseek4G(fd, offset, mode, &resultoffset);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("seek4G = %s offset = %u\n", grp_fs_err(ret, NULL), resultoffset);
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("seek4G = %d offset = %u\n", ret, resultoffset);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}
#endif /* GRP_FS_ENABLE_OVER_2G */

/*************************************************************************/
/* FUNCTION   : test_cmd_create                                          */
/*                                                                       */
/* DESCRIPTION: create command                                           */
/*************************************************************************/
int test_cmd_create(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int type;
    int prot;
    int attr;
    int ret;

    if (ac != 5) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &type)) {
        grp_fs_printf("%s: invalid type %s\n", av[0], av[2]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[3], &prot)) {
        grp_fs_printf("%s: invalid prot %s\n", av[0], av[3]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[4], &attr)) {
        grp_fs_printf("%s: invalid attr %s\n", av[0], av[3]);
        return(-1);
    }
    ret = grp_fs_create(av[1], type, prot, attr);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("create = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("create = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_geta                                            */
/*                                                                       */
/* DESCRIPTION: get attr command                                         */
/*************************************************************************/
int test_cmd_geta(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    grp_fs_dir_ent_t dirent;
    void test_print_dir(grp_fs_dir_ent_t *, int);

    if (ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_get_attr(av[1], &dirent);
    if (ret >= 0) {
        dirent.pucName = av[1];
        test_print_dir(&dirent, 1);
    }
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("get_attr = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("get_attr = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_conv_time                                           */
/*                                                                       */
/* DESCRIPTION: convert time                                             */
/*************************************************************************/
static int test_conv_time(unsigned char *str)
{
    int val;
    grp_time_tm_t TM;

    for (val = 0; *str && *str != '/'; str++)
        val = val * 10 + *str - '0';
    if (*str++ != '/')
        return(-1);
    TM.sYear = (short)((val < 50)? val + 2000: val + 1900);
    for (val = 0; *str && *str != '/'; str++)
        val = val * 10 + *str - '0';
    TM.ucMon = (grp_uchar_t)val;
    if (*str++ != '/')
        return(-1);
    for (val = 0; *str && *str != ' '; str++)
        val = val * 10 + *str - '0';
    TM.ucDay = (grp_uchar_t)val;
    if (*str == 0) {
        TM.ucHour = TM.ucMin = TM.ucSec = 0;
    } else {
        if (*str++ != ' ')
            return(-1);
        for (val = 0; *str && *str != ':'; str++)
            val = val * 10 + *str - '0';
        TM.ucHour = (grp_uchar_t)val;
        if (*str++ != ':')
            return(-1);
        for (val = 0; *str && *str != ':'; str++)
            val = val * 10 + *str - '0';
        TM.ucMin = (grp_uchar_t)val;
        if (*str++ != ':')
            return(-1);
        for (val = 0; *str; str++)
            val = val * 10 + *str - '0';
        TM.ucSec = (grp_uchar_t)val;
    }
    return(grp_time_mktime(&TM));
}

/*************************************************************************/
/* FUNCTION   : test_cmd_seta                                            */
/*                                                                       */
/* DESCRIPTION: set attr command                                         */
/*************************************************************************/
int test_cmd_seta(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int prot;
    int attr;
    int ct, mt, at;
    int ret;
    grp_fs_dir_ent_t dent;

    if (ac != 7) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &prot)) {
        grp_fs_printf("%s: invalid prot %s\n", av[0], av[2]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[3], &attr)) {
        grp_fs_printf("%s: invalid attr %s\n", av[0], av[3]);
        return(-1);
    }
    if ((ct = test_conv_time(av[4])) < 0) {
        grp_fs_printf("%s: invalid ct %s\n", av[0], av[4]);
        return(-1);
    }
    if ((mt = test_conv_time(av[5])) < 0) {
        grp_fs_printf("%s: invalid mt %s\n", av[0], av[5]);
        return(-1);
    }
    if ((at = test_conv_time(av[6])) < 0) {
        grp_fs_printf("%s: invalid at %s\n", av[0], av[6]);
        return(-1);
    }
    dent.uiProtect = prot;
    dent.uiAttr = attr;
    dent.iCTime = ct;
    dent.iMTime = mt;
    dent.iATime = at;
    dent.sNameSize = 0;
    ret = grp_fs_set_attr(av[1], &dent);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("set_attr = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("set_attr = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

/*************************************************************************/
/* FUNCTION   : test_cmd_trunc                                           */
/*                                                                       */
/* DESCRIPTION: truncate command                                         */
/*************************************************************************/
int test_cmd_trunc(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int fd;
    int offset;
    int ret = 0;

    if (ac != 3) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[1], &fd)) {
        grp_fs_printf("%s: invalid fd %s\n", av[0], av[1]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &offset)) {
        grp_fs_printf("%s: invalid offset %s\n", av[0], av[2]);
        return(-1);
    }
    ret = grp_fs_truncate(fd, offset);
#if(GRP_FS_MINIMIZE_LEVEL < 1)
    grp_fs_printf("trunc = %s\n", grp_fs_err(ret, NULL));
#else  /* GRP_FS_MINIMIZE_LEVEL < 1 */
	grp_fs_printf("trunc = %d\n", ret);
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
    return(ret);
}

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_utime                                           */
/*                                                                       */
/* DESCRIPTION: utime command                                            */
/*************************************************************************/
int test_cmd_utime(test_cmd_t *cmd, int ac, unsigned char **av)
{
    struct timeval tv[2];
    int ret;
    struct stat statb;

    if (ac != 4) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if ((tv[0].tv_sec = test_conv_time(av[2])) < 0) {
        grp_fs_printf("%s: bad atime %s\n", av[0], av[2]);
        return(-1);
    }
    if ((tv[1].tv_sec = test_conv_time(av[3])) < 0) {
        grp_fs_printf("%s: bad mtime %s\n", av[0], av[3]);
        return(-1);
    }
    ret = utimes(av[1], tv);
    grp_fs_printf("utime = %s\n", grp_fs_err(ret, NULL));
    ret = stat(av[1], &statb);
    grp_fs_printf("stat = %s\n", grp_fs_err(ret, NULL));
    if (ret == 0) {
#ifdef GRP_FS_ENABLE_OVER_2G
        grp_fs_printf("mode = 0%o, size = %u, dev = %d\n", 
#else  /* GRP_FS_ENABLE_OVER_2G */
        grp_fs_printf("mode = 0%o, size = %d, dev = %d\n", 
#endif /* GRP_FS_ENABLE_OVER_2G */
            statb.st_mode, statb.st_size, statb.st_dev);
        test_cmd_print_time("CT:", statb.st_ctime);
        test_cmd_print_time(" MT:", statb.st_mtime);
        test_cmd_print_time(" AT:", statb.st_atime);
        grp_fs_printf("\n");
    }
    return(0);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_inv_fs                                          */
/*                                                                       */
/* DESCRIPTION: invfs command                                            */
/*************************************************************************/
int test_cmd_inv_fs(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;

    if (ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_invalidate_fs_dev((const char *)av[1]);
    grp_fs_printf("invfs = %s\n", grp_fs_err(ret, NULL));
    return(ret);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_check_fs                                        */
/*                                                                       */
/* DESCRIPTION: chkfs command                                            */
/*************************************************************************/
int test_cmd_check_fs(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    grp_uchar_t vol_name[16];
    grp_uint32_t vol_ser;
    int vol_name_len = sizeof(vol_name);

    if (ac != 2) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    ret = grp_fs_check_fs_dev((const char *)av[1], vol_name, &vol_name_len,
                                &vol_ser);
    grp_fs_printf("chkfs = %s, %.*s(%d) 0x%x\n", grp_fs_err(ret, NULL), 
                    vol_name_len, vol_name, vol_name_len, vol_ser);
    return(ret);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */

#if(GRP_FS_MINIMIZE_LEVEL < 1)
/*************************************************************************/
/* FUNCTION   : test_cmd_get_err                                         */
/*                                                                       */
/* DESCRIPTION: geterr command                                           */
/*************************************************************************/
int test_cmd_get_err(test_cmd_t *cmd, int ac, unsigned char **av)
{
    int ret;
    int mode;
    int dev;
    int blk;
    int i, n;
    unsigned int need;
    unsigned char buf[0x1000];
    grp_fs_err_binfo_t *ep;
    
    if (ac != 4) {
        grp_fs_printf("Usage: %s\n", cmd->cmd_desc);
        return(-1);
    }
    if (test_cmd_eval_arg(av[1], &mode)) {
        grp_fs_printf("%s: invalid mode %s\n", av[0], av[1]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[2], &dev)) {
        grp_fs_printf("%s: invalid dev %s\n", av[0], av[2]);
        return(-1);
    }
    if (test_cmd_eval_arg(av[3], &blk)) {
        grp_fs_printf("%s: invalid blk %s\n", av[0], av[3]);
        return(-1);
    }
    ret = grp_fs_get_error(mode, dev, blk, buf, sizeof(buf), &need);
    grp_fs_printf("get_err = %s\n", grp_fs_err(ret, NULL));
    n = 0;
    for (i = 0; n < ret; i++) {
        ep = (grp_fs_err_binfo_t *)(buf + n);
        grp_fs_printf("%d: dev=0x%x blk=0x%x type=%d sft=%d off=0x%x sz=%d\n",
                        i, ep->iDev, ep->uiBlk, ep->ucBufType, ep->ucBlkShift,
                        ep->uiBlkOff, ep->uiSize);
        n += sizeof(*ep);
        if (mode & (GRP_FS_GE_CONTENT|GRP_FS_GE_RELEASE))
            n += ep->uiSize;
    }
    return(ret);
}
#endif /* GRP_FS_MINIMIZE_LEVEL < 1 */
